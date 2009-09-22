//******************************************************************************
//*************  Copyright (C) 2006 - Patrick Bellasi **************************
//******************************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** Patrick Bellasi. The programs may be used and/or copied only with the
//** written permission from the author or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//******************************************************************************
//******************** Module information **************************************
//**
//** Project:       ControlBox (0.1)
//** Description:   ModuleDescription
//**
//** Filename:      DeviceSignals.h
//** Owner:         Patrick Bellasi
//** Creation date:  21/06/2006
//**
//******************************************************************************
//******************** Revision history ****************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------- --------------------
//**
//**
//******************************************************************************

#ifndef _DEVICESIGNALSLINUX_H
#define _DEVICESIGNALSLINUX_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/devices/DeviceSignals.h>


#define DEFAULT_SIGNAL_INPUT_EVENT "/dev/input/event0"


namespace controlbox {
namespace device {

/// A DeviceSignals for AXIS platform.
class DeviceSignalsLinuxEvt : public DeviceSignals {

public:

protected:

    static DeviceSignalsLinuxEvt * d_instance;

    // The input events file path
    std::string d_eventsPath;

    // The input events file descriptor
    int d_fdEvents;

public:

    /// Get an object instance.
    /// @param logName the log category
    static DeviceSignals * getInstance(std::string const & logName = "DeviceSignals");

    ~DeviceSignalsLinuxEvt();

protected:

    DeviceSignalsLinuxEvt(std::string const & logName);

    /// Update the current value for each signal
    exitCode updateSignalStatus(t_signalMask & status);

    /// Update low-level interrupt configuration
    exitCode configureInterrupts(void);

    /// Wait for an interrupt to happens
    exitCode waitInterrupt(t_signalMask & status);

};

} //namespace device
} //namespace controlboxza
#endif
