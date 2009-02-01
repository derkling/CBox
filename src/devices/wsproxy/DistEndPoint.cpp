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
        EndPoint(WS_EP_DIST, EPTYPE_DIST, paramBase, logName+".DistEndPoint"),
        d_devGPRS(0) {
	std::ostringstream lable("");
	std::string epCfg;

	// Loading the GPRS device that handle this EndPoint
	lable.str("");
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

	// Load EndPoint Configuration
	lable.str("");
	lable << paramBase.c_str() << "_srv";
	d_endpoint = d_configurator.param(lable.str().c_str(), "");
	if ( !d_endpoint.size() ) {
		LOG4CPP_WARN(log, "No EndPoint defined for DIST Server [%s]", d_name.c_str());
		return;
	}

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

	if (d_devGPRS) {
		d_devGPRS->disconnect();
		delete d_devGPRS;
	}

}

exitCode DistEndPoint::suspending() {
	LOG4CPP_DEBUG(log, "Disconnecting GPRS");
	d_devGPRS->disconnect();
	return OK;
}

exitCode DistEndPoint::upload(unsigned int & epEnabledQueues, std::string const & msg, EndPoint::t_epRespList &respList) {
	unsigned int l_isEnabled = 0x0;
	_ns1__uploadData soapMsg;
	_ns1__uploadDataResponse wsResp;
	std::string strMsg = msg;
	std::ostringstream error("Data upload to DIST server ERROR: ");
	int wsresult = 0;
	exitCode result = OK;


	// Checking if this File EndPoint queue is enabled
	l_isEnabled = epEnabledQueues && d_epQueueMask;
	if ( l_isEnabled == 0x0 ) {
		LOG4CPP_DEBUG(log, "    [%c(%hu) - %s] is DISABLED",
			getQueueLable(d_epQueueMask),
			d_failures, d_name.c_str() );
	}

	// Checking if a GPRS device has been correctly configured
	if ( !d_devGPRS ) {
		LOG4CPP_WARN(log, "Unable to upload data, devGPRS not present");
		return GPRS_DEVICE_NOT_PRESENT;
	}

	// Foramtting a SOAP message
	soapMsg.dati = &strMsg;
	d_csoap.endpoint = d_endpoint.c_str();
	LOG4CPP_DEBUG(log, "SOAP DATA [%s]", strMsg.c_str());

	// Ensuring proper APN is activated
	LOG4CPP_DEBUG(log, "DIST-%s: connecting GPRS netlink [%s]...", d_name.c_str(), d_netlink.c_str());
	result = d_devGPRS->connect(d_netlink);
	if ( result != OK ) {
		LOG4CPP_WARN(log, "Unable to connect GPRS");
		LOG4CPP_DEBUG(log, "DIST-%s: gprs connect failed [%d]", d_name.c_str(), result);
		return result;
	}

	// Call WebService Remote Upload Method
	LOG4CPP_DEBUG(log, "DIST-%s: uploading to DIST Server [%s]", d_name.c_str(), d_name.c_str());

	LOG4CPP_DEBUG(log, "    [%c(%hu) - %s]",
		getQueueLable(d_epQueueMask),
		d_failures, d_name.c_str() );

	wsresult = d_csoap.__ns3__uploadData ( &soapMsg, &wsResp );

	if ( wsresult != SOAP_OK ) {
		LOG4CPP_ERROR(log, "DIST-%s: upload FAILURE, WebService Stub returned with code %d",
			d_name.c_str(), wsresult);
		logSOAPFault(d_csoap.soap);
		return WS_UPLOAD_FAULT;
	}

	// Marking message as processed by this queue
	epEnabledQueues ^= d_epQueueMask;
	LOG4CPP_DEBUG(log, "DIST-%s: upload PROCESSED by queue [%s]",
		d_name.c_str(), d_name.c_str());

	result = checkResponce(wsResp.__any, respList);
	switch (result) {
	case OK:
		// Confirm upload
		LOG4CPP_INFO(log, "DIST-%s: upload CONFIRMED",
			d_name.c_str());
		break;
	case OUT_OF_MEMORY:
		// Keep message into queue, retry later
		LOG4CPP_WARN(log, "DIST-%s: Out-of-Memory, discarding message", d_name.c_str());
		break;
	case WS_FORMAT_ERROR:
		// Discard message
		LOG4CPP_WARN(log, "DIST-%s: Format error, discarding message", d_name.c_str());
		break;
	default:
		LOG4CPP_WARN(log, "DIST-%s: unexpected result code (%d) from checkResponce",
			d_name.c_str(), result);
		break;
	}

	return result;

}

exitCode
DistEndPoint::checkResponce(char * xml, EndPoint::t_epRespList &respList) {
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
	resp->epCode = d_epQueueMask;

	// Looking for ERROR responce from server
	pvalue = strcasestr(xml, "<KO>");
	if ( pvalue ) {
		pvalue += 4; // jumping after <KO>
		pvalue[6] = 0; // Null-terminating error code
		LOG4CPP_DEBUG(log, "DIST-%s: server returned ERROR [%s]",
			d_name.c_str(), pvalue);
		result = WS_FORMAT_ERROR;
	}

	// The previous call return NULL if ther is any KO in the responce
	resp->result = (pvalue == NULL);
	if ( !resp->result ) {
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
			LOG4CPP_WARN(log, "DIST-%s: parse error on server recponce",
				d_name.c_str());
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
