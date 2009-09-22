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


#ifndef _DEVICEDS_H
#define _DEVICEDS_H

#include <cc++/thread.h>
#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/base/Worker.h>
#include <controlbox/base/Device.h>
#include <controlbox/base/comsys/CommandGenerator.h>
#include <controlbox/devices/DeviceSignals.h>
#include <controlbox/devices/DeviceI2CBus.h>
#include <controlbox/devices/DeviceTime.h>
#include <controlbox/devices/SignalHandler.h>
#include <controlbox/base/comsys/EventGenerator.h>
#include <controlbox/base/comsys/EventDispatcher.h>

#define DS_DEFAULT_SYSFSBASE	"/sys/bus/i2c/devices"
#define DS_MAX_ATTRIB_LENGTH	16
#define DS_DEFAULT_PA_INTRLINE	"4"

#define DS_MAX_ID_LENGTH	15
#define DS_MAX_VALUE_LENGTH	15
#define DS_MAX_DESC_LENGTH	63
#define DS_DESC_START		5

#define DS_MAX_PCA9555_SENSORS	2

/// us delay for the low-pass interrupt filter
#define DS_LOW_PASS_FILTER_DELAY 200

/// The sensor configuration entry base string
#define DS_CONF_BASE		"DigitalSensor_"
/// The first configuration parameter ID, appended to "DigitalSensor_"
#define DS_FIRST_ID		1
/// Maximum number of configuration parameters
#define DS_MAX_SENSORS		16
/// The code of an undefined event (event not to be reported to the remote server)
#define DS_EVENT_NULL		0x0

namespace controlbox {
namespace device {

/// A DeviceDigitalSensors is a CommandGenerator that...
/// blha blha blha<br>
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>DigitalSensor_[n]</b> - <i>Default: ""</i><br>
///		A digital sensor definition following this pattern:<br>
///		TODO explain the pattern
///	</li>
///	<li>
///	</li>
/// </ul>
/// @see CommandHandler
class DeviceDigitalSensors : public comsys::CommandGenerator,
				private comsys::EventDispatcher,
				public SignalHandler,
				public Device,
				public Worker {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

public:

    /// Handled Messages
    enum cmdType {
	DIGITAL_SENSORS_EVENT = (DEVICE_DS*SERVICES_RANGE)+1,
    };
    typedef enum cmdType t_cmdType;

    /// A sensor lable ID.
    typedef char t_dsId[DS_MAX_ID_LENGTH+1];

    /// An attrinute lable ID
    typedef char t_attrId[DS_MAX_ATTRIB_LENGTH+1];

    typedef unsigned short t_bus;
    typedef unsigned short t_address;
    typedef unsigned short t_port;
    /// The bit offset within a port.
    typedef u8 t_bitOffset;

    typedef u8 t_portStatus;

    enum digitalSensorProtocol {
        DS_PROTO_I2C = 0,
        DS_PROTO_SYSFS,
        DS_PROTO_GPIO
    };
    typedef enum digitalSensorProtocol t_digitalSensorProtocol;

    struct sysfsDigitalSensor {
        t_bus bus;			///> The I2C bus to use
        t_address address;		///> The chip address
        t_port port;			///> The port where the pin is mapped
        t_bitOffset bit;		///> The pin's bit within the port
    };
    typedef struct sysfsDigitalSensor t_sysfsDigitalSensor;

    union dsAddress {
        t_sysfsDigitalSensor sysfsAddress;		///> Addressing for sysfs access
    };
    typedef union dsAddress t_dsAddress;

    typedef u8 t_eventCode;

    /// A value lable ID.
    typedef char t_dsValue[DS_MAX_VALUE_LENGTH+1];

    typedef u8 t_eventParam;

    enum digitalSensorState {
	DS_STATE_LOW = 0,
	DS_STATE_HIGH,
	DS_STATE_UNK,			///> Unknowen state, if either the sensor
					///> is disabled or could not be read
    };
    typedef enum digitalSensorState t_digitalSensorState;
#define stateDescriptionArray(_name_)					\
	const char *_name_[] = {						\
		"Low",	\
		"High",				\
		"Unknown"		\
	};

    enum digitalSensorTrigger {
	DS_SIGNAL_NEVER	= 0x0,		///> Never signal state changes
	DS_SIGNAL_ON_LOW = 0x1,		///> Signal when the going low
	DS_SIGNAL_ON_HIGH = 0x2,	///> Signal when going high
	DS_SIGNAL_ON_BOTH = 0x3		///> Signal level change
    };
    typedef enum digitalSensorTrigger t_digitalSensorTrigger;

    struct digitalSensor {
	t_dsId id;				///> sensor ID
	t_digitalSensorProtocol proto;		///> the addressing protocol used
	t_dsAddress address;			///> the sensor address
	t_eventCode event;			///> the event monitored by the sensor [0 = don't generate an event]
	t_dsValue lvalue;			///> low value description
	t_dsValue hvalue;			///> high value description
	bool hasParam;				///> whatever this event has a associated parameter
	t_eventParam param;			///> an optional parameter associated to the event
	t_digitalSensorTrigger trigger;		///> event notify triggering
	bool enabled;				///> set if the sensor should be monitored
	unsigned short prio;				///> event priority @see WSProxyCommandHandler.h
	char description[DS_MAX_DESC_LENGTH+1];	///> sensor textual description
	t_digitalSensorState lastState;		///> last state
    };
    typedef struct digitalSensor t_digitalSensor;

    /// Maps t_dsId to sensor descriptior
    typedef map<std::string, t_digitalSensor*> t_dsMap;

    /// Maps bit position to sensors descriptior
    typedef map<t_bitOffset, t_digitalSensor*> t_bitMap;

    /// A sysfs attrbute associated to a sensor port.
    /// Each sensor port has an attribute and that type bind such attribute
    /// to its state and a configuration suitable to decode its bits.
    struct attribute {
	t_attrId id;				///> The attribute id
	t_portStatus status;			///> The port status
	t_bitMap bits;				///> Sensors bitmapping
	t_address addr;	// I2C Address
	t_port reg;	// Chip Command Register
    };
    typedef struct attribute t_attribute;

    /// A list of sensor attributes
    typedef map<std::string, t_attribute*> t_attrMap;

	/// A PCA9555 device input status.
	struct pca9555 {
		time_t timestamp;
		t_port port0;
		t_port port1;
	};
	typedef struct pca9555 t_pca9555;

	/// Address of sensors
	/// @note must match t_pcaSensors entries
	typedef t_address t_pcaAddr[DS_MAX_PCA9555_SENSORS];

	/// Status of sensors
	/// @note must match t_pcaSensorsAddrs entries
	typedef t_pca9555 * t_pcaStateVect;

	/// A list of readed t_pcaSensors
	typedef list<t_pcaStateVect> t_pcaStateList;


protected:

    static DeviceDigitalSensors * d_instance;

    /// The Configurator to use for getting configuration params
    Configurator & d_config;

    /// The event generator that monitor for new signals
    class DigitalSignalMonitor : protected EventGenerator {
        public:
        	DigitalSignalMonitor(EventDispatcher &disp);
        private:
        	/// The signal device used to receive interrupts
        	DeviceSignals * d_signals;
    };

    /// The signal device used to receive interrupts
    DeviceSignals * d_signals;

    /// The I2CBus adapter to use to query the device
    DeviceI2CBus * d_i2cBus;

    /// The Time Device to use
    DeviceTime * d_time;

    /// The sysfs base configuration path
    std::string d_sysfsbase;

//    /// The interrupt line to use (on PortA)
//     unsigned short d_intrLine;

    /// The map of configured digital sensors
    t_dsMap sensors;

    /// The map of confgiured attributes
    t_attrMap attrbutes;

	/// Address of PCA devices
	t_pcaAddr d_pcaAddrs;

	/// Number of PCA9555 devices loaded
	unsigned short d_pcaDevices;

	/// The list of PCA9555 sensors reads
	t_pcaStateList d_pcaStateList;

	/// The next state to be notified
	t_pcaStateVect d_pcaNextState;

	/// Number of notifies received
	unsigned int d_notifies;


    /// The logger to use locally.
    log4cpp::Category & log;

//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
public:

    /// Return an instance of an Analo Sensor Device
    static DeviceDigitalSensors * getInstance(std::string const & logName = "DeviceDS");

    /// Class destructor.
    ~DeviceDigitalSensors();

    inline unsigned count();

    /// Return the actual state for the required sensor.
    /// @param dsId the sensor ID to query
    /// @return -1 on error, the current state otherwise
    t_digitalSensorState state(t_dsId dsId);

protected:

    /// Create a new DeviceDigitalSensors.
    DeviceDigitalSensors(std::string const & logName);

    inline exitCode loadDigitalEventsDescription(void);

    inline exitCode loadSensorConfiguration(void);

    inline DeviceDigitalSensors::t_digitalSensor * parseCfgString(std::string const & dsCfg);

    inline DeviceDigitalSensors::t_attribute * confAttribute(DeviceDigitalSensors::t_digitalSensor * pDs);

    inline bool validateParams(DeviceDigitalSensors::t_digitalSensor * pDs);

    inline void logSensorParams(DeviceDigitalSensors::t_digitalSensor * pDs);

    inline t_digitalSensor * findById (t_dsId dsId);

    inline t_attribute * findAttrById (t_attrId attrId);

    inline t_digitalSensor * lookUpBit (t_bitMap &map, t_bitOffset bit);

    inline exitCode getPortStatus(t_attribute const & anAttr, t_portStatus & value);

    inline exitCode getPortNextStatus(t_attribute const & anAttr, t_portStatus & value);

    /// Initialize the current sensors status.
    inline exitCode initSensors(void);

    inline exitCode notifySensorEvent(t_digitalSensor & aSensor);

    inline exitCode notifySensorChange(t_digitalSensor & aSensor, t_digitalSensorState state);

    inline exitCode checkPortStatusChange(t_attribute & anAttr);



    inline exitCode updateSensors(void);

    /// Digital sensors reads queuing
    void signalNotify(void);


    void sensorsNotify (void);

    /// Digital sensors reads processing
    void run(void);


};


} //namespace device
} //namespace controlbox
#endif

