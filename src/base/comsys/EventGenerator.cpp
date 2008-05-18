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


#include "EventGenerator.ih"

namespace controlbox {
namespace comsys {


EventGenerator::EventGenerator(std::string const & logName) :
        Object("comlibs."+logName),
        Generator(),
        dispatcher(0),
        enabled(false),
        running(false) {

    LOG4CPP_DEBUG(log, "EventGenerator::EventGenerator(std::string const & logName)");

    LOG4CPP_WARN(log, "EventGenerator created without an associated Dispatcher");

}


EventGenerator::EventGenerator(Dispatcher * dispatcher, bool enabled,  std::string const & logName) :
        Object("comlibs."+logName),
        Generator(),
        dispatcher(dispatcher),
        enabled(false), // The enabled state MUST be changed only within the enable method
        running(false) {

    LOG4CPP_DEBUG(log, "EventGenerator::EventGenerator(Dispatcher * dispatcher, bool enabled,  std::string const & logName)");

    if (enabled) {
        enable();
    }

}


exitCode EventGenerator::setDispatcher(Dispatcher * dispatcher, bool enabled) {

    LOG4CPP_DEBUG(log, "EventGenerator::setDispatcher(Dispatcher * dispatcher, bool enabled)");

    this->dispatcher = dispatcher;

    if (enabled) {
        enable();
    }

    return OK;

}


exitCode EventGenerator::enable()
throw(exceptions::InitializationException) {

    LOG4CPP_DEBUG(log, "EventGenerator::enable()");

    if ( !dispatcher ) {
        LOG4CPP_ERROR(log, "Trying to enable a generator without a linked Dispatcher");
        throw new exceptions::InitializationException();
    }

    if (!enabled ) {
        enabled = true;
        //checking if the thread is already running
        if ( !running ) {
            // otherwise start the execution
            LOG4CPP_INFO(log, "Starting the Generator Thread execution");
            start();
            running = true;
            return GEN_THREAD_STARTED;
        }
    }

    return OK;
}

exitCode EventGenerator::disable() {

    LOG4CPP_DEBUG(log, "EventGenerator::disable()");

    enabled = false;
    return OK;

}


exitCode EventGenerator::notify() {

    LOG4CPP_DEBUG(log, "EventGenerator::notify()");

    if (enabled) {
        LOG4CPP_INFO(log, "Event dispatching");
        dispatcher->dispatch();
        return OK;
    } else {
        LOG4CPP_WARN(log, "Lost event because the Generator is disabled");
        return GEN_NOT_ENABLED;
    }

}


} //namespace comsys
} //namespace controlbox
