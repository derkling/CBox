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


#ifndef _DEVICEODO_H
#define _DEVICEODO_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Device.h>

namespace controlbox {
namespace device {

/// A DeviceOdometer is an Interface to device exporting Odometer functionalities.
class DeviceOdometer {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

public:

    /// Messages generables by objects of this interface
    enum cmdType {
	ODOMETER_EVENT_MOVE = (controlbox::Device::DEVICE_ODO*SERVICES_RANGE)+1,
	ODOMETER_EVENT_STOP,
	ODOMETER_EVENT_OVER_SPEED,
	ODOMETER_EVENT_EMERGENCY_BREAK,
	ODOMETER_EVENT_SAFE_SPEED,
	ODOMETER_EVENT_DIST_ALARM,
    };
    typedef enum cmdType t_cmdType;

    enum speedUnits {
	MS = 0,		///> speed on m/s
	KMH,		///> speed on km/h
	SPEED_COUNT	///> last entry marker
    };
    typedef enum speedUnits t_speedUnits;

    enum distUnits {
	M = 0,		///> distance on m
	KM,		///> distance on km
	DIST_COUNT	///> last entry marker
    };
    typedef enum distUnits t_distUnits;

//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
public:

    /// The currnent speed in the required unit of measure.
    /// @param unit the required unit of measure, by default [km/h]
    /// @return the current speed in the required unit of measure
    virtual double odoSpeed(t_speedUnits unit = KMH) = 0;

    /// The distance covered, since last reset, in the required unit of measure.
    /// @param unit the required unit of measure, by default [meters]
    /// @return the distance covered in the required unit of measure
    virtual double distance(t_distUnits unit = M) = 0;

    /// Read current configured speed alarm.
    virtual double speedAlarm(t_speedUnits unit = KMH) = 0;

    /// Read current configured emergency break decelleration alarm.
    virtual double emergencyBreakAlarm(t_speedUnits unit = KMH) = 0;

    /// Configure (initial) distance.
    /// Set the current distance value to the specified one's
    /// @param distance the distance value to configure
    /// @param unit the unit of measure for the specified distance, by default [meters]
    virtual exitCode setDistance(double distance, t_distUnits unit = M) = 0;

    /// Configure the speed alarm.
    /// Define the specified speed as the level that will trigger an alarm.
    /// @param speed the minumum speed value that trigger an alarm
    /// @param unit the unit of measure for the specified speed, by default [km/h]
    /// @return OK on success
    virtual exitCode setSpeedAlarm(double speed, t_speedUnits unit = KMH) = 0;

    /// Configure the emergency break alarm.
    /// @param acceleration the minimum negative acceleration value that trigger an alarm
    /// @param unit the unit of measure for the specified speed, by default [km/h]
    virtual exitCode setEmergencyBreakAlarm(double acceleration, t_speedUnits unit = KMH) = 0;

};

} //namespace device
} //namespace controlbox
#endif

