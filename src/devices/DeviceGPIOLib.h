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
//** Filename:      Filename
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


#ifndef _DEVICEGPIOLIB_H
#define _DEVICEGPIOLIB_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Device.h>
#include <controlbox/devices/DeviceGPIO.h>
#include <sysfs/libsysfs.h>
#include <cc++/thread.h>

namespace controlbox {
namespace device {

/// A DeviceGPIOAxis is a CommandGenerator that...
/// blha blha blha<br>
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///	</li>
///	<li>
///	</li>
/// </ul>
/// @see CommandHandler
class DeviceGPIOLib : public DeviceGPIO {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

protected:

	static DeviceGPIOLib * d_instance;

	// The sysfs mount path
	char d_mntPath[SYSFS_PATH_MAX];

	// NOTE: last entry should always be ATTR_COUNT
	enum attrType {
		GPRS_PWR,
		GPRS_RST,
		GPRS_IO,
		TTY_MUX0,
		TTY_MUX1,
		ADC_MUX0,
		ADC_MUX1,
		ADC_MUX2,
		ADC_MUX3,
		ATTR_COUNT,
	};
	typedef enum attrType t_attrType;

	struct attrData {
		const char *name;
		const char *descr;
#define	ATTR_DIR	0x01
#define ATTR_VAL	0x02
		unsigned char mask;
		struct sysfs_attribute *dir;
		struct sysfs_attribute *val;
	};
	typedef struct attrData t_attrData;

	// The sysfs attributes (direction and value for each type)
	// NOTE: the content should match t_attrType
	static t_attrData d_attr[ATTR_COUNT];


//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
public:

	/// Return an instance of a GPIO Device
	static DeviceGPIO * getInstance(std::string const & logName = "DeviceGPIO.Lib");

	/// Class destructor.
	~DeviceGPIOLib();


	/// Reset the specified GPRS device
	exitCode gprsReset(unsigned short gprs);

	/// Get the power on/off state of the specified GPRS device
	bool gprsPowered(unsigned short gprs);


	/// Switch to the specified TTY multiplexed port
	exitCode ttySelect(unsigned short port);


	/// Switch to the specified ADC multiplexed port
	exitCode adcSelect(unsigned short port);


protected:

	/// Create a new DeviceGPIOLib.
	DeviceGPIOLib(std::string const & logName);

	exitCode gprsSwitch(unsigned short gprs);


	exitCode getGpioDir(t_attrType type, DeviceGPIO::t_gpioDir & dir);

	exitCode setGpioDir(t_attrType type, DeviceGPIO::t_gpioDir dir = DIR_OUT);

	exitCode getGpioVal(t_attrType type, int & val);

	exitCode setGpioVal(t_attrType type, int val);

};


} //namespace device
} //namespace controlbox
#endif

