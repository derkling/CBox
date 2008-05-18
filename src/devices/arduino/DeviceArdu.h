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


#ifndef _DEVICEARDU_H
#define _DEVICEARDU_H

#include <cc++/thread.h>
#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/devices/DeviceSignals.h>
#include <controlbox/devices/DeviceTime.h>
#include <controlbox/base/comsys/CommandGenerator.h>
#include <controlbox/base/Device.h>
#include <controlbox/devices/DeviceOdometer.h>
#include <controlbox/devices/DeviceGPS.h>

#define ARDU_DEFAULT_SYSFSBASE		"/sys/bus/i2c/devices"
#define ARDU_DEFAULT_I2CADDR		"0-0050"

/// The rounding factor for sysfs parameters
#define ARDU_ROUND_FACTOR		10000
/// The interrupt line for odometer alarms
#define ARDU_DEFAULT_PA_INTRLINE	"4"
/// The number of pulses generated each meter
#define ARDU_DEFAULT_PPM		"16"
/// Default minimum speed that trigger an alarm in [ms] (120km/h ~= 33m/s)
#define ARDU_DEFAULT_HIGHSPEED_ALARM	"33"
/// Default minimum speed deceleration that trigger a break alarm in [m/s]
#define ARDU_DEFAULT_EMERGENCY_BREAK	"10"
/// Default initial distance [m/s]
#define ARDU_DEFAULT_INIT_DISTANCE	"0"

#define ARDU_SYSFS_ATTRIB_BUFSIZE	16
#define ARDU_MAX_ID_LENGTH		15
#define ARDU_MAX_DESC_LENGTH		63

namespace controlbox {
namespace device {

/// A DeviceArdu is a CommandGenerator that...
/// blha blha blha<br>
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>Odomether_[n]</b> - <i>Default: ""</i><br>
///	</li>
///	<li>
///	</li>
/// </ul>
/// @see CommandHandler
class DeviceArdu : public Device,
			public comsys::CommandGenerator,
			public device::DeviceOdometer,
			public device::DeviceGPS {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

protected:

    enum sysfsValue {
	REG_EVENT = 0x00,	///> RW - ACK interrupts and read event register
	ODO_PPM,		///> RW - Pulse-per-meter [p/m]
	ODO_PCOUNT,		///> RO - Pulses count since last reset
	ODO_TOTM,		///> RW - Total distance [m]
	ODO_FREQ,		///> RO - Current pulses frequency [Hz]
	ODO_SPEED,		///> RO - Current speed [m/s]*M
	ODO_SPEEDALARM,		///> RW - Minimum [m/s]*M speed that trigger an OVER-SPEED alarm
	ODO_BREAKALARM,		///> RW - Minimum [m/s] decelleration that trigger a EMERGENCY-BREAK alarm
	GPS_LAT,
	GPS_LON,
	GPS_UTC,
	GPS_VAL,
	GPS_KMH,
	GPS_DIR,
	GPS_FIX,
	GPS_PDOP,
	GPS_HDOP,
	GPS_VDOP,
	GPS_DATE,
	GPS_KNOTS,
	GPS_VAR,
    };
    typedef enum sysfsValue t_sysfsValue;

    static char *d_sysfsParams[];

   enum arduEvents {
	MOVE		= 0x1,
	STOP		= 0x2,
	OVER_SPEED	= 0x4,
	EMERGENCY_BREAK = 0x8
   };
   typedef enum arduEvents t_arduEvents;

    /// The DeviceArdu unique instance
    static DeviceArdu * d_instance;

    /// The Configurator to use for getting configuration params
    Configurator & d_config;

    /// The signal device used to receive interrupts
    DeviceSignals * d_signals;

    /// The Time Device to use
    DeviceTime * d_time;

    /// The sysfs path
    std::string d_sysfspath;

    /// The interrupt line to use (on PortA)
    unsigned short d_intrLine;

    /// The ppm generated by the device.
    unsigned short d_ppm;

    /// The high speed alarm threshold in [m/s]
    unsigned d_highSpeedAlarm;

    /// The minimum decelleration that trigger an alarm [m/s]
    unsigned d_emergencyBreakAlarm;

    /// The initial distance [m]
    unsigned long d_initDistance;

    /// The logger to use locally.
    log4cpp::Category & log;

//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
public:

    /// Return an instance of an Odometer Device
    static DeviceArdu * getInstance(std::string const & logName = "DeviceArdu");

    /// Class destructor.
    ~DeviceArdu();

//--- Odometer interface implementation
    /// The currnent speed in the required unit of measure.
    /// @param unit the required unit of measure, by default [km/h]
    /// @return the current speed in the required unit of measure
    double odoSpeed(t_speedUnits unit = KMH);

    /// The distance covered, since last reset, in the required unit of measure.
    /// @param unit the required unit of measure, by default [meters]
    /// @return the distance covered in the required unit of measure
    double distance(t_distUnits unit = M);

    /// Read current configured speed alarm.
    double speedAlarm(t_speedUnits unit = KMH);

    /// Read current configured emergency break decelleration alarm.
    double emergencyBreakAlarm(t_speedUnits unit = KMH);

    /// Configure (initial) distance.
    /// Set the current distance value to the specified one's
    /// @param distance the distance value to configure
    /// @param unit the unit of measure for the specified distance, by default [meters]
    exitCode setDistance(double distance, t_distUnits unit = M);

    /// Configure the speed alarm.
    /// Define the specified speed as the level that will trigger an alarm.
    /// @param speed the minumum speed value that trigger an alarm
    /// @param unit the unit of measure for the specified speed, by default [km/h]
    /// @return OK on success
    exitCode setSpeedAlarm(double speed, t_speedUnits unit = KMH);

    /// Configure the emergency break alarm.
    /// @param acceleration the minimum negative acceleration value that trigger an alarm
    /// @param unit the unit of measure for the specified speed, by default [km/h]
    exitCode setEmergencyBreakAlarm(double acceleration, t_speedUnits unit = KMH);

//--- GPS interface implementation

    DeviceGPS::t_fixStatus fixStatus();

    double lat(void);

    double lon(void);

    /// Get current longitude.
    /// The current formatted latitude. By default is returned in ISO6709
    /// format (+DDD.DDDD), otherwise it is formatted into a string containing
    /// the latitude and a comma separated N/S indicator.
    /// @return the current longitude in the format:<br>
    ///		<i>dddmm.mmmm,EWindicator</i><br>
    ///	An empty string is returned if it's not possible to know the current
    ///	longitude.
    string longitude(bool iso = true);

    /// Get current latitude.
    /// The current formatted latitude. By default is returned in ISO6709
    /// format (+DD.DDDD), otherwise it is formatted into a string containing
    /// the latitude and a comma separated E/W indicator.
    /// @return the current latitude in the format:<br>
    ///		<i>ddmm.mmmm,EWindicator</i><br>
    ///	An empty string is returned if it's not possible to know the current
    ///	latitude.
    string latitude(bool iso = true);

    double gpsSpeed(DeviceGPS::t_measureSystems system = DeviceGPS::DEVICEGPS_UMS_ISO);

    /// Get current course.
    /// Return the last monitored course, expressed in the required
    /// course reference.
    /// @param system the required course reference:<br>
    ///	<ul>
    ///		<li>DEVICEGPS_CT_MAGNETIC</li> magnetic course
    ///		<li>DEVICEGPS_CT_TERRESTRIAL</li> terrestrial course
    ///	</ul>
    /// @return the last monitored course.
    /// @note this method say nothing about the validity of the returne course, it
    ///		only return the last releaved course.
    /// @see t_courseType
    unsigned course(DeviceGPS::t_courseType type = DeviceGPS::DEVICEGPS_CT_MAGNETIC);

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
    string time(bool utc = false);


protected:

    /// Create a new DeviceArdu.
    DeviceArdu(std::string const & logName);

    /// Configure the ARDUINO device
    exitCode initOdometer(void);

    /// Tirgger a notification for the event specified.
    /// @param event the t_arduEvents to notify
    exitCode notifyOdoEvent(unsigned short event);

    /// Check for alarms signals and eventually send a notify
    exitCode checkAlarms(void);

    /// Read a value from a sysfs attribute
    exitCode getSysfsValue(t_sysfsValue idx, unsigned long & value);

    /// Upload SOAP message's thread body.
    void run(void);

};

} //namespace device
} //namespace controlbox
#endif

