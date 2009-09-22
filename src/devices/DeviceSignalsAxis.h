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

#ifndef _DEVICESIGNALSAXIS_H
#define _DEVICESIGNALSAXIS_H

#include <pthread.h>
#include <signal.h>

#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/device/DeviceSignal.h>

#define DEVICESIGNALS_DEV_PATH			"/dev/gpioa"

#define DEVICESIGNALS_DEVICEOPEN_LINE		0
#define DEVICESIGNALS_ODOGPS_LINE		4
#define DEVICESIGNALS_BATTERY_LINE		5
#define DEVICESIGNALS_MMC_LINE			6
#define DEVICESIGNALS_GPIO_LINE			7

namespace controlbox {
namespace device {

/// A DeviceSignals for AXIS platform.
class DeviceSignalsAxis : public DeviceSignal {

public:

protected:

    static DeviceSignalsAxis * d_instance;

    /// The PortA device filepath.
    std::string d_devpath;

    /// The PortA device file handler.
    int fd_dev;


public:

    /// Get an object instance.
    /// @param logName the log category
    static DeviceSignals * getInstance(std::string const & logName = "DeviceSignals");

    ~DeviceSignals();

protected:
	
    /// Update the current value for each signal
    exitCode updateSignalStatus(t_signalMask & status);

    /// Update low-level interrupt configuration
    exitCode configureInterrupts(void);

    /// Wait for an interrupt to happens
    exitCode waitInterrupt(t_signalMask & status);

};

} //namespace device
} //namespace controlbox
#endif
