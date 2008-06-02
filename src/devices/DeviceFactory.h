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


#ifndef _DEVICEFACTORY_H
#define _DEVICEFACTORY_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/DeviceDB.h>
#include <controlbox/base/Device.h>
//-----[ Supported devices ]----------------------------------------------------
#include <controlbox/devices/DeviceSignals.h>
#include <controlbox/devices/DeviceI2CBus.h>
#include <controlbox/devices/DeviceTime.h>
#include <controlbox/devices/FileWriterCommandHandler.h>
#include <controlbox/devices/PollEventGenerator.h>
#include <controlbox/devices/gprs/DeviceGPRS.h>
#include <controlbox/devices/DeviceGPS.h>
#include <controlbox/devices/DeviceOdometer.h>
#include <controlbox/devices/DeviceAnalogSensors.h>
#include <controlbox/devices/DeviceGPIO.h>
#include <controlbox/devices/DeviceDigitalSensors.h>
// #include <controlbox/devices/arduino/DeviceArdu.h>
#include <controlbox/devices/atgps/DeviceATGPS.h>
#include <controlbox/devices/te/DeviceTE.h>
#include <controlbox/devices/wsproxy/WSProxyCommandHandler.h>
#include <controlbox/base/comsys/CommandDispatcher.h>



namespace controlbox {
namespace device {

/// This calss export methods to build devices.
/// Each device is build and (eventually) registered into the DeviceDB
/// making attention on build also all required dependenant devices.
/// @note This class is in charge to maintain inter-devices dependencies
///		by creating and registering all devices needed/used by a new
///		one when this last is required.
class DeviceFactory {

private:

    /// The (only) instance of this class.
    static DeviceFactory * d_instance;

    /// The DeviceDB to use for reference.
    DeviceDB & d_devDB;

    /// The logger.
    log4cpp::Category & log;

public:

    /// Get an instance of DeviceFactory
    /// DeviceFactory is a singleton class, this method provide
    /// a pointer to the (eventually just created) only one instance.
    static DeviceFactory * getInstance(std::string const & logName = "DeviceFactory");


    /// Default destructor
    ~DeviceFactory();

    DeviceSignals * getDeviceSignals(std::string const & logName = "DeviceSignals");

    DeviceI2CBus * getDeviceI2CBus(std::string const & logName = "DeviceI2CBus");

    DeviceTime * getDeviceTime(std::string const & logName = "DeviceTime");


    /// Return a FileWriterCommandhandler
    FileWriterCommandHandler * getDeviceFileWriter(const std::string & fileName,
            std::string const & logName = "XMLFileWriter",
            bool append = true);

    /// Return a PollEventGenerator
    PollEventGenerator * getDevicePoller(timeout_t pollTime,
                                         std::string const & logName = "Poller");

    /// Return a DeviceGPRS.
    DeviceGPRS * getDeviceGPRS(std::string const & logName = "DeviceGPRS",
		unsigned short module = 0);

    /// Return a DeviceGPS.
    DeviceGPS * getDeviceGPS(std::string const & logName = "DeviceGPS",
                             DeviceGPS::t_protocols proto = DeviceGPS::DEVICEGPS_PROTO_ATGPS);
                             //DeviceGPS::t_protocols proto = DeviceGPS::DEVICEGPS_PROTO_ARDU);

    /// Return a DeviceODO.
    DeviceOdometer * getDeviceODO(std::string const & logName = "DeviceODO");

//    /// Return a DeviceArdu.
//    DeviceArdu * getDeviceArdu(std::string const & logName = "DeviceArdu");

    /// Return a DeviceATGPS.
    DeviceATGPS * getDeviceATGPS(std::string const & logName = "DeviceATGPS");

    /// Return a DeviceAS.
    DeviceAnalogSensors * getDeviceAS(std::string const & logName = "DeviceAS");

    /// Return a DeviceGPIO.
    DeviceGPIO * getDeviceGPIO(std::string const & logName = "DeviceGPIO");

    /// Return a DeviceDS.
    DeviceDigitalSensors * getDeviceDS(std::string const & logName = "DeviceDS");

    /// Return a DeviceTE
    DeviceTE * getDeviceTE(std::string const & logName = "DeviceTE");

    /// Return a WSProxy.
    WSProxyCommandHandler * getWSProxy(std::string const & logName = "WSProxy");



protected:

    /// Create a new DeviceFactory.
    DeviceFactory(std::string const & logName);


};

}// namespace device
}// namespace controlbox

#endif

