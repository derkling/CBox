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

#ifndef _DEVICESIGNALS_H
#define _DEVICESIGNALS_H

#include <pthread.h>
#include <signal.h>

#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/base/Worker.h>
#include <controlbox/base/comsys/CommandGenerator.h>
#include <controlbox/devices/DeviceTime.h>
#include <controlbox/base/comsys/Dispatcher.h>
#include <controlbox/devices/SignalHandler.h>

namespace controlbox {
namespace device {

/// A DeviceSignals.
class DeviceSignals : public comsys::CommandGenerator,
			public Device,
			public Worker {

public:

    /// Handled Messages
    enum cmdType {
	SYSTEM_EVENT = (DEVICE_SINGALS*SERVICES_RANGE)+1,
    };
    typedef enum cmdType t_cmdType;

    enum signalType {
	SIGNAL_DEVICEOPEN = 0,
	SIGNAL_OCG,
	SIGNAL_BATTERY,
	SIGNAL_MMC,
	SIGNAL_GPIO,
	SIGNAL_COUNT,	// This must be the lasry
    };
    typedef enum signalType t_signalType;

    /// Interrupt line names
    /// This vector should match t_signalType's entries
    static const char *signalTypeName[];

    enum signalTrigger {
	TRIGGER_ON_NONE = 0,	///> Disable interrupt on this line
	TRIGGER_ON_LOW,		///> Signal when the going low
	TRIGGER_ON_HIGH,	///> Signal when going high
	TRIGGER_ON_BOTH,	///> Signal level change
    };
    typedef enum signalTrigger t_signalTrigger;

    /// Interrupt levels names
    /// This vector should match t_interruptTrigger's entries
    static const char *signalTriggerName[];

    // A mask of signals (bits should match t_signalType)
    typedef unsigned short t_signalMask;

    // A mask of triggers.
    // For each signal define if a trigger is configured for HIGH or/and LOW
    // levels
    struct triggerMasks {
        t_signalMask loSignals;      ///> Low  triggered lines
        t_signalMask hiSignals;      ///> High triggered lines
    };
    typedef struct triggerMasks t_triggerMasks;

    struct handler {
	SignalHandler * sh;				///> the signel handler to notify
	t_signalMask signal;				///> the interrupt line of interest
	t_signalTrigger trigger;			///> the triggering level
#define DEVICESIGNALS_HANDLER_NAME_MAXLEN	16
	char name[DEVICESIGNALS_HANDLER_NAME_MAXLEN];	///> the name of this handler
    };
    typedef struct handler t_handler;

    ///> Maps interrupts to handlers to be notified about.
    typedef multimap<unsigned short, t_handler *> t_hMap;

    typedef pair<unsigned short, t_handler *> t_binding;

protected:

    /// The Configurator to use for getting configuration params
    Configurator & d_config;

    /// The Time Device to use
    DeviceTime * d_time;

    /// Maps interrupt lines to handlers
    t_hMap handlers;

    /// Current enabled triggers
    t_triggerMasks d_curTriggers;

    /// Current signal value: this is a bitmask where each bit represent
    ///  the state of the corresponding signal (bits should metch t_signal)
    t_signalMask d_signalStatus;

    /// Define TURE for the interrupt lines that are asserted
    bool d_signalPending[SIGNAL_COUNT];

    /// Number of interrupt handlers registered for each interrupt line
    unsigned short handlersCount[SIGNAL_COUNT];

    /// The logger to use locally.
    log4cpp::Category & log;

public:

    ~DeviceSignals();


    /// Register a signal handler.
    /// @param i the signal of interest (@see t_signalType)
    /// @param d the device to notify
    /// @param l trigger level
    /// @return OK on registration success
    exitCode registerHandler(unsigned short s, SignalHandler * sh, const char *name = 0, t_signalTrigger t = TRIGGER_ON_BOTH);

    /// Unsubscribe a signal handler.
    /// @param t the thread to deregister
    exitCode unregisterHandler(SignalHandler * sh);


    /// Return TRUE if the device is battery powered
    bool isBatteryPowered(void);


    exitCode notifyPowerOn() {
	    return notifyPower(true);
    };

    exitCode notifyPowerOff() {
	    return notifyPower(false);
    };

protected:

    /// Create a new DeviceSignals.
    /// In order to get a valid instance of that class
    /// the builder method shuld be used.
    /// @param logName the log category.
    /// @see getInstance
    DeviceSignals(std::string const & logName);
   
    /// Notify Power State
    exitCode notifyPower(bool on);

     /// Notify Device has been opened
    exitCode notifyDeviceOpen(bool open);
 
    /// Notify a thread with a signal.
    exitCode notifySignal(t_handler & handler, t_signalMask & signals);

    /// Update the triggers levels that must generate an interrupt
    exitCode updateTriggers(void);

    /// Verify interrupts locally
    exitCode checkInterrupts(t_signalMask & curStatus);

    /// Update the current value for each signal
    virtual exitCode updateSignalStatus(t_signalMask & status) = 0;

    /// Update low-level interrupt configuration
    virtual exitCode configureInterrupts(void) = 0;

    /// Wait for an interrupt to happens
    virtual exitCode waitInterrupt(t_signalMask & status) = 0;

    /// This method provide to configure the signal dispatcher and actually start it.
    void run(void);

};

} //namespace device
} //namespace controlbox
#endif

