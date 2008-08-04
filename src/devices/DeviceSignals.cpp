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
	d_isBatteryPowered(false),
	CommandGenerator(logName),
	log(Device::log) {
	DeviceFactory * df = DeviceFactory::getInstance();

	LOG4CPP_DEBUG(log, "DeviceSignals(logName=%s)", logName.c_str());

	// Registering device into the DeviceDB
	dbReg();
	d_time = df->getDeviceTime();

	d_devpath = d_config.param("Signal_devpath", DEVICESIGNALS_DEV_PATH);
	LOG4CPP_INFO(log, "Using PortA device [%s]", d_devpath.c_str());

	d_curMask.loLines = 0x00;
	d_curMask.hiLines = 0x00;
	d_curMask.boLines = 0x00;

	// Opening input port
	initSignals();

	// Reading current levels state
	getLevels(d_levels);

	// Initializing battery interrupt
	// NOTE battery input line is LOW when running on battery
	d_isBatteryPowered = !( d_levels & (0x1 << DEVICESIGNALS_BATTERY_LINE) );
	confInterrupt( (1<<DEVICESIGNALS_BATTERY_LINE), !d_isBatteryPowered);

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

}

DeviceSignals * DeviceSignals::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceSignals(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceSignals::getInstance()");
	return d_instance;

}


exitCode DeviceSignals::confInterrupt(t_intMask lines, bool onLow) {

	if ( onLow ) {
		d_curMask.loLines |=  lines;
		d_curMask.hiLines &= ~lines;
	} else {
		d_curMask.loLines &= ~lines;
		d_curMask.hiLines |=  lines;
	}

	return updateInterrupts();

}


// NOTE using boLines require to switch the interrupt configuration each times
//	the input change!!!... otehrwise we will get an endless internet storm!
exitCode DeviceSignals::updateInterrupts(void) {
	int err;
	t_intMask curMask;

	LOG4CPP_DEBUG(log, "Updating interrupts - Lo [0x%02X], Hi [0x%02X]",
				d_curMask.loLines,
				d_curMask.hiLines);

#ifdef CONTROLBOX_CRIS

// curMask=0x0C;
// 	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_SETGET_OUTPUT), &curMask);
// 	if (err == -1) {
// 		LOG4CPP_ERROR(log, "Failed to set output lines to [0x%02X], %s", curMask, strerror(errno));
// 		return INT_DEV_FAILED;
// 	}
// 	LOG4CPP_DEBUG(log, "Outputs configuration [0x%02X]", curMask);


	// NOTE we force ZERO on line 1 and 2 that are used by the foxboard LEDs
	curMask = (d_curMask.loLines | d_curMask.hiLines | d_curMask.boLines) && 0xF3;
	LOG4CPP_DEBUG(log, "Setting current interrupt mask to [0x%02X]", curMask);
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_SETGET_INPUT), &curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set input lines to [0x%02X], %s", curMask, strerror(errno));
		return INT_DEV_FAILED;
	}
	LOG4CPP_DEBUG(log, "Inputs configuration [0x%02X]", curMask);


	curMask = d_curMask.loLines | d_curMask.hiLines | d_curMask.boLines;
	LOG4CPP_DEBUG(log, "Resetting alarms configuration for lines [0x%02X]", curMask);
	ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_CLRALARM), curMask );
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to clear input lines alarms, %s", strerror(errno));
		return INT_DEV_FAILED;
	}

	curMask = d_curMask.hiLines;
	LOG4CPP_DEBUG(log, "Configuring HIGH alarms for lines [0x%02X]", curMask);
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_HIGHALARM), curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set high alarms, %s", strerror(errno));
		return INT_DEV_FAILED;
	}

	curMask = d_curMask.loLines;
	LOG4CPP_DEBUG(log, "Configuring LOW alarms for lines [0x%02X]", curMask);
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_LOWALARM), curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set low alarms, %s", strerror(errno));
		return INT_DEV_FAILED;
	}
#endif

	return OK;

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
	pHandler->line = (0x1 << i);
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
		LOG4CPP_DEBUG(log, "ORing loLines mask with 0x%02X", (0x1 << i));
		break;
	case INTERRUPT_ON_HIGH:
		d_curMask.hiLines |= (0x1 << i);
		LOG4CPP_DEBUG(log, "ORing hiLines mask with 0x%02X", (0x1 << i));
		break;
	case INTERRUPT_ON_BOTH:
		d_curMask.boLines |= (0x1 << i);
		LOG4CPP_DEBUG(log, "ORing boLines mask with 0x%02X", (0x1 << i));
		break;
	}

	updateInterrupts();

	LOG4CPP_INFO(log, "Registerd new signal [%d] handler [%s:%lu]",
			s, name[0] ? name : "UNK", t);

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

// FIXME before remove alarms for this andler ensure that there is NO other
//	handlers registerd for the same alarm lines!!!
// 	if (handlers.find(i) != handlers.end()) {
// 		switch (level) {
// 		case INTERRUPT_ON_LOW:
// 			d_curMask.loLines |= (0x1 << i);
// 			LOG4CPP_DEBUG(log, "ANDing loLines mask with 0x%02X", ~(0x1 << i));
// 			break;
// 		case INTERRUPT_ON_HIGH:
// 			d_curMask.hiLines |= (0x1 << i);
// 			LOG4CPP_DEBUG(log, "ANDing hiLines mask with 0x%02X", ~(0x1 << i));
// 			break;
// 		case INTERRUPT_ON_BOTH:
// 			d_curMask.boLines |= (0x1 << i);
// 			LOG4CPP_DEBUG(log, "ANDing boLines mask with 0x%02X", ~(0x1 << i));
// 			break;
// 		}
// 	}

	return OK;
}

exitCode DeviceSignals::notifySignal(t_handler & handler, t_intMask & levels) {
	int ret;
	t_intMask lineStatus;

// 	// Checking if this line has changed since last notify
// 	if ( !(handler.line & d_changedBits) ) {
// 		// This line has already been notified
// 		LOG4CPP_DEBUG(log, "Line [%d] not changed since last reading [%s]",
// 					handler.line,
// 					(levels & handler.line) ? "HIGH" : "LOW"
// 					);
// 		return OK;
// 	}

	// Cheking if the thread require a notification for this signal
	lineStatus = levels & handler.line;
	if ( ((handler.level==INTERRUPT_ON_LOW) &&  lineStatus) ||
	     ((handler.level==INTERRUPT_ON_HIGH) &&  !lineStatus) ) {
		// No need to notify this handler
		LOG4CPP_DEBUG(log, "Line [%d] not notified on [%s]",
					handler.line,
					(handler.level==INTERRUPT_ON_LOW) ? "HIGH" : "LOW"
					);
		return OK;
	}

	LOG4CPP_DEBUG(log, "Notify line [%d] is [%s]",
					handler.line,
					(lineStatus) ? "HIGH" : "LOW"
					);

	LOG4CPP_DEBUG(log, "Sending signal [%d] to thread [%s:%lu]",
			handler.signo,
			(handler.name[0]) ? handler.name : "UNK",
			handler.thread);
	d_pendingAck++;
	handler.thread->signalThread(handler.signo);

	return OK;

}

exitCode DeviceSignals::ackSignal() {

	d_pendingAck--;
	LOG4CPP_DEBUG(log, "New signal ACK received, [%d] still pending", d_pendingAck);

	if (!d_pendingAck) {
		// Unlocking the Signal thread
		this->signalThread(SIGCONT);
	}

}

bool DeviceSignals::isBatteryPowered(void) {
	return d_isBatteryPowered;
}

exitCode DeviceSignals::powerOn(bool on) {
	comsys::Command * cSgd;

	cSgd = comsys::Command::getCommand(SYSTEM_EVENT,
		Device::DEVICE_SINGALS, "DEVICE_SINGALS",
		name());
	if ( !cSgd ) {
		LOG4CPP_FATAL(log, "Unable to build a new Command");
		return OUT_OF_MEMORY;
	}
	cSgd->setParam( "dist_evtType", "0D");
	cSgd->setParam( "dist_evtData", on ? "1" : "0" );
	cSgd->setParam( "timestamp", d_time->time() );

	// Notifying command
	notify(cSgd);

	return OK;
}

// NOTE battery input line is LOW when running on battery
exitCode DeviceSignals::checkBattery(t_intMask & levels) {
	bool acPowered;
	t_intMask curMask;
	int err;
	comsys::Command * cSgd;

	acPowered = (levels & (0x1 << DEVICESIGNALS_BATTERY_LINE));

	// Check if we had a state change
	if ( d_isBatteryPowered == acPowered ) {

		// Reconfiguring BAT interrupt to be triggered on state changing
		confInterrupt((1<<DEVICESIGNALS_BATTERY_LINE), acPowered);

		d_isBatteryPowered = !acPowered;

		LOG4CPP_INFO(log, "Switched power to %s",
			d_isBatteryPowered ? "BAT" : "AC" );
/*
		cSgd = comsys::Command::getCommand(SYSTEM_EVENT,
			Device::DEVICE_SINGALS, "DEVICE_SINGALS",
			name());
		if ( !cSgd ) {
			LOG4CPP_FATAL(log, "Unable to build a new Command");
			return OUT_OF_MEMORY;
		}

		cSgd->setParam( "dist_evtType", "0xFF");
		cSgd->setParam( "dist_evtData", d_isBatteryPowered ? "1" : "0" );
		cSgd->setParam( "timestamp", d_time->time() );

		// Notifying command
		notify(cSgd);
*/
	}

	return OK;

}

exitCode DeviceSignals::getLevels(t_intMask & levels) {
	int err;

	LOG4CPP_DEBUG(log, "Reading current input levels...");

#ifdef CONTROLBOX_CRIS
	if (!fd_dev) {
		LOG4CPP_ERROR(log, "Device not opened");
		return INT_DEV_FAILED;
	}

	ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_READ_INBITS), &levels);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to read input lines, %s", strerror(errno));
		return INT_DEV_FAILED;
	}
	LOG4CPP_DEBUG(log, "Inputs levels [0x%02X]", levels);
#else
# warning Using WaitForInterrupt DUMMY implementation
	LOG4CPP_DEBUG(log, "Dummy getLevels return [0x00]");
	levels = 0x00;
#endif

	return OK;

}

exitCode DeviceSignals::waitForIntr(t_intMask & levels) {
	fd_set set;
	int err;
	exitCode result;

	LOG4CPP_DEBUG(log, "Waiting for interrupts...");

#ifdef CONTROLBOX_CRIS
	if (!fd_dev) {
		LOG4CPP_ERROR(log, "Device not opened");
		return INT_WFI_FAILED;
	}

	FD_ZERO(&set);
	FD_SET(fd_dev, &set);
	if ( select(fd_dev+1, &set, NULL, NULL, NULL) < 0) {
		LOG4CPP_WARN(log, "select failed, %s", strerror(errno));
		return INT_SELECT_FAILED;
	}

	result = getLevels(levels);
	if ( result == OK ) {
		d_changedBits = d_levels ^ levels;
		d_levels = levels;
	}

	return result;

#else
# warning Using WaitForInterrupt DUMMY implementation
	::sleep(60);
	LOG4CPP_DEBUG(log, "Dummy WaitForInterrupt tirggered");
	return OK;
#endif

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

		// Checking Battery status
		checkBattery(levels);

		// Check if one of the registered handlers input has changed
// 		if ( ! (curMask && d_changedBits) ) {
// 			LOG4CPP_DEBUG(log, "Int received, no handlers for this line");
// 			continue;
// 		}

		LOG4CPP_DEBUG(log, "Notifying all registered handlers...");
		d_pendingAck = 0;
		anH = handlers.begin();
		while (anH != handlers.end()) {
			notifySignal(*(anH->second), levels);
			anH++;
		}

		if (d_pendingAck) {
			LOG4CPP_DEBUG(log, "Waiting acknowledge from [%d] notified handler/s", d_pendingAck);
			waitSignal(SIGCONT);
		}

	}

	return;

}

}// namespace device
}// namespace controlbox
