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

const char *DeviceSignals::intrName[] = {
	"Device Open Alarm",
	"ODO-GPS Alarm",
	"Battery Powered",
	"MemoryCard detect",
	"Digital Sensors Alarm",
};

DeviceSignals * DeviceSignals::d_instance = 0;

DeviceSignals::DeviceSignals(std::string const & logName) :
	Device(Device::DEVICE_SINGALS, logName, logName),
	CommandGenerator(logName),
	d_config(Configurator::getInstance()),
	d_isBatteryPowered(false),
	log(Device::log) {
	DeviceFactory * df = DeviceFactory::getInstance();
	t_intMask curMask;
	int err;


	LOG4CPP_DEBUG(log, "DeviceSignals(logName=%s)", logName.c_str());

	// Registering device into the DeviceDB
	dbReg();
	d_time = df->getDeviceTime();

	d_devpath = d_config.param("Signal_devpath", DEVICESIGNALS_DEV_PATH);
	LOG4CPP_DEBUG(log, "Using PortA device [%s]", d_devpath.c_str());

	d_curMask.loLines = 0x00;
	d_curMask.hiLines = 0x00;
	d_curMask.boLines = 0x00;

	// Opening input port
	initSignals();


#ifdef CONTROLBOX_CRIS
	// Configuring OUTPUT Pins
	curMask=0x0C;
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_SETGET_OUTPUT), &curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set output lines to [0x%02X], %s", curMask, strerror(errno));
	}
	LOG4CPP_DEBUG(log, "Outputs configuration [0x%02X]", curMask);

	// Configuring INPUT Pins
	curMask=0xF3;
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_SETGET_INPUT), &curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set input lines to [0x%02X], %s", curMask, strerror(errno));
	}
	LOG4CPP_DEBUG(log, "Inputs configuration [0x%02X]", curMask);

	// Reading current levels state
	getLevels(d_levels);

	// Resetting all interrupts (especially from LED1, LED2 and SW)
	curMask = 0xFF;
	LOG4CPP_DEBUG(log, "Resetting alarms configuration for lines [0x%02X]", curMask);
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_CLRALARM), curMask );
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to clear input lines alarms, %s", strerror(errno));
	}
	LOG4CPP_DEBUG(log, "All alarms have been cleared [0x%02X]", curMask);

	// Initializing LIGHT interrupt
	// NOTE LIGHT interrupt line is LOW when dark (aka device box is closed)
	d_isDeviceOpen = ( d_levels & (0x1 << DEVICESIGNALS_DEVICEOPEN_LINE) );
	confInterrupt( (1<<DEVICESIGNALS_DEVICEOPEN_LINE), d_isDeviceOpen);
	LOG4CPP_INFO(log, "cBox device %s", d_isDeviceOpen ? "OPENED" : "CLOSED" );
	LOG4CPP_DEBUG(log, "Configuring LIGHT interrupt on PA%d", DEVICESIGNALS_DEVICEOPEN_LINE);
	handlersNumber[INT_DEVICEOPEN] = 1;

// 	// Initializing ODO-GPS interrupt
// 	// NOTE ODO-GPS interrupt line is LOW when DIGITAL state is changed
// 	d_isOdoGpsAlarm = !( d_levels & (0x1 << DEVICESIGNALS_ODOGPS_LINE) );
// 	confInterrupt( (1<<DEVICESIGNALS_ODOGPS_LINE), !d_isOdoGpsAlarm);
// 	LOG4CPP_DEBUG(log, "Configuring ODO-GPS interrupt on PA%d", DEVICESIGNALS_ODOGPS_LINE);
	handlersNumber[INT_ODOGPS] = 0;
	d_isOdoGpsAlarm = false;

	// Initializing BATTERY interrupt
	// NOTE battery interrupt line is LOW when running on battery
	d_isBatteryPowered = !( d_levels & (0x1 << DEVICESIGNALS_BATTERY_LINE) );
	confInterrupt( (1<<DEVICESIGNALS_BATTERY_LINE), !d_isBatteryPowered);
	LOG4CPP_INFO(log, "cBox %s powered", d_isBatteryPowered ? "BATTERY" : "AC" );
	LOG4CPP_DEBUG(log, "Configuring BATTERY interrupt on PA%d", DEVICESIGNALS_BATTERY_LINE);
	handlersNumber[INT_BATTERY] = 1;

	// Initializing MMC interrupt
	// NOTE MMC interrupt line is LOW when MMC is present
	d_isMmcPresent = !( d_levels & (0x1 << DEVICESIGNALS_MMC_LINE) );
	confInterrupt( (1<<DEVICESIGNALS_MMC_LINE), !d_isMmcPresent);
	LOG4CPP_INFO(log, "Memory is %sPRESENT", d_isMmcPresent ? "" : "NOT ");
	LOG4CPP_DEBUG(log, "Configuring MMC interrupt on PA%d", DEVICESIGNALS_MMC_LINE);
	handlersNumber[INT_MMC] = 1;

// 	// Initializing GPIO interrupt
// 	// NOTE GPIO interrupt line is LOW when GPIO changed states
// 	d_isGpioAlarm = !( d_levels & (0x1 << DEVICESIGNALS_GPIO_LINE) );
// 	confInterrupt( (1<<DEVICESIGNALS_GPIO_LINE), !d_isGpioAlarm);
// 	LOG4CPP_DEBUG(log, "Configuring DS interrupt on PA%d", DEVICESIGNALS_GPIO_LINE);
	handlersNumber[INT_GPIO] = 0;
	d_isGpioAlarm = false;

#endif

	// Start the working thread
	this->start();

}

exitCode DeviceSignals::initSignals() {

#ifdef CONTROLBOX_CRIS
	if ((fd_dev = open(d_devpath.c_str(), O_RDWR))<0) {
		LOG4CPP_ERROR(log, "failed to open device [%s], %s\n", d_devpath.c_str(), strerror(errno));
		fd_dev = 0;
		return INT_DEV_FAILED;
	}
#endif

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

// NOTE we use a bitmask to disable foxboards LED1, LED2 and SW1 lines!
	lines &= ~0x0E;

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
	int err = 0;
	t_intMask curMask = 0x0;

	LOG4CPP_DEBUG(log, "Updating interrupts - Lo [0x%02X], Hi [0x%02X]",
				d_curMask.loLines,
				d_curMask.hiLines);

#ifdef CONTROLBOX_CRIS

	curMask = d_curMask.loLines | d_curMask.hiLines | d_curMask.boLines;
	LOG4CPP_DEBUG(log, "Resetting alarms configuration for lines [0x%02X]", curMask);
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_CLRALARM), curMask );
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

exitCode DeviceSignals::registerHandler(unsigned short i, SignalHandler * sh, const char *p_name, t_interruptTrigger l) {
	t_hMap::iterator anH;
	t_handler * pHandler;
// 	int err;
// 	t_intMask curMask;

	if ( i >= INT_COUNT ) {
		return INT_LINE_UNK;
	}

	// Checking for duplications
	anH = handlers.find(i);
	while (anH != handlers.end() &&
		anH->first == i &&
		anH->second->sh != sh ) {
		anH++;
	}

	if ( anH != handlers.end() && anH->first != i) {
		LOG4CPP_WARN(log, "Device-Interrupt association already defined");
		return INT_ALREADY_DEFINED;
	}

	pHandler = new t_handler();
	pHandler->sh = sh;
	pHandler->line = i;
	pHandler->level = l;
	if (p_name)
		strncpy(pHandler->name, p_name, DEVICESIGNALS_HANDLER_NAME_MAXLEN);
	else
		memset(pHandler->name, 0, DEVICESIGNALS_HANDLER_NAME_MAXLEN);
	handlers.insert(t_binding(i, pHandler));

	LOG4CPP_INFO(log, "Registerd new '%s' signal handler [%s]",
			intrName[i], p_name[0] ? p_name : "UNK");


	switch ( i ) {
	case INT_ODOGPS:
		// Initializing ODO-GPS interrupt
		// NOTE ODO-GPS interrupt line is LOW when DIGITAL state is changed
		d_isOdoGpsAlarm = !( d_levels & (0x1 << DEVICESIGNALS_ODOGPS_LINE) );
		confInterrupt( (1<<DEVICESIGNALS_ODOGPS_LINE), !d_isOdoGpsAlarm);
		LOG4CPP_DEBUG(log, "Configuring ODO-GPS interrupt on PA%d", DEVICESIGNALS_ODOGPS_LINE);
		handlersNumber[i]++;
		break;
	case INT_GPIO:
		// Initializing GPIO interrupt
		// NOTE GPIO interrupt line is LOW when GPIO changed states
		d_isGpioAlarm = !( d_levels & (0x1 << DEVICESIGNALS_GPIO_LINE) );
		confInterrupt( (1<<DEVICESIGNALS_GPIO_LINE), !d_isGpioAlarm);
		LOG4CPP_DEBUG(log, "Configuring DS interrupt on PA%d", DEVICESIGNALS_GPIO_LINE);
		handlersNumber[i]++;
		break;
	}

	// Unlocking the Signal thread
	this->signalThread(SIGCONT);

	return OK;
}

exitCode DeviceSignals::unregisterHandler(SignalHandler * sh) {
	t_hMap::iterator anH;
	unsigned short i;
	t_interruptTrigger level;

	//FIXME check for threads registering more than one signal!!!

	anH = handlers.begin();
	while (anH != handlers.end() &&
		anH->second->sh != sh) {
		anH++;
	}

	if (anH == handlers.end()) {
		LOG4CPP_WARN(log, "No handlers registered by this thread");
		return INT_NO_HANDLER;
	}

	LOG4CPP_DEBUG(log, "Unregistering device handler [%s]",
			anH->second->name[0] ? anH->second->name : "UNK");

	i = anH->first;

	handlersNumber[i]--;
	switch ( i ) {
	case INT_ODOGPS:
		// Disabling ODO-GPS interrupts
		// TODO
		break;
	case INT_GPIO:
		// Disabling GPIO interrupts
		// TODO
		break;
	}


	level = anH->second->level;
	delete anH->second;
	handlers.erase(anH);

	return OK;
}

exitCode DeviceSignals::notifySignal(t_handler & p_handler, t_intMask & levels) {
// 	int ret = 0;
	t_intMask lineMask = 0x0;
	t_intMask lineStatus = 0x0;

// 	// Checking if this line has changed since last notify
// 	if ( !(p_handler.line & d_changedBits) ) {
// 		// This line has already been notified
// 		LOG4CPP_DEBUG(log, "Line [%d] not changed since last reading [%s]",
// 					p_handler.line,
// 					(levels & p_handler.line) ? "HIGH" : "LOW"
// 					);
// 		return OK;
// 	}

	// Cheking if the thread require a notification for this signal
// 	lineStatus = levels & p_handler.line;
	switch ( p_handler.line ) {
	case INT_DEVICEOPEN:
		lineMask = 0x1<<DEVICESIGNALS_DEVICEOPEN_LINE;
		break;
    	case INT_ODOGPS:
		lineMask = 0x1<<DEVICESIGNALS_ODOGPS_LINE;
		break;
    	case INT_BATTERY:
		lineMask = 0x1<<DEVICESIGNALS_BATTERY_LINE;
		break;
    	case INT_MMC:
		lineMask = 0x1<<DEVICESIGNALS_MMC_LINE;
		break;
    	case INT_GPIO:
		lineMask = 0x1<<DEVICESIGNALS_GPIO_LINE;
		break;
	default:
		lineMask = 0x0;
	}
	lineStatus = levels & lineMask;
	if ( ( (p_handler.level==INTERRUPT_ON_LOW) &&  lineStatus) ||
	     ( (p_handler.level==INTERRUPT_ON_HIGH) &&  !lineStatus) ) {
		// No need to notify this handler
		LOG4CPP_DEBUG(log, "Line [%s] not notified on [%s]",
					intrName[p_handler.line],
					(p_handler.level==INTERRUPT_ON_LOW) ? "HIGH" : "LOW"
					);
		return OK;
	}

	LOG4CPP_DEBUG(log, "Notify line [%s] is [%s]",
					intrName[p_handler.line],
					(lineStatus) ? "HIGH" : "LOW"
					);

	LOG4CPP_DEBUG(log, "Notifying device [%s]",
			(p_handler.name[0]) ? p_handler.name : "UNK");
	p_handler.sh->signalNotify();

	return OK;

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
	cSgd->setPrio(0);

	// Notifying command
	notify(cSgd);

	return OK;
}

// NOTE battery input line is LOW when running on battery
exitCode DeviceSignals::checkInterrupts(t_intMask & levels) {
	bool acPowered;
	bool deviceOpen;
	bool mmcPresent;
// 	t_intMask curMask;
// 	int err;
// 	comsys::Command * cSgd;

//----- Checking DEVICE status
	deviceOpen = (levels & (0x1 << DEVICESIGNALS_DEVICEOPEN_LINE));
	if ( d_isDeviceOpen != deviceOpen ) {

		// Reconfiguring DEVICEOPEN interrupt to be triggered on state changing
		confInterrupt((1<<DEVICESIGNALS_DEVICEOPEN_LINE), deviceOpen);

		d_isDeviceOpen = deviceOpen;

		LOG4CPP_INFO(log, "Device %s",
			d_isDeviceOpen ? "OPENED" : "CLOSED" );
	}


//----- Checking BATTERY status
	acPowered = (levels & (0x1 << DEVICESIGNALS_BATTERY_LINE));
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

//----- Checking MMC status
	mmcPresent = !(levels & (0x1 << DEVICESIGNALS_MMC_LINE));
	if ( d_isMmcPresent != mmcPresent ) {

		// Reconfiguring MMC interrupt to be triggered on state changing
		confInterrupt((1<<DEVICESIGNALS_MMC_LINE), !mmcPresent);

		d_isMmcPresent = mmcPresent;

		LOG4CPP_INFO(log, "MMC %s",
			d_isMmcPresent ? "INSERTED" : "REMOVED" );
	}

	return OK;

}

exitCode DeviceSignals::getLevels(t_intMask & levels) {
	int err = 0;

	LOG4CPP_DEBUG(log, "Reading current input levels...");

#ifdef CONTROLBOX_CRIS
	if (!fd_dev) {
		LOG4CPP_ERROR(log, "Device not opened");
		return INT_DEV_FAILED;
	}

	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_READ_INBITS), &levels);
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
	int err = 0;
	exitCode result = OK;

	LOG4CPP_DEBUG(log, "Waiting for interrupts...");

#ifdef CONTROLBOX_CRIS
	if (!fd_dev) {
		LOG4CPP_ERROR(log, "Device not opened");
		return INT_WFI_FAILED;
	}

	FD_ZERO(&set);
	FD_SET(fd_dev, &set);
	err = select(fd_dev+1, &set, NULL, NULL, NULL);
	if ( err < 0) {
		// An EINTR is received every time an handler is added
		// this should not result in an warning
		LOG4CPP_DEBUG(log, "select failed, %s", strerror(errno));
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
	exitCode result;

	d_pid = getpid();
	LOG4CPP_INFO(log, "DeviceSignal thread (%u) started", d_pid);

	sigInstall(SIGCONT);
	while (1) {

		// Waiting for at least one handler to be defined
		curMask = d_curMask.loLines | d_curMask.hiLines | d_curMask.boLines;
		if ( !curMask ) {
			LOG4CPP_DEBUG(log, "Waiting for at least one handler is installed");
			waitSignal(SIGCONT);
		}

		if (waitForIntr(levels) != OK) {
			LOG4CPP_DEBUG(log, "lost interrupt?");
			continue;
		}

LOG4CPP_ERROR(log, "Levels [0x%02X]", levels);

		// Checking Battery status
		result = checkInterrupts(levels);
// 		if ( result == INT_ACBAT_SWITCH ) {
// 			// This was an AC/BAT switch... continue with signals monitoring...
// 			continue;
// 		}

		LOG4CPP_DEBUG(log, "Notifying all registered handlers...");
		anH = handlers.begin();
		while (anH != handlers.end()) {
			notifySignal(*(anH->second), levels);
			anH++;
		}

	}

	return;

}

}// namespace device
}// namespace controlbox
