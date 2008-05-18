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
//** Description:   Enfora GPRS device back-end
//**
//** Filename:      Enfora-h
//** Owner:         Patrick Bellasi
//** Creation date:  28/07/2007
//**
//******************************************************************************
//******************** Revision history ****************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------- --------------------
//**
//**
//******************************************************************************


#ifndef _ENFORA_H
#define _ENFORA_H

#include <controlbox/devices/gprs/DeviceGPRS.h>
#include <controlbox/base/Utility.h>

namespace controlbox {
namespace device {

/// A Enfora is a DeviceGPRS for Enfora's GPRS devices management.
/// This class provide access to Enfora's GSM/GPRS devices connected to
/// a TTY (serial) port. The class provide initialization
/// code and methods to tear up a GPRS connection and accessing
/// to others supported features as: modem state query, send SMS,
/// etc. <br>
/// Supported models are:
/// <ul>
///	<li>Enfora GPRS module &lt;module number&gt;</li>
/// </ul>
/// <br>
/// <h5>Configuration params availables for this class:</h5>
///		Since we could use more than one GPRS module, in the following params [N] must be
///		a single digit number in the range [0-9].
/// <ul>
///	<li>
///	</li>
/// </ul>
/// <br>
/// <i>Notes on GPRS support</i>
/// <ul>
///	<li>
///	AT+CGDCONT=1,”IP”,””,””,0,0 may be entered for networks that dynamically
///	assign the APN. Contact your service provider for correct APN information.
///	</li>
///	<li>
///
///	</li>
///	<li>
///
///	</li>
///	<li>
///
///	</li>
///	<li>
///
///	</li>
/// <ul>
/// @see DeviceGPRS
class Enfora : public DeviceGPRS  {

public:

protected:

public:

    /// Create a new Enfora GPRS device initially disabled.
    /// In order to enable it you need to attach a Dispatcher.
    /// The new GPRS device, as default logger category, has
    /// the class namespace "controlbox.comlibs.Enfora"
    /// @param module the module number, used to look for configuration
    ///		params. It <b>must be<b> a single digit number in the
    ///		range [0-9]
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    /// @throw SerialConfigurationException on failing accessing the TTY port
    Enfora(short module, std::string const & logName = "Enfora")
    	throw (exceptions::SerialConfigurationException*);

    /// Class destructor.
    ~Enfora();

    /// Setup the TTY port.
    /// Setup a tty port with the (configuration) specified
    /// params.
    /// @return OK on success, the modem has been switched
    /// into Data Mode,
    /// GPRS_TTY_OPEN_FAILURE on tty open failure
    /// @note once configured, the tty is closed.
    exitCode initDeviceGPRS();

protected:

    /// Switch off Data Mode.
    /// If a switch from data mode to command mode is supported by the
    /// modem device, provide to switch to the required mode.
    /// @param forse set true to force a command mode switch
    /// (by terminating pppd)
    /// @return OK on switch success
    /// @note this method suppose that Data Mode is the current one
    exitCode switchToCommandMode (ttymodem & modem, bool force = false);

    /// Switch-back to Data Mode.
    /// If a switch from command mode to data mode is supported by the
    /// modem device, provide to switch to the required mode.
    /// @return OK on switch success
    /// @note this method suppose that Command Mode is the current one
    exitCode switchToDataMode (ttymodem & modem);

    /// Notify friend server about IP change.
    exitCode notifyFriends();

};

}// namespace gprs
}// namespace controlbox
#endif
