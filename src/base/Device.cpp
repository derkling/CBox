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


#include "Device.ih"


namespace controlbox {

const char *Device::d_deviceTypeName[] = {
        "Unknowen",
        "Poller",
        "FileWriter",
        "Proxy",
        "Signal",
        "Time",
        "TE",
        "GPRS",
        "ARDU",
        "ATGPS",
        "GPS",
        "AnalogSensor",
        "DigitalSensor",
		"GPIO",
        "Odometer",
        "CanBUS",
        "Display",
        "InCabinDevice"
};


Device::Device(t_deviceType type, short int id, std::string const & logName) :
        Object("device."+logName),
        d_name(logName),
        d_type(type),
        d_id("") {

    LOG4CPP_DEBUG(log, "Device::Device(%s)", d_name.c_str());

    std::ostringstream strId("");
    strId << id;

    d_id = strId.str();

}

Device::Device(t_deviceType type, std::string const & id, std::string const & logName) :
        Object("device."+logName),
        d_name(logName),
        d_type(type),
        d_id(id) {

    LOG4CPP_DEBUG(log, "Device::Device(id=%s, %s)", d_id.c_str(), d_name.c_str());

}


Device::~Device() {
    DeviceDB * d_devDB = DeviceDB::getInstance();

    LOG4CPP_DEBUG(log, "Device::~Device()");

    // Deregistering the device from the DeviceDB
    if ( d_devDB->unregisterDevice(this, d_type, d_id) != OK ) {
        LOG4CPP_DEBUG(log, "Failed to unregister the device [%s] from DB", d_id.c_str());
    }

}

/*
inline
std::string Device::name() {
	return d_name;
}
*/

exitCode Device::dbReg(bool override) {
    DeviceDB * d_devDB = DeviceDB::getInstance();

    LOG4CPP_INFO(log, "Registering device [%s] into DeviceDB", d_name.c_str());

    // Registering the device into the DeviceDB
    if ( d_devDB->registerDevice(this, d_type, d_id, override) != OK ) {
        LOG4CPP_WARN(log, "Failed to register the device [%s] into DeviceDB", d_id.c_str());
        return DB_REGISTRATION_FAILURE;
    }

    return OK;

}

exitCode Device::dbRegInterface(Device::t_deviceType const & type, Device::t_deviceId const & id, bool override) {
    DeviceDB * d_devDB = DeviceDB::getInstance();

    LOG4CPP_INFO(log, "Registering interface [%d:%s] for device [%s] into DeviceDB", type, id.c_str(), d_name.c_str());

    // Registering the device into the DeviceDB
    if ( d_devDB->registerDevice(this, type, id, override) != OK ) {
        LOG4CPP_WARN(log, "Failed to register the interface [%d:%s] device [%s] into DeviceDB", type, id.c_str(), d_id.c_str());
        return DB_REGISTRATION_FAILURE;
    }

    return OK;
}


}
