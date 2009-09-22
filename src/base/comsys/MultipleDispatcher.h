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


#ifndef _MULTIPLEDISPATCHER_H
#define _MULTIPLEDISPATCHER_H


#include <controlbox/base/comsys/Dispatcher.h>
#include <controlbox/base/Utility.h>

#include <list>


namespace controlbox {
namespace comsys {

/// Dispatch to multiple Handlers
/// This class allow to dispatch the same event/command to
/// multiple handlers by defining a list of dispatchers.
/// Each dispatcher must be correctly initialized and linked
/// to an handler of interest.
class MultipleDispatcher : public Dispatcher {

protected:
    std::list<Dispatcher *> dispatchers;

public:

    /// Build a new MultipleDispatcher
    MultipleDispatcher(std::string const & logName = "MultipleDispatcher");

    ~MultipleDispatcher();

    /// Attach a new Dispatcher
    exitCode addDispatcher(Dispatcher const & dispatcher);

    /// Remove a Dispatcher
    exitCode removeDispatcher(Dispatcher const & dispatcher);

    /// Get the number of dispatcher currently linked
    int handlersCount();

    /// Notify each associated handler about a new happening
    /// This is usually done with a call to
    /// Handler::notifyEvent() on each associated hendler
    exitCode dispatchEvent();

    /// Dispatch a command to each associated handler.
    /// If not suspended, dispatch the default command (or the specified one) to the associated Handler.
    /// Otherwise the command is queued for future dispatching.
    /// The default command, if needed, must be built by the subclasses's constructors.
    /// @param the Command to dispatch, if NULL the default command will be dispatched (default NULL)
    /// @return Core:OK on success, Core::DIS_SUSPENDED if the Command has been queued.
    exitCode dispatchCommand(Command * command);

};

} //namespace comsys
} //namespace controlbox
#endif
