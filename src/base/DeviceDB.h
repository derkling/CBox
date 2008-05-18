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


#ifndef _DEVICEDB_H
#define _DEVICEDB_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Device.h>

namespace controlbox {

/// This interface define methods to save and retrive
/// unique instances of system devices, either
/// Generators, Dispatcher or Handlers, identified by their
/// Device::t_deviceType.
class DeviceDB {

private:

    struct deviceEntry {
        Device::t_deviceType type;
        Device::t_deviceId id;
        Device * device;
    };
    typedef struct deviceEntry t_deviceEntry;

    typedef multimap<Device::t_deviceType, t_deviceEntry> t_dbEntry;

    static DeviceDB * d_instance;

    t_dbEntry d_deviceDB;

    log4cpp::Category & log;

public:

    /// Get an instance of DeviceDB
    /// DeviceDB is a singleton class, this method provide
    /// a pointer to the (eventually just created) only one instance.
    static DeviceDB * getInstance();


    /// Default destructor
    ~DeviceDB();

    /// Register a device into DB
    /// The pointer to a Device is saved into the DB and
    /// associated to the specified deviceCode.
    /// @param owerride set TRUE to override any eventually already saved
    ///		Device's pointer; default FALSE
    exitCode registerDevice(Device * device, Device::t_deviceType const & type,  Device::t_deviceId const & id, bool override = false);

    /// Unregister a device from the DB
    exitCode unregisterDevice(Device * device, Device::t_deviceType const & type, Device::t_deviceId const & id);


    /// Get a pointer to a Device from the DB
    /// Return a generic Device pointer to a one of
    /// the device of class t_deviceType registered into the DB<br>
    /// @return a Device's pointer to the specified t_deviceType if
    /// present into the DB, a zero-pointer if ther's no one such
    /// device registerd into the class.
    Device * getDevice(Device::t_deviceType const & type);

    Device * getDevice(Device::t_deviceType const & type, short int id);

    Device * getDevice(Device::t_deviceType const & type, std::string const & id);

    /// Print registered Devices
    /// This method return a string with a report of registerd devices.
    /// @param type the class of device to print; print all devices
    ///		if is Device::UNDEF
    std::string printDB(Device::t_deviceType const & type = Device::UNDEF);

protected:

    /// Create a new DeviceDB
    DeviceDB(std::string const & logName = "DeviceDB");

    inline DeviceDB::t_dbEntry::iterator find(Device::t_deviceType const & type);

    inline DeviceDB::t_dbEntry::iterator find(Device::t_deviceType const & type, Device::t_deviceId const & id);


};

}// controlbox namespace

#endif
