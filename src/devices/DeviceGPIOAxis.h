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


#ifndef _DEVICEGPIOAXIS_H
#define _DEVICEGPIOAXIS_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Device.h>
#include <controlbox/devices/DeviceGPIO.h>
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
class DeviceGPIOAxis : public DeviceGPIO {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

protected:

	enum gpioLine {
		GPIO_GPRS1_PWR = 0,
		GPIO_GPRS1_STATE,
		GPIO_GPRS1_RST,
		GPIO_GPRS2_PWR,
		GPIO_GPRS2_STATE,
		GPIO_GPRS2_RST,
		GPIO_OCG_RST,
		GPIO_TTYMUX_B0,
		GPIO_TTYMUX_B1,
	};
	typedef enum gpioLine t_gpioLine;

	static DeviceGPIOAxis * d_instance;

//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
public:

	/// Return an instance of a GPIO Device
	static DeviceGPIO * getInstance(std::string const & logName = "DeviceGPIO.Axis");

	/// Class destructor.
	~DeviceGPIOAxis();


	/// Reset the specified GPRS device
	exitCode gprsReset(unsigned short gprs);

	/// Get the power on/off state of the specified GPRS device
	bool gprsPowered(unsigned short gprs);


	/// Switch to the specified TTY multiplexed port
	exitCode ttySelect(unsigned short port);


	/// Switch to the specified ADC multiplexed port
	exitCode adcSelect(unsigned short port);


protected:

	/// Create a new DeviceGPIOAxis
	DeviceGPIOAxis(std::string const & logName);

	exitCode gprsSwitch(unsigned short gprs);


	exitCode gpioWrite(t_gpioLine gpio, t_gpioOperation op);

	t_gpioState gpioRead(t_gpioLine gpio);

};


} //namespace device
} //namespace controlbox
#endif

