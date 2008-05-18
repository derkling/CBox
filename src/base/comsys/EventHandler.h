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
//** Owner:         Patrick Bellasi (derkling@gmail.com)
//** Creation date:  21/06/2006
//**
//***************************************************************************************
//******************** Revision history *************************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------------------------- -----------------------------
//**          06/07/2006 Remembre to handle pointer on copyright
//**                     constructor (for Processo *)
//**
//***************************************************************************************


#ifndef _EVENTHANDLER_H
#define _EVENTHANDLER_H


#include <controlbox/base/comsys/Handler.h>

namespace controlbox {
namespace comsys {

/// A simple Handler.
/// An EventHandler is a simple handler that usually react to notifications by performing some simple tasks.
class EventHandler : public Object, public Handler {

protected:


public:

    /// Create a new EventProcessor with an associated logName.
    /// The new EventProcessor, as default logger category, has
    /// the class namespace "controlbox.comlibs.ep"<br>
    /// <b>N.B.<b> exported serivces range is initialized by that class
    ///	constructor such that any call to Handler::checkService will
    ///	fail thorwing an exception. Concrete subclasses MUST
    ///	inizialize those attribs with meaningful values in order
    ///	to correctly handle services exported.
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    EventHandler(std::string const & logName = "EventHandler");

    /// Class destructor
    ~EventHandler();


};

} //namespace comsys
} //namespace controlbox
#endif
