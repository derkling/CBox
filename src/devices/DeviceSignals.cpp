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
//** Filename:      DeviceSignal.cpp
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

#include "DeviceSignals.ih"

namespace controlbox {
namespace device {

DeviceSignals * DeviceSignals::d_instance = 0;

DeviceSignals::DeviceSignals(std::string const & logName) :
	Device(Device::DEVICE_SINGALS, logName, logName),
	d_config(Configurator::getInstance()),
	CommandGenerator(logName),
	log(Device::log) {
	DeviceFactory * df;
	
	LOG4CPP_DEBUG(log, "DeviceSignals(logName=%s)", logName.c_str());
	
	// Registering device into the DeviceDB
	dbReg();
	
	d_devpath = d_config.param("Signal_devpath", DEVICESIGNALS_DEV_PATH);
	LOG4CPP_INFO(log, "Using PortA device [%s]", d_devpath.c_str());
	
	d_curMask.loLines = 0x00;
	d_curMask.hiLines = 0x00;
	d_curMask.boLines = 0x00;
	
	initSignals();
	
	// Start the working thread
	this->start();
	
}

exitCode DeviceSignals::initSignals() {
	
	if ((fd_dev = open(d_devpath.c_str(), O_RDWR))<0) {
		LOG4CPP_ERROR(log, "failed to open device [%s], %s\n", d_devpath.c_str(), strerror(errno));
		fd_dev = 0;
		return INT_DEV_FAILED;
	}
	
	return OK;
	
}

DeviceSignals::~DeviceSignals() {
	t_hMap::iterator anH;
	
	LOG4CPP_INFO(log, "Stopping DeviceSignals");
	
	::close(fd_dev);
	
	// Cleaning up mappings
	anH = handlers.begin();
	while (anH != handlers.end()) {
		delete anH->second;
		anH++;
	}
	handlers.clear();
	
	// Registering device into the DeviceDB
	dbReg();
	
}

DeviceSignals * DeviceSignals::getInstance(std::string const & logName) {
	
	if ( !d_instance ) {
		d_instance = new DeviceSignals(logName);
	}
	
	LOG4CPP_DEBUG(d_instance->log, "DeviceSignals::getInstance()");
	return d_instance;

}

exitCode DeviceSignals::registerHandler(t_interrupt i, ost::PosixThread * t, int s, const char *name, t_interruptTrigger l) {
	t_hMap::iterator anH;
	t_handler * pHandler;
	int err;
	t_intMask curMask;
	
	if ( i >= INT_COUNT ) {
		return INT_LINE_UNK;
	}
	
	// Checking for duplications
	anH = handlers.find(i);
	while (anH != handlers.end() &&
		anH->first == i &&
		anH->second->thread != t ) {
		anH++;
	}
	
	if ( anH != handlers.end() && anH->first != i) {
		LOG4CPP_WARN(log, "Thread-Interrupt association already defined");
		return INT_ALREADY_DEFINED;
	}
	
	pHandler = new t_handler();
	pHandler->thread = t;
	pHandler->level = l;
	pHandler->signo = s;
	if (name)
		strncpy(pHandler->name, name, DEVICESIGNALS_HANDLER_NAME_MAXLEN);
	else
		memset(pHandler->name, 0, DEVICESIGNALS_HANDLER_NAME_MAXLEN);
	handlers.insert(t_binding(i, pHandler));
	
	switch (l) {
	case INTERRUPT_ON_LOW:
		d_curMask.loLines |= (0x1 << i);
		LOG4CPP_DEBUG(log, "ORing loLines mask with 0x%X", (0x1 << i));
		break;
	case INTERRUPT_ON_HIGH:
		d_curMask.hiLines |= (0x1 << i);
		LOG4CPP_DEBUG(log, "ORing hiLines mask with 0x%X", (0x1 << i));
		break;
	case INTERRUPT_ON_BOTH:
		d_curMask.boLines |= (0x1 << i);
		LOG4CPP_DEBUG(log, "ORing boLines mask with 0x%X", (0x1 << i));
		break;
	}

#ifdef CONTROLBOX_CRIS
	// FIXME use an inline function to build the all mask
	curMask = d_curMask.loLines | d_curMask.hiLines | d_curMask.boLines;
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_SETGET_INPUT), &curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set input lines to [0x%X], %s", curMask, strerror(errno));
		return INT_DEV_FAILED;
	}
	LOG4CPP_DEBUG(log, "Inputs configuration [0x%X]", curMask);
#endif
	
	LOG4CPP_INFO(log, "Registerd new signal [%d] handler [%s:%lu], cur mask [0x%X]",
			s, name[0] ? name : "UNK", t, curMask);
			
	// Unlocking the Signal thread
	this->signalThread(SIGCONT);
	
	return OK;
}

exitCode DeviceSignals::unregisterHandler(ost::PosixThread * t) {
	t_hMap::iterator anH;
	t_interrupt i;
	t_interruptTrigger level;
	
	//FIXME check for threads registering more than one signal!!!
	
	anH = handlers.begin();
	while (anH != handlers.end() &&
		anH->second->thread != t) {
		anH++;
	}
	
	if (anH == handlers.end()) {
		LOG4CPP_WARN(log, "No handlers registered by this thread");
		return INT_NO_HANDLER;
	}
	
	LOG4CPP_INFO(log, "Unregistering signal [%d] handler [%s:%lu]",
			anH->second->signo,
			anH->second->name[0] ? anH->second->name : "UNK",
			t);
	
	i = anH->first;
	level = anH->second->level;
	delete anH->second;
	handlers.erase(anH);
	
	if (handlers.find(i) != handlers.end()) {
		switch (level) {
		case INTERRUPT_ON_LOW:
			d_curMask.loLines |= (0x1 << i);
			LOG4CPP_DEBUG(log, "ANDing loLines mask with 0x%X", ~(0x1 << i));
			break;
		case INTERRUPT_ON_HIGH:
			d_curMask.hiLines |= (0x1 << i);
			LOG4CPP_DEBUG(log, "ANDing hiLines mask with 0x%X", ~(0x1 << i));
			break;
		case INTERRUPT_ON_BOTH:
			d_curMask.boLines |= (0x1 << i);
			LOG4CPP_DEBUG(log, "ANDing boLines mask with 0x%X", ~(0x1 << i));
			break;
		}
	}
	
	return OK;
}

exitCode DeviceSignals::notifySignal(t_handler & handler) {
	int ret;
	
	LOG4CPP_DEBUG(log, "Sending signal [%d] to thread [%s:%lu]",
			handler.signo,
			(handler.name[0]) ? handler.name : "UNK",
			handler.thread);
	
	handler.thread->signalThread(handler.signo);
	
	return OK;
	
}


exitCode DeviceSignals::waitForIntr(t_intMask & levels) {
	fd_set set;
	int err;
	t_intMask curMask;
	
	LOG4CPP_DEBUG(log, "Wait for interrupt");

#ifdef CONTROLBOX_CRIS
	if (!fd_dev) {
		LOG4CPP_ERROR(log, "Device not opened");
		return INT_WFI_FAILED;
	}
	
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_READ_INBITS), &levels);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to read input lines, %s", strerror(errno));
		return INT_DEV_FAILED;
	}
	
	// FIXME use an inline function to build the all mask
	curMask = d_curMask.loLines | d_curMask.hiLines | d_curMask.boLines;
	ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_CLRALARM), curMask );
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to clear input lines alarms, %s", strerror(errno));
		return INT_DEV_FAILED;
	}
	LOG4CPP_DEBUG(log, "levels [0x%X], curMaks [0x%X]", levels, curMask);
	
	/* Set HIGH alarm for low bits */
// 	LOG4CPP_DEBUG(log, "Setting high alarms [0x%X]", (~levels & d_curMask));
// 	ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_HIGHALARM), (~levels & d_curMask));
	LOG4CPP_DEBUG(log, "Setting high alarms [0x%X]", (~levels & (d_curMask.hiLines | d_curMask.boLines)));
	ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_HIGHALARM), (~levels & (d_curMask.hiLines | d_curMask.boLines)));
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set high alarms, %s", strerror(errno));
		return INT_DEV_FAILED;
	}
	/* Set LOW alarm for high bits */
// 	LOG4CPP_DEBUG(log, "Setting low alarms [0x%X]", (levels & d_curMask));
// 	ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_LOWALARM), levels & d_curMask);
	LOG4CPP_DEBUG(log, "Setting low alarms [0x%X]", (levels & (d_curMask.loLines | d_curMask.boLines)));
	ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_LOWALARM), (levels & (d_curMask.loLines | d_curMask.boLines)));
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set low alarms, %s", strerror(errno));
		return INT_DEV_FAILED;
	}
#endif

	FD_ZERO(&set);
	FD_SET(fd_dev, &set);
	if ( select(fd_dev+1, &set, NULL, NULL, NULL) < 0) {
		LOG4CPP_WARN(log, "select failed, %s", strerror(errno));
		return INT_SELECT_FAILED;
	}
	
#ifdef CONTROLBOX_CRIS
	ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_READ_INBITS), &levels);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to read input lines, %s", strerror(errno));
		return INT_DEV_FAILED;
	}
#else
	char buf[5];
	buf[4] = 0;
	read(fd_dev, buf, 4);
	levels = atoi((const char *)buf);  //FIXME should we use some keyboard binding to emulate interrupt lines
#endif
	
	return OK;
	
}

void DeviceSignals::run(void)  {
	t_intMask levels;
	t_hMap::iterator anH;
	t_intMask curMask;
	
	LOG4CPP_INFO(log, "monitoring thread started");
	
	sigInstall(SIGCONT);
	while (1) {
		
		// Waiting for at least one handler to be defined
		curMask = d_curMask.loLines | d_curMask.hiLines | d_curMask.boLines;
		if (!curMask) {
			LOG4CPP_DEBUG(log, "Waiting for at least one handler is installed");
			waitSignal(SIGCONT);
		}
		
		if (waitForIntr(levels) != OK) {
			LOG4CPP_DEBUG(log, "lost interrupt?");
			continue;
		}
		
		LOG4CPP_DEBUG(log, "Notifying all registered handlers...");
		anH = handlers.begin();
		while (anH != handlers.end()) {
			notifySignal(*(anH->second));
			anH++;
		}
		
	}
	
	return;
	
}

}// namespace device
}// namespace controlbox
