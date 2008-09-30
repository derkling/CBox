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


#ifndef _POLLEVENTGENERATOR_H
#define _POLLEVENTGENERATOR_H


#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/EventGenerator.h>
#include <controlbox/base/Device.h>
#include <string>



namespace controlbox {
namespace device {

/// A PollEventGenerator is a simple EventGenerator that could be used to
/// generate polling events. Once constructed and initialized, that class
/// ciclically generate a new event every pollTime milliseconds.
/// This events are disapatched to the associated Disaptcher when the
/// class is in the enabled state: otherwise poll events are definitively
/// lost.
/// @see EventGenerator
class PollEventGenerator : public Device, public comsys::EventGenerator {

public:
    /// Generated Messages
    enum cmdType {
        SEND_POLL_DATA = (EG_POLLER*SERVICES_RANGE)+1,
    };
    typedef enum cmdType t_cmdType;

protected:

    timeout_t d_pollTime;

    /// The logger to use locally.
    log4cpp::Category & log;

public:

    /// Create a new PollEventGenerator initially disabled.
    /// In order to enable it you need to attach a Dispatcher.
    /// The new PollEventGenerator, as default logger category, has
    /// the class namespace "controlbox.comlibs.PollEventGenerator"
    /// @param pollTime the number of millisecond between each successive
    ///		polling events that will be generated (while not
    ///		suspended)
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    PollEventGenerator(timeout_t pollTime,
                       std::string const & logName = "PollEventGenerator");

    /// Create a new PollEventGenerator associated to the specified Dispatcher
    /// The new PollEventGenerator, as default logger category, has
    /// the class namespace "controlbox.comlibs.eg"
    /// @param pollTime the number of millisecond between each successive
    ///		polling events that will be generated (while not
    ///		suspended)
    /// @param dispatcher the dispatcher to with send events notifications
    /// @param enabled set false if the generator has to be initially disabled (default true)
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    PollEventGenerator(timeout_t pollTime, comsys::Dispatcher * dispatcher,
                       bool enabled = true,
                       std::string const & logName = "PollEventGenerator");

    /// Class destructor
    ~PollEventGenerator();

    /// Define the pollTime
    /// If the PollEventGenerator is already in enabled state, the pollTime
    /// change occours in reason of reset value: if reset is true
    /// we start immediatly with the new interval (the upcoming event is lost)
    /// otherwise, if reset is false, the pollTime change occur since the next
    /// cycle (the upcoming event is generated still with the old
    /// pollTime value)
    /// @param pollTime the number of millisecond between each successive
    ///		polling events that will be generated (while not
    ///		suspended)
    /// @param reset true if the pollTime change has to happen immediatly,
    ///		false if it start from the next cycle
    ///		<i>(default false)</i>
    /// @return OK on success
    exitCode setPollTime(timeout_t pollTime, bool reset = false);

    /// Return the current pollTime
    /// @return the current pollTime in milliseconds
    timeout_t pollTime() const;

    /// Generate a new event asyncronously
    void trigger();


protected:

    /// The thread cycle
    /// This method, hinerited from Thread class, it's a simple
    /// thread body that: sleep for pollTime milliseconds, awke sending
    /// a pollEvent (if enabled), and so on in an endless loop.
    void   run (void);

};

} //namespace device
} //namespace controlbox
#endif
