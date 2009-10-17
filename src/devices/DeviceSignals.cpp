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

const char *DeviceSignals::signalTypeName[] = {
	"Device Open Alarm",
	"OCG Alarm",
	"Battery Powered",
	"Memory Detect",
	"Digital Sensors Alarm",
};

const char *DeviceSignals::signalTriggerName[] = {
	"NONE",
	"LOW",
	"HIGH",
	"BOTH",
};

DeviceSignals::DeviceSignals(std::string const & logName) :
	CommandGenerator(logName),
	Device(Device::DEVICE_SINGALS, logName, logName),
	Worker(Device::log, "SIG", 0),
	d_config(Configurator::getInstance()),
	log(Device::log) {
	DeviceFactory * df = DeviceFactory::getInstance();


	LOG4CPP_DEBUG(log, "DeviceSignals(logName=%s)", logName.c_str());

	// Registering device into the DeviceDB
	dbReg();
	d_time = df->getDeviceTime();

	d_curTriggers.loSignals = 0x00;
	d_curTriggers.hiSignals = 0x00;

	//NOTE: Derived classes should update the signals too
	d_signalStatus = 0x00;
	
	//FIXME: it could happen that registerThread send a signal before
	// the next working thread has started and registered the signal
	// handler.
	// In that case an abort will happens!!!

	// Start the working thread
	this->start();

}

DeviceSignals::~DeviceSignals() {
	t_hMap::iterator anH;

	LOG4CPP_INFO(log, "Stopping DeviceSignals");

	d_doExit = true;
	this->ost::Thread::resume();

	// Cleaning up mappings
	anH = handlers.begin();
	while (anH != handlers.end()) {
		delete anH->second;
		anH++;
	}
	handlers.clear();

}

exitCode DeviceSignals::registerHandler(unsigned short s, SignalHandler * sh, const char *p_name, t_signalTrigger t) {
	t_hMap::iterator anH;
	t_handler * pHandler;

	if ( s >= SIGNAL_COUNT ) {
		return INT_LINE_UNK;
	}

	// Checking for duplications
	anH = handlers.find(s);
	while (anH != handlers.end() &&
		anH->first == s &&
		anH->second->sh != sh ) {
		anH++;
	}

	if ( anH != handlers.end() && anH->first != s) {
		LOG4CPP_WARN(log, "Device-Signal association already defined");
		return INT_ALREADY_DEFINED;
	}

	pHandler = new t_handler();
	pHandler->sh = sh;
	pHandler->signal = s;
	pHandler->trigger = t;
	if (p_name)
		strncpy(pHandler->name, p_name, DEVICESIGNALS_HANDLER_NAME_MAXLEN);
	else
		memset(pHandler->name, 0, DEVICESIGNALS_HANDLER_NAME_MAXLEN);
	handlers.insert(t_binding(s, pHandler));
	handlersCount[s]++;

	LOG4CPP_INFO(log, "Registerd new '%s' signal handler [%s], triggering on %s level",
			signalTypeName[s], p_name[0] ? p_name : "UNK", signalTriggerName[t]);

	// Unlocking the Signal thread
	signalWorker();

	return OK;
}

exitCode DeviceSignals::unregisterHandler(SignalHandler * sh) {
	t_hMap::iterator anH;
	unsigned short s;
	t_signalTrigger trigger;

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

	LOG4CPP_DEBUG(log, "Unregistering device signal handler [%s]",
			anH->second->name[0] ? anH->second->name : "UNK");

	s = anH->first;
	handlersCount[s]--;

	delete anH->second;
	handlers.erase(anH);

	return OK;
}

bool DeviceSignals::isBatteryPowered(void) {
	return ( d_signalStatus & (0x1<<SIGNAL_BATTERY) );
}

exitCode DeviceSignals::notifyPower(bool on) {
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
	notifyCommand(cSgd);

	return OK;
}

exitCode DeviceSignals::notifyDeviceOpen(bool open) {
	comsys::Command * cSgd;

	cSgd = comsys::Command::getCommand(SYSTEM_EVENT,
		Device::DEVICE_SINGALS, "DEVICE_SINGALS",
		name());
	if ( !cSgd ) {
		LOG4CPP_FATAL(log, "Unable to build a new Command");
		return OUT_OF_MEMORY;
	}
	cSgd->setParam( "dist_evtType", "22");
	cSgd->setParam( "dist_evtData", open ? "1" : "0" );
	cSgd->setParam( "timestamp", d_time->time() );
	cSgd->setPrio(0);

	// Notifying command
	notifyCommand(cSgd);

	return OK;
}

exitCode DeviceSignals::notifySignal(t_handler & p_handler, t_signalMask & signals) {
	t_signalMask signalMask = 0x0;
	t_signalMask signalStatus = 0x0;

	signalMask = 0x1 << p_handler.signal;
	signalStatus =  d_signalStatus & signalMask;

	// Cheking if the thread require a notification for this signal
	if ( ( (p_handler.trigger==TRIGGER_ON_LOW) &&  signalStatus) ||
	     ( (p_handler.trigger==TRIGGER_ON_HIGH) &&  !signalStatus) ) {
		// No need to notify this handler
		LOG4CPP_DEBUG(log, "Line [%s] not notified to [%s] on [%s]",
					signalTypeName[p_handler.signal],
					p_handler.name ? : "UNDEF",
					(p_handler.trigger==TRIGGER_ON_LOW) ? "HIGH" : "LOW"
					);
		return OK;
	}

	LOG4CPP_DEBUG(log, "Notify to [%s] line [%s] is [%s]",
					p_handler.name ? : "UNDEF",
					signalTypeName[p_handler.signal],
					signalStatus ? "HIGH" : "LOW"
					);

	p_handler.sh->signalNotify();

	return OK;

}

// NOTE battery input line is LOW when running on battery
exitCode DeviceSignals::checkInterrupts(t_signalMask & curStatus) {
	t_signalMask oldStatus;

	oldStatus = d_signalStatus;
	d_signalStatus = curStatus;

#define SIGNAL_CHANGED(_signal)	\
	( (oldStatus & (0x1<<_signal)) ^ (curStatus & (0x1<<_signal)) )

#define SIGNAL_VALUE(_signal) \
	( (d_signalStatus & (0x1<<_signal)) ? 1 : 0 )

//----- Checking DEVICE status
	if ( SIGNAL_CHANGED(SIGNAL_DEVICEOPEN) ) {
		LOG4CPP_INFO(log, "Device %s",
			SIGNAL_VALUE(SIGNAL_DEVICEOPEN) ? "OPENED" : "CLOSED" );
		notifyDeviceOpen(SIGNAL_VALUE(SIGNAL_DEVICEOPEN));
	}

//----- Checking OCG alarm
	if ( SIGNAL_CHANGED(SIGNAL_OCG) ) {
		LOG4CPP_INFO(log, "OCG Alarm %s",
			SIGNAL_VALUE(SIGNAL_OCG) ? "ON" : "OFF" );
	}

//----- Checking BATTERY status
	if ( SIGNAL_CHANGED(SIGNAL_BATTERY) ) {
		LOG4CPP_INFO(log, "Switched power to %s",
			SIGNAL_VALUE(SIGNAL_BATTERY) ? "BAT" : "AC" );
		notifyPower(SIGNAL_VALUE(SIGNAL_BATTERY));
	}

//----- Checking MMC status
	if (  SIGNAL_CHANGED(SIGNAL_MMC) ) {
		LOG4CPP_INFO(log, "MMC %s",
			SIGNAL_VALUE(SIGNAL_MMC) ? "INSERTED" : "REMOVED" );
	}

//----- Checking GPIO alarm
	if (  SIGNAL_CHANGED(SIGNAL_GPIO) ) {
		LOG4CPP_INFO(log, "GPIO Alarm %s",
			SIGNAL_VALUE(SIGNAL_GPIO) ? "ON" : "OFF" );
	}
	return OK;

}

exitCode DeviceSignals::updateTriggers(void) {
	exitCode result;

	// Reading current levels state
	result = updateSignalStatus(d_signalStatus);
	if ( result != OK ) {
		return result;
	}

	// Setting 'Trigger on LOW' for lines that are HIGH
	LOG4CPP_DEBUG(log, "Switch triggers on LOW for [0x%02X]", d_signalStatus);
	d_curTriggers.loSignals |=  d_signalStatus;
	d_curTriggers.hiSignals &= ~d_signalStatus;

	// Setting 'Trigger on HIGH' for lines that are LOW
	LOG4CPP_DEBUG(log, "Switch triggers on HIGH for [0x%02X]", ~d_signalStatus);
	d_curTriggers.loSignals &= d_signalStatus;
	d_curTriggers.hiSignals |= ~d_signalStatus;

	configureInterrupts();

	return OK;

}


void DeviceSignals::run(void)  {
	t_signalMask levels;
	t_hMap::iterator anH;
	t_signalMask curMask;
	t_signalMask newStatus;

	// Wait until at least one client is registered
	suspendWorker();	

	// FIXME this should be correctly initialized by the base class...
	// but without this the cycle is not entered
	while ( !d_doExit ) {

		// Waiting for at least one handler to be defined
		curMask = d_curTriggers.loSignals | d_curTriggers.hiSignals;
		if ( !curMask ) {
			LOG4CPP_DEBUG(log, "Waiting for at least one handler installed");
			suspendWorker();
		}

		// Updating interrupts configuration
		if (updateTriggers() != OK ) {
			LOG4CPP_WARN(log, "Failed updating interrupt configuration");
			LOG4CPP_DEBUG(log, "Waiting 5s before retrying... ");
			pollWorker(5000);
			continue;
		}

		// Waiting for next interrupt
		if (waitInterrupt(d_signalStatus) != OK) {
			LOG4CPP_DEBUG(log, "lost interrupt?");
			continue;
		}

	}

	return;

}


}// namespace device
}// namespace controlbox
