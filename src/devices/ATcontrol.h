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


#ifndef _ATCONTROL_H
#define _ATCONTROL_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/base/Worker.h>
#include <controlbox/devices/DeviceInCabin.h>
#include<cc++/serial.h>

#define ATCONTROL_DEFAULT_DEVICE 	"/dev/ttyUSB0:9600:8:n:1"
#define ATCONTROL_SENTENCE_MAXLENGTH	256
#define ATCONTROL_SENTENCE_DELAY	1000


//-----[ AT Protocol Configuration ]--------------------------------------------
/// Default string terminator character to use
#define ATCONTROL_DEFAULT_DELIMITER	"\r"
/// By default don't echo received AT commands
#define ATCONTROL_DEFAULT_ECHOMODE	"NO"
/// By default don't send the AT command exit code (aka OK/ERROR)
#define	ATCONTROL_DEFAULT_EXITCODE	"NO"
/// Max number of chars in an AT command (not comprising the initial 'AT+')
#define ATCONTROL_COMMAND_MAXLENGTH	10





namespace controlbox {
namespace device {

/// Class defining a ATcontrol interface.
/// A ATcontrol allow to ...<br>
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>control_AT_tty_port</b> - <i>Default: ATCONTROL_DEFAULT_DEVICE</i><br>
///		The TTY port to use.
///		Using this param one can pass initial serial device parameters immediately
///		following the device name in a single string,
///		as in "/dev/tty3a:9600,7,e,1", as an example.<br><br>
///	</li>
///	<li>
///		<b>control_AT_delimiter</b> - <i>Default: ATCONTROL_DEFAULT_DELIMITER</i><br>
///		The delimiter character to use for AT commands<br>
///	</li>
///	<li>
///		<b>control_AT_tty_doEcho</b> - <i>Default: ATCONTROL_DEFAULT_ECHOMODE</i><br>
///		Whatever to echo received AT commands<br>
///	</li>
///	<li>
///		<b>control_AT_tty_sendExitCode</b> - <i>Default: ATCONTROL_DEFAULT_EXITCODE</i><br>
///		Whatewer to append OK/ERROR exit code as last responce to AT commands<br>
///	</li>
/// </ul>
/// @see Querible
class ATcontrol : public DeviceInCabin,
		public Worker {
//------------------------------------------------------------------------------
//				PUBLIC TYPES
//------------------------------------------------------------------------------
public:


//------------------------------------------------------------------------------
//				PRIVATE TYPES
//------------------------------------------------------------------------------
private:

//------------------------------------------------------------------------------
//				PRIVATE MEMBERS
//------------------------------------------------------------------------------
protected:

    static ATcontrol * d_instance;

    /// The Configurator to use for getting configuration params
    Configurator & d_configurator;

    /// The filepath of the input control file
    std::string d_controlAt;

    /// Whatever the parser is running
    bool d_isParsing;

    /// The file from witch receive AT commands
    ost::ttystream d_tty;

    /// The tty lines delimiter
    char d_delimiter;

    /// Whatever echos AT commands
    bool d_doEcho;

    /// Whatever send exitCode
    bool d_sendExitCode;

    /// The logger to use locally.
    log4cpp::Category & log;


//------------------------------------------------------------------------------
//				PUBLIC METHODS
//------------------------------------------------------------------------------
public:

    /// Get an instance of ATcontrol
    /// ATcontrol is a singleton class, this method provide
    /// a pointer to the (eventually just created) only one instance.
    static ATcontrol * getInstance();


    /// Default destructor
    ~ATcontrol();

    inline std::string name() const {
        return "ATcontrol";
    }



//------------------------------------------------------------------------------
//				PRIVATE METHODS
//------------------------------------------------------------------------------
protected:

    /// Create a new ATcontrol
    ATcontrol(std::string const & logName = "ATcontrol");

    inline void initATcontrol();

    inline exitCode doConfigure(void);

    /// Export local supported queries.
    /// @note DeviceInCabin export it's own query: here we should export
    ///	only others control queries.
    inline exitCode exportQuery();

    exitCode query(t_query & query);

    void doParse (void);

    Querible * parseQuery (std::string const & atCommand, Querible::t_query & theQuery);

    void run(void);

};


}// namespace device
}// namespace controlbox
#endif

