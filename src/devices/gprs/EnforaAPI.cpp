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
//** Description:   EnforaAPI GPRS device back-end
//**
//** Filename:      EnforaAPI.cpp
//** Owner:         Patrick Bellasi
//** Creation date:  28/07/2007
//**
//******************************************************************************
//******************** Revision history ****************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------- --------------------
//**
//**
//******************************************************************************


#include "EnforaAPI.ih"

namespace controlbox {
namespace device {

const char *EnforaAPI::d_apiHeaders[] = {
	"\0\0\0\0",	// Empty
	"\0\0\1\0",	// Request unsolicited
	"\0\0\2\0",	// Get unsolicited
	"\0\0\3\0",	// Error
	"\0\1\4\0",	// Sent AT command
	"\0\1\5\0",	// Get AT responce
	"\0\10\1\0",	// ACK message
	"\0\10\2\0",	// Wakeup
	"\0\11\1\0"	// Send password
};

const char EnforaAPI::d_atResults[][12] = {
	{
//	 \r   \n     O     K   \r   \n    0
	0xd, 0xa, 0x4f, 0x4b, 0xd, 0xa, 0x0},
	{
//	 \r   \n     E     R     R     O     R   \r   \n    0
	0xd, 0xa, 0x45, 0x52, 0x52, 0x4f, 0x52, 0xd, 0xa, 0x0},
	{
//	 \r   \n     C     O     N     N     E     C     T   \r   \n    0
	0xd, 0xa, 0x43, 0x4f, 0x4e, 0x4e, 0x45, 0x43, 0x54, 0xd, 0xa, 0x0}
};

EnforaAPI::EnforaAPI(short module, std::string const & logName)
	throw (exceptions::SerialConfigurationException*) :
        DeviceGPRS(module, DeviceGPRS::DEVICEGPRS_MODEL_ENFORA, logName),
	d_gpio(0),
        d_apiEnabled(false) {
	DeviceFactory * df = DeviceFactory::getInstance();
	exitCode result;

	LOG4CPP_DEBUG(log, "EnforaAPI(std::string const &)");

	// Needed to switch-on/off the device
	d_gpio = df->getDeviceGPIO();

	// Module specific initialization
	result = initDeviceGPRS();
	if ( result!=OK ) {
		LOG4CPP_FATAL(log, "Failed to initialize the GPRS device");
		cleanUp();
		throw new exceptions::SerialConfigurationException("Unable to open TTY port");
	}

	// Modem identification reading device ids
	getDeviceIds();
	//TODO check if the modem is supported by this class
	//NOTE modem identifications should be done using AUTOSENSING! ;-)

	// Loading configuration profile (if not already done)
	result = loadConfiguration();
	if ( result==GPRS_RESET_REQUIRED ) {
		result = initDeviceGPRS();
		if ( result!=OK ) {
			LOG4CPP_FATAL(log, "Failed to initialize the GPRS device");
			cleanUp();
			throw new exceptions::SerialConfigurationException("Unable to open TTY port");
		}
	}

	LOG4CPP_INFO(log, "Starting PPPD monitor...");
	d_monitorPppd.start();


// FIXME this has to be removed, because the API could not respond since
// 	we are trying to attach GSM network... we try into the upload thread
//	to enable the API
/*
	// Configure the UDPAPI
	// Switch to GPRS-disconnected mode
	// Configure the host-to-modem PPP link
	result = enableAPI();
	if ( result!=OK ) {
		LOG4CPP_FATAL(log, "Failed to enable UDP API interface");
		cleanUp();
	}
*/

}

EnforaAPI::~EnforaAPI() {

    LOG4CPP_DEBUG(log, "Destroying the EnforaAPI module [%d]", d_module);
    disableAPI();
    powerOff();

}

exitCode
EnforaAPI::powerOn(bool reset) {
	exitCode result;

	if ( reset ) {
		LOG4CPP_WARN(log, "Resetting");
		result = d_gpio->gprsReset(d_module);
		if ( result!=OK ) {
			return GPRS_RESET_REQUIRED;
		}
		LOG4CPP_DEBUG(log, "Modem powered-up, waiting 10s before continuing...");
		::sleep(10);
		return OK;
	}

	result = d_gpio->gprsPowerOn(d_module);
	if (result != OK ) {
		return GPRS_RESET_REQUIRED;
	}

	return OK;
}

exitCode
EnforaAPI::powerOff() {
	LOG4CPP_INFO(log, "Powering-off");
	d_gpio->gprsPowerOff(d_module);
	return OK;
}

exitCode EnforaAPI::initDeviceGPRS() {
	exitCode result;
	t_stringVector resp;
	unsigned short retry_pwr;
	unsigned short level;

	// Ensuring API is disabled
	disableAPI();

	for (retry_pwr=3; retry_pwr; retry_pwr--) {

		// Powering on modem
		result = powerOn(true);
		if (result) {
			LOG4CPP_WARN(log, "Failed to PowerOn modem");
			break;
		}

		if (retry_pwr<3) {
			LOG4CPP_WARN(log, "Waiting %ds before query modem...",
				5*(3-retry_pwr));
			::sleep(5*(3-retry_pwr));
		}

		result = d_tty->openSerial(true, &resp);
		if (result == OK ) {
			LOG4CPP_DEBUG(log, "AT command port opened");
		}

		// Reading signal level to check if the modem communication is
		// correctly working
		result = signalLevel(level);
		if (result==OK)
			break;

		LOG4CPP_WARN(log, "AT command mode NOT WORKING");
		d_tty->closeSerial();

		if (result==OK)
			break;

		LOG4CPP_WARN(log, "Modem communication failed, restarting modem and retrying in 5s");

		//Waiting few seconds before retry
		powerOff();
		::sleep(5);

	}

	if (result!=OK) {
		LOG4CPP_FATAL(log, "GPRS not working");
		return result;
	}
	LOG4CPP_DEBUG(log, "GPRS modem ready in AT command mode");
	return result;

}


exitCode EnforaAPI::loadConfiguration() {
	t_stringVector resp;
	std::string value;
	char * l_start;


	// Loading configuration params
	d_pppdCommand	= d_config.param(paramName("api_pppdCommand"), "");

	LOG4CPP_DEBUG(log, "API configuration, local %s:%d - remote %s:%d",
				API_LOCAL_IP, API_LOCAL_PORT,
				API_REMOTE_IP, API_REMOTE_PORT);

	d_smsNotifyNumber = d_config.param(paramName("sms_number"), ENFORA_SMS_NUMBER);


// 	// Enabling 900/1800 MHz frequency selection
// 	d_tty->sendSerial("AT%BAND=1,3", &resp);
// 	// Searching operator
// 	d_tty->sendSerial("AT+COPS", &resp);


	// Autosensing current configuration
	if ( d_tty->sendSerial("AT$USRVAL?", &resp) != OK ) {
		LOG4CPP_DEBUG(log, "Failed reading current configuration");
		return GPRS_TTY_MODEM_NOT_RESPONDING;
	}

	l_start = strstr(resp[0].c_str(), API_CONF_VER);
	if ( l_start ) {
		LOG4CPP_DEBUG(log, "Modem already configured");
		return OK;
	}

	LOG4CPP_WARN(log, "Loading modem configuration...");
LOG4CPP_WARN(log, "Modem configuration DISABLED by code (AT$USRVAL=%s)", resp[0].c_str());
return OK;

	// USE AT$USRVAL to store the configuration revision number


	// Resetting to factory defaults
	d_tty->sendSerial("AT&F;E0;Q0;V1;&C1");

	// Enabling 900/1800 MHz frequency selection
	d_tty->sendSerial("AT%BAND=1,3", &resp);

	// Configuring GPIO direction
	d_tty->sendSerial("AT$IOCFG=00000000000000000000");
	d_tty->sendSerial("AT$IOGPA=00000000000000000000");

	// Configuring hardware flow control (RTS/CTS)
	// FIXME if we change flow control here we MUST reconfigure the
	//	d_tty port!!!
	// NOTE this command is buggy:
	// - documentation name it AT+IFC
	// - modem accept only 1 parameter and not two!
// 	d_tty->sendSerial("AT$IFC=2,2");

	// Disabling Power-Up Message
	d_tty->sendSerial("AT$PWRMSG=,1");



	// GSM not registered (not searching)
	d_tty->sendSerial("AT$EVENT=1,0,9,0,0");
	// Setting LOW GPIO1
	d_tty->sendSerial("AT$EVENT=1,3,8,0,0");

	// GSM registration denied or unk problems
	d_tty->sendSerial("AT$EVENT=2,0,9,3,4");
	// Setting LOW GPIO1
	d_tty->sendSerial("AT$EVENT=2,3,8,0,0");

	// Searching for GSM
	d_tty->sendSerial("AT$EVENT=3,0,9,2,2");
	// Slow falshing GPIO1
	d_tty->sendSerial("AT$EVENT=3,3,32,458753,0");

	// GSM registered (home network) or roaming
	d_tty->sendSerial("AT$EVENT=4,0,9,1,1");
	// Setting HIGH GPIO1
	d_tty->sendSerial("AT$EVENT=4,3,16,0,0");

	// GSM registered (roaming)
	d_tty->sendSerial("AT$EVENT=5,0,9,5,5");
	// Setting HIGH GPIO1
	d_tty->sendSerial("AT$EVENT=5,3,16,0,0");

	// GPRS not registered (not serching) reg denied or unk problems
	d_tty->sendSerial("AT$EVENT=6,0,10,0,0");
	// Setting HIGH GPIO2
	d_tty->sendSerial("AT$EVENT=6,3,17,0,0");

	// GPRS registration denied or unk problems
	d_tty->sendSerial("AT$EVENT=7,0,10,3,4");
	// Setting HIGH GPIO2
	d_tty->sendSerial("AT$EVENT=7,3,17,0,0");

	// Searching for GPRS
	d_tty->sendSerial("AT$EVENT=8,0,10,2,2");
	// Slow falshing GPIO2
	d_tty->sendSerial("AT$EVENT=8,3,33,458753,0");

	// GPRS registered (home network) or roaming
	d_tty->sendSerial("AT$EVENT=9,0,10,1,1");
	// Setting LOW GPIO2
	d_tty->sendSerial("AT$EVENT=9,3,9,0,0");

	// GPRS registered (roaming)
	d_tty->sendSerial("AT$EVENT=10,0,10,5,5");
	// Setting LOW GPIO2
	d_tty->sendSerial("AT$EVENT=10,3,9,0,0");

	// Getting IP Address
	d_tty->sendSerial("AT$EVENT=11,0,11,1,1");
	// Slow falshing GPIO1
	d_tty->sendSerial("AT$EVENT=11,3,32,1,0");

	// Losing IP Address
	d_tty->sendSerial("AT$EVENT=12,0,11,0,0");
	// Setting HIGH GPIO1
	d_tty->sendSerial("AT$EVENT=12,3,16,0,0");



	// Configuring for textual SMS
	d_tty->sendSerial("AT+CMGF=1");

	// Saving confgiuration revision to flash memory
	d_tty->sendSerial("AT$USRVAL=" API_CONF_VER , &resp);

	// Saving current settings
	d_tty->sendSerial("AT&W");

	return GPRS_RESET_REQUIRED;

}


exitCode
EnforaAPI::checkAPI() {
	t_apiCommand apiMsg;
	t_apiResponce apiResp;
	exitCode result;

	LOG4CPP_DEBUG(log, "Checking if API is correctly working...");
	memcpy(apiMsg.field.data, "AT\0", 3);
	result = sendAT(apiMsg, apiResp);
	if ( result != OK ) {
		LOG4CPP_ERROR(log, "API not working");
		return result;
	}

	LOG4CPP_DEBUG(log, "API is working correctly");
	return OK;

}


exitCode
EnforaAPI::enableAPI() {
	exitCode result;
	t_stringVector resp;
	short sec = 60;		// Seconds to wait for PPP daemon being Up
	char atCmd[64];

	if ( d_apiEnabled ) {
		LOG4CPP_DEBUG(log, "API interface is [ON]");
	} else {
		LOG4CPP_DEBUG(log, "API interface is [OFF]");
	}

	if (d_apiEnabled) {
		return checkAPI();
	}

	LOG4CPP_INFO(log, "Enabling API interface...");

	// Setting modem IP and local IP as friend server
// 	d_tty->sendSerial("AT$UDPAPI=\"" API_REMOTE_IP "\",1721", &resp);
	sprintf(atCmd, "AT$UDPAPI=\"%s\",%d", API_REMOTE_IP, API_REMOTE_PORT);
	d_tty->sendSerial(atCmd, &resp);

//TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
//TODO Use parameters for friend server configuration
// 	d_tty->sendSerial("AT$FRIEND=01,1,\"" API_LOCAL_IP "\"," API_REMOTE_PORT ",2", &resp);
	sprintf(atCmd, "AT$FRIEND=01,1,\"%s\",%d,2", API_LOCAL_IP, API_REMOTE_PORT);
	d_tty->sendSerial(atCmd, &resp);

	// Configuring dynamic IP notifications
	d_tty->sendSerial("AT$WAKEUP=2,1", &resp);
	d_tty->sendSerial("AT$ACKTM=5,5,0", &resp);
	// FIXME use device ID
	d_tty->sendSerial("AT$MDMID=\"cBox00001\"", &resp);


	// Configure netlink for non-GPRS connection mode
	d_tty->sendSerial("AT$HOSTIF=3", &resp);

#if 0
// NOTE this code has been moved within the gprsUP command
	d_tty->sendSerial(d_supportedLinks[linkname]->pdpContext, &resp);
#else
	// Setting a dummy CGDCONT to enable UDP API interface
	// This command must be configured with a valid one before enabling
	// GPRS connection
// 	d_tty->sendSerial("AT+CGDCONT=1,\"IP\",\"ibox.tim.com\",,0,0", &resp);
#endif
	// NOTE the command preceding this one must clear all serial output!!!
	d_tty->sendSerial("ATD*99***1#");

	// Releasing TTY devince for pppd
	d_tty->closeSerial();

	LOG4CPP_DEBUG(log, "Configuration done: staring PPP daemon...");

	// Start host-side PPP
	LOG4CPP_DEBUG(log, "Starting PPPD [%s]...", d_pppdCommand.c_str());
	if ( system(d_pppdCommand.c_str()) ) {
		LOG4CPP_ERROR(log, "Failed starting non-GPRS connection: %s", strerror(errno));
		return GPRS_PPPD_FAILURE;
	}

	// NOTE d_apiEnabled will be set true by pppNotifyState once the PPP
	//	daemon will be Up and Running

	// Waiting for PPP being up and running
	// We use busy waiting to avoid blocking on PPP daemon up...
	while (sec-- && !d_apiEnabled) {
		LOG4CPP_DEBUG(log, "Waiting for ppp being UP [%02d s]...", sec);
		::sleep(1);
	}

	if (!d_apiEnabled) {
		return GPRS_PPPD_FAILURE;
	}

	result = checkAPI();

	return result;

}

exitCode
EnforaAPI::disableAPI() {
	int result, pid;
	std::ostringstream cmdLine("");
	short sec = 60;		// Seconds to wait for PPP daemon being Donw

	if (!d_apiEnabled) {
		LOG4CPP_DEBUG(log, "GPRS API already disabled");
		return OK;
	}

	LOG4CPP_WARN(log, "Disabling GPRS API...");

	pid = getPppdPid();

	if (pid>0) {
		cmdLine << "kill -TERM " << pid;
		LOG4CPP_DEBUG(log, "API, disabling non-GPRS connection [%s]... ", cmdLine.str().c_str());
		result = system(cmdLine.str().c_str());
		if ( result ) {
			LOG4CPP_ERROR(log, "API, failed disabling non-GPRS connection [%d]", result);
			return GPRS_PPPD_FAILURE;
		}
	} else {
		LOG4CPP_DEBUG(log, "ppp daemon already stopped");
	}

	LOG4CPP_DEBUG(log, "Closing API local socket");
	::close(d_modemSocket);

	// Waiting for PPP being donw
	// We use busy waiting to avoid blocking on PPP daemon up...
	while (sec-- && d_apiEnabled) {
		LOG4CPP_DEBUG(log, "Waiting for ppp being DOWN [%02d s]...", sec);
		::sleep(1);
	}

	if (d_apiEnabled) {
		return GPRS_PPPD_FAILURE;
	}

	LOG4CPP_DEBUG(log, "GPRS API interface disabled");

	return OK;

}

exitCode
EnforaAPI::resetAPI() {
	exitCode result;
	t_stringVector resp;


	result = disableAPI();

/*
	//Software GPRS RESET
	LOG4CPP_WARN(log, "Doing SW reset...");
	d_tty->sendSerial("AT$RESET", &resp);
	::sleep(5);

	//Cheking if modem communication is working correctly
	LOG4CPP_WARN(log, "Checking AT command mode communication...");
	result = d_tty->sendSerial("AT", &resp);
	if ( result == OK ) {
		LOG4CPP_DEBUG(log, "Modem working in AT command mode");
		result = enableAPI();
		return result;
	}
*/

	// HW RESET needed
	LOG4CPP_DEBUG(log, "Doing HW reset...");
	result = powerOn(true);
	if ( result != OK ) {
		return result;
	}

	result = enableAPI();
	return result;

}

exitCode
EnforaAPI::initSocket() {
	struct hostent *rip;

	// Configuring local socket
	d_modemSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if ( d_modemSocket == -1 ) {
		LOG4CPP_ERROR(log, "Failed creating API socket: %s", strerror(errno));
		return GPRS_SOCKET_CREATION_FAILED;
	}
	LOG4CPP_DEBUG(log, "API local socket created [%d]", d_modemSocket);

	memset(&d_apiLocalAddr, 0, sizeof(d_apiLocalAddr));
	d_apiLocalAddr.sin_family = AF_INET;
	d_apiLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	d_apiLocalAddr.sin_port = htons(API_LOCAL_PORT);
	if ( bind(d_modemSocket, (struct sockaddr*)&d_apiLocalAddr, sizeof(d_apiLocalAddr)) < 0 ) {
		LOG4CPP_ERROR(log, "Failed binding API socket: %s", strerror(errno));
		return GPRS_SOCKET_BINDING_FAILED;
	}

	LOG4CPP_DEBUG(log, "API local socket binded");

	// Preparing remote address
	rip = gethostbyname(API_REMOTE_IP);
	if ( !rip ) {
		LOG4CPP_ERROR(log, "Failed resolving remte IP: %s", strerror(errno));
		::close(d_modemSocket);
		return GPRS_SOCKET_BINDING_FAILED;
	}
	memset(&d_apiRemoteAddr, 0, sizeof(d_apiRemoteAddr));
	d_apiRemoteAddr.sin_family = AF_INET;
	memcpy(&d_apiRemoteAddr.sin_addr, rip->h_addr, rip->h_length);
	d_apiRemoteAddr.sin_port = htons(API_REMOTE_PORT);

	LOG4CPP_DEBUG(log, "API remote address prepared");

	return OK;
}

exitCode
EnforaAPI::addHeader(t_apiCommand & msg, t_apiType t, bool toString) {
	char strHeader[ENFORAAPI_API_HEADER_SIZE];
	unsigned int i;

	if (!toString) {
		memcpy(msg.header, (void*)d_apiHeaders[t], ENFORAAPI_API_HEADER_SIZE);
		return OK;
	}

	for (i=0; i<ENFORAAPI_API_HEADER_SIZE; i++) {
		strHeader[i] = (char)'0' + (char)d_apiHeaders[t][i];
	}

	memcpy(msg.header, strHeader, ENFORAAPI_API_HEADER_SIZE);
	return OK;
}

exitCode
EnforaAPI::sendAT(t_apiCommand & msg, t_apiResponce & resp, unsigned int timeout) {
	int net_result = GPRS_RESET_REQUIRED;
	exitCode result = OK;
	unsigned short retry;

	// Setting message header
	addHeader(msg, SENDAT);

	for (retry=1; retry<4; retry++) {

		net_result = sendto(d_modemSocket, msg.raw,
						ENFORAAPI_API_HEADER_SIZE+strlen(msg.field.data), 0,
						(struct sockaddr*)&d_apiRemoteAddr,
						sizeof(d_apiRemoteAddr));
		if (net_result < 0) {
			LOG4CPP_WARN(log, "API, Failed sending AT command: %s", strerror(errno));
			LOG4CPP_DEBUG(log, "API, Retrying sending AT command in 2s...");
			::sleep(2);
			continue;
		}

		result = getResponce(resp, timeout);
		if ( result==OK ) {
			return OK;
		}

		if (result == GPRS_API_SELECT_FAILED ||
			result == GPRS_API_SELECT_TIMEOUT ||
			result == GPRS_API_ATRECV_FAILED ) {
			LOG4CPP_DEBUG(log, "API not working, device reset required");
			return GPRS_RESET_REQUIRED;
		}

		if ( result == GPRS_API_ATRECV_ERRORS ) {
			LOG4CPP_DEBUG(log, "API communication errors, retrying in 5s");
			::sleep(5);
		}

	}

	return result;

}

exitCode
EnforaAPI::getResponce(t_apiResponce & resp, unsigned int timeout) {
	int result = 0;
	char rx[512];
	char *buf = 0;
	char *pos = buf;
	int totBytes = 0;
	struct sockaddr_in remote;
	socklen_t rlen;
	fd_set rfds;
	struct timeval tv;
	unsigned short retry;

	// Using data field as buffer for rebuilding the complete responce message
	buf = resp.field.data;

	// Watch modem (filedescriptor) for input available
	FD_ZERO(&rfds);
	FD_SET(d_modemSocket, &rfds);
	// Wait up to timeout seconds.
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

// 	LOG4CPP_DEBUG(log, "API, select timeout [%hu]s", timeout);
	if (log.	isDebugEnabled()) {
		std::ostringstream formatter("%s:%d - ");
		log.log(::log4cpp::Priority::DEBUG, "%25s:%05d - " "API, select timeout [%hu]s", __FILE__, __LINE__, timeout);
	}

	result = select(d_modemSocket+1, &rfds, NULL, NULL, &tv);
	if (result == -1) {
		LOG4CPP_ERROR(log, "Select failed: %s", strerror(errno));
		return GPRS_API_SELECT_FAILED;
	}
	if (result == 0) {
		LOG4CPP_WARN(log, "Select timeout: modem not responding");
		return GPRS_API_SELECT_TIMEOUT;
	}

// 	while( result>0 ) {

// 		for (retry=1; retry<4; retry++) {
			result = recvfrom(d_modemSocket, rx, 512, MSG_DONTWAIT,
					(struct sockaddr*)&remote, &rlen);
// 			if ( result >=0 ) {
// 				break;
// 			}
// 			::sleep(1);
// 		}
		if (result < 0) {
			LOG4CPP_ERROR(log, "API failed receiving AT command responce: %s", strerror(errno));
			return GPRS_API_ATRECV_FAILED;
		}

		// Checking for errors
		if ( rx[ENFORAAPI_API_HEADER_TYPE_POS] ==
			d_apiHeaders[ERROR][ENFORAAPI_API_HEADER_TYPE_POS]) {
			buf[0] = 0;
			LOG4CPP_WARN(log, "API wrong AT command responce format");
			return GPRS_API_ATRECV_ERRORS;
		}

		// Building responce string (discarding header info)
		strncpy(buf+totBytes, rx+ENFORAAPI_API_HEADER_SIZE,
			ENFORAAPI_AT_RESP_MAXLEN-ENFORAAPI_API_HEADER_SIZE-totBytes);

		//if ( !memcmp(rx+ENFORAAPI_API_HEADER_SIZE, d_atResults[AT_OK], 6) ) {
		if ( strstr(rx+ENFORAAPI_API_HEADER_SIZE, "OK") ) {
			LOG4CPP_DEBUG(log, "API, OK returned by modem");
// 			break;
		}

// 		if ( !memcmp(rx+ENFORAAPI_API_HEADER_SIZE, d_atResults[AT_CONNECT], 11) ) {
		if ( strstr(rx+ENFORAAPI_API_HEADER_SIZE, "CONNECT") ) {
			LOG4CPP_DEBUG(log, "API, CONNECT returned by modem");
// 			break;
		}

// 		if ( !memcmp(rx+ENFORAAPI_API_HEADER_SIZE, d_atResults[AT_ERROR], 9) ) {
		if ( strstr(rx+ENFORAAPI_API_HEADER_SIZE, "ERROR") ) {
			LOG4CPP_DEBUG(log, "API, ERROR returned by modem");
			// Anyway, even in this case, the communication was successfull
// 			break;
		}

// 		// Taking away header and leading \0
// 		result-=(ENFORAAPI_API_HEADER_SIZE+1);
// 		pos+=result;
// 		totBytes+=result;

// 	};

	// Coping the header thaking it from last received UDP message
// 	memcpy(resp.header, rx, ENFORAAPI_API_HEADER_SIZE);

	return OK;

}

exitCode
EnforaAPI::updateNetworkConfiguration(void) {
	t_apiCommand msg;
	t_apiResponce resp;
	int result;
	unsigned octet[4];
	unsigned short i;
	std::ostringstream netConf("");
	bool updateDNS = false;

	LOG4CPP_DEBUG(log, "Retriving remote connections parametrs...");

	memcpy(msg.field.data, "AT$NETIP?\0", 10);
	result = sendAT(msg, resp);
	if (result) {
		LOG4CPP_ERROR(log,  "API, SEND[AT$NETIP?] failed");
		return GPRS_API_GPRS_UP_PARAMS_FAILED;
	}

	LOG4CPP_DEBUG(log, "API, SEND[AT$NETIP?]: %s", resp.field.data);

// NOTE resolver need IP address without leading ZERO on each octet, otherwise
//	it not works
// 	Parsing IP addresses of local interface, DNS1 and DNS2
// 	$NETIP: "217.202.048.082",  "213.230.155.094", "213.230.130.222"

	resp.field.data[24] = 0;
	for (i=0; i<4; i++) {
		sscanf(resp.field.data+(9+(i*4)), "%u", &octet[i]);
	}
	netConf.str("");
	netConf << octet[0] << "." << octet[1] << "." << octet[2] << "." << octet[3];

	// Checking if the IP address is valid
	if ( octet[0] == 0 ) {
		LOG4CPP_WARN(log, "Invalid local IP address [%s]",
			netConf.str().c_str() );
//FIXME prima di tornare ERRORE (TRY LATER).... riprovare altre 3 volte a leggerlo...
		return GPRS_API_GPRS_UP_FAILED;
	}

	if ( d_localIP != netConf.str() ) {
		d_localIP = netConf.str();
		LOG4CPP_INFO(log, "LocalIP changed to: %s", d_localIP.c_str());
		// TODO notify server about IP changed... we could simply
		// upload a brand new poll message: this will bring in the
		// new IP address... ;-)
		// NOTE this method is called each time a new message
		// has to be uploaded: we need instead an anync method to update
		// the IP address the server has when it change even if we have not
		// a new message to upload.
	}

	resp.field.data[44] = 0;
	for (i=0; i<4; i++) {
		sscanf(resp.field.data+(29+(i*4)), "%u", &octet[i]);
	}
	netConf.str("");
	netConf << octet[0] << "." << octet[1] << "." << octet[2] << "." << octet[3];
	if ( d_dns1 != netConf.str() ) {
		d_dns1 = netConf.str();
		LOG4CPP_INFO(log, "   DNS1 changed to: %s", d_dns1.c_str());
		updateDNS = true;
	}

	resp.field.data[63] = 0;
	for (i=0; i<4; i++) {
		sscanf(resp.field.data+(48+(i*4)), "%u", &octet[i]);
	}
	netConf.str("");
	netConf << octet[0] << "." << octet[1] << "." << octet[2] << "." << octet[3];
	if ( d_dns2 != netConf.str() ) {
		d_dns2 = netConf.str();
		LOG4CPP_INFO(log, "   DNS2 changed to: %s", d_dns2.c_str());
		updateDNS = true;
	}

	if ( updateDNS ) {
		netConf.str("");
		netConf << "echo nameserver " << d_dns1 << " > /etc/resolv.conf && ";
		netConf << "echo nameserver " << d_dns2 << " >> /etc/resolv.conf";
		LOG4CPP_DEBUG(log, "Configuring system resolver [%s]...", netConf.str().c_str());
		result = system(netConf.str().c_str());
		if ( result ) {
			LOG4CPP_ERROR(log, "Failed configuring system resolver [%d]", result);
			return GPRS_RESOLVER_CONFIGURE_FAILED;
		}
	}

	return OK;
}

exitCode
EnforaAPI::gprsUP(std::string const & linkname) {
	t_apiCommand msg;
	t_apiResponce resp;
	exitCode result;
	const char * atCmd;


	if (d_netStatus == LINK_UP) {
		LOG4CPP_WARN(log, "GPRS connection already UP");
		return OK;
	}

	LOG4CPP_INFO(log, "Activating GPRS connection [%s]...", linkname.c_str());

	atCmd = (d_supportedLinks[linkname]->pdpContext).c_str();
	memcpy(msg.field.data, atCmd, strlen(atCmd));
	result = sendAT(msg, resp);
	if ( result!=OK ) {
		LOG4CPP_WARN(log, "API, Failed SEND[%s]: %s", atCmd, resp.field.data);
		return result;
	}

	memcpy(msg.field.data, "AT$CONN\0", 8);
// 	result = sendAT(msg, resp);
	result = sendAT(msg, resp, ENFORAAPI_AT_CMD_TIMEOUT);
	if ( result!=OK ) {
		LOG4CPP_WARN(log, "API, Failed SEND[AT$CONN]: %s", resp.field.data);
		return result;
	}

	if ( !strstr(resp.field.data, "CONNECT") ) {
		LOG4CPP_WARN(log, "API,Connect failed [%s]", resp.field.data);
		return GPRS_API_GPRS_UP_FAILED;
	}

	result = updateNetworkConfiguration();
	if ( result!=OK ) {
		return result;
	}

	d_netStatus = LINK_UP;
	return result;

}

exitCode
EnforaAPI::gprsDOWN() {
	t_apiCommand msg;
	t_apiResponce resp;
	exitCode result;

	if (d_netStatus == LINK_DOWN) {
		return OK;
	}

#if 0
	memcpy(msg.field.data, "AT$DISC\0", 8);
	result = sendAT(msg, resp);
	if (result) {
		LOG4CPP_ERROR(log, "API, SEND[AT$DISC] failed");
		return GPRS_API_GPRS_DOWN_FAILED;
	}

	LOG4CPP_DEBUG(log, "API, SEND[AT$DISC]: %s", resp.field.data);

	if ( !strstr(resp.field.data, "NO CARRIER") ) {
		LOG4CPP_WARN(log, "API, disconnection failed");
		return GPRS_API_GPRS_DOWN_FAILED;
	}
#else
	memcpy(msg.field.data, "AT+CGATT=0\0", 11);
	result = sendAT(msg, resp, ENFORAAPI_AT_CMD_TIMEOUT);
	if (result) {
		LOG4CPP_ERROR(log, "API, SEND[AT+CGATT=0] failed");
		return GPRS_API_GPRS_DOWN_FAILED;
	}

	LOG4CPP_DEBUG(log, "API, SEND[AT+CGATT=0]: %s", resp.field.data);

	if ( !strstr(resp.field.data, "OK") ) {
		LOG4CPP_WARN(log, "API, disconnection failed");
		return GPRS_API_GPRS_DOWN_FAILED;
	}
#endif

	d_netStatus = LINK_DOWN;
	return OK;

}


exitCode EnforaAPI::notifyFriends() {
//     static const std::string * d_prevNetlinkName = 0;
//     unsigned short count;
//     std::ostringstream cmd("");
//     ttymodem modem((*this));	// The TTY port stream
//
//     // Notification configuration should be done only on netlink changing
//     if ( d_prevNetlinkName == d_curNetlinkName ) {
//     	LOG4CPP_DEBUG(log, "Netlink not changed, friends notify configuration don't need to be updated");
//     	return OK;
//     }
//
//     d_prevNetlinkName = d_curNetlinkName;
//
//     LOG4CPP_INFO(log, "Updating friend's server keep-alive configuration");
//
//     // Configuring firend server
//     cmd << "AT$MDMID=" << d_simNumber;
//     if ( modem.sendAT(cmd.str(), "OK") != OK ) {
//     	LOG4CPP_ERROR(log, "Configuring friend server [%s] notification failed",
//     	d_curNetlinkConf->lserv[count-1].c_str());
//     }
//
//     count = d_curNetlinkConf->lserv.size();
//     count = (count>10) ? 10 : count; //EnforaAPI support up to 10 firends only
//     while ( count ) {
//     	LOG4CPP_DEBUG(log, "Notifier added for friend server [%s]",
//     				d_curNetlinkConf->lserv[count-1].c_str());
//     	//TODO add AT commands to send notifications
//
// 	cmd.flush();
//     	cmd << "AT$FRIEND=" << count << ",1," << d_curNetlinkConf->lserv[count-1];
//     	if ( modem.sendAT(cmd.str(), "OK") != OK ) {
//     		LOG4CPP_ERROR(log, "Configuring friend server [%s] notification failed",
//     					d_curNetlinkConf->lserv[count-1].c_str());
//     	}
//     	count--;e
//     }

	return OK;

}


exitCode
EnforaAPI::tryConnect(std::string const & linkname) {
	t_supportedLinks::iterator it;
	exitCode result;
// 	pid_t pid;

//--- Here we don't know wich connection is up

	// Check if the REQUIRED connection is already up
	// In this case we must ensure a working connection by:
	// - having API interface working
	// - being registered within the GPRS network
	// - having an updated network configuration
	if ( (d_netStatus >= LINK_GOING_UP) &&
		(d_curNetlinkName) &&
		(linkname == *d_curNetlinkName) ) {


		LOG4CPP_DEBUG(log, "The required netlink [%s] is already up", linkname.c_str());

		// Ensuring API interface is up and running
		result = enableAPI();
		if ( result!=OK ) {
			LOG4CPP_ERROR(log, "API interface NOT WORKING");
			return result;
		}

		// Checking GPRS Network Registration
		result = gprsRegistered();
		if ( result != OK ) {
			LOG4CPP_DEBUG(log, "GPRS Network DETACHED");
			return result;
		}

// FIXME: check the IP address to ensure network attach....

		// Updating GPRS network settings
		result = updateNetworkConfiguration();
		if ( result != OK ) {
			d_netStatus = LINK_DOWN;
			LOG4CPP_ERROR(log, "Network Configuration FAILED");
			return result;
		}

		// The connection is UP and correctly WORKING
		return OK;
	}


//--- IF we get here the REQUIRED connection is NOT UP

	// Check if the required connection is supported
	LOG4CPP_DEBUG(log, "Looking if the required netlink [%s] is supported...", linkname.c_str());
	it = d_supportedLinks.find(linkname);
	if (it == d_supportedLinks.end()) {
		LOG4CPP_ERROR(log, "Netlink [%s] not supported by this GPRS device", linkname.c_str());
		return GPRS_NETLINK_NOT_SUPPORTED;
	}
	LOG4CPP_DEBUG(log, "Required netlink [%s] supported", linkname.c_str());

	// If we are already connected... first we should disconnect
	if ( d_netStatus >= LINK_GOING_UP ) {
		LOG4CPP_DEBUG(log, "Disconnecting current connection [%s]",
				d_curNetlinkName->c_str());
		result = disconnect();
		if ( result!=OK ) {
			LOG4CPP_DEBUG(log, "Failed disconnecting current connection");
			return result;
		}
	}
	LOG4CPP_DEBUG(log, "No active connections");



//TODO qui il modem VA SPENTO e riacceso SOLO dopo lo switching della SIM
	// Ensuring modem is powered up
	result = powerOn(false);
	if ( result ) {
		LOG4CPP_ERROR(log, "GPRS modem powered-on");
		return result;
	}
	LOG4CPP_DEBUG(log, "Modem is PoweredUp");




//--- IF we get here we are certenly DISCONNECTED

	d_netStatus = LINK_GOING_UP;
	LOG4CPP_DEBUG(log, "Connecting GPRS...");

/*
	// Configure params for the new netlink (SIM, PDPCTX, DIAL...)
	if ( !d_curNetlinkConf) {
		//TODO SIM Card initialization: we select as first that required
		//		by the firt connection we have to enable
		// Place here the code for SIM card activation
	} else {
		if (d_curNetlinkConf->sim != it->second->sim) {

			//TODO place here the code for SIM card swapping

			LOG4CPP_WARN(log, "SIM Card swapping NOT YET SUPPORTED, aborting!");
			return GPRS_RESET_REQUIRED;
		}
	}
*/
//MARKLINE;
	d_curNetlinkName = &(it->first);
	d_curNetlinkConf = it->second;

//MARKLINE;
	// Checking if the API is already running
	result = enableAPI();
	if ( result != OK ) {
		LOG4CPP_ERROR(log, "API, failed enabling non-GPRS connection");
		return result;
	}
//MARKLINE;

	// Checking GPRS Registration Status
	result = gprsRegistered();
	if ( result != OK ) {
		d_netStatus = LINK_DOWN;
		LOG4CPP_WARN(log, "GPRS network NOT available");
		return result;
	}

	// Enabling GPRS connection
	result = gprsUP(linkname);
	if ( result != OK ) {
		d_netStatus = LINK_DOWN;
		LOG4CPP_ERROR(log, "API, failed enabling GPRS");
		return result;
	}
	d_curLinkname = linkname;

	d_netStatus = LINK_UP;
	LOG4CPP_INFO(log, "GPRS Connection [%s] UP", linkname.c_str());

	// Notify new IP address
	if ( d_smsNotifyNumber.size() ) {
		LOG4CPP_DEBUG(log, "Sending IP address via SMS...");
		sendSMS(d_smsNotifyNumber.c_str(), d_localIP);
	}

	return OK;

}

exitCode
EnforaAPI::connect(std::string const & linkname) {
	exitCode result = OK;

	LOG4CPP_DEBUG(log, "Trying GPRS connection [%s]...", linkname.c_str());
	result = tryConnect(linkname);

	if ( result == OK ) {
		LOG4CPP_DEBUG(log, "GPRS connected");
		return OK;
	}

	if ( result == GPRS_NETLINK_NOT_SUPPORTED ) {
		// Noting can be done in this case: reconfiguration needed
		LOG4CPP_WARN(log, "GPRS connect failed, link not supported");
		return GPRS_NETLINK_NOT_SUPPORTED;
	}


	// FIXME here we should trace all possible exit codes rapresenting anomalies
	// conditions and eventually require an API or GPRS reset
	if ( result == GPRS_API_PARSE_ERROR ) {
		LOG4CPP_WARN(log, "Resetting GPRS due to API parsing errors");
		result = GPRS_RESET_REQUIRED;
	}

	if ( result == GPRS_RESET_REQUIRED ) {
		LOG4CPP_WARN(log, "Connect failed, RESET required", result);
		resetAPI();
		return GPRS_RESET_REQUIRED;
	}

	LOG4CPP_WARN(log, "Connect failed with code [%d]", result);

	return result;

}

exitCode
EnforaAPI::disconnect() {
	exitCode result;

	// Check if the connection is already down
	if ( d_netStatus <= LINK_GOING_DOWN ) {
		LOG4CPP_DEBUG(log, "The GPRS netlink is already down");
		return OK;
	}

	d_netStatus = LINK_GOING_DOWN;
	LOG4CPP_DEBUG(log, "Disconnecting GPRS...");

	result = gprsDOWN();
	if (result) {
		LOG4CPP_ERROR(log, "API, failed disabling GPRS");
		return result;
	}

	return OK;
}

void
EnforaAPI::pppNotifyState(bool running) {

	if ( running ) {
		// Wait few time to let ppp configure network interfaces...
		::sleep(5);

		// Initializing sockets for API interface usage
		initSocket();
		::sleep(5);
	}

	d_apiEnabled = running;
	if ( running ) {
		LOG4CPP_DEBUG(log, "Updating API state [ON]");
	} else {
		LOG4CPP_DEBUG(log, "Updating API state [OFF]");
	}

}


exitCode
EnforaAPI::sendSMS(std::string number, std::string text) {
	std::ostringstream buf("");
	t_apiCommand msg;
	t_apiResponce resp;
	t_stringVector vresp;
	exitCode result;

// 	LOG4CPP_WARN(log, "FIXME: SMS sending disabled by code");
// 	return OK;

// Ctrl+Z => ASCII: 26 (0x1A)

	LOG4CPP_DEBUG(log, "Sending SMS to [%s]: [%s]", number.c_str(), text.c_str());

	// Building AT command: AT+CMGS="+39number"
	buf << "AT+CMGS=\"" << number << "\"";

	if (d_apiEnabled) {

		LOG4CPP_DEBUG(log, "Sending SMS using API interface");

		memcpy(msg.field.data, buf.str().c_str(), buf.str().size());
		msg.field.data[buf.str().size()] = 0;

		LOG4CPP_DEBUG(log, "SEND[%s]", msg.field.data);

		result = sendAT(msg, resp, ENFORAAPI_AT_CMD_TIMEOUT);
		if (result) {
			LOG4CPP_ERROR(log, "API, SEND[%s] failed", buf.str().c_str());
			return GPRS_API_GPRS_UP_FAILED;
		}

		memcpy(msg.field.data, text.c_str(), text.size());
		// Appending Ctrl+Z and NULL to AT command
		msg.field.data[text.size()] = 0x1A;
		msg.field.data[text.size()+1] = 0;

		LOG4CPP_DEBUG(log, "SEND[%s]", msg.field.data);

		result = sendAT(msg, resp, ENFORAAPI_AT_CMD_TIMEOUT);
		if (result) {
			LOG4CPP_ERROR(log, "API, SEND[%s] failed", text.c_str());
			return GPRS_API_GPRS_UP_FAILED;
		}



// 		// Setting message header
// 		addHeader(msg, SENDAT);
//
// 		// Sending destination number
// 		strcpy(msg.field.data, buf.str().c_str());
// 		result = sendto(d_modemSocket, msg.raw,
// 				ENFORAAPI_API_HEADER_SIZE+strlen(msg.field.data), 0,
// 				(struct sockaddr*)&d_apiRemoteAddr,
// 				sizeof(d_apiRemoteAddr));
// 		if (result < 0) {
// 			LOG4CPP_ERROR(log, "Failed sending AT+CMGS number: %s", strerror(errno));
// 			return GPRS_API_ATSEND_FAILED;
// 		}
// 		// Reading back the enter message prompt...
// 		getResponce(resp);
//
// 		// Sending text message
// 		strncpy(msg.field.data, text.c_str(), 158);
// 		msg.field.data[strlen(msg.field.data)] = 0x1A;
// 		msg.field.data[strlen(msg.field.data)+1] = 0;
// 		result = sendto(d_modemSocket, msg.raw,
// 				ENFORAAPI_API_HEADER_SIZE+strlen(msg.field.data), 0,
// 				(struct sockaddr*)&d_apiRemoteAddr,
// 				sizeof(d_apiRemoteAddr));
// 		if (result < 0) {
// 			LOG4CPP_ERROR(log, "Failed sending AT+CMGS text: %s", strerror(errno));
// 			return GPRS_API_ATSEND_FAILED;
// 		}
//
// 		// Reading modem responce
// 		return getResponce(resp);
		// TODO check modem responce

	} else {
		LOG4CPP_DEBUG(log, "Sending SMS using AT interface");
		d_tty->sendSerial(buf.str());
		d_tty->sendSerial(text);
		d_tty->sendSerial(std::string("\26"), &vresp);
		// TODO check modem responce
	}


	return OK;
}

exitCode
EnforaAPI::gsmRegister() {
	t_apiCommand msg;
	t_apiResponce resp;
	t_stringVector vresp;
	exitCode result;
	char * l_start, *end;

	if (d_apiEnabled) {
		LOG4CPP_DEBUG(log, "API, Registering to GSM network...");

		LOG4CPP_DEBUG(log, "Operator selection... ");
		memcpy(msg.field.data, "AT+COPS\0", 8);
		result = sendAT(msg, resp, ENFORAAPI_AT_CMD_TIMEOUT);
		if ( result != OK ) {
			LOG4CPP_ERROR(log, "API, SEND[AT+COPS] failed");
			return result;
		}

		LOG4CPP_DEBUG(log, "API, SEND[AT+COPS]: %s", resp.field.data);

		// Check command result
		l_start = strstr(resp.field.data, "ERROR");
		if ( l_start ) {
			LOG4CPP_WARN(log, "Operator selection failed");
			LOG4CPP_DEBUG(log, "AT+COPS responce [%s]", resp.field.data);
			return GPRS_NETWORK_UNREGISTERED;
		}

// TODO this must be checked?!?!
// NOTE if previous command not returned ERROR: we should be able to read the
//	operator we are attached to
// 		// Check command result
// 		l_start = strstr(resp.field.data, "OK");
// 		if ( l_start ) {
// 			LOG4CPP_INFO(log, "Operator selection success");
// 			LOG4CPP_DEBUG(log, "AT+COPS responce [%s]", resp.field.data);
// 			return OK;
// 		}

		memcpy(msg.field.data, "AT+COPS?\0", 9);
		result = sendAT(msg, resp);
		if ( result != OK ) {
			LOG4CPP_ERROR(log, "API, SEND[AT+COPS?] failed");
			return result;
		}

		LOG4CPP_DEBUG(log, "API, SEND[AT+COPS?]: %s", resp.field.data);

		// responce example
		//	not working network: 	+COPS: 0
		// 	working network:	"+COPS: 0,0,"I TIM""
		l_start = strstr(resp.field.data, "\"");
		if ( !l_start ) {
			LOG4CPP_WARN(log, "Failed parsing AT+COPS? responce [%s]", resp.field.data);
			return GPRS_API_PARSE_ERROR;
		}
		l_start+=1;

		end = strstr(l_start, "\"");
		(*end) = 0;

		LOG4CPP_INFO(log, "Using GSM operator [%s]", l_start);

	} else {
		LOG4CPP_DEBUG(log, "AT, Register to GSM network...");
		d_tty->sendSerial("AT+COPS", &vresp);
		d_tty->sendSerial("AT+COPS?", &vresp);
	}

	return OK;
}

exitCode
EnforaAPI::signalLevel(unsigned short & level) {
	t_apiCommand msg;
	t_apiResponce resp;
	exitCode result = OK;
	t_stringVector vresp;
	char * pos;

	if (d_apiEnabled) {
		LOG4CPP_DEBUG(log, "Query signal level using API interface");

//MARKLINE;
		memcpy(msg.field.data, "AT+CSQ\0", 7);
		result = sendAT(msg, resp);
		if ( result!=OK ) {
			LOG4CPP_ERROR(log, "API, SEND[AT+CSQ] failed");
			return result;
		}

		LOG4CPP_DEBUG(log, "API, SEND[AT+CSQ]: %s", resp.field.data);

		// responce example "+CSQ: 22,1"
		pos = strstr(resp.field.data, ",");
		if (!pos) {
			LOG4CPP_WARN(log, "Failed parsing AT+CSQ responce [%s]", resp.field.data);
			level = 0;
			return GPRS_RESET_REQUIRED;
		}
		(*pos) = 0;
		sscanf(resp.field.data+5, "%hu", &level);
		LOG4CPP_DEBUG(log, "Signal level [%hu]", level);

	} else {
		LOG4CPP_DEBUG(log, "Query signal level using AT interface");
		result = d_tty->sendSerial("AT+CSQ", &vresp);
	}

	return result;
}

exitCode
EnforaAPI::gsmStatus(unsigned short & p_status) {
	t_apiCommand msg;
	t_apiResponce resp;
	t_stringVector vresp;
	exitCode result;
	char * pos;

	if (d_apiEnabled) {
		LOG4CPP_DEBUG(log, "Query GSM status using API interface");

		memcpy(msg.field.data, "AT+CREG?\0", 9);
		result = sendAT(msg, resp);
		if ( result != OK ) {
			LOG4CPP_ERROR(log, "API, SEND[AT+CREG?] failed");
			return result;
		}

		LOG4CPP_DEBUG(log, "API, SEND[AT+CREG?]: %s", resp.field.data);

		// responce example "+CREG: 0,1"
		pos = strstr(resp.field.data, ",");
		if (!pos) {
			LOG4CPP_WARN(log, "Failed parsing AT+CREG? responce [%s]", resp.field.data);
			p_status = 0;
			return GPRS_API_PARSE_ERROR;
		}
		sscanf(pos+1, "%hu", &p_status);

	} else {
		LOG4CPP_DEBUG(log, "Query GSM status using AT interface");
		d_tty->sendSerial("AT+CREG?", &vresp);
		//TODO implement the returning of the value
		p_status = 0;
	}

	return OK;
}

exitCode
EnforaAPI::gprsStatus(unsigned short & p_status) {
	t_apiCommand msg;
	t_apiResponce resp;
	t_stringVector vresp;
	exitCode result;
	char * pos;

	if (d_apiEnabled) {

		result = gprsAttach(true);
		if ( result != OK ) {
			return result;
		}

		LOG4CPP_DEBUG(log, "Query GPRS status using API interface");

		memcpy(msg.field.data, "AT+CGREG?\0", 10);
		result = sendAT(msg, resp);
		if ( result != OK ) {
			LOG4CPP_ERROR(log, "API, SEND[AT+CGREG?] failed");
			return result;
		}

		LOG4CPP_DEBUG(log, "API, SEND[AT+CGREG?]: %s", resp.field.data);

		// responce example "+CGREG: 0,1"
		pos = strstr(resp.field.data, ",");
		if (!pos) {
			LOG4CPP_WARN(log, "Failed parsing AT+CGREG? responce [%s]", resp.field.data);
			p_status = 0;
			return GPRS_API_PARSE_ERROR;
		}
		sscanf(pos+1, "%hu", &p_status);

	} else {
		LOG4CPP_DEBUG(log, "Query GPRS status using AT interface");
		d_tty->sendSerial("AT+CGREG?", &vresp);
		//TODO implement the returning of the value
		p_status = 0;
	}

	return OK;
}

exitCode
EnforaAPI::gprsRegistered(bool resetOnFailure) {
	unsigned short l_status;
	unsigned short level;
	exitCode result;

	// Checking GSM registration status
	result = gsmStatus(l_status);
	if ( result != OK ) {
		LOG4CPP_ERROR(log, "Failed querying GSM Network registration status");
		return result;
	}

	LOG4CPP_DEBUG(log, "GSM registration status [%hu]", l_status);
	switch (l_status) {
	case 0:
		LOG4CPP_ERROR(log, "GSM not registered (not searching)");
		result = GPRS_NETWORK_REGDENIED;
		break;
	case 1:
		LOG4CPP_DEBUG(log, "GSM registered (home network)");
		result = OK;
		break;
	case 2:
		LOG4CPP_WARN(log, "GSM not registered (searching)");
		result = GPRS_NETWORK_UNREGISTERED;
		break;
	case 3:
		LOG4CPP_ERROR(log, "GSM registration denied");
		result = GPRS_NETWORK_REGDENIED;
		break;
	case 5:
		LOG4CPP_DEBUG(log, "GSM registered (roaming)");
		result = OK;
		break;
	default:
		LOG4CPP_ERROR(log, "Unknowen");
		result = GPRS_NETWORK_REGDENIED;
		break;
	}

	if ( result != OK ) {
		if ( result == GPRS_NETWORK_UNREGISTERED ){
			// Try later... we are already searching
			return GPRS_NETWORK_UNREGISTERED;
		}
		result = gsmRegister();
		if ( result != OK ) {
			return result;
		}
	}

	// Checking Signal Level
//MARKLINE;
	result = signalLevel(level);
	if ( result != OK ) {
		LOG4CPP_ERROR(log, "Failed querying GSM signal level");
		return result;
	}

	// Checking GPRS registration status
	result = gprsStatus(l_status);
	if ( result != OK ) {
		LOG4CPP_ERROR(log, "Failed querying GPRS Network registration status");
		return result;
	}

	LOG4CPP_DEBUG(log, "GPRS registration status [%hu]", l_status);
	switch (l_status) {
	case 0:
		LOG4CPP_ERROR(log, "GPRS not registered (not searching)");
		result = GPRS_NETWORK_REGDENIED;
		break;
	case 1:
		LOG4CPP_DEBUG(log, "GPRS registered (home network)");
		result = OK;
		break;
	case 2:
		LOG4CPP_WARN(log, "GPRS not registered (searching)");
		result = GPRS_NETWORK_UNREGISTERED;
		break;
	case 3:
		LOG4CPP_ERROR(log, "GPRS registration denied");
		result = GPRS_NETWORK_REGDENIED;
		break;
	case 5:
		LOG4CPP_DEBUG(log, "GPRS registered (roaming)");
		result = OK;
		break;
	default:
		LOG4CPP_ERROR(log, "Unknowen");
		result = GPRS_NETWORK_REGDENIED;
		break;
	}

	if ( result != OK ) {
		// Detaching GPRS network
		gprsAttach(false);
	}

	return result;

}

exitCode
EnforaAPI::gprsAttach(bool on) {
	t_apiCommand msg;
	t_apiResponce resp;
	t_stringVector vresp;
	exitCode result;
	char * pos;
	unsigned short l_status;

	if (d_apiEnabled) {

		memcpy(msg.field.data, "AT+CGATT?\0", 10);
		result = sendAT(msg, resp);
		if ( result != OK ) {
			LOG4CPP_ERROR(log, "API, SEND[AT+CGATT?] failed");
			return result;
		}

		LOG4CPP_DEBUG(log, "API, SEND[AT+CGATT?]: %s", resp.field.data);

		// responce example "+CGATT: 1"
		pos = strstr(resp.field.data, ":");
		if (!pos) {
			LOG4CPP_WARN(log, "Failed parsing 'AT+CGATT?' responce: %s", resp.field.data);
			//suppose we are detached and send the attach command
			l_status = 0;
		} else {
			sscanf(pos+2, "%hu", &l_status);
		}

		if ( on ) {

			if ( l_status!=0 ) {
				LOG4CPP_DEBUG(log, "GPRS network is attached");
				return OK;
			}

			LOG4CPP_INFO(log, "Attaching GPRS network...");

			memcpy(msg.field.data, "AT+CGATT=1\0", 11);
			result = sendAT(msg, resp, ENFORAAPI_AT_CMD_TIMEOUT);
			if ( result != OK ) {
				LOG4CPP_ERROR(log, "API, SEND[AT+CGATT=1] failed");
				return result;
			}

			LOG4CPP_DEBUG(log, "API, SEND[AT+CGATT=1]: %s", resp.field.data);

			LOG4CPP_DEBUG(log, "Waiting 5s second to complete GPRS attach...");
			::sleep(5);

		} else {

			if ( l_status==0 ) {
				LOG4CPP_DEBUG(log, "GPRS network deatached");
				return OK;
			}

			LOG4CPP_INFO(log, "Detaching GPRS network...");

			memcpy(msg.field.data, "AT+CGATT=0\0", 11);
			result = sendAT(msg, resp, ENFORAAPI_AT_CMD_TIMEOUT);
			if ( result != OK ) {
				LOG4CPP_ERROR(log, "API, SEND[AT+CGATT=0] failed");
				return result;
			}

			LOG4CPP_DEBUG(log, "API, SEND[AT+CGATT=0]: %s", resp.field.data);

		}

	} else {
		LOG4CPP_DEBUG(log, "Query GPRS status using AT interface");
		d_tty->sendSerial("AT+CGATT?", &vresp);
		//TODO implement the returning of the value
		l_status = 0;
	}

	return OK;

}


// FIXME this code MUST start a detached thread and not the RUN method of this class.
// This class is a CommandGenerator: thus starting the run method will require
// to have a dispacher defined... otherwise the command could not be delivered...
// We don't set the 'enabled' attribute: this allow to disable the command dispatching
// until a dispatcher is attached...
exitCode
EnforaAPI::runParser() {
	LOG4CPP_DEBUG(log, "Starting the log parser");
	this->start();
	return OK;
};

void
EnforaAPI::run (void) {

// 	d_pid = getpid();
// 	LOG4CPP_INFO(log, "EnforaAPI thread (%u) started", d_pid);
	threadStartNotify("GPRS");

	do {
		::sleep(60);
	} while( !d_doExit );

	threadStopNotify();

// 	std::ostringstream tName;
//
//
// 	tName << "run_" << d_name << "-" << d_module;
// 	PosixThread::setName(tName.str().c_str());
// 	d_runThread = this;
//
// 	LOG4CPP_DEBUG(log, "Starting thread [%s], code @ [%p]",
// 			PosixThread::getName(),
// 			d_runThread);
//
// 	pppdMonitor();



//NOTE we should place the following code (for unsolicited messages hearing)
//	in a thread by itself.

/*
	int			ear;
	struct sockaddr_in	la;
	struct sockaddr_in	ra;
	socklen_t		ll;
	socklen_t		rl;
	int 			count;
	char 			msg[ENFORAAPI_AT_RESP_MAXLEN];

	// Creating socket for Unsolicited messages
	ear = socket(PF_INET, SOCK_DGRAM, 0);
	if ( ear == -1 ) {
		LOG4CPP_ERROR(log, "Failed creating EAR socket: %s",
				strerror(errno));
		return;
	}

	memset(&la, 0, sizeof(la));
	la.sin_family = AF_INET;
	la.sin_addr.s_addr = htonl(INADDR_ANY);
	la.sin_port = htons(DEFAULT_LOCAL_HEAR_PORT);
	if ( bind(ear, (struct sockaddr*)&la, sizeof(la)) < 0 ) {
		LOG4CPP_ERROR(log, "Failed binding EAR socket: %s",
				strerror(errno));
		return;
	}

	LOG4CPP_INFO(log, "Binded EAR socket on port [%lu]", ntohs(la.sin_port));


#if 0
/// FIXME this should be done by the non-GPRS activation method... ;-)
	// Register for unsolecited reception
	if (firstRun) {
		addHeader(msg, REQU);
		count = sendto(modem, msg, ENFORAAPI_API_HEADER_SIZE, 0,
				(struct sockaddr*)&raddr, sizeof(raddr));
		if ( g_exitOnFail && (count < 0) ) {
			cerr << "Unsolicited registration failed" << endl;
			pthread_exit(NULL);
		}
		firstRun = false;
	}
#endif

	while (1) {
		count = recvfrom(ear, msg, ENFORAAPI_AT_RESP_MAXLEN, 0,
					(struct sockaddr *)&ra, &rl);
		if ( count<0 ) {
			cerr << "EAR reception failed" << endl;
			continue;
		}

		// Checking for errors
		if ( msg[ENFORAAPI_API_HEADER_TYPE_POS] ==
			d_apiHeaders[GETU][ENFORAAPI_API_HEADER_TYPE_POS] ) {
			LOG4CPP_ERROR(log, "EAR reception error");
		}

//FIXME
continue;
// 		if (!g_showUnsolicited) continue;

		msg[count] = 0;

		switch(msg[1]) {
		case 0:	// Unsolicited message
// 			printUnsolicited(msg);
			break;
		case 10: // Wakeup message
// 			sendACK();
			break;
		}
	}

	::close(ear);
*/

}

}// namespace gprs
}// namespace controlbox
