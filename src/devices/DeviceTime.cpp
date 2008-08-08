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

#include "DeviceTime.ih"

namespace controlbox {
namespace device {

DeviceTime * DeviceTime::d_instance = 0;


DeviceTime::DeviceTime(std::string const & logName) :
	Device(Device::DEVICE_TIME, logName, logName),
	CommandGenerator(logName),
	d_devGPS(0),
	d_devGPRS(0),
	log(Device::log) {
	DeviceFactory * df;

	LOG4CPP_DEBUG(log, "DeviceTime(logName=%s)", logName.c_str());

	// Registering device into the DeviceDB
	dbReg();

	// Linking here all systems that could be used to get a time
	df = DeviceFactory::getInstance();

}

DeviceTime * DeviceTime::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceTime(logName);
		d_instance->dbReg();
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceTime::getInstance()");

	return d_instance;

}

string DeviceTime::time(bool utc) const {
	char str[26];
	time_t t;
	struct tm *tmp;
	std::string strTime;

	LOG4CPP_DEBUG(log, "time(utc=%s)", utc ? "YES" : "NO" );

	// First: trying to get time using GPS
	if ( d_devGPS ) {
		strTime = d_devGPS->time(utc);
		if ( strTime.size() ) {
			return strTime.c_str();
		}
	}

	// Second: use GPRS time
	if ( d_devGPRS ) {
		strTime = d_devGPRS->time(utc);
		if ( strTime.size() ) {
			return strTime.c_str();
		}
	}

	// Last chanche: use systime
	t = std::time(0);
	tmp = localtime(&t);
	if (tmp == NULL) {
		LOG4CPP_ERROR(log, "Unable to get locatime");
		return string("1970-01-01T00:00:00+00:00");
	}
	if (strftime(str, 25, "%FT%T%z", tmp) == 0) {
		LOG4CPP_ERROR(log, "Failure on time formatting");
		return string("1970-01-01T00:00:00+00:00");
	}
	// Adjusting TimeZone...
	str[24] = str[23];
	str[23] = str[22];
	str[22] = ':';
	str[25] = 0;

	return string(str);

}

void DeviceTime::run(void)  {

	LOG4CPP_DEBUG(log, "DeviceTime thread started");

	// TODO: To Implement
	return;

}

}// namespace device
}// namespace controlbox
