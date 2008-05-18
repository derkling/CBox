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
//** Filename:      Enfora.cpp
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


#include "Enfora.ih"

namespace controlbox {
namespace device {



Enfora::Enfora(short module, std::string const & logName)
	throw (exceptions::SerialConfigurationException*) :
        DeviceGPRS(module, DeviceGPRS::DEVICEGPRS_MODEL_ENFORA, logName) {
	exitCode error;

    LOG4CPP_DEBUG(log, "Enfora(std::string const &)");

    // Module specific initialization
    error = initDeviceGPRS();
    switch (error) {
    case OK:
    	return;
    case GPRS_TTY_OPEN_FAILURE:
    	LOG4CPP_FATAL(log, "Failed to build the GPRS device");
    	cleanUp();
    	throw new exceptions::SerialConfigurationException("Unable to open TTY port");
    }

}

Enfora::~Enfora() {

    LOG4CPP_DEBUG(log, "Destroying the Enfora module [%d]", d_module);

}



exitCode Enfora::initDeviceGPRS() {
    ttymodem modem((*this));	// The TTY port stream
    std::string ttyConfigRequired("");
    exitCode result;
    short retry;
    t_stringVector lresp;

    LOG4CPP_DEBUG(log, "Enfora::initDeviceGPRS()");

//     // Loading the TTY port configuration string
//     d_ttyConfig += d_config.param(paramName("tty"), DEVICEGPRS_DEFAULT_GPRS_DEVICE);

    LOG4CPP_DEBUG(log, "Current module TTY port configuration: %s", d_ttyConfig.c_str());

    // Opening the TTY port
    modem.open(d_ttyConfig.c_str());
    if ( ! modem ) {
        LOG4CPP_FATAL(log, "Unable to open TTY port [%s]", d_ttyConfig.c_str());
        modem.close();
        return GPRS_TTY_OPEN_FAILURE;
    }

    // Restore factory defaults
    retry = 2;
    do {
    	result = modem.sendAT("ATZ");
    } while ( (result != OK) && retry--);
    switch (result) {
    case OK:
    	break;
    default:
    	LOG4CPP_ERROR(log, "Modem not responding, initialization aborted");
    	modem.close();
    	return GPRS_TTY_MODEM_NOT_RESPONDING;
    }

    // Initialization string (if provided by configuration)
    if ( d_atInitString.length() ) {
    	modem.sendAT(d_atInitString.c_str());
    }





    //TODO Checking device model using AT+GMM

    //TODO Load the saved profile (storing our preferences)
    //  This method shuld also provide to program and save a profile
    //  if its the first time we use the modem...

    //TODO Disabling echo ATE 0
    //NOTE this seem to doesn't work



    // reading device ids
    getDeviceIds();

    modem.close();
    return OK;

}

exitCode Enfora::notifyFriends() {
    static const std::string * d_prevNetlinkName = 0;
    unsigned short count;
    std::ostringstream cmd("");
    ttymodem modem((*this));	// The TTY port stream

    // Notification configuration should be done only on netlink changing
    if ( d_prevNetlinkName == d_curNetlinkName ) {
    	LOG4CPP_DEBUG(log, "Netlink not changed, friends notify configuration don't need to be updated");
    	return OK;
    }

    d_prevNetlinkName = d_curNetlinkName;
    
    LOG4CPP_INFO(log, "Updating friend's server keep-alive configuration");

    // Configuring firend server
    cmd << "AT$MDMID=" << d_simNumber;
    if ( modem.sendAT(cmd.str(), "OK") != OK ) {
    	LOG4CPP_ERROR(log, "Configuring friend server [%s] notification failed",
    	d_curNetlinkConf->lserv[count-1].c_str());
    }
    
    count = d_curNetlinkConf->lserv.size();
    count = (count>10) ? 10 : count; //Enfora support up to 10 firends only
    while ( count ) {
    	LOG4CPP_DEBUG(log, "Notifier added for friend server [%s]",
    				d_curNetlinkConf->lserv[count-1].c_str());
    	//TODO add AT commands to send notifications

	cmd.flush();
    	cmd << "AT$FRIEND=" << count << ",1," << d_curNetlinkConf->lserv[count-1];
    	if ( modem.sendAT(cmd.str(), "OK") != OK ) {
    		LOG4CPP_ERROR(log, "Configuring friend server [%s] notification failed",
    					d_curNetlinkConf->lserv[count-1].c_str());
    	}
    	count--;
    }

}


exitCode Enfora::switchToCommandMode (ttymodem & modem, bool force) {
    t_stringVector lines;
    exitCode result;

    //NOTE The escape sequence requires a guard period of 1 second before
    //	and after entering +++. Other wise the +++ will be considered data
    //	and forwarded as data.

    switch ( d_mode ) {

    case DeviceGPRS::DEVICEGPRS_MODE_DATA:
    
    	LOG4CPP_DEBUG(log, "Switching to command mode...");
	
	// Set CommandMode here to avoid looping on switchToCommandMode calls
        d_mode = DEVICEGPRS_MODE_COMMAND;

        // Wait for 1s silence in order to grant the reception of escape sequence
        // We suspend the pppd process in order to ensure the 1s silence
        result = suspendPppDaemon();
        switch (result) {
        case OK:
        	break;
        default:
        	d_mode = DEVICEGPRS_MODE_COMMAND;
        	return result;
        }



	LOG4CPP_DEBUG(log, "STOP ME NOW");
	sleep(10000);





        // Sending the escape sequence WITHOUT reading output
        sleep(1000);
        result = modem.sendAT("+++", "OK", 0, 0, 200, false);
        sleep(1000);

        // NOW we could read the modem output
        result =  modem.readFromTTY("OK", 2000, 3, 0);
        switch (result) {
        case OK:
        	break;
        default:
        	LOG4CPP_ERROR(log, "Escape sequence failed");
        	d_mode = DEVICEGPRS_MODE_DATA;
        	return GPRS_AT_ESCAPE_FAILED;
        }

        return OK;
    }

    LOG4CPP_WARN(log, "Switch to Command Mode NOT supportd from current mode [%d]", d_mode);
    return GPRS_AT_NOT_SUPPORTED;

}

exitCode Enfora::switchToDataMode (ttymodem & modem) {
    exitCode result;


    switch ( d_mode ) {

    case DeviceGPRS::DEVICEGPRS_MODE_COMMAND:

        d_mode = DEVICEGPRS_MODE_DATA;

        // Sending the escape sequence without reading output
        result = modem.sendAT("ATO", "OK", 0, 1, 1, false);
        switch (result) {
        case OK:
        	break;
        default:
        	LOG4CPP_ERROR(log, "Escape sequence failed");
        	d_mode = DEVICEGPRS_MODE_COMMAND;
        	return GPRS_AT_ESCAPE_FAILED;
        }

        result = resumePppDaemon();
        switch (result) {
        case OK:
        	break;
        default:
        	d_mode = DEVICEGPRS_MODE_COMMAND;
        	return result;
        }
        return OK;
    }

    LOG4CPP_WARN(log, "Switch back to Data Mode NOT supportd from current mode [%d]", d_mode);
    return GPRS_AT_NOT_SUPPORTED;

}


}// namespace gprs
}// namespace controlbox
