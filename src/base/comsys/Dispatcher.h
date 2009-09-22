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


#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/Handler.h>
#include <controlbox/base/comsys/Command.h>

namespace controlbox {
namespace comsys {

/// Dispatch happening notifications to Handlers
/// This interface, with reference to the Command Pattern, allow to issue requests to
/// objects without knowing anything about the operation being requested or the receiver of the request.
/// A Dispatcher bind a Generator to an Handler in a blind way. When a Generator throw a message,
/// the Dispatcher is in charge to notify the Handler about that happening.
/// A Dispatcher could be suspended, in such a state all messages coming from a Generator are
/// saved for delayed dispatching.
/// When a dispatcher is re-enabled we could decide if to dispatch all eventually queued
/// messages or otherwise to discard them.
class Dispatcher {

public:

    virtual ~Dispatcher() {};

    /// Define the Handler to witch dispatch new events and, if not
    /// otherwise specified, by default activate the dispatcher.
    virtual exitCode setHandler(Handler * handler, bool suspended = false) = 0;

    /// Suspend notifications
    /// New happenings are not dispetched but eventually delayed till reactivation.
    /// @return Core::OK
    virtual exitCode suspend() = 0;

    /// Enable dispatching
    /// Enable the dispatching of new events and, by default,
    /// dispatche all queued events. Otherwise queued events are flushed.
    /// @param discard set to true to discard queued messages while in suspended state (default flase)
    /// @return Core::OK on success, Core::GEN_NO_HANDLER if there's not handlers defined
    virtual exitCode resume(bool discard) = 0;

    /// Notify the associated handler about a new happening
    /// This is usually done with a call to
    /// Handler::notifyEvent()
    virtual exitCode dispatchEvent(bool clean = true) = 0;

    /// Dispatch a command to Handler.
    /// If not suspended, dispatch the default command (or the specified one) to the associated Handler.
    /// Otherwise the command is queued for future dispatching.
    /// The default command, if needed, must be built by the subclasses's constructors.
    /// @param the Command to dispatch, if NULL the default command will be dispatched (default NULL)
    /// @return Core:OK on success, Core::DIS_SUSPENDED if the Command has been queued.
    virtual exitCode dispatchCommand(Command * command, bool clean = true) = 0;

};

} //namespace comsys
} //namespace controlbox
#endif
