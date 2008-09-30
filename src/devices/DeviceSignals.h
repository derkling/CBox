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
#include <controlbox/devices/DeviceTime.h>
#include <controlbox/base/comsys/CommandGenerator.h>
#include <controlbox/base/comsys/Dispatcher.h>
#include <controlbox/devices/SignalHandler.h>

#define DEVICESIGNALS_HANDLER_NAME_MAXLEN	16
#define DEVICESIGNALS_DEV_PATH			"/dev/gpioa"

#define DEVICESIGNALS_DEVICEOPEN_LINE		0
#define DEVICESIGNALS_ODOGPS_LINE		4
#define DEVICESIGNALS_BATTERY_LINE		5
#define DEVICESIGNALS_MMC_LINE			6
#define DEVICESIGNALS_GPIO_LINE			7

namespace controlbox {
namespace device {

/// A DeviceSignals.
class DeviceSignals : public Device, public comsys::CommandGenerator {

public:

    /// Handled Messages
    enum cmdType {
	SYSTEM_EVENT = (DEVICE_SINGALS*SERVICES_RANGE)+1,
    };
    typedef enum cmdType t_cmdType;

    enum interrupt {
    	INT_DEVICEOPEN = 0,
    	INT_ODOGPS,
    	INT_BATTERY,
    	INT_MMC,
    	INT_GPIO,
    	INT_COUNT	// This must be the last entry
    };
    typedef enum interrupt t_interrupt;

    /// Interrupt line names
    /// This vector should match t_interrupt's entries
    static const char *intrName[];

    typedef unsigned long t_intMask;

    enum interruptTrigger {
    	INTERRUPT_NONE=0,		///> Disable interrupt on this line
	INTERRUPT_ON_LOW,		///> Signal when the going low
	INTERRUPT_ON_HIGH,		///> Signal when going high
	INTERRUPT_ON_BOTH		///> Signal level change
    };
    typedef enum interruptTrigger t_interruptTrigger;

    struct handler {
	SignalHandler * sh;				///> the signel handler to notify
	t_intMask line;					///> the interrupt line of interest
	t_interruptTrigger level;			///> the triggering level
	char name[DEVICESIGNALS_HANDLER_NAME_MAXLEN];	///> the name of this handler
    };
    typedef struct handler t_handler;

    ///> Maps interrupts to handlers to be notified about.
    typedef multimap<unsigned short, t_handler *> t_hMap;

    typedef pair<unsigned short, t_handler *> t_binding;


    struct masks {
	t_intMask loLines;	///> Low only triggered lines
	t_intMask hiLines;	///> High only triggered lines
	t_intMask boLines;	///> Both only triggered lines
    };
    typedef struct masks t_masks;

protected:

    static DeviceSignals * d_instance;

    /// The Configurator to use for getting configuration params
    Configurator & d_config;

    /// The Time Device to use
    DeviceTime * d_time;

    /// The PortA device filepath.
    std::string d_devpath;

    /// The PortA device file handler.
    int fd_dev;

    /// Maps interrupt lines to handlers
    t_hMap handlers;

    /// Current enabled interrupt mask
    t_masks d_curMask;

    /// Last readed input levels
    t_intMask d_levels;

    /// Mask of bits Chenged since last reading
    t_intMask d_changedBits;

    /// True if LIGHT alarm is asserted
    bool d_isDeviceOpen;

    /// True if ODO-GPS alarm is asserted
    bool d_isOdoGpsAlarm;

    /// True if running on battery
    bool d_isBatteryPowered;

    /// True if MMC is present
    bool d_isMmcPresent;

    /// True if DS alarm is asserted
    bool d_isGpioAlarm;

    unsigned short handlersNumber[INT_COUNT];

    /// The logger to use locally.
    log4cpp::Category & log;

public:

    /// Get an object instance.
    /// @param logName the log category
    static DeviceSignals * getInstance(std::string const & logName = "DeviceSignals");

    ~DeviceSignals();

    /// Register a signal handler.
    /// @param i the interrupt of interest (@see t_interrupt)
    /// @param d the device to notify
    /// @param l trigger level
    /// @return OK on registration success
    exitCode registerHandler(unsigned short i, SignalHandler * sh, const char *name = 0, t_interruptTrigger l = INTERRUPT_ON_BOTH);

    /// Unsubscribe a signal handler.
    /// @param t the thread to deregister
    exitCode unregisterHandler(SignalHandler * sh);

    /// Return TRUE if the device is battery powered
    bool isBatteryPowered(void);

    /// Notify Power On Status
    exitCode powerOn(bool on = true);


protected:

    /// Init signal device.
    exitCode initSignals();

    /// Create a new DeviceSignals.
    /// In order to get a valid instance of that class
    /// the builder method shuld be used.
    /// @param logName the log category.
    /// @see getInstance
    DeviceSignals(std::string const & logName);

    /// Return current input levels
    exitCode getLevels(t_intMask & levels);

    /// Configure lines to generata an interrupt on the specified level.
    /// @param bitmask of lines to configure
    /// @param onLow set true to generate an interrupt when lines go low
    exitCode confInterrupt(t_intMask lines, bool onLow = true);

    /// Reconfigure interrupt lines
    exitCode updateInterrupts(void);

    exitCode waitForIntr(t_intMask & levels);

    exitCode checkInterrupts(t_intMask & levels);

    /// Notify a thread with a signal.
    exitCode notifySignal(t_handler & handler, t_intMask & levels);

    /// Thread body.
    /// This method provide to configure the signal dispatcher and actually start it.
    void run(void);

};

} //namespace device
} //namespace controlbox
#endif
