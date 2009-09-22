//***************************************************************************************
//*************  Copyright (C) 2006 - Patrick Bellasi ***********************************
//***************************************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** Patrick Bellasi. The programs may be used and/or copied only with the
//** written permission from the author or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//***************************************************************************************
//******************** Module information ***********************************************
//**
//** Project:       ControlBox (0.1)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Patrick Bellasi
//** Creation date:  21/06/2006
//**
//***************************************************************************************
//******************** Revision history *************************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------------------------- -----------------------------
//**
//**
//***************************************************************************************


#include "PollEventGenerator.ih"

namespace controlbox {
namespace device {


PollEventGenerator::PollEventGenerator(timeout_t p_pollTime, std::string const & logName) :
        Device(Device::EG_POLLER, p_pollTime, logName),
	Worker(Device::log, "cbw_POL", 0),
        comsys::EventGenerator(logName),
        d_pollTime(p_pollTime),
        log(Device::log) {

    LOG4CPP_DEBUG(log, "PollEventGenerator(unsigned int, std::string const &)");

    LOG4CPP_WARN(log, "Created a PollEventGenerator without an associated Dispatcher");

    // Registering device into the DeviceDB
    dbReg(true);

}


PollEventGenerator::PollEventGenerator(timeout_t p_pollTime, comsys::Dispatcher * dispatcher,
                                       bool enabled,
                                       std::string const & logName) :
        Device(Device::EG_POLLER, p_pollTime, logName),
	Worker(Device::log, "cbw_POL", 0),
        // NOTE we must ensure the pollTime is set correctly before starting the thread (if enabled)
        comsys::EventGenerator(dispatcher, false, logName),
        d_pollTime(p_pollTime),
        log(Device::log) {

    LOG4CPP_DEBUG(log, "PollEventGenerator(unsigned int, Dispatcher *, bool, string const &)");

    // Registering device into the DeviceDB
    dbReg(true);

    if ( enabled ) {
	this-enable();
    }

}


PollEventGenerator::~PollEventGenerator() {

    LOG4CPP_INFO(log, "Terminating the PollEventGenerator events generation thread... ");

    // Thread::terminate() should always be called at the start of any
    // destructor of a class derived from Thread to assure the remaining
    // part of the destructor is called without the thread still executing.
    this->terminate();

}


exitCode PollEventGenerator::setPollTime(timeout_t p_pollTime, bool reset) {

    LOG4CPP_DEBUG(log, "PollEventGenerator::setPollTime(timeout_t, bool");

    d_pollTime = p_pollTime;

    return OK;

}


timeout_t PollEventGenerator::pollTime() const {

    LOG4CPP_DEBUG(log, "PollEventGenerator::pollTime()");

    return d_pollTime;

}

void PollEventGenerator::trigger() {
	LOG4CPP_DEBUG(log, "Async Polling");
	notify(false);
}

void   PollEventGenerator::run (void) {

	pollWorker(d_pollTime);
	
	while ( !d_doExit ) {

        	LOG4CPP_DEBUG(log, "Polling");
		notify(false);

		LOG4CPP_DEBUG(log, "Next poll within %d ms", d_pollTime);
		pollWorker(d_pollTime);
	}

}


} //namespace device
} //namespace controlbox
