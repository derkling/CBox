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


#ifndef _EVENTGENERATOR_H
#define _EVENTGENERATOR_H


#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/Generator.h>
#include <controlbox/base/comsys/Dispatcher.h>
#include <string>



namespace controlbox {
namespace comsys {

///And EventGenerator is a simple Generator that could be used to notify a Dispatcher about someting happening.
class EventGenerator : public Object, public Generator {

protected:

    Dispatcher * dispatcher;

    /// Set TRUE to enable the events notification to the
    /// associated Dispatcher
    bool enabled;

    /// Set TRUE if the associated Thread is running
    bool running;


public:

    /// Create a new EventGenerator initially disabled.
    /// In order to enable it you need to attach a Dispatcher.
    /// The new EventGenerator, as default logger category, has
    /// the class namespace "controlbox.comlibs.eg"
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    EventGenerator(std::string const & logName = "EventGenerator");

    /// Create a new EventGenerator associated to the specified Dispatcher
    /// The new EventGenerator, as default logger category, has
    /// the class namespace "controlbox.comlibs.eg"
    /// @param dispatcher the dispatcher to with send events notifications
    /// @param enabled set false if the generator has to be initially disabled (default true)
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    EventGenerator(Dispatcher * dispatcher, bool enabled = true,  std::string const & logName = "EventGenerator");

    /// Class destructor.
    /// @note this destructor ensume the associated thread will be terminated
    ~EventGenerator() {};

    /// Define the dispatcher to witch notify new events.
    /// By default, if not otherwise specified, the EventGenerator
    /// continue to be disabled
    exitCode setDispatcher(Dispatcher * dispatcher, bool enabled = false);

    /// Enable the notification of generated events.
    /// The first call to this method is in charge to start a new thread
    /// which code si defined by the Thread::run() code's implementation.
    /// @return OK if the Generator has been successfully enabled
    /// @return GEN_THREAD_STARTED if the Generator has been successfully
    ///		enabled and the corresponding thread has benn started
    /// @throw exceptions::InitializationException if the Generator couldn't be enabled because
    ///		it's not yet been defined a Dispatcher
    exitCode enable()
    throw(exceptions::InitializationException);

    /// Disable the notification of generated events
    /// Events generated while disabled are definitively lost
    exitCode disable();


protected:

    /// Notify the associated event dispatcher about a new event
    /// This is usually done with a call to
    /// Dispatcher::dispatch() or Dispatcher::dispatch(Command)
    exitCode notify();


};

} //namespace comsys
} //namespace controlbox
#endif
