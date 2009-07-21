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


#include "DeviceGPIO.ih"


namespace controlbox {
namespace device {

DeviceGPIO::DeviceGPIO(std::string const & logName) :
	Device(Device::DEVICE_GPIO, 0, logName),
	d_config(Configurator::getInstance()),
	log(Device::log) {

	LOG4CPP_DEBUG(log, "DeviceGPIO(const std::string &, bool)");

	// Registering device into the DeviceDB
	dbReg();

}

DeviceGPIO::~DeviceGPIO(void) {

}


exitCode DeviceGPIO::gprsPowerOn(unsigned short gprs) {

	if ( gprsPowered(gprs) )
		return OK;

	gprsSwitch(gprs);

	// At power-on: waiting GPRS AT interface to startup
	::sleep(10);

	if ( !gprsPowered(gprs) ) {
		LOG4CPP_WARN(log, "Failed powering on GPRS-%s",
			(gprs == GPRS1) ? "1" : "2");
		return GPRS_DEVICE_POWER_ON_FAILURE;
	}

	LOG4CPP_DEBUG(log, "GPRS-%s powered up",
			(gprs==GPRS1) ? "1" : "2");

	return OK;

}

exitCode DeviceGPIO::gprsPowerOff(unsigned short gprs) {

	if ( !gprsPowered(gprs) )
		return OK;

	gprsSwitch(gprs);
	::sleep(5);

	if ( gprsPowered(gprs) ) {
		LOG4CPP_WARN(log, "Failed powering off GPRS-%s",
			(gprs == GPRS1) ? "1" : "2");
		return GPRS_DEVICE_POWER_OFF_FAILURE;
	}

	LOG4CPP_DEBUG(log, "GPRS-%s powered off",
			(gprs==GPRS1) ? "1" : "2");

	return OK;
}

exitCode DeviceGPIO::ttyLock(unsigned short port) {

	if (port==TTYMUX_SINGLE)
		return OK;

	// Acquiring mux lock to satefly use the port
	LOG4CPP_DEBUG(log, "Waiting for MUX1 lock...");

	d_ttyLock.enterMutex();
	LOG4CPP_DEBUG(log, "TTY mux lock acquired by port [%c]", 'A'-1+port);

	// Switching MUX to te required port
	ttySelect(port);

	return OK;

}

exitCode DeviceGPIO::ttyUnLock(unsigned short port) {

	if (port==TTYMUX_SINGLE)
		return OK;

	d_ttyLock.leaveMutex();
	LOG4CPP_DEBUG(log, "TTY mux lock released by port [%c]", 'A'-1+port);

	return OK;
}

exitCode DeviceGPIO::adcLock(unsigned short port) {
	return OK;
}

exitCode DeviceGPIO::adcUnLock(unsigned short port) {
	return OK;
}

}// namespace device
}// namespace controlbox
