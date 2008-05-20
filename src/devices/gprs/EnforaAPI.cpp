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
	if (result!=OK) {
		LOG4CPP_FATAL(log, "Failed to build the GPRS device");
		cleanUp();
		throw new exceptions::SerialConfigurationException("Unable to open TTY port");
	}

	// Modem identification reading device ids
	getDeviceIds();
	//TODO check if the modem is supported by this class
	//NOTE modem identifications should be done using AUTOSENSING! ;-)

	// Loading configuration profile (if not already done)
	loadConfiguration();

	// Configure the UDPAPI

	// Switch to GPRS-disconnected mode

	// Configure the host-to-modem PPP link

}

EnforaAPI::~EnforaAPI() {

    LOG4CPP_DEBUG(log, "Destroying the EnforaAPI module [%d]", d_module);
    disableAPI();
    powerOff();

}

exitCode
EnforaAPI::powerOn(bool reset) {

	if (reset) {
		LOG4CPP_WARN(log, "Resetting GPRS device");
		d_gpio->gprsReset(d_module);
		return OK;
	}

	LOG4CPP_INFO(log, "Powering-on GPRS device");
	d_gpio->gprsPower(d_module, device::DeviceGPIO::GPIO_ON);
	return OK;
}

exitCode
EnforaAPI::powerOff() {
	LOG4CPP_DEBUG(log, "Powering-off GPRS device");
	d_gpio->gprsPower(d_module, device::DeviceGPIO::GPIO_OFF);
	return OK;
}

exitCode EnforaAPI::initDeviceGPRS() {
	exitCode result;
	t_stringVector resp;
	unsigned short retry_pwr, retry_com;

	// Ensuring API is disabled
	disableAPI();

	for (retry_pwr=3; retry_pwr; retry_pwr--) {

		// Powering on modem
		result = powerOn(false);
		if (result) {
			LOG4CPP_WARN(log, "Failed to PowerOn modem");
			break;
		}

		// Checking if modem is responding
		for (retry_com=1; retry_com<4; retry_com++) {

			// Opening the TTY using the initialization string
			//   (if provided by DeviceSerial configuration)
			result = d_tty->openSerial(true, &resp);
			if (result==OK)
				break;

			d_tty->closeSerial();
			LOG4CPP_DEBUG(log, "Modem communication failed, retyring in %d s",
						  retry_com*5);

			//Waiting few seconds before retry
			::sleep(retry_com*5);
		}

		if (result==OK)
			break;

		LOG4CPP_DEBUG(log, "Modem communication failed, resetting modem and retrying in 5s");

		//Waiting few seconds before retry
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
	std::ostringstream buf("AT+CSCA=+");
	std::string value;

	// Loading configuration params

	d_pppdCommand	= d_config.param(paramName("api_pppdCommand"), "");

	d_apiRemoteIP	= d_config.param(paramName("api_remoteIP"), DEFAULT_REMOTE_IP);
	value = d_config.param(paramName("api_remotePort"), DEFAULT_REMOTE_PORT);
	sscanf(value.c_str(), "%i", &d_apiRemotePort);

	d_apiLocalIP	= d_config.param(paramName("api_localIP"), DEFAULT_LOCAL_IP);
	value = d_config.param(paramName("api_localPort"), DEFAULT_LOCAL_PORT);
	sscanf(value.c_str(), "%i", &d_apiLocalPort);

	LOG4CPP_DEBUG(log, "API configuration, local %s:%d - remote %s:%d",
				d_apiLocalIP.c_str(), d_apiLocalPort,
				d_apiRemoteIP.c_str(), d_apiRemotePort);


	// Autosensing current configuration
	if ( d_tty->sendSerial("AT$IOCFG?", &resp) != OK ) {
		LOG4CPP_DEBUG(log, "Failed reading current configuration");
		return GPRS_TTY_MODEM_NOT_RESPONDING;
	}

	//TODO parse responce to get the current IOCFG
	// responce format:
	// "$IOCFG: 00000000000000000000 00000000000000000000"
	if (resp[0] == "$IOCFG: 00000000000000000000  00000000000000000000") {
		LOG4CPP_DEBUG(log, "Modem already configured");
		return OK;
	}

	// Resetting to factory defaults
	d_tty->sendSerial("AT&F;E0;Q0;V1;&C1");

	// Configuring GPIO direction
	d_tty->sendSerial("AT$IOCFG=00000000000000000000");
	d_tty->sendSerial("AT$IOGPA=00000000000000000000");

	// GSM not registered
	d_tty->sendSerial("AT$EVENT=1,0,9,0,0");
	// Setting LOW GPIO1
	d_tty->sendSerial("AT$EVENT=1,3,8,0,0");

	// Searching for GSM or problems
	d_tty->sendSerial("AT$EVENT=2,0,9,2,4");
	// Setting HIGH GPIO1
	d_tty->sendSerial("AT$EVENT=2,3,16,0,0");

	// GSM registered @ home network
	d_tty->sendSerial("AT$EVENT=3,0,9,1,1");
	// Slow falshing GPIO1
	d_tty->sendSerial("AT$EVENT=3,3,32,458753,0");

	// GSM registered in roaming
	d_tty->sendSerial("AT$EVENT=4,0,9,5,5");
	// Slow falshing GPIO1
	d_tty->sendSerial("AT$EVENT=4,3,32,458753,0");

	// GPRS not registered
	d_tty->sendSerial("AT$EVENT=5,0,11,0,0");
	// Setting LOW GPIO2
	d_tty->sendSerial("AT$EVENT=5,3,9,0,0");

	// GPRS registered to home network NOT IP
	d_tty->sendSerial("AT$EVENT=6,0,11,1,1");
	// Slow falshing GPIO2
	d_tty->sendSerial("AT$EVENT=6,3,33,458753,0");

	// Set HIGH GPIO3 at modem power on
	d_tty->sendSerial("AT$EVENT=7,0,8,1,1");
	// Setting HIGH GPIO3
	d_tty->sendSerial("AT$EVENT=7,3,18,0,0");

	// Configuring for textual SMS
	d_tty->sendSerial("AT+CMGF=1");

	// Saving current settings
	d_tty->sendSerial("AT&W");

}


exitCode
EnforaAPI::enableAPI(std::string const & linkname) {
	int result;
	t_stringVector resp;
	short sec = 60;		// Seconds to wait for PPP daemon being Up

// 	if ( ! (d_tty->isOpen()) ) {
// 		if (d_tty->openSerial(false, &resp) != OK) {
// 			LOG4CPP_DEBUG(log, "Failed enabling API: unable to open TTY");
// 			return GPRS_TTY_OPEN_FAILURE;
// 		}
// 	}

	if (d_apiEnabled) {
		LOG4CPP_WARN(log, "API seem to be already enabled");
		return OK;
	}

	LOG4CPP_DEBUG(log, "Enabling API...");

	// Setting modem IP and local IP as friend server
	d_tty->sendSerial("AT$UDPAPI=\"192.168.1.101\",1720", &resp);

//TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
//TODO Use parameters for friend server configuration
	d_tty->sendSerial("AT$FRIEND=01,1,\"151.20.55.27\",1721,2", &resp);

	// Configuring dynamic IP notifications
	d_tty->sendSerial("AT$WAKEUP=2,1", &resp);
	d_tty->sendSerial("AT$ACKTM=5,5,0", &resp);
	// FIXME use device ID
	d_tty->sendSerial("AT$MDMID=\"cBox_v0.9_#00001\"", &resp);

	// Configure netlink for non-GPRS connection mode
	d_tty->sendSerial("AT$HOSTIF=3", &resp);
	d_tty->sendSerial(d_supportedLinks[linkname]->pdpContext, &resp);

	// NOTE the command preceding this one must clear all serial output!!!
	d_tty->sendSerial(d_supportedLinks[linkname]->AtDial);

	// Releasing TTY devince for pppd
	d_tty->closeSerial();

	LOG4CPP_DEBUG(log, "Configurtion done: staring PPP daemon...");

	// Start host-side PPP
	LOG4CPP_DEBUG(log, "Starting non-GPRS connection [%s]...", d_pppdCommand.c_str());
	result = system(d_pppdCommand.c_str());
	if ( result ) {
		LOG4CPP_ERROR(log, "Failed starting non-GPRS connection");
		return GPRS_PPPD_FAILURE;
	}

	// NOTE d_apiEnabled will be set true by pppNotifyState once the PPP
	//	daemon will be Up and Running

	// Waiting for PPP being up and running
	// We use busy waiting to avoid blocking on PPP daemon up...
	while (sec-- && !d_apiEnabled) {
		LOG4CPP_DEBUG(log, "Waiting for ppp being up [%d s]...", sec);
		::sleep(1);
	}

	if (!d_apiEnabled)
		return GPRS_PPPD_FAILURE;

	// Wait few time to let ppp configure network interfaces...
	::sleep(5);
	initSocket();
	::sleep(5);

	return OK;

}

exitCode
EnforaAPI::disableAPI() {
	int result, pid;
	std::ostringstream cmdLine("");

	if (!d_apiEnabled) {
		return OK;
	}

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

	d_apiEnabled = false;
	LOG4CPP_DEBUG(log, "GPRS API interface disabled");


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
	d_apiLocalAddr.sin_port = htons(d_apiLocalPort);
	if ( bind(d_modemSocket, (struct sockaddr*)&d_apiLocalAddr, sizeof(d_apiLocalAddr)) < 0 ) {
		LOG4CPP_ERROR(log, "Failed binding API socket: %s", strerror(errno));
		return GPRS_SOCKET_BINDING_FAILED;
	}

	LOG4CPP_DEBUG(log, "API local socket binded");

	// Preparing remote address
	rip = gethostbyname(d_apiRemoteIP.c_str());
	if ( !rip ) {
		LOG4CPP_ERROR(log, "Failed resolving remte IP: %s", strerror(errno));
		::close(d_modemSocket);
		return GPRS_SOCKET_BINDING_FAILED;
	}
	memset(&d_apiRemoteAddr, 0, sizeof(d_apiRemoteAddr));
	d_apiRemoteAddr.sin_family = AF_INET;
	memcpy(&d_apiRemoteAddr.sin_addr, rip->h_addr, rip->h_length);
	d_apiRemoteAddr.sin_port = htons(d_apiRemotePort);

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

	for (i=0; i<ENFORAAPI_API_HEADER_SIZE; i++)
		strHeader[i] = (char)'0' + (char)d_apiHeaders[t][i];

	memcpy(msg.header, strHeader, ENFORAAPI_API_HEADER_SIZE);
	return OK;
}

exitCode
EnforaAPI::sendAT(t_apiCommand & msg, t_apiResponce & resp) {
	int net_result;
	exitCode result;
	unsigned short retry;

	// Setting message header
	addHeader(msg, SENDAT);

	for (retry=1; retry<4; retry++) {

		net_result = sendto(d_modemSocket, msg.raw,
						ENFORAAPI_API_HEADER_SIZE+strlen(msg.field.data), 0,
						(struct sockaddr*)&d_apiRemoteAddr,
						sizeof(d_apiRemoteAddr));
		if (net_result < 0) {
			LOG4CPP_WARN(log, "API failed sending AT command: %s", strerror(errno));
			LOG4CPP_DEBUG(log, "API communication errors, retrying in 2s");
			::sleep(2);
			break;
		}

		result = getResponce(resp);
		if (result==OK)
			break;

		switch(result) {
			case GPRS_API_SELECT_FAILED:
			case GPRS_API_SELECT_TIMEOUT:
				// Reset
				LOG4CPP_INFO(log, "API not working, resetting device before retrying");
				initDeviceGPRS();
				enableAPI(d_curLinkname);
				break;
			case GPRS_API_ATRECV_FAILED:
			case GPRS_API_ATRECV_ERRORS:
				// Retry
				LOG4CPP_DEBUG(log, "API communication errors, retrying in 5s");
				::sleep(5);
				break;
		}
	}

	return result;

}

exitCode
EnforaAPI::getResponce(t_apiResponce & resp) {
	int result;
	char rx[512];
	char *buf;
	char *pos = buf;
	int totBytes = 0;
	struct sockaddr_in remote;
	socklen_t rlen;
	fd_set rfds;
	struct timeval tv;

	// Using data field as buffer for rebuilding the complete responce message
	buf = resp.field.data;

	// Watch modem (filedescriptor) for input available
	FD_ZERO(&rfds);
	FD_SET(d_modemSocket, &rfds);
	// Wait up to five seconds.
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	result = select(d_modemSocket+1, &rfds, NULL, NULL, &tv);
	if (result == -1) {
		LOG4CPP_ERROR(log, "Select failed: %s", strerror(errno));
		return GPRS_API_SELECT_FAILED;
	}
	if (result == 0) {
		LOG4CPP_WARN(log, "Select timeout: modem not responding");
		return GPRS_API_SELECT_TIMEOUT;
	}

	do {
		result = recvfrom(d_modemSocket, rx, 512, MSG_DONTWAIT,
				(struct sockaddr*)&remote, &rlen);
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

// 		if ( !memcmp(rx+result-7, d_atResults[AT_OK], 6) ||
// 			!memcmp(rx+result-12, d_atResults[AT_CONNECT], 11) )
		if ( !memcmp(rx+ENFORAAPI_API_HEADER_SIZE, d_atResults[AT_OK], 6) ||
			!memcmp(rx+ENFORAAPI_API_HEADER_SIZE, d_atResults[AT_CONNECT], 11) )
			LOG4CPP_DEBUG(log, "API, OK/CONNECT returned by modem");
			break;

// 		if ( !memcmp(rx+result-10, d_atResults[AT_ERROR], 9) ) {
		if ( !memcmp(rx+ENFORAAPI_API_HEADER_SIZE, d_atResults[AT_ERROR], 9) ) {
			LOG4CPP_DEBUG(log, "API, ERROR returned by modem");
			break; //anyway the communication was successfull
		}

		// Taking away header and leading \0
		result-=(ENFORAAPI_API_HEADER_SIZE+1);
		pos+=result;
		totBytes+=result;

	} while(1);

	// Coping the header thaking it from last received UDP message
	memcpy(resp.header, rx, ENFORAAPI_API_HEADER_SIZE);

	return OK;

}


exitCode
EnforaAPI::gprsUP() {
	t_apiCommand msg;
	t_apiResponce resp;
	int result;
// 	char *dns1, *dns2;
	std::ostringstream netConf("");
	unsigned short retry;

	if (d_netStatus == LINK_UP) {
		LOG4CPP_WARN(log, "GPRS connection already UP");
		return OK;
	}

	LOG4CPP_INFO(log, "Activating GPRS connection...");

	memcpy(msg.field.data, "AT$CONN\0", 8);
	result = sendAT(msg, resp);
	if (result!=OK) {
		LOG4CPP_WARN(log, "API, SEND[AT$CONN]: %s", resp.field.data);
		return GPRS_API_GPRS_UP_FAILED;
	}


	if ( !strstr(resp.field.data, "CONNECT") ) {
		LOG4CPP_WARN(log, "API, GPRS connect failed");
		return GPRS_API_GPRS_UP_FAILED;
	}

	LOG4CPP_DEBUG(log, "Retriving remote connections parametrs...");

	memcpy(msg.field.data, "AT$NETIP?\0", 10);
	result = sendAT(msg, resp);
	if (result) {
		LOG4CPP_ERROR(log,  "API, SEND[AT$NETIP?] failed");
		return GPRS_API_GPRS_UP_PARAMS_FAILED;
	}

	LOG4CPP_DEBUG(log, "API, SEND[AT$NETIP?]: %s", resp.field.data);

/* Parsing IP addresses of local interface, DNS1 and DNS2
$NETIP: "217.202.048.082",  "213.230.155.094", "213.230.130.222"
*/
	resp.field.data[24] = 0;
	d_localIP = std::string(resp.field.data+9);
	resp.field.data[44] = 0;
	d_dns1 = std::string(resp.field.data+29);
	resp.field.data[63] = 0;
	d_dns2 = std::string(resp.field.data+48);
// 	dns1 = resp.field.data+29; resp.field.data[44] = 0;
// 	dns2 = resp.field.data+48; resp.field.data[63] = 0;
	LOG4CPP_DEBUG(log, "DNS1: %s, DNS2: %s", d_dns1.c_str(), d_dns2.c_str());

	netConf << "echo nameserver " << d_dns1 << " > /etc/resolv.conf && ";
	netConf << "echo nameserver " << d_dns2 << " >> /etc/resolv.conf";
	LOG4CPP_DEBUG(log, "Configuring system resolver [%s]...", netConf.str().c_str());
	result = system(netConf.str().c_str());
	if ( result ) {
		LOG4CPP_ERROR(log, "Failed configuring system resolver [%d]", result);
		return GPRS_RESOLVER_CONFIGURE_FAILED;
	}

//TODO Removing default route: this command could fails... anyway that's not a problem!
// 	netConf.str("");
// 	netConf << "route add default gw " << d_apiRemoteIP << " ppp0";
// 	LOG4CPP_DEBUG(log, "Configuring routing table [%s]...", netConf.str().c_str());
// 	result = system(netConf.str().c_str());
// 	if ( result ) {
// 		LOG4CPP_ERROR(log, "Failed configuring routing table [%d]", result);
// 		return GPRS_ROUTING_CONFIGURE_FAILED;
// 	}

	netConf.str("");
	//netConf << "route add default gw " << d_apiRemoteIP << " ppp0";
// 	netConf << "route add default gw 192.168.1.101 ppp0";

//JUST FOR TEST WE REMOVE THIS LINE
// 	netConf << "route add default dev ppp0";


	LOG4CPP_DEBUG(log, "Configuring routing table [%s]...", netConf.str().c_str());
	result = system(netConf.str().c_str());
	if ( result ) {
		LOG4CPP_ERROR(log, "Failed configuring routing table [%d], %s", result, strerror(errno));
		return GPRS_ROUTING_CONFIGURE_FAILED;
	}

	d_netStatus = LINK_UP;
	return OK;

}

exitCode
EnforaAPI::gprsDOWN() {
	t_apiCommand msg;
	t_apiResponce resp;
	int result;

	if (d_netStatus == LINK_DOWN) {
		return OK;
	}

	memcpy(msg.field.data, "AT$DISC", 8);
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
EnforaAPI::connect(std::string const & linkname) {
	t_supportedLinks::iterator it;
	t_netlink * netlink;
	exitCode result;
	pid_t pid;
	unsigned short status;

	// Check if the required connection is already up
	if ( (d_netStatus >= LINK_GOING_UP) &&
		(d_curNetlinkName) &&
		(linkname == *d_curNetlinkName) ) {

		// Checking if API is Up and GPRS connected...
		result = gprsStatus(status);
		if (!result) {
			//TODO Check the "status" value!!!

			LOG4CPP_DEBUG(log, "The required netlink [%s] is already up", linkname.c_str());
			return OK;
		} else {
			LOG4CPP_DEBUG(log, "Checking if API interface is Up...");
			pid = getPppdPid();
			if (pid == -1) {
				LOG4CPP_DEBUG(log, "API interface is DOWN!");
				d_apiEnabled = false;
			}
			return OK;
		}
	}

	// Check if the required connection is supported
	it = d_supportedLinks.find(linkname);
	if (it == d_supportedLinks.end()) {
		LOG4CPP_ERROR(log, "Netlink [%s] not supported by this GPRS device", linkname.c_str());
		return GPRS_NETLINK_NOT_SUPPORTED;
	}

	// If we are already connected... first we should disconnect
	if ( d_netStatus >= LINK_GOING_UP ) {
		LOG4CPP_DEBUG(log, "Switching GPRS connection from [%s] to [%s]",
				d_curNetlinkName->c_str(), linkname.c_str());
		disconnect();
	}

	LOG4CPP_DEBUG(log, "Connecting GPRS...");
	d_netStatus = LINK_GOING_UP;

	// Configure params for the new netlink (SIM, PDPCTX, DIAL...)
	if ( !d_curNetlinkConf) {
		//TODO SIM Card initialization: we select as first that required
		//		by the firt connection we have to enable
		// Place here the code for SIM card activation
	} else {
		if (d_curNetlinkConf->sim != it->second->sim) {

			//TODO place here the code for SIM card swapping

			LOG4CPP_WARN(log, "SIM Card swapping NOT YET SUPPORTED, aborting!");
			return GENERIC_ERROR;
		}
	}
	d_curNetlinkName = &(it->first);
	d_curNetlinkConf = it->second;

	// Checking if the API is already running
	LOG4CPP_DEBUG(log, "Checking for API state [%s]", d_apiEnabled ? "ON" : "OFF" );
	if (!d_apiEnabled) {
		result = enableAPI(linkname);
		if (result) {
			LOG4CPP_ERROR(log, "API, failed enabling non-GPRS connection");
			return result;
		}
		d_curLinkname = linkname;
	} else {
		// Checking API is working, if not: reset GPRS device!
		result = gprsStatus(status);
		if ( result!=OK ) {
			LOG4CPP_WARN(log, "GPRS not working: resetting device");
			d_netStatus = LINK_DOWN;
			powerOn(true);
			return result;
		}
	}

	result = gprsUP();
	if (result!=OK) {
		LOG4CPP_ERROR(log, "API, failed enabling GPRS");
		d_netStatus = LINK_DOWN;
		return result;
	}

	LOG4CPP_DEBUG(log, "Connection successfull, sending IP address via SMS...");

//FIXME: Add configuration control to enable/disable sms sending
// 	sendSMS("+393473153808", d_localIP);

	d_netStatus = LINK_UP;
	return OK;

}

exitCode
EnforaAPI::disconnect() {
	exitCode result;

	// Check if the connection is already down
	if ( d_netStatus <= LINK_GOING_DOWN ) {
		LOG4CPP_DEBUG(log, "The GPRS netlink is already down");
		return OK;
	}

	LOG4CPP_DEBUG(log, "Disconnecting GPRS...");
	d_netStatus = LINK_GOING_DOWN;

	result = gprsDOWN();
	if (result) {
		LOG4CPP_ERROR(log, "API, failed disabling GPRS");
		return result;
	}

	d_netStatus = LINK_DOWN;
	return OK;
}

void
EnforaAPI::pppNotifyState(bool running) {

	LOG4CPP_DEBUG(log, "Updating API state [%s]", running ? "ON" : "OFF");
	d_apiEnabled = running;

}


exitCode
EnforaAPI::sendSMS(std::string number, std::string text) {
	std::ostringstream buf("");
	t_apiCommand msg;
	t_apiResponce resp;
	t_stringVector vresp;
	int result;

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

		result = sendAT(msg, resp);
		if (result) {
			LOG4CPP_ERROR(log, "API, SEND[%s] failed", buf.str().c_str());
			return GPRS_API_GPRS_UP_FAILED;
		}

		memcpy(msg.field.data, text.c_str(), text.size());
		// Appending Ctrl+Z and NULL to AT command
		msg.field.data[text.size()] = 0x1A;
		msg.field.data[text.size()+1] = 0;

		LOG4CPP_DEBUG(log, "SEND[%s]", msg.field.data);

		result = sendAT(msg, resp);
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
EnforaAPI::signalLevel(unsigned short & level) {
	t_apiCommand msg;
	t_apiResponce resp;
	exitCode result;
	t_stringVector vresp;
	char * pos;

	if (d_apiEnabled) {
		LOG4CPP_DEBUG(log, "Query signal level using API interface");

		memcpy(msg.field.data, "AT+CSQ\0", 7);
		result = sendAT(msg, resp);
		if (result) {
			LOG4CPP_ERROR(log, "API, SEND[AT+CSQ] failed");
			return GPRS_API_GPRS_UP_FAILED;
		}

		LOG4CPP_DEBUG(log, "API, SEND[AT+CSQ]: %s", resp.field.data);

		// responce example "+CSQ: 22,1"
		pos = strstr(resp.field.data, ",");
		if (!pos) {
			LOG4CPP_DEBUG(log, "Failed parsing AT+CSQ responce [%s]", resp.field.data);
			level = 0;
			return GPRS_API_GPRS_UP_FAILED;
		}
		(*pos) = 0;
		sscanf(resp.field.data+5, "%hu", &level);
		//LOG4CPP_DEBUG(log, "Signal level [%s] [%hu]", resp.field.data+5, level);
		LOG4CPP_DEBUG(log, "Signal level [%hu]", level);

	} else {
		LOG4CPP_DEBUG(log, "Query signal level using AT interface");
		d_tty->sendSerial("AT+CSQ", &vresp);
	}

	return OK;
}

exitCode
EnforaAPI::gprsStatus(unsigned short & status) {
	t_apiCommand msg;
	t_apiResponce resp;
	t_stringVector vresp;
	exitCode result;
	char * pos;

	if (d_apiEnabled) {
		LOG4CPP_DEBUG(log, "Query GPRS status using API interface");

		memcpy(msg.field.data, "AT+CGREG?\0", 10);
		result = sendAT(msg, resp);
		if (result) {
			LOG4CPP_ERROR(log, "API, SEND[AT+CGREG?] failed");
			return result;
		}

		LOG4CPP_DEBUG(log, "API, SEND[AT+CGREG?]: %s", resp.field.data);

		// responce example "+CGREG: 0,1"
		pos = strstr(resp.field.data, ",");
		if (!pos) {
			LOG4CPP_DEBUG(log, "Failed parsing AT+CGREG? responce [%s]", resp.field.data);
			status = 0;
			return GPRS_API_GPRS_UP_FAILED;
		}
		sscanf(pos+1, "%hu", &status);
		//LOG4CPP_DEBUG(log, "GPRS registration state [%s] [%hu]", pos+1, status);
		LOG4CPP_DEBUG(log, "GPRS registration state [%hu]", status);

	} else {
		LOG4CPP_DEBUG(log, "Query GPRS status using AT interface");
		d_tty->sendSerial("AT+CGREG?", &vresp);
		//TODO implement the returning of the value
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
	std::ostringstream tName;


	tName << "run_" << d_name << "-" << d_module;
	PosixThread::setName(tName.str().c_str());
	d_runThread = this;

	LOG4CPP_DEBUG(log, "Starting thread [%s], code @ [%p]",
			PosixThread::getName(),
			d_runThread);

	pppdMonitor();



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
