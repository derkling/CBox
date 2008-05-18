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


#include "SonyEricsson.ih"

namespace controlbox {
namespace device {



SonyEricsson::SonyEricsson(short module, std::string const & logName) :
        DeviceGPRS(module, DeviceGPRS::DEVICEGPRS_MODEL_SONYERICSSON, logName) {

    LOG4CPP_DEBUG(log, "SonyEricsson(std::string const &)");

    // Module specific initialization
    initDeviceGPRS();

}



SonyEricsson::~SonyEricsson() {

    LOG4CPP_DEBUG(log, "Destroying the SonyEricsson module [%d]", d_module);

}



exitCode SonyEricsson::initDeviceGPRS() {
    ost::ttystream tty;
    std::string ttyConfigRequired("");

    LOG4CPP_DEBUG(log, "SonyEricsson::initDeviceGPRS()");

    // Building the TTY port DEFAULT configuration string
    d_ttyConfig += d_config.param(paramName("tty_port_default"), DEVICEGPRS_DEFAULT_GPRS_DEVICE);

    LOG4CPP_DEBUG(log, "Current module default TTY port configuration: %s", d_ttyConfig.c_str());

    // Opening the port with default configuration
    tty.open(d_ttyConfig.c_str());
    if ( ! tty ) {
        LOG4CPP_FATAL(log, "Unable to open TTY port [%s]", d_ttyConfig.c_str());
        return GPRS_TTY_OPEN_FAILURE;
    }

    //TODO: Checking device model using AT+GMM

    //TODO: Load the saved profile (storing our preferences)
    //	This method shuld also provide to program and save a profile
    //	if its the first time we use the modem...


    //TODO: Disabling echo ATE 0
    // NOTE: this seem to doesn't work



    // Building the TTY port REQUESTED configuration string
    ttyConfigRequired = d_config.param(paramName("tty_port_required"), "");

    // Performing the module specific port configuration
    // TODO: better control on tty required copnfiguration validity
    //	(could be possible to specify only some fields?!?)
    if ( d_config.param(paramName("tty_speed")).size() &&  d_ttyConfig != ttyConfigRequired ) {
        LOG4CPP_INFO(log, "Switch to the requested module TTY port configuration: %s", ttyConfigRequired.c_str());
        // TODO: remove this and implement following code
        return OK;

        d_ttyConfig = ttyConfigRequired;


    }

    return OK;

}

exitCode SonyEricsson::switchToCommandMode (ttymodem & modem, bool force) {
    t_stringVector lines;

    switch ( d_mode ) {

    case DeviceGPRS::DEVICEGPRS_MODE_DATA:

        // Preparing for switch-less ATcommand
        d_mode = DEVICEGPRS_MODE_COMMAND;
        // Sending the escape sequence without reaading output
        modem.sendAT("+++", "OK", 0, 1, false);
        // Wait 2s of silent time
        //sleep(2000);
        // Readming modem output
        //modem.readFromTTY("OK", 1, 0);


        return OK;

    default:
        LOG4CPP_WARN(log, "Switch to Command Mode NOT supportd from current mode [%d]", d_mode);
    }

    return GPRS_AT_NOT_SUPPORTED;


}


// By default Switch-back from Command Mode to Data Mode is not supported
exitCode SonyEricsson::switchToDataMode (ttymodem & modem) {

    // Switching required.
    LOG4CPP_DEBUG(log, "Switching module [%d] to Data Mode... ", d_module);



    return OK;

}


}// namespace gprs
}// namespace controlbox
