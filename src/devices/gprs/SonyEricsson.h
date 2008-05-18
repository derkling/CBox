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


#ifndef _SONYERICSSON_H
#define _SONYERICSSON_H



#include <controlbox/devices/gprs/DeviceGPRS.h>
#include <controlbox/base/Utility.h>


namespace controlbox {
namespace device {

/// A SonyEricsson is a DeviceGPRS for SonyEricsson's GPRS devices management.
/// This class provide access to SonyEricsson's GSM/GPRS devices connected to
/// a TTY (serial) port. The class provide initialization
/// code and methods to tear up a GPRS connection and accessing
/// to others supported features as: modem state query, send SMS,
/// etc. <br>
/// Supported models are:
/// <ul>
///	<li>SonyEricsson GPRS module &lt;module number&gt;</li>
/// </ul>
/// <br>
/// <h5>Configuration params availables for this class:</h5>
///		Since we could use more than one GPRS module, in the following params [N] must be
///		a single digit number in the range [0-9].
/// <ul>
///	<li>
///	</li>
/// </ul>
/// @see DeviceGPRS
class SonyEricsson : public DeviceGPRS  {


public:



protected:



public:

    /// Create a new SonyEricsson GPRS device initially disabled.
    /// In order to enable it you need to attach a Dispatcher.
    /// The new GPRS device, as default logger category, has
    /// the class namespace "controlbox.comlibs.SonyEricsson"
    /// @param module the module number, used to look for configuration
    ///		params. It <b>must be<b> a single digit number in the
    ///		range [0-9]
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    SonyEricsson(short module, std::string const & logName = "SonyEricsson");


    /// Class destructor.
    ~SonyEricsson();


    /// Setup the TTY port.
    /// Setup a tty port with the (configuration) specified
    /// params.
    /// @return OK on success, the modem has been switched
    ///	into Data Mode,
    ///	GPRS_TTY_OPEN_FAILURE on tty open failure
    /// @note once configured, the tty is closed.
    exitCode initDeviceGPRS();

protected:

    /// Switch off Data Mode.
    /// If a switch from data mode to command mode is supported by the
    /// modem device, provide to switch to the required mode.
    /// @param forse set true to force a command mode switch
    ///	(by terminating pppd)
    /// @return OK on switch success
    /// @note this method suppose that Data Mode is the current one
    exitCode switchToCommandMode (ttymodem & modem, bool force = false);

    /// Switch-back to Data Mode.
    /// If a switch from command mode to data mode is supported by the
    /// modem device, provide to switch to the required mode.
    /// @return OK on switch success
    /// @note this method suppose that Command Mode is the current one
    exitCode switchToDataMode (ttymodem & modem);



};

}// namespace gprs
}// namespace controlbox
#endif
