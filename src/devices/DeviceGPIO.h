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
//** Description:   GPIO Interface
//**
//** Filename:      DeviceGPIO.h
//** Owner:         Patrick Bellasi
//** Creation date: 11/07/2009
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

/// A DeviceGPIO is an abstract interface for controlling some
/// specific board devices status.
//
/// This interface allows to control GPRS power state, TTY mux selection and
/// ADC mux selection.
///
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
	/// Handled Message
	enum cmdType {
		GPIO_EVENT = (controlbox::Device::DEVICE_GPIO*SERVICES_RANGE)+1,
	};
	typedef enum cmdType t_cmdType;


	enum gpioState {
		LOW = 0,
		HIGH,
		UNDEF,
	};
	typedef enum gpioState t_gpioState;

	enum gpioDir {
		DIR_OUT = 0,
		DIR_IN,
		DIR_UNDEF,
	};
	typedef enum gpioDir t_gpioDir;

	enum gpioOperation {
		SET,
		CLEAR,
		TOGGLE,
		GET,
	};
	typedef enum gpioOperation t_gpioOperation;

	enum gprsDevice {
		GPRS1 = 0,
		GPRS2,
	};
	typedef enum gprsDevice t_gprsDevice;


	// bit coding:
	// pssssx
	// p    => port (TTY1,TTY2)
	// ssss => channel [0-3]
	// x	=> mux flag (0 single, 1 muxed)
	enum ttySwitch {
		TTYMUX_SINGLE = 0,	// Special value: not multiplexed Port
		TTYMUX_PORT1,
		TTYMUX_PORT2,
		TTYMUX_PORT3,
		TTYMUX_PORT4,
	};
	typedef enum ttySwitch t_ttySwitch;

	/// Multiplexed TTY mutex for exclusive access
	ost::Mutex d_ttyLock;


	enum adcSwitch {
		ACDMUX_SINGLE = 0,	// Special value: not multiplexed Port
		ADCMUX_PORT1,
		ADCMUX_PORT2,
		ADCMUX_PORT3,
		ADCMUX_PORT4,
		ADCMUX_PORT5,
		ADCMUX_PORT6,
		ADCMUX_PORT7,
		ADCMUX_PORT8,
		ADCMUX_PORT9,
		ADCMUX_PORT10,
		ADCMUX_PORT11,
		ADCMUX_PORT12,
		ADCMUX_PORT13,
		ADCMUX_PORT14,
		ADCMUX_PORT15,
	};
	typedef enum adcSwitch t_adcSwitch;

	/// Multiplexed ADC mutex for exclusive access
	ost::Mutex d_adcLock;


	/// The Configurator to use for getting configuration params
	Configurator & d_config;

	/// The logger to use locally.
	log4cpp::Category & log;

//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
public:

	/// Class destructor.
	~DeviceGPIO(void);

	// Power On the specified GPRS device
	virtual exitCode gprsPowerOn(unsigned short gprs);

	// Power Off the specified GPRS device
	virtual exitCode gprsPowerOff(unsigned short gprs);

	/// Reset the specified GPRS device
	virtual exitCode gprsReset(unsigned short gprs) = 0;

	/// Get the power on/off state of the specified GPRS device
	virtual bool gprsPowered(unsigned short gprs) = 0;


	/// Lock the TTY mux for the specified port, and select this port
	virtual exitCode ttyLock(unsigned short port);

	/// UnLock the TTY mux for the specified port
	virtual exitCode ttyUnLock(unsigned short port);

	/// Switch to the specified TTY multiplexed port
	virtual exitCode ttySelect(unsigned short port) =0;


	/// Lock the ADC mux for the specified port, and select this port
	virtual exitCode adcLock(unsigned short port);

	/// UnLock the ADC mux for the specified port
	virtual exitCode adcUnLock(unsigned short port);

	/// Switch to the specified ADC multiplexed port
	virtual exitCode adcSelect(unsigned short port) = 0;

protected:

	/// Create a new DeviceGPIO
	DeviceGPIO(std::string const & logName);

	/// Switch the GPRS state
	virtual exitCode gprsSwitch(unsigned short gprs) = 0;

};

} //namespace device
} //namespace controlbox
#endif

