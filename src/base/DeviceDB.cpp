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


#include "DeviceDB.ih"


namespace controlbox {

DeviceDB * DeviceDB::d_instance = 0;


DeviceDB::DeviceDB(std::string const & logName) :
        log(log4cpp::Category::getInstance("controlbox."+logName)) {
}

DeviceDB * DeviceDB::getInstance() {

    if ( !d_instance ) {
        d_instance = new DeviceDB("DeviceDB");
    }

    LOG4CPP_DEBUG(d_instance->log, "DeviceDB::getInstance()");

    return d_instance;
}

DeviceDB::~DeviceDB() {

    LOG4CPP_DEBUG(log, "DeviceDB::~DeviceDB()");

    // flushing the db <map>
    d_deviceDB.clear();

}

exitCode DeviceDB::registerDevice(Device * device, Device::t_deviceType const & type, Device::t_deviceId const & id, bool override) {
    DeviceDB::t_deviceEntry entry;
    t_dbEntry::iterator it;

    LOG4CPP_DEBUG(log, "DeviceDB::registerDevice(Device * device, type=%d, id='%s', override=%s)", type, id.c_str(), override ? "YES" : "NO" );

    entry.type = type;
    entry.id = id;
    entry.device = device;

    it = find (entry.type, entry.id);

    // Eventually avoid device duplication (if overriding is not required)
    if ( (it != d_deviceDB.end()) ) {
        if ( override ) {
            d_deviceDB.erase(it);
        } else {
            LOG4CPP_WARN(log, "Trying to duplicate a device [%s]", id.c_str());
            return DB_DEVICE_DUPLICATE;
        }
    }

    d_deviceDB.insert( pair<Device::t_deviceType, t_deviceEntry>(type,entry) );

    if ( override && (it != d_deviceDB.end()) ) {
        LOG4CPP_WARN(log, "Overriding device [%s] registration into class type [%d]", id.c_str(), type);
    } else {
        LOG4CPP_INFO(log, "Registered new device [%s (id: %s)] of class type [%d:%s]", device->name().c_str(), id.c_str(), type, Device::d_deviceTypeName[type]);
    }

    return OK;

}


exitCode DeviceDB::unregisterDevice(Device * device, Device::t_deviceType const & type, Device::t_deviceId const & id) {
    t_dbEntry::iterator it;

    LOG4CPP_DEBUG(log, "DeviceDB::unregisterDevice(Device * device, type=%d, id='%s')", type, id.c_str());

    it = find (type, id);

    // If the device was NOT registered
    if ( (it == d_deviceDB.end()) ||
            (device != (it->second).device ) ) {
        LOG4CPP_WARN(log, "Trying to remove a device [%s] NOT registered", id.c_str());
        return DB_DEVICE_NOT_EXIST;
    }

    d_deviceDB.erase(it);
    LOG4CPP_INFO(log, "Unregistered device [%s] of class type [%d:%s]", id.c_str(), type, Device::d_deviceTypeName[type]);

    return OK;

}


inline
DeviceDB::t_dbEntry::iterator DeviceDB::find(Device::t_deviceType const & type) {
    t_dbEntry::iterator first;
    t_dbEntry::iterator it;
    t_deviceEntry entry;

    LOG4CPP_DEBUG(log, "DeviceDB::find(type=%d)", type);

    it = d_deviceDB.find(type);

    // At this point: either it == d_deviceDB.end() or point
    //  to a the first device within the specified class
    return it;
}

inline
DeviceDB::t_dbEntry::iterator DeviceDB::find(Device::t_deviceType const & type, Device::t_deviceId const & id) {
    t_dbEntry::iterator first;
    t_dbEntry::iterator it;
    t_deviceEntry entry;

    LOG4CPP_DEBUG(log, "DeviceDB::find(type=%d, id='%s')", type, id.c_str());

    it = first = find(type);
    // Checking among alle lables of required type
    if ( it != d_deviceDB.end() ) {
        LOG4CPP_DEBUG(log, "Checking devices in class type=%d", first->first);
        do {
            entry = it->second;
            LOG4CPP_DEBUG(log, "Checking [%s]... ", entry.id.c_str());
            if ( entry.id == id ) {
                LOG4CPP_DEBUG(log, "Device [%s] FOUND!", id.c_str());
                return it;
            }
            it++;
        } while ( (it != d_deviceDB.end()) &&
                  (it->first == first->first) );

        LOG4CPP_DEBUG(log, "Search finished!");



        if ( it == d_deviceDB.end() ||
                (it->first != first->first) ) {
            // No duplicated lables found!
            LOG4CPP_DEBUG(log, "Device [%s] not present in DB", id.c_str());
            it = d_deviceDB.end();
            return it;
        }
    }
    // At this point: either it == d_deviceDB.end() or point
    //  to a device with the same lable (ready to ovverride it)
    return it;
}

Device * DeviceDB::getDevice(Device::t_deviceType const & type, std::string const & id) {
    t_dbEntry::iterator it;

    it = find (type, id);
    // Eventuallu avoid device duplication (if overriding is not required)
    if ( it == d_deviceDB.end() ) {
        LOG4CPP_DEBUG(log, "The required device [%s] is not registered", id.c_str());
        return 0;
    }

    return (it->second).device;

}

Device * DeviceDB::getDevice(Device::t_deviceType const & type, short int id) {
    std::ostringstream strId("");

    strId << id;
    return getDevice(type, strId.str());

}


Device * DeviceDB::getDevice(Device::t_deviceType const & type) {
    t_dbEntry::iterator it;

    it = find (type);
    // Eventuallu avoid device duplication (if overriding is not required)
    if ( it == d_deviceDB.end() ) {
        LOG4CPP_INFO(log, "There is no one device of the required class [%d] registerd", type);
        return 0;
    }

    return (it->second).device;
}


std::string DeviceDB::printDB(Device::t_deviceType const & type) {
    t_dbEntry::iterator it, first;
    unsigned count = 0;
    std::ostringstream dump("");

    if ( type != Device::UNDEF) {
        it = first = find(type);
        while ( (it != d_deviceDB.end()) &&
                (it->first == first->first) ) {
            count++;
            dump << "\n" << type << " " << (it->second).id;
            it++;
        }
    } else {
        it = d_deviceDB.begin();
        while ( it != d_deviceDB.end() ) {
            count++;
            dump << "\n" << (it->second).type << " " << (it->second).id;
            it++;
        }
    }

    dump << "\n" << count << " registered device/s";
    if ( type ) {
        dump << " of class " << type;
    }

    return dump.str();
}

}
