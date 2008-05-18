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


#ifndef _GENERATOR_H
#define _GENERATOR_H

#include <controlbox/base/Object.h>
#include <cc++/thread.h>
#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/Dispatcher.h>


namespace controlbox {
namespace comsys {

/// A Generator is an object that could notify a Dispatcher about something happening.
/// The Generator could be enabled or disabled.
/// Events generated while the Generator is disabled are not notified and become definitively lost.
class Generator : protected ost::PosixThread {

public:

    virtual ~Generator() {};

    /// Define the dispatcher to witch notify new events.
    /// By default, if not otherwise specified, the Generator
    /// still remain disabled
    virtual exitCode setDispatcher(Dispatcher * dispatcher, bool enabled = false) = 0;

    /// Enable the notification of generated events.
    /// The first call to this method is in charge to start a new thread
    /// which code si defined by the Thread::run() code's implementation.
    /// @return OK if the Generator has been successfully enabled
    /// @return GEN_THREAD_STARTED if the Generator has been successfully
    ///		enabled and the corresponding thread has benn started
    /// @throw exceptions::InitializationException if the Generator couldn't be enabled because
    ///		it's not yet been defined a Dispatcher
    virtual exitCode enable()
    throw(exceptions::InitializationException) = 0;

    /// Disable the notification of generated events
    /// Events generated while disabled are definitively lost
    virtual exitCode disable() = 0;


protected:

    /// Notify the associated event dispatcher about a new event
    /// This is usually done with a call to
    /// Dispatcher::dispatch() or Dispatcher::dispatch(Command)
    virtual exitCode notify() = 0;

};

} //namespace comsys
} //namespace controlbox
#endif
