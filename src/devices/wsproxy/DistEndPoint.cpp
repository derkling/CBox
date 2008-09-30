//******************************************************************************
//*************  Copyright (C) 2006 - Patrick Bellasi **************************
//******************************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** Patrick Bellasi. The programs may be used and/or copied only with the
//** written permission from the author or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//******************************************************************************
//******************** Module information **************************************
//**
//** Project:       ControlBox (0.1)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Patrick Bellasi
//** Creation date:  21/06/2006
//**
//******************************************************************************
//******************** Revision history ****************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------- --------------------
//**
//**
//******************************************************************************

#include "DistEndPoint.ih"


namespace controlbox {
namespace device {

DistEndPoint::DistEndPoint(std::string const & paramBase, std::string const & logName) :
        EndPoint(EndPoint::WS_EP_DIST, paramBase, logName+".DistEndPoint"),
        d_devGPRS(0) {
	std::ostringstream lable("");
	int epId;
	std::string epCfg;

	// Setting the EndPoint Name
	d_name = "DistEndPoint";

// #if 1
	// Loading the GPRS device that handle this EndPoint
	lable << paramBase.c_str() << "_apn";
	d_netlink = d_configurator.param(lable.str().c_str(), "");

	if ( !d_netlink.size() ) {
		LOG4CPP_WARN(log, "No APN defined for DIST Server EndPoint");
		return;
	}

	d_devGPRS = DeviceGPRS::getInstance(d_netlink);
	if ( !d_devGPRS ) {
		LOG4CPP_ERROR(log, "Unable to find a GPRS supporting the required APN [%s]", d_netlink.c_str());
		return;
	}

	// Starting the GPRS device thread
	LOG4CPP_DEBUG(log, "Starting GPRS device thread...");
	d_devGPRS->runParser();



// #else
//
// 	// Forziamo il loading del modulo 0...
// 	LOG4CPP_WARN(log, "Forcing load of GPRS device 0...");
// 	d_devGPRS = DeviceGPRS::getInstance(0);
// 	if ( !d_devGPRS ) {
// 		LOG4CPP_ERROR(log, "Failed loading device GPRS");
// 	}
//
// #endif

// NOTE This code should be enabled to support multiple server IP
//      but all WebService having the same HTTP path on all server.
//      This will be required for data redundancy

	// Load EndPoint Configuration
	LOG4CPP_INFO(log, "Loading DIST servers...");
	epId = DIST_EP_FIRST_ID;
	while ( epId <= DIST_EP_MAXNUM ) {

		lable.str("");
		lable << paramBase.c_str() << "_ep" << epId;
		epCfg = d_configurator.param(lable.str(), "");

		if ( !epCfg.size() ) {
			break;
		}

		// Saving this server endpoint
		srvIpAddrList.push_back(epCfg);

		LOG4CPP_INFO(log, "DIST server %d [%s]", epId, epCfg.c_str());

		// Looking for next EndPoint definition
		epId++;
	};


// Disable KEEP_ALIVE connections
LOG4CPP_WARN(log, "gSOAP KeepAlive: DISABLED");
d_csoap.soap->imode &= ~SOAP_IO_KEEPALIVE;
d_csoap.soap->omode &= ~SOAP_IO_KEEPALIVE;

// Configuring TIMEOUTS
// NOTE A positive value measures the timeout in seconds. A negative timeout
//	value measures the timeout in microseconds (10-6 sec).
//	A value of zero disables timeout.
//	When a timeout occurs in send/receive operations, a SOAP_EOF exception
//	will be raised ("end of file or no input").
// 	The soap.connect_timeout specifies the timeout value for soap_call_ns__method calls.
//	The soap.accept_timeout specifies the timeout value for soap_accept(&soap) calls.
//	The soap.send_timeout and soap.recv_timeout specify the timeout values for
//	non-blocking socket I/O operations.
// NOTE Caution: Many Linux versions do not support non-blocking connect().
//	Therefore, setting soap.connect_timeout for non-blocking soap_call_ns__method
//	calls may not work under Linux.
#define Sec *1
#define uSec *-1
#define mSec *-1000
LOG4CPP_WARN(log, "gSOAP connect/send/recv Timeouts: 5/5/30 [s]");
d_csoap.soap->connect_timeout = 5 Sec;
// d_csoap.soap->accept_timeout = 5;
d_csoap.soap->send_timeout = 5 Sec;
d_csoap.soap->recv_timeout = 30 Sec;

#if 0
   // Configuring the WebService endpoint
   lable.str("");
   lable << paramBase.c_str() << "_ep0";

   d_endpoint = d_configurator.param(lable.str().c_str(), "");
   d_csoap.endpoint = d_endpoint.c_str();

   LOG4CPP_INFO(log, "Service endpoint configured [%s]", d_endpoint.c_str());
#endif

}

DistEndPoint::~DistEndPoint() {

	LOG4CPP_DEBUG(log, "Clearing DIST server list...");
	srvIpAddrList.clear();


	if (d_devGPRS) {
		d_devGPRS->disconnect();
		delete d_devGPRS;
	}

}

exitCode DistEndPoint::suspending() {
// 	LOG4CPP_DEBUG(log, "Disconnecting GPRS due to upload thread suspension");
// 	d_devGPRS->disconnect();
	return OK;
}

exitCode DistEndPoint::upload(unsigned int & epEnabledQueues, std::string const & msg, EndPoint::t_epRespList &respList) {
	_ns1__uploadData soapMsg;
	_ns1__uploadDataResponse wsResp;
	std::string strMsg = msg;
	t_serverIpAddrList::iterator srv;
	unsigned short srvNumber = 0;
	std::ostringstream error("Data upload to DIST server ERROR: ");
	int wsresult;
	exitCode result = OK;
// 	int retry = 3;

	if (!d_devGPRS) {
		LOG4CPP_WARN(log, "Unable to upload data, devGPRS not present");
		return GPRS_DEVICE_NOT_PRESENT;
	}

	// NOTE: This is just a test
	//	strMsg = string("1;2006-11-23T12:38:48+01:00;2006-11-23T12:38:47+01:00;2006-11-23T12:52:47+01:00;DaricomDriver;DaricomTractor;DaricomTruck;UNKNOWNCIM;+99.9999;+999.9999;09;This is just a TEST from Daricom Sr");

	// Foramtting a SOAP message
	soapMsg.dati = &strMsg;
	LOG4CPP_DEBUG(log, "SOAP DATA [%s]", strMsg.c_str());

	// Ensuring proper APN is activated
	LOG4CPP_DEBUG(log, "EP-DIST: connecting GPRS netlink [%s]...", d_netlink.c_str());
	result = d_devGPRS->connect(d_netlink);
	if ( result != OK ) {
		LOG4CPP_WARN(log, "Unable to connect GPRS");
		LOG4CPP_DEBUG(log, "EP-DIST: gprs connect failed [%d]", result);
		return result;
	}

	// Call WebService Remote Upload Method
	LOG4CPP_DEBUG(log, "EP-DIST: uploading DIST servers... ");

	srvNumber = 0;
	srv = srvIpAddrList.begin();
	while ( srv != srvIpAddrList.end() ) {

		srvNumber++;

		if ( !srvEnabled(epEnabledQueues, srvNumber) ) {
			// Selecting next DIST server
			srv++;
			continue;
		}

		d_csoap.endpoint = (*srv).c_str();
		LOG4CPP_INFO(log, "    [%c - %s]", getQueueLable(getQueueMask(epEnabledQueues, srvNumber)), d_csoap.endpoint);

		wsresult = d_csoap.__ns3__uploadData ( &soapMsg, &wsResp );

#if 0
		// Retry trasmission on errors
		// TODO process error codes... on some case we could return immediatly (e.g. netowrk unreachable)
		// FIXME make "retry" and "sleep" time parameters
		while (retry-- && wsresult==400) {
			LOG4CPP_DEBUG(log, "EP-DIST: retry trasmission in 2s...");
			::sleep(2);
			wsresult = d_csoap.__ns3__uploadData ( &soapMsg, &wsResp );
		}
#endif

		if ( wsresult != SOAP_OK ) {

			LOG4CPP_ERROR(log, "EP-DIST: upload FAILURE, WebService Stub returned with code %d", wsresult);
			logSOAPFault(d_csoap.soap);
			result = WS_UPLOAD_FAULT;

		} else {
			// Marking message as processed by this queue
			resetQueueBit(epEnabledQueues, srvNumber);
			LOG4CPP_DEBUG(log, "EP-DIST: upload PROCESSED by queue [%s]", d_csoap.endpoint);

			result = checkResponce(srvNumber, wsResp.__any, respList);
			switch (result) {
			case OK:
				// Confirm upload
				LOG4CPP_DEBUG(log, "EP-DIST: upload CONFIRMED by queue [%s]", d_csoap.endpoint);
				break;
			case OUT_OF_MEMORY:
				// Keep message into queue, retry later
				LOG4CPP_WARN(log, "Out-of-Memory, discarding message");
				goto exit_error;
				break;
			case WS_FORMAT_ERROR:
				// Discard message
				LOG4CPP_WARN(log, "Format error, discarding message");
				goto exit_error;
				break;
			default:
				LOG4CPP_WARN(log, "EP-DIST: unexpected result code (%d) from checkResponce", result);
				break;
			}

		}

		// Selecting next DIST server
		srv++;
	}

	LOG4CPP_DEBUG(log, "EP-DIST: data SUCCESSFULLY upload to all DIST servers");
	return OK;

exit_error:
	LOG4CPP_WARN(log, "EP-DIST: data upload FAILED  by queue [%d] DIST server");
	return result;

}

bool
DistEndPoint::srvEnabled(unsigned int & epEnabledQueues, unsigned short srvNumber) {
	unsigned int bit = 0;
	unsigned int shift = 0;

	shift = d_qmShiftCount+srvNumber-1;
	LOG4CPP_DEBUG(log, "EP-DIST: bit: %d, shiftCount: %d, SrvNum: %d, shift: %d",
				bit, d_qmShiftCount, srvNumber, shift);

	bit = ((unsigned int)0x1<<shift);
	LOG4CPP_DEBUG(log, "EP-DIST: Checking Queue BitMask [0x%02X], EnabledQueues [0x%02X]",
			bit, epEnabledQueues);

	LOG4CPP_DEBUG(log, "EP-DIST: Server %d is %s",
				srvNumber, (epEnabledQueues & bit) ? "ENABLED" : "DISABLED" );

	return (epEnabledQueues & bit);

}


unsigned int
DistEndPoint::getQueueMask(unsigned int & epEnabledQueues, unsigned short srvNumber) {
	unsigned int bit = 0;
	unsigned int shift = 0;

	shift = d_qmShiftCount+srvNumber-1;
	LOG4CPP_DEBUG(log, "EP-DIST: bit: %d, shiftCount: %d, SrvNum: %d, shift: %d",
				bit, d_qmShiftCount, srvNumber, shift);

	bit = ((unsigned int)0x1<<shift);
	return bit;

}


void
DistEndPoint::resetQueueBit(unsigned int & epEnabledQueues, unsigned short srvNumber) {
	unsigned int bit;

	bit = getQueueMask(epEnabledQueues,srvNumber);
	LOG4CPP_DEBUG(log, "EP-DIST: Resetting Queue BitMask [0x%02X], EnabledQueues [0x%02X]",
			bit, epEnabledQueues);

	epEnabledQueues ^= bit;

}

exitCode
DistEndPoint::checkResponce(unsigned short srvNumber, char * xml, EndPoint::t_epRespList &respList) {
	char * pcode;
	char * pvalue;
	char * pend;
	unsigned short code;
	exitCode result = OK;
	t_epResp * resp;
	t_epCmd * p_epCmd;

	LOG4CPP_DEBUG(log, "Checking DIST server responce [%s]", xml);

	resp = new t_epResp();
	if (resp==0) {
		LOG4CPP_WARN(log, "Failed allocating new resp entry");
		return OUT_OF_MEMORY;
	}

	resp->epType = WS_EP_DIST;
	resp->epCode = srvNumber;

	// Looking for ERROR responce from server
	pvalue = strcasestr(xml, "<KO>");
	if (pvalue) {
		pvalue += 4; // jumping after <KO>
		pvalue[6] = 0; // Null-terminating error code
		LOG4CPP_DEBUG(log, "EP-DIST: server returned ERROR [%s]", pvalue);
		result = WS_FORMAT_ERROR;
	}

	// The previous call return NULL if ther is any KO in the responce
	resp->result = (pvalue == NULL);
	if (!resp->result) {
		resp->errorCode = std::string(pvalue);
	}

	LOG4CPP_DEBUG(log, "Result [%s], Code [%s]", (resp->result) ? "OK" : "KO", (resp->result) ? "-" : (resp->errorCode).c_str());

	// Looking for Server Commands
	pcode = strcasestr(xml, "<ans>");
	while (pcode != NULL) {
		// Parsing a server responce
		pcode += 11; // jumping after <ans><code>
		pvalue = pcode+16; //jumping after XX</code><value>
		pend = strcasestr(pvalue, "</value>");
		if (pend == NULL) {
			LOG4CPP_WARN(log, "Malformed XML, parsing failed");
			break;
		}
		(*pend) = 0; // Terminating value string
		pend++;

		if ( sscanf(pcode, "%hu", &code) == EOF ) {
			LOG4CPP_WARN(log, "EP-DIST: parse error on server recponce");
			break;
		}

		LOG4CPP_DEBUG(log, "Processing server command [%u: %s]", code, pvalue);

		p_epCmd = new t_epCmd;
		switch (code) {
		case 1:		//01 CIM
		case 2:		//02 messaggio per l’autista
		case 3:		//03 frequenza di trasmissione dei dati di funzionamento dell’autobotte (messaggi di tipo 01)
		case 4:		//04 piano di carico
		case 5:		//05 costante odometrica
		case 6:		//06 impostazione odometro
		case 8:		//08 attivazione/disattivazione telemetria
			p_epCmd->code = code;
			p_epCmd->value = std::string(pvalue);
			break;
		case 7:		//07 indirizzo web service
			LOG4CPP_INFO(log, "Updating DIST servers IP addresses...");
			// TODO DIST servers IP addresses to be implemented
			LOG4CPP_WARN(log, "DIST SERVERS IP ADDRESSES UPDATE: TO BE IMPLEMENTED");
			break;
		default:
			LOG4CPP_WARN(log, "Undefined server command [%d: %s]", code, pvalue);
		};

		// Appending command to responce
		(resp->cmds).push_back(p_epCmd);

		// Looking for next command
		pcode = strcasestr(pend, "<ans>");
	}

	// Saving this resp into responce list
	respList.push_back(resp);

	return result;
}

void
DistEndPoint::logSOAPFault(struct soap * csoap) {
    const char *c, *v = NULL, *s, **d;

    if (csoap->error) {
        d = soap_faultcode(csoap);
        if (!*d)
            soap_set_fault(csoap);
        c = *d;
        if (csoap->version == 2)
            v = *soap_faultsubcode(csoap);
        s = *soap_faultstring(csoap);
        d = soap_faultdetail(csoap);
        LOG4CPP_ERROR(log, "%s%d fault: %s [%s]",
                      csoap->version ? "SOAP 1." : "Error ",
                      csoap->version ? (int)csoap->version : csoap->error,
                      c,
                      v ? v : "no subcode");
        LOG4CPP_ERROR(log, "  Error:    [%s]", s ? s : "[no reason]");
        LOG4CPP_ERROR(log, "  Details:  [%s]", d && *d ? *d : "[no detail]");
    }
}

}// namespace device
}// namespace controlbox
