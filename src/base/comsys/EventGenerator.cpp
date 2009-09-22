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
        d_dispatcher(0),
        d_enabled(false) {

	LOG4CPP_DEBUG(log, "EventGenerator::EventGenerator(std::string const & logName)");

	LOG4CPP_WARN(log, "EventGenerator created without an associated Dispatcher");

}


EventGenerator::EventGenerator(Dispatcher * dispatcher, bool enabled,  std::string const & logName) :
        Object("comlibs."+logName),
        Generator(),
        d_dispatcher(dispatcher),
        d_enabled(false) {

	LOG4CPP_DEBUG(log, "EventGenerator::EventGenerator(Dispatcher * dispatcher, bool enabled,  std::string const & logName)");

	if ( enabled )
		enable();

}


exitCode EventGenerator::setDispatcher(Dispatcher * dispatcher, bool enabled) {

	LOG4CPP_DEBUG(log, "EventGenerator::setDispatcher(Dispatcher * dispatcher, bool enabled)");

	d_dispatcher = dispatcher;

	if (enabled)
		enable();

	return OK;

}


exitCode EventGenerator::enable() {

	LOG4CPP_DEBUG(log, "EventGenerator::enable()");

	if ( !d_dispatcher ) {
		LOG4CPP_ERROR(log, "Trying to enable a generator without a linked Dispatcher");
		return CS_DISPATCH_FAILURE;
	}

	if (!d_enabled )
		d_enabled = true;

	return OK;
}

exitCode EventGenerator::disable() {

	LOG4CPP_DEBUG(log, "EventGenerator::disable()");

	d_enabled = false;
	return OK;

}


exitCode EventGenerator::notifyEvent(bool clean) {

	LOG4CPP_DEBUG(log, "EventGenerator::notifyEvent()");

	if (d_enabled) {
		LOG4CPP_INFO(log, "Event dispatching");
		d_dispatcher->dispatchEvent(clean);
		return OK;
	}

	LOG4CPP_WARN(log, "Lost event because the Generator is disabled");
	return GEN_NOT_ENABLED;

}

} //namespace comsys
} //namespace controlbox
