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


#ifndef _DEVICEATGPS_H
#define _DEVICEATGPS_H

#include <cc++/thread.h>
#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/devices/DeviceSignals.h>
#include <controlbox/devices/DeviceTime.h>
#include <controlbox/base/comsys/CommandGenerator.h>
#include <controlbox/devices/DeviceSerial.h>
#include <controlbox/base/Device.h>
#include <controlbox/devices/DeviceOdometer.h>
#include <controlbox/devices/DeviceGPS.h>

// #define ATGPS_DEFAULT_SYSFSBASE		"/sys/bus/i2c/devices"
// #define ATGPS_DEFAULT_I2CADDR		"0-0050"
//
// /// The rounding factor for sysfs parameters
// #define ATGPS_ROUND_FACTOR		10000
/// The interrupt line for odometer alarms
#define ATGPS_DEFAULT_PA_INTRLINE	"4"
/// The number of pulses generated each meter
#define ATGPS_DEFAULT_PPM		"16"
/// Default minimum speed that trigger an alarm in [ms] (80km/h ~= 22m/s)
#define ATGPS_DEFAULT_HIGHSPEED_ALARM	"22"
/// Default minimum speed deceleration that trigger a break alarm in [m/s] (30km/h/s ~= 8m/s^2)
#define ATGPS_DEFAULT_EMERGENCY_BREAK	"8"
/// Default initial distance [m/s]
#define ATGPS_DEFAULT_INIT_DISTANCE	"0"
//
// #define ATGPS_SYSFS_ATTRIB_BUFSIZE	16
// #define ATGPS_MAX_ID_LENGTH		15
// #define ATGPS_MAX_DESC_LENGTH	63

namespace controlbox {
namespace device {

/// A DeviceATGPS is a CommandGenerator that...
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
class DeviceATGPS : public Device,
			public comsys::CommandGenerator,
			public device::DeviceOdometer,
			public device::DeviceGPS {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

protected:

    /// The command supported by the ATGPS device
    enum atgpsCmds {
	//0
	REG_EVENT = 0x00,	///> RW - ACK interrupts and read event register
	REG_MONITOR,		///> RW - Read/Set the 'continuous monitoring' value (0=disabled)
	ODO_PPM,		///> RW - Pulse-per-meter [p/m]
	ODO_PCOUNT,		///> RO - Pulses count since last reset
	ODO_TOTM,		///> RW - Total distance [m]
	// 5
	ODO_FREQ,		///> RO - Current pulses frequency [Hz]
	ODO_SPEED,		///> RO - Current speed [m/s]*M
	ODO_SPEEDALARM,		///> RW - Minimum [m/s]*M speed that trigger an OVER-SPEED alarm
	ODO_BREAKALARM,		///> RW - Minimum [m/s] decelleration that trigger a EMERGENCY-BREAK alarm
	GPS_LON,
	// 10
	GPS_LAT,
	GPS_UTC,
	GPS_VAL,
	GPS_KMH,
	GPS_DIR,
	// 15
	GPS_FIX,
	GPS_PDOP,
	GPS_HDOP,
	GPS_VDOP,
	GPS_DATE,
	// 20
	GPS_KNOTS,
	GPS_VAR,
    };
    typedef enum atgpsCmds t_atgpsCmds;

    /// The list of AT Commands <i>read value</i> string.
    /// Those strings are referred to the Query command and have this format:<br>
    ///	AT+CMD where CMD is the substring command.
    /// The 'read format' and 'set value' are derived by the 'read value' by appending
    /// '?' or '=VALUE' respectively
    /// Those values should map the t_atgpsCmds values.
    /// <b>Note:</b> an empty string means the corresponding t_atgpsCmds command
    /// is not an AT command to send to the deivce but a command addressed to this
    /// object (i.e. some values are computed by this class starting from other
    /// device provided values)
    /// @see t_atgpsCmds
    static char *d_atgpsCmds[];

    /// Possible values for the events reported by the REG_EVENT command.
    /// Events are reported as a 32 bitmask composed by the oring of this
    /// possible values
    enum atgpsEvents {
    	// ODO Events
	MOVE		= 0x00000001,
	STOP		= 0x00000002,
	OVER_SPEED	= 0x00000004,
	EMERGENCY_BREAK	= 0x00000008,
	SAFE_SPEED	= 0x00000010,
	// GPS Events
	FIX_GET		= 0x00000100,
	FIX_LOSE	= 0x00000200,
    };
    typedef enum atgpsEvents t_atgpsEvents;

    /// The DeviceATGPS unique instance
    static DeviceATGPS * d_instance;

    /// The Configurator to use for getting configuration params
    Configurator & d_config;

    /// The signal device used to receive interrupts
    DeviceSignals * d_signals;

    /// The Time Device to use
    DeviceTime * d_time;

    /// The interrupt line used by the signal device (on PortA)
    unsigned short d_intrLine;

    /// The TTY port used
    DeviceSerial * d_tty;

    /// The lock for accessing query interface
    ost::Mutex	d_lock;

    /// The sysfs path
//     std::string d_sysfspath;

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
    static DeviceATGPS * getInstance(std::string const & logName = "DeviceATGPS");

    /// Class destructor.
    ~DeviceATGPS();



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

    unsigned fixStatus();

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

    /// Check for alarms signals and eventually send a notify
    exitCode checkAlarms(bool notify = true);

protected:

    /// Create a new DeviceATGPS.
    DeviceATGPS(std::string const & logName);

//      /// Open the ATGPS device
//     exitCode openDevice(void);

    /// Configure the ATGPS device
    exitCode initDevice(void);

    /// Tirgger a notification for the event specified.
    /// @param event the t_atgpsEvents to notify
    exitCode notifyEvent(unsigned short event);

    inline exitCode getDeviceLocalValue(t_atgpsCmds idx, char * buf, int & len);

    inline exitCode getDeviceRemoteValue(t_atgpsCmds idx, char * buf, int & len);

    /// Read a string value from the device.
    /// Send a 'read value' command to the device and return the buffer
    /// with the responce string.
    /// @param idx the t_atgpsCmds command to send
    /// @param buf a pointer to a buffer to hold the responce string
    /// @param len the size of the buffer, at return the number of bytes received
    /// @return OK on success, otherwise the content of buf is not valid
    exitCode getDeviceValue(t_atgpsCmds idx, char * buf, int & len);

    /// Read an int value from the device.
    exitCode getDeviceValue(t_atgpsCmds idx, int & value);

    /// Read an unsigned long value from the device.
    exitCode getDeviceValue(t_atgpsCmds idx, unsigned long & value);

    /// Read a double value from the device.
    exitCode getDeviceValue(t_atgpsCmds idx, double & value);

    /// Set a value into the device.
    /// Send a 'set value' command to the device to set the specified value.
    /// @param idx the t_atgpsCmds command to send
    /// @param buf a pointer to a buffer holding the value to send
    exitCode setDeviceValue(t_atgpsCmds idx, const char * value);

    /// Device hearing and command dispatching thread body.
    void run(void);

};

} //namespace device
} //namespace controlbox
#endif

