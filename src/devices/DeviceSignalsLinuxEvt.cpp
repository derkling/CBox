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

#include "DeviceSignalsLinuxEvt.ih"

namespace controlbox {
namespace device {

DeviceSignalsLinuxEvt * DeviceSignalsLinuxEvt::d_instance = 0;

DeviceSignalsLinuxEvt::DeviceSignalsLinuxEvt(std::string const & logName) :
	DeviceSignals(logName) {


	d_eventsPath = d_config.param("Signal_input_event", DEFAULT_SIGNAL_INPUT_EVENT);

	// Opening input event interface
	if ((d_fdEvents = ::open(d_eventsPath.c_str(), O_RDONLY))<0) {
		LOG4CPP_ERROR(log, "failed to open input event interface [%s], %s\n", d_eventsPath.c_str(), strerror(errno));
		d_fdEvents = 0;
	}

	// Start the notification thread
	runWorker();

}



DeviceSignalsLinuxEvt::~DeviceSignalsLinuxEvt() {

	if (d_fdEvents)
		::close(d_fdEvents);

}

DeviceSignals * DeviceSignalsLinuxEvt::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceSignalsLinuxEvt(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceSignalsLinuxEvt::getInstance()");
	return d_instance;

}



// NOTE using boLines require to switch the interrupt configuration each times
//	the input change!!!... otehrwise we will get an endless interrupt storm!
exitCode DeviceSignalsLinuxEvt::configureInterrupts(void) {
	return OK;
}

exitCode DeviceSignalsLinuxEvt::updateSignalStatus(t_signalMask & status) {
	// No need to update on this impleentation: we already have the
	// updated state every time we receive an event.
	return OK;
}

exitCode DeviceSignalsLinuxEvt::waitInterrupt(t_signalMask & status) {
	fd_set set;
	int i, result;
	struct input_event event[2];

	if (!d_fdEvents) {
		LOG4CPP_ERROR(log, "input event file interface not initialized");
		return INT_DEV_FAILED;
	}

	LOG4CPP_DEBUG(log, "Waiting for an event...");

	FD_ZERO(&set);
	FD_SET(d_fdEvents, &set);
	result = ::select(d_fdEvents+1, &set, NULL, NULL, NULL);
	if ( result < 0) {
		// An EINTR is received every time an handler is added
		// this should not result in an warning
		LOG4CPP_DEBUG(log, "select failed, %s", strerror(errno));
		return INT_SELECT_FAILED;
	}

	// Reading input event
	for (i=0; i<2; i++) {
		result = ::read(d_fdEvents, &event[i], sizeof(struct input_event));
		if ( result<=0 ) {
			LOG4CPP_ERROR(log, "failed reading input event %d", i);
			return INT_SELECT_FAILED;
		}
	}

	LOG4CPP_DEBUG(log, "input event, time: %ld.%ld, type: 0x%X, code: 0x%X, value: %d",
			event[0].time.tv_sec, event[0].time.tv_usec,
			event[0].type, event[0].code, event[0].value);

	if ( event[0].value ) {
		status |= 0x1<<event[0].code;
	} else {
		status &= ~(0x1<<event[0].code);
	}


	LOG4CPP_DEBUG(log, "current lines status [0x%02X]", status);

	return OK;
}


}// namespace device
}// namespace controlbox
