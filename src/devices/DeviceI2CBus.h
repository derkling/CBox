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

#ifndef _DEVICEI2C_H
#define _DEVICEI2C_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/base/Device.h>

// Mutex definitions
#include <cc++/thread.h>

// Native types definitions
#include <linux/types.h>

#define DEFAULT_BUS_I2C_DEVPATH		"/dev/i2c-0"

namespace controlbox {
namespace device {

/// A DeviceI2CBus.
class DeviceI2CBus : public Device {

public:

	typedef __u8 t_i2cCommand;

	typedef __u8 t_i2cReg;

protected:

	static DeviceI2CBus * d_instance;

	/// The Configurator to use for getting configuration params
	Configurator & d_config;

	/// The logger to use locally.
	log4cpp::Category & log;

	/// The I2C adapter device filepath
	std::string d_devpath;

	/// The I2C adapter device
	int fd_dev;

	/// The last configured device address
	int d_devAddr;

	/// Mutex for exclusive access to I2C bus
	ost::Mutex d_busLock;

public:

    /// Get an object instance.
    /// @param logName the log category
    static DeviceI2CBus * getInstance(std::string const & logName = "DeviceI2CBus");

    ~DeviceI2CBus();

    exitCode read(int addr, t_i2cCommand * cmd, short & cmdLen, t_i2cReg * reg, short regLen);


protected:

    /// Init I2C Bus device.
    exitCode initDevice();

    /// Create a new DeviceI2CBus.
    /// In order to get a valid instance of that class
    /// the builder method shuld be used.
    /// @param logName the log category.
    /// @see getInstance
    DeviceI2CBus(std::string const & logName);

    exitCode selectDevice(int addr);

};

} //namespace device
} //namespace controlbox
#endif
