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


#include "CommandGenerator.ih"


namespace controlbox {
namespace comsys {


CommandGenerator::CommandGenerator(std::string const & logName, int pri) :
        EventGenerator(0, false, logName, pri) {

	LOG4CPP_DEBUG(log, "CommandGenerator::CommandGenerator(std::string const & logName)");

}


CommandGenerator::CommandGenerator(Dispatcher * dispatcher, bool enabled,  std::string const & logName, int pri) :
        EventGenerator(dispatcher, enabled, logName, pri) {

	LOG4CPP_DEBUG(log, "CommandGenerator::CommandGenerator(Dispatcher * dispatcher, bool enabled,  std::string const & logName)");

}

exitCode CommandGenerator::notify(Command * command, bool clean) {

	LOG4CPP_DEBUG(log, "CommandGenerator::notify()");

	if (d_enabled) {
		LOG4CPP_INFO(log, "Command dispatching");
		d_dispatcher->dispatch(command, clean);
		return OK;
	}

	LOG4CPP_WARN(log, "Command not dispatched because the Generator is disabled");
	return GEN_NOT_ENABLED;

}


} //namespace comsys
} //namespace controlbox
