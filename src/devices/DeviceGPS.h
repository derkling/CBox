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


#ifndef _DEVICEGPS_H
#define _DEVICEGPS_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Device.h>

namespace controlbox {
namespace device {

/// A DeviceGPS is an Interface to devices exporting GPS functionalities.
class DeviceGPS {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

public:

    /// Messages generables by objects of this interface
    enum cmdType {
	GPS_FIX_UPDATE = (controlbox::Device::DEVICE_GPS*SERVICES_RANGE)+1,
    };
    typedef enum cmdType t_cmdType;

    /// Supported parser protocols
    enum protocols {
    	DEVICEGPS_PROTO_ATGPS = 0,
	DEVICEGPS_PROTO_ARDU,
	DEVICEGPS_PROTO_NMEA,
    };
    typedef enum protocols t_protocols;

    /// Cardinal points
    enum cardPoint {
    	DEVICEGPS_CARD_NORTH,
    	DEVICEGPS_CARD_EAST,
    	DEVICEGPS_CARD_SOUTH,
    	DEVICEGPS_CARD_WEST
    };
    typedef enum cardPoint t_cardPoint;

    /// Manual or Automatic 2D/3D mode
    enum operatingMode {
    	DEVICEGPS_MODE_MANUAL,	/// Manual, forced to operate in 2D or 3D
    	DEVICEGPS_MODE_AUTO	/// Automatic, 3D/2D
    };
    typedef enum operatingMode t_operatingMode;

    /// Available fixes
    enum fixStatus {
    	DEVICEGPS_FIX_NA = 0,	/// Fix not available
    	DEVICEGPS_FIX_ASSIST,	/// Assisted fix available
    	DEVICEGPS_FIX_2D,	/// 2D fix available
    	DEVICEGPS_FIX_3D	/// 3D fix available
    };
    typedef enum fixStatus t_fixStatus;

    /// Units of measure system
    enum measureSystem {
	DEVICEGPS_UMS_ISO,	///> speed in Km/h
	DEVICEGPS_UMS_MARINE	///> speed in Knots
    };
    typedef enum measureSystem t_measureSystems;

    /// Course reference
    enum courseType {
	DEVICEGPS_CT_MAGNETIC,
	DEVICEGPS_CT_TERRESTRIAL
    };
    typedef enum courseType t_courseType;

//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
public:

    /// Get current fix status
    /// @return the GPS signal level as defined by the DeviceGPS::t_fixStatus type.
    /// @see t_fixStatus
    virtual t_fixStatus fixStatus() = 0;

    /// Get current longitude.
    /// The current formatted latitude. By default is returned in ISO6709
    /// format (+DDD.DDDD), otherwise it is formatted into a string containing
    /// the latitude and a comma separated N/S indicator.
    /// @return the current longitude in the format:<br>
    ///		<i>dddmm.mmmm,EWindicator</i><br>
    ///	An empty string is returned if it's not possible to know the current
    ///	longitude.
    virtual string longitude(bool iso = true) = 0;

    /// Get current latitude.
    /// The current formatted latitude. By default is returned in ISO6709
    /// format (+DD.DDDD), otherwise it is formatted into a string containing
    /// the latitude and a comma separated E/W indicator.
    /// @return the current latitude in the format:<br>
    ///		<i>ddmm.mmmm,EWindicator</i><br>
    ///	An empty string is returned if it's not possible to know the current
    ///	latitude.
    virtual string latitude(bool iso = true) = 0;

    /// Get current speed (over ground).
    /// Return the last monitored speed, expressed in the required
    /// measurement system.
    /// @param system the required measure system, [Km/h] is the default
    /// @return the last monitored speed.
    /// @note this method say nothing about the validity of the returne speed, it
    ///		only return the last releaved speed.
    /// @see t_measureSystems
    virtual double gpsSpeed(t_measureSystems system = DEVICEGPS_UMS_ISO) = 0;

    /// Get current course.
    /// Return the last monitored course, expressed in the required
    /// course reference.
    /// @param system the required course reference, magnetic course is the default
    /// @return the last monitored speed.
    /// @note this method say nothing about the validity of the returne course, it
    ///		only return the last releaved course.
    /// @see t_courseType
    virtual unsigned course(t_courseType type = DEVICEGPS_CT_MAGNETIC) = 0;

    /// Get current Time in ISO 8601 format.
    /// Complete date plus hours, minutes, seconds and a decimal fraction of a
    /// second:<br>
    /// <center>YYYY-MM-DDThh:mm:ssTZD <i>(eg 1997-07-16T19:20:30+01:00)</i></center><br>
    /// where:<br>
    /// <ul>
    ///      <li>YYYY = four-digit year</li>
    ///      <li>MM   = two-digit month (01=January, etc.)</li>
    ///      <li>DD   = two-digit day of month (01 through 31)</li>
    ///      <li>hh   = two digits of hour (00 through 23) (am/pm NOT allowed)</li>
    ///      <li>mm   = two digits of minute (00 through 59)</li>
    ///      <li>ss   = two digits of second (00 through 59)</li>
    ///      <li>TZD  = time zone designator (Z or +hh:mm or -hh:mm)</li>
    /// </ul>
    /// Times are expressed in UTC (Coordinated Universal Time),
    /// with a special UTC designator ("Z").
    /// @param UTC set true to use UTC time zone designator (Z)
    virtual string time(bool utc = false) = 0;

};

} //namespace device
} //namespace controlbox
#endif

