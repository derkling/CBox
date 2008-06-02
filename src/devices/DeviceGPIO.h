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


#ifndef _DEVICEGPIO_H
#define _DEVICEGPIO_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/base/Device.h>
#include <cc++/thread.h>

namespace controlbox {
namespace device {

/// A DeviceGPIO is a CommandGenerator that...
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
class DeviceGPIO : public Device {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

public:

    /// Handled Messages
    enum cmdType {
	GPIO_EVENT = (DEVICE_GPIO*SERVICES_RANGE)+1,
    };
    typedef enum cmdType t_cmdType;

	enum gprsDevice {
		GPRS1 = 0,
		GPRS2 = 1,
	};
	typedef enum gprsDevice t_gprsDevice;

	enum ttySwitch {
		TTY_SINGLE = 0,		// Special value: not multiplexed Port
		TTY1_PORT0 = 1,
		TTY1_PORT1 = 3,
		TTY1_PORT2 = 5,
		TTY1_PORT3 = 7,
		TTY2_PORT0 = 9,
		TTY2_PORT1 = 11,
		TTY2_PORT2 = 13,
		TTY2_PORT3 = 15,
	};
	typedef enum ttySwitch t_ttySwitch;

	enum gpioState {
		GPIO_OFF = 0,
		GPIO_ON,
		GPIO_UNDEF,
	};
	typedef enum gpioState t_gpioState;

protected:

	enum gpioLine {
		GPIO_GPRS1_PWR = 0,
		GPIO_GPRS1_STATE,
		GPIO_GPRS1_RST,
		GPIO_GPRS2_PWR,
		GPIO_GPRS2_STATE,
		GPIO_GPRS2_RST,
		GPIO_TTY1_MUX1,
		GPIO_TTY1_MUX2,
		GPIO_TTY2_MUX1,
		GPIO_TTY2_MUX2,
	};
	typedef enum gpioLine t_gpioLine;

	enum ttyMuxDevice {
		TTY1 = 0,
		TTY2 = 1,
	};
	typedef enum ttyMuxDevice t_ttyMuxDevice;

	enum gpioDir {
		GPIO_OUT = 0,
		GPIO_IN,
	};
	typedef enum gpioDir t_gpioDir;

	enum gpioOperation {
		GPIO_SET,
		GPIO_CLEAR,
		GPIO_TOGGLE,
		GPIO_GET,
	};
	typedef enum gpioOperation t_gpioOperation;

	static DeviceGPIO * d_instance;

	/// The Configurator to use for getting configuration params
	Configurator & d_config;

	/// Multiplexed TTY mutex for exclusive access
	ost::Mutex d_ttyLock[2];

	/// The logger to use locally.
	log4cpp::Category & log;

//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
public:

    /// Return an instance of a GPIO Device
    static DeviceGPIO * getInstance(std::string const & logName = "DeviceGPIO");

    /// Class destructor.
    ~DeviceGPIO();

	/// Power on/off the specified GPRS device
	exitCode gprsPower(unsigned short gprs, t_gpioState state);
	/// Reset the specified GPRS device
	exitCode gprsReset(unsigned short gprs);
	/// Get the power on/off state of the specified GPRS device
	bool gprsIsPowered(unsigned short gprs);

	/// Lock the TTY mux for the specified port, and select this port
	exitCode ttyLock(unsigned short port);

	/// UnLock the TTY mux for the specified port
	exitCode ttyUnLock(unsigned short port);

	/// Switch to the specified TTY multiplexed port
	exitCode ttySelect(unsigned short port);

protected:

    /// Create a new DeviceGPIO.
    DeviceGPIO(std::string const & logName);

	exitCode gpioWrite(t_gpioOperation op, t_gpioLine gpio);
	t_gpioState gpioRead(t_gpioLine gpio);


};


} //namespace device
} //namespace controlbox
#endif

