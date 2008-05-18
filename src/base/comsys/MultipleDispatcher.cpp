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


#include "MultipleDispatcher.ih"

namespace controlbox {
namespace comsys {

MultipleDispatcher::MultipleDispatcher(std::string const & logName) {}

MultipleDispatcher::~MultipleDispatcher() {
    // Remove all handlers
}

exitCode MultipleDispatcher::addDispatcher(Dispatcher const & dispatcher) {
}

exitCode MultipleDispatcher::removeDispatcher(Dispatcher const & dispatcher) {}

int MultipleDispatcher::handlersCount() {}

exitCode dispatch() {}

exitCode dispatch(Command * command) {}


} //namespace comsys
} //namespace controlbox
