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


#ifndef _DEVICE_H
#define _DEVICE_H

#include <controlbox/base/Object.h>
#include <controlbox/base/Utility.h>


namespace controlbox {


/// This interface
class Device : public Object {

public:

    /// Define a unique identifier forma each kind of Device.
    /// A device is an entity that could provide some kind
    /// of services. Each device must conform to an interface,
    /// the deviceType is an identifier of services exported by
    /// a piece of code implementing this interface.
    /// NOTE those entryes must match the string lables defined into
    /// Device::d_deviceTypeName
    enum deviceType {
        UNDEF = 0,		/// undefined device
        EG_POLLER,
        CH_FILEWRITER,
        WSPROXY,
        DEVICE_SINGALS,
        DEVICE_I2CBUS,
        DEVICE_TIME,
        DEVICE_TE,
        DEVICE_GPRS,
        DEVICE_ARDU,
        DEVICE_ATGPS,
        DEVICE_GPS,
        DEVICE_AS,
        DEVICE_DS,
	DEVICE_GPIO,
        DEVICE_ODO,
        DEVICE_CAN,
        DEVICE_DISP,		///< Display
        DEVICE_IC		///< In Cabin Device
    };
    typedef enum deviceType t_deviceType;

    /// The identifier of a device within a class.
    /// This param is required only for multi-instance device class.
    typedef std::string t_deviceId;

    /// Device type name.
    /// Each device in t_deviceType has a mnemonic name defined by entries
    /// of this array.
    //NOTE entries should match those of t_deviceType;
    static const char *d_deviceTypeName[];

protected:

    /// The device type.
    /// This value identify the device class.
    t_deviceType d_type;

    /// The device identifier.
    /// This string define a unique device instance within the
    /// device class specified by 'type'.
    /// This value is usually used to define the logger category for
    /// the device.
    t_deviceId d_id;

    /// The device name.
    /// The device string name, this is taken by the logName string
    /// passed into the constructor.
    std::string d_name;

public:

    /// Create a new Device.
    /// @param type the device class
    /// @param id the device identifier within its class
    /// @param logName the logger category to use
    Device(t_deviceType type, short int id, std::string const & logName);

    /// Create a new Device.
    /// @param type the device class
    /// @param id the device identifier within its class
    /// @param logName the logger category to use
    Device(t_deviceType type, std::string const & id, std::string const & logName);

    /// Default destructor.
    virtual ~Device();

    inline std::string const & name() {
        return d_name;
    };

    // TODO: Control point management methods




protected:

    /// Register the device into a DeviceDB.
    /// @note this method shuld be called within each derived Device
    ///	class contructor, right before returning from the
    ///	contructor itself.
    virtual exitCode dbReg(bool override = false);

    /// Register an interface exported by a Device into the DeviceDB
    exitCode dbRegInterface(Device::t_deviceType const & type, Device::t_deviceId const & id = "0", bool override = false);


};

}// controlbox namespace

#endif
