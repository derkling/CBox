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

#include "DeviceSerial.ih"

namespace controlbox {
namespace device {


DeviceSerial::DeviceSerial(std::string const & base, std::string const & logName) :
			d_config(Configurator::getInstance()),
			d_ttyConfig(""),
			d_ttyMuxPort(0),
			d_gpio(0),
			d_detachedMode(false),
			d_base(base),
			d_isOpen(false),
			log(log4cpp::Category::getInstance(logName)) {
	DeviceFactory * df = DeviceFactory::getInstance();
	char * muxStart = 0;
	char * confStart = 0;
	char newTTYConf[128];
	std::string l_param;

	LOG4CPP_DEBUG(log, "Loading serial device configuration");

	//----- Loading TTY configuration params
	//TODO: Verify 'd_config' initialization

	// Loading the TTY port configuration string
	d_ttyConfig = d_config.param(paramName("tty_device"),
					DEVICESERIAL_DEFAULT_DEVICE);

	// Look if it is a multiplexed port
	muxStart = strchr(d_ttyConfig.c_str(), '-');
	if (muxStart) {
		sprintf(newTTYConf, "%s", d_ttyConfig.c_str());
		muxStart = strchr(newTTYConf, '-');
		// NOTE virtual ports = [1..4], configuration string = [A..D]
		d_ttyMuxPort = muxStart[1]-'A'+1;
// 		sscanf(muxStart, "%hu", &d_ttyMuxPort);
		confStart = strchr(muxStart, ':');
		sprintf(muxStart, "%s", confStart);

		d_ttyConfig = std::string(newTTYConf);
		d_gpio = df->getDeviceGPIO();     // Needed to switch mux TTYs
		d_detachedMode = true;

		LOG4CPP_INFO(log, "Using multiplexed TTY [%s], on virtual port [%c]",
					  d_ttyConfig.c_str(), ('A'-1)+d_ttyMuxPort);
	}

	memset(d_lineTerminator, 0, 3);
	strncpy(d_lineTerminator,
		d_config.param(
		  paramName("tty_lineTerminator"),
		  DEVICESERIAL_DEFAULT_LINE_TERMINATOR).substr(0,2).c_str(),
		2);

	d_initString = d_config.param(paramName("tty_initString"),
					DEVICESERIAL_DEFAULT_INIT_STRING);

	l_param = d_config.param(paramName("tty_flowCtrl"), DEVICESERIAL_DEFAULT_FLOW_CTRL);
	sscanf(l_param.c_str(), "%u", &d_flowCtrl);

	l_param = d_config.param(paramName("tty_respDelay"), DEVICESERIAL_DEFAULT_RESPONCE_DELAY);
	sscanf(l_param.c_str(), "%u", &d_respDelay);

	l_param = d_config.param(paramName("tty_respTimeout"), DEVICESERIAL_DEFAULT_RESPONCE_TIMEOUT);
	sscanf(l_param.c_str(), "%u", &d_respTimeout);

}


DeviceSerial::~DeviceSerial() {

}

exitCode
DeviceSerial::detachedMode(bool enable) {
	if ( d_detachedMode && !enable ) {
		LOG4CPP_INFO(log, "Transaction start");
		openSerial(false);
	}
	if ( !d_detachedMode && enable ) {
		closeSerial(false);
		LOG4CPP_INFO(log, "Transaction end");
	}
	d_detachedMode = enable;
	return OK;
}

exitCode
DeviceSerial::openSerial(bool p_init, t_stringVector * resp) {

	// Return if we are already open
	if ( d_isOpen ) {
		return OK;
	}

	if (!d_ttyConfig.size()) {
		LOG4CPP_ERROR(log, "Failed opening serial: missing configuration");
		return GPRS_TTY_OPEN_FAILURE;
	}

	// Checking if the port is multiplexed
	if (d_gpio) {
		LOG4CPP_DEBUG(log, "Waiting for mutex acquisition and port switching");
		d_gpio->ttyLock(d_ttyMuxPort);
		LOG4CPP_INFO(log, "Opening TTY [%s], virtual port [%c]", d_ttyConfig.c_str(), ('A'-1)+d_ttyMuxPort);
	} else {
		LOG4CPP_INFO(log, "Opening TTY [%s]", d_ttyConfig.c_str());
	}

	// Openinf the port
	this->open(d_ttyConfig.c_str());
	if ( !(*this) ) {
		if (d_gpio) {
			LOG4CPP_ERROR(log, "Failed opening TTY [%s], virtual port [%c]", d_ttyConfig.c_str(), ('A'-1)+d_ttyMuxPort);
		} else {
			LOG4CPP_INFO(log, "Failed opening TTY [%s]", d_ttyConfig.c_str());
		}
		return TTY_OPEN_FAILURE;
	}
	// Disabling TTY buffers
	this->interactive(true);

	// Configuring Flow Control
	switch (d_flowCtrl) {
	case ost::Serial::flowSoft:
		LOG4CPP_DEBUG(log, "Setting software flow control (XON/XOFF)");
		this->setFlowControl(ost::Serial::flowSoft);
		break;
	case ost::Serial::flowHard:
		LOG4CPP_DEBUG(log, "Setting hardware flow control (RTS/CTS)");
		this->setFlowControl(ost::Serial::flowHard);
		break;
	default:
		LOG4CPP_DEBUG(log, "Disabling flow control");
		this->setFlowControl(ost::Serial::flowNone);
	}

	d_isOpen = true;

	if (p_init) {
		LOG4CPP_INFO(log, "Sending initialization string [%s]...", d_initString.c_str());
		sendSerial(d_initString, resp);
	}

	return OK;
}

bool
DeviceSerial::isOpen() {
	return d_isOpen;
}

exitCode
DeviceSerial::closeSerial(bool p_sync) {

	if ( !d_isOpen )
		return OK;

	if (p_sync)
		sync();

	this->close();
	d_isOpen = false;

	// Checking if the port is multiplexed
	if (d_gpio) {
		LOG4CPP_DEBUG(log, "Releasing port mutex");
		d_gpio->ttyUnLock(d_ttyMuxPort);
	}

	return OK;
}


exitCode
DeviceSerial::sendSerial(std::string str, t_stringVector * resp) {
	exitCode result;

// 	if (d_detachedMode && d_gpio) {
// 		// acquiring port mutex and opening port
// 		LOG4CPP_DEBUG(log, "Writing port in detached mode");
		openSerial();
// 	}

	if ( !d_isOpen ) {
		LOG4CPP_WARN(log, "Failed writing: TTY is not open");
		return TTY_NOT_OPEN;
	}

	(*this) << str.c_str() << '\xD';//d_atCmdEscape.c_str();
	sync();

	LOG4CPP_DEBUG(log, "TTY_SEND: %s", str.c_str());
	ost::Thread::sleep(d_respDelay);

	if (!resp) {
		if (d_gpio) {
			// releasing port mutex, synching and closing port
			closeSerial(true);
		}
		return OK;
	}

	result = readSerial(resp);

	if (d_detachedMode && d_gpio) {
		// releasing port mutex, synching and closing port
		closeSerial(true);
	}

	return result;

}

exitCode
DeviceSerial::readSerial(t_stringVector * resp, bool blocking) {
	char cbuf[256];
	exitCode result = TTY_NOT_RESPONDING;

// 	if (d_detachedMode && d_gpio) {
// 		// acquiring port mutex and opening port
// 		LOG4CPP_DEBUG(log, "Reading port in detached mode");
		openSerial ();
// 	}

	if ( !d_isOpen ) {
		LOG4CPP_WARN(log, "Failed reading: TTY is not open");
		return TTY_NOT_OPEN;
	}

//TODO implement the blocking call for this method

	// At least ONE line is expected in order to have a successfully read
	if (!isPending(ost::Serial::pendingInput, d_respTimeout)) {
		LOG4CPP_WARN(log, "device not responding");
		return  TTY_NOT_RESPONDING;
	}

	result = OK;
	do {
		cbuf[0] = 0;
		getline(cbuf, 256);
		if (cbuf[1]) { // This will throw away empty lines
			// Delating terminating "\r"
			cbuf[strlen(cbuf)-1] = 0;
			if (resp) { // Save only if required...
				resp->push_back( std::string(cbuf) );
			} // while debugging is always allowed
			LOG4CPP_DEBUG(log, "TTY_READ: %s", cbuf);
		}
	} while ( isPending(ost::Serial::pendingInput, 100) );

	if (d_detachedMode && d_gpio) {
		// releasing port mutex, synching and closing port
		closeSerial(true);
	}

	return result;

}


exitCode
DeviceSerial::sendSerial(const char * buf, const int len) {
#ifdef CONTROLBOX_DEBUG
	char str[512];
#endif

// 	if (d_detachedMode && d_gpio) {
// 		// acquiring port mutex and opening port
// 		LOG4CPP_DEBUG(log, "Writing port in detached mode");
		openSerial ();
// 	}

	if ( !d_isOpen ) {
		LOG4CPP_WARN(log, "Failed writing: TTY is not open");
		return TTY_NOT_OPEN;
	}

	this->aWrite(buf, len);
	(*this) << "\x0D";//d_atCmdEscape.c_str();
	sync();

#ifdef CONTROLBOX_DEBUG
	printHexBuf(buf, len, str, 512);
	LOG4CPP_DEBUG(log, "TTY_SEND: %s 0D", str);
#endif

	ost::Thread::sleep(d_respDelay);

	if (d_detachedMode && d_gpio) {
		// releasing port mutex, synching and closing port
		closeSerial(true);
	}

	return OK;

}


exitCode
DeviceSerial::readSerial(char * resp, int & len, bool blocking) {
	int count = 0;
#ifdef CONTROLBOX_DEBUG
	char str[4096];
#endif

// 	if (d_detachedMode && d_gpio) {
// 		// acquiring port mutex and opening port
// 		LOG4CPP_DEBUG(log, "Reading port in detached mode");
		openSerial ();
// 	}

	if ( !d_isOpen ) {
		LOG4CPP_WARN(log, "Failed reading: TTY is not open");
		return TTY_NOT_OPEN;
	}

//TODO implement the blocking call for this method

	// At least ONE char is expected in order to have a successfully read
	if (!isPending(ost::Serial::pendingInput, d_respTimeout)) {
		LOG4CPP_WARN(log, "device not responding");
		resp[0] = 0;
		return  TTY_NOT_RESPONDING;
	}

	do {
		count += this->aRead(resp+count, len-count);
// FIXME: we should use 'd_lineTerminator'
//	} while ( resp[count-1] != d_lineTerminator &&
	} while ( resp[count-1] != '\x0D' &&
			count<len &&
			isPending(ost::Serial::pendingInput, 100)
		);


#ifdef CONTROLBOX_DEBUG
	printHexBuf(resp, count, str, 4096);
	LOG4CPP_DEBUG(log, "TTY_READ_HEX   [%d bytes]: %s", count, str);
	printBuf(resp, count, str, 4096);
	LOG4CPP_DEBUG(log, "TTY_READ_ASCII [%d bytes]: %s", count, str);
#endif

	// NULL-Terminating the received buffer
	resp[count] = 0;
	len = count;

	if (d_detachedMode && d_gpio) {
		// releasing port mutex, synching and closing port
		closeSerial(true);
	}

	return OK;

}

void
DeviceSerial::printHexBuf(const char * buf, unsigned len, char * str, unsigned size) {
	unsigned i;

	for (i=0; i<len && ((i+1)*3)<size; i++)
		sprintf(str+(i*3), " %02X", buf[i]);

	str[i*3] = 0;

}

void
DeviceSerial::printBuf(const char * buf, unsigned len, char * str, unsigned size) {
	unsigned i;
	char * pos;
	char * limit;

	pos = str; limit = str+size;
	for (i=0; i<len; i++) {
		if ( isprint(buf[i]) ) {
			// print char for printable bytes
			sprintf(pos, "%c", buf[i]);
			pos++;
		} else {
			// print HEX for non printable bytes
			sprintf(pos, " 0x%02X ", buf[i]);
			pos += 6;
		}
		if ( (pos+6) >= limit )
			break;
	}

	(*pos) = 0;

}




} // namespace gprs
} // namespace controlbox
