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


#ifndef _DEVICEAS_H
#define _DEVICEAS_H

#include <cc++/thread.h>
#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/devices/DeviceTime.h>
#include <controlbox/base/comsys/CommandGenerator.h>

#define AS_DEFAULT_DEVICE	"/dev/i2c"
#define AS_DEFAULT_SYSFSBASE	"/sys/bus/i2c/devices"

#define AS_MAX_ID_LENGTH	15
#define AS_MAX_UNIT_LENGTH	7
#define AS_MAX_VALUE_LENGTH	15
#define AS_MAX_SYSFS_LENGTH	24
#define AS_MAX_DESC_LENGTH	63

/// The sensor configuration entry base string
#define AS_CONF_BASE		"AnalogSensor_"
/// The first configuration parameter ID, appended to "AnalogSensor_"
#define AS_FIRST_ID		1
/// Maximum number of configuration parameters
#define AS_MAX_SENSORS		16

#define AS_MAX_ID_LENGTH	15
#define AS_MAX_DESC_LENGTH	63
#define AS_DESC_START		7
/// Minimun allowed Poll time in milliseconds
#define AS_MIN_POLL_TIME	500

#ifndef CONTROLBOX_CRIS
typedef struct _I2C_DATA {
    unsigned int length;
    unsigned char slave;
    unsigned char reg;
    unsigned char data[512];
} I2C_DATA;
#endif

namespace controlbox {
namespace device {

/// A DeviceAnalogSensors is a CommandGenerator that...
/// blha blha blha<br>
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>AnalogSensor_[n]</b> - <i>Default: ""</i><br>
///		An analog sensor definition following this pattern:<br>
///		TODO: explain the pattern
///	</li>
///	<li>
///		<b>AnalogSensor_i2cDevice</b> - <i>Default: AS_DEFAULT_DEVICE</i><br>
///		The device to use to access I2C bus<br>
///	</li>
/// </ul>
/// @see CommandHandler
class DeviceAnalogSensors : public comsys::CommandGenerator, public Device {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

protected:
    // Forward declarations
    class Monitor;

public:

    /// Handled Messages
    enum cmdType {
	ANALOG_SENSORS_EVENT = (DEVICE_AS*SERVICES_RANGE)+1,
    };
    typedef enum cmdType t_cmdType;

    typedef char t_asId[AS_MAX_ID_LENGTH+1];

    typedef u8 t_bus;
    typedef u8 t_address;
    typedef u8 t_channelNumber;

    enum analogSensorProtocol {
        AS_PROTO_I2C = 0,
        AS_PROTO_SYSFS,
        AS_PROTO_GPIO
    };
    typedef enum analogSensorProtocol t_analogSensorProtocol;

    struct sysfsAnalogSensor {
        t_bus bus;			///> The I2C bus to use
        t_address address;		///> The chip address
        t_channelNumber channel;	///> The channel number
    };
    typedef struct sysfsAnalogSensor t_sysfsAnalogSensor;

    struct i2cAnalogSensor {
        unsigned short chipAddress;
        unsigned short reg;
    };
    typedef struct i2cAnalogSensor t_i2cAnalogSensor;

    union asAddress {
        t_i2cAnalogSensor	i2cAddress;	///> Addressing for I2C access
        t_sysfsAnalogSensor	sysfsAddress;	///> Addressing for sysfs access
    };
    typedef union asAddress t_asAddress;
    
    enum asAlarmState {
    	NO_ALARM = 0,
    	LOW_ALARM,
    	HIGH_ALARM
    };
    typedef enum asAlarmState t_asAlarmState;

    typedef char t_sysfsPath[AS_MAX_SYSFS_LENGTH];

    typedef u8 t_eventCode;

    typedef u8 t_eventParam;

    typedef u8 t_channelValue;

    /// A value lable ID.
    typedef char t_asValue[AS_MAX_VALUE_LENGTH+1];


    struct analogSensor {
        t_asId id;				///> sensor ID
        t_analogSensorProtocol proto;		///> sensor protocol
        t_asAddress address;			///> sensor addressing
        t_sysfsPath sysfsPath;			///> Sysfs filepath
        unsigned minSample;			///> minimun sampled value
        unsigned maxSample;			///> maximun sampled value
        float minValue;				///> minimun phisical value
        float maxValue;				///> maximun phisical value
        char unit[AS_MAX_UNIT_LENGTH+1];	///> unit of measure
        bool enabled;				///> whatever this sensor is enabled
        unsigned downLimit;			///> the lower limit that trigger an alarm
        unsigned upperLimit;			///> the upper limit that trigger an alarm
        t_asAlarmState alarmState;		///> the last notified alarm
        t_asValue lvalue;			///> the value associated to a lower limit violation
        t_asValue hvalue;			///> the value associated to an upper limit violation
        t_eventCode event;			///> the event monitored by the sensor [0 = don't generate an event]
        bool hasParam;				///> whatever this event has a associated parameter
        t_eventParam param;			///> an optional parameter associated to the event
        unsigned alarmPollTime;			///> milliseconds to wait for polling
        char description[AS_MAX_DESC_LENGTH+1];	///> textual description
        t_channelValue lastSample;		///> last read sample
        DeviceAnalogSensors::Monitor * monitor;
    };
    typedef struct analogSensor t_analogSensor;

    /// A map of analog sensors.
    typedef map<std::string, t_analogSensor*> t_asMap;

    /// A list of analog sensors identifiers
    typedef list<std::string> t_asIdList;


protected:

    class Monitor : public ost::PosixThread {
    public:
        Monitor(DeviceAnalogSensors * device, t_analogSensor * pAs, timeout_t d_pollTime);
        ~Monitor() {};
        void run (void);
    protected:
        DeviceAnalogSensors * d_device;
        timeout_t d_pollTime;
        t_analogSensor * d_pAs;
    };
    // Allowing inner class to access DeviceAnalogSensors members
    friend class Monitor;

    static DeviceAnalogSensors * d_instance;

    /// The Configurator to use for getting configuration params
    Configurator & d_config;

    /// The Time Device to use
    DeviceTime * d_time;

    /// The sysfs base configuration path
    std::string d_sysfsbase;

    /// The map of availables analog sensors
    t_asMap analogSensors;

    /// Whatever some sensors use the i2c device
    bool loadI2C;

    /// The I2C device to use
    int fdI2C;

    /// Set true when at least one sensor shuld be monitored for
    /// alarm signal generation
    bool alarmEnables;

    /// The number of milliseconds
    unsigned alarmPoolTime;

    /// The logger to use locally.
    log4cpp::Category & log;

//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
public:

    /// Return an instance of an Analo Sensor Device
    static DeviceAnalogSensors * getInstance(std::string const & logName = "DeviceAS");

    /// Class destructor.
    ~DeviceAnalogSensors();

    inline unsigned count();

    unsigned listId(t_asIdList & asIdList);

    /// Read the value of a sensor.
    /// @param value the readed value on success
    /// @param update by default query the device, otherwise
    ///		(if false) return the last readed sample
    /// @return OK on success
    exitCode read(std::string asId, float & value, bool update = true);

    /// @return true on safety range respected.
    /// @note this method return true even if the device doesn't exist
    ///		or the required device is not alarm monitored.
    inline bool checkSafety(std::string asId);



protected:

    /// Create a new DeviceAnalogSensors.
    DeviceAnalogSensors(std::string const & logName);

    inline t_analogSensor * parseCfgString(std::string const & asCfg);

    inline exitCode loadSensorConfiguration(void);

    inline exitCode loadI2Cbus ();

    inline bool validateParams(t_analogSensor * pAs);

    inline void logSensorParams(t_analogSensor * pAs);

    inline t_channelValue updateSensor(t_analogSensor * pAs);

    inline short unsigned startMonitors();

    inline exitCode refresSensors();

    inline int sample(t_analogSensor * theSensor, bool update = true);

    /// @param theSample the sample value to convert, by default the lastSample
    ///		readed for this sensor
    inline float sampleToValue(t_analogSensor * pAs, int theSample = -1);

    inline unsigned valueToSample(t_analogSensor * pAs, float value);

    inline exitCode notifySensorEvent(t_analogSensor & aSensor);

    inline std::string getAlarmStrEvent(t_analogSensor & aSensor);

    inline t_analogSensor * findById (std::string asId);

    /// @pre the specified analog sensor must be on I2C Bus (pAs->protocol == AS_PROTO_I2C)
    /// @pre the I2C device must be opened (fdI2C != 0)
    inline exitCode readI2C(t_analogSensor * pAs, t_channelValue & value);

    inline exitCode readSysfs(DeviceAnalogSensors::t_analogSensor * pAs, t_channelValue & value);

    /// @return true on safety range respected.
    /// @note this method return true even if the device doesn't exist
    ///		or the required device is not alarm monitored.
    bool checkSafety(t_analogSensor * pAs, bool notify = false);

    void notifyAlarm(t_analogSensor * pAs);

    /// Upload SOAP message's Thread body.
    void run(void);

};


} //namespace device
} //namespace controlbox
#endif

