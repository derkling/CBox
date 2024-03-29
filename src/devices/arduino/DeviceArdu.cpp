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


#include "DeviceArdu.ih"

namespace controlbox {
namespace device {

DeviceArdu * DeviceArdu::d_instance = 0;

const char *DeviceArdu::d_sysfsParams[] = {
	"reg_event",
	"odo_ppm",
	"odo_pcount",
	"odo_totm",
	"odo_freq",
	"odo_speed",
	"odo_speedLimit",
	"odo_decelLimit",
	"gps_lat",
	"gps_lon",
	"gps_utc",
	"gps_val",
	"gps_kmh",
	"gps_dir",
	"gps_fix",
	"gps_pdop",
	"gps_hdop",
	"gps_vdop",
	"gps_date",
	"gps_knots",
	"gps_var",
};


DeviceArdu * DeviceArdu::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceArdu(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceArdu::getInstance()");
	return d_instance;
}

DeviceArdu::DeviceArdu(std::string const & logName) :
	CommandGenerator(logName),
	Device(Device::DEVICE_ARDU, 0, logName),
	d_config(Configurator::getInstance()),
	d_notifies(0),
	log(Device::log) {

	DeviceFactory * df = DeviceFactory::getInstance();
	std::string buf;


	LOG4CPP_DEBUG(log, "DeviceArdu(const std::string &, bool)");

	// Registering device and exported interfaces into the DeviceDB
	dbReg();
	dbRegInterface(Device::DEVICE_GPS);
	dbRegInterface(Device::DEVICE_ODO);

	d_signals = df->getDeviceSignals();	// Needed to sens ODO alarms
	d_time = df->getDeviceTime();		// Needed to send alarm messages

	d_sysfspath = d_config.param("Odometer_sysfsbase", ARDU_DEFAULT_SYSFSBASE);
	d_sysfspath += "/" + d_config.param("Odometer_i2cAddress", ARDU_DEFAULT_I2CADDR);
	LOG4CPP_INFO(log, "Sysfspath [%s]", d_sysfspath.c_str());

	d_intrLine = atoi(d_config.param("Odometer_PA_intrline", ARDU_DEFAULT_PA_INTRLINE).c_str());
	LOG4CPP_INFO(log, "Using PA interrupt line [%d]", d_intrLine);

	d_ppm = atoi(d_config.param("Odometer_ppm", ARDU_DEFAULT_PPM).c_str());
	LOG4CPP_INFO(log, "PPM [%d]", d_ppm);

	d_highSpeedAlarm = (unsigned)strtoul(
		d_config.param("Odometer_hiSpeedAlarm", ARDU_DEFAULT_HIGHSPEED_ALARM).c_str(),
		(char **)NULL, 10);

	d_emergencyBreakAlarm = (unsigned)strtoul(
		d_config.param("Odometer_emergencyBreak", ARDU_DEFAULT_EMERGENCY_BREAK).c_str(),
		(char **)NULL, 10);

	d_initDistance = strtoul(
		d_config.param("Odometer_initDistance", ARDU_DEFAULT_INIT_DISTANCE).c_str(),
		(char **)NULL, 10);

// 	d_factors[MS] = 1 / (1e6 * d_ppm);
// 	d_factors[KMH] = 3.6 / (1e6 * d_ppm);

	initOdometer();

}

DeviceArdu::~DeviceArdu() {
/*
	LOG4CPP_INFO(log, "Stopping DeviceArdu");*/

}


inline exitCode
DeviceArdu::initOdometer(void) {

	// Configuring PPM

	// Configuring initial distance

	// Configuring Alarms triggers

	// Enabling Odometer

	return OK;

}

double
DeviceArdu::lat() {
	unsigned long llat;

	if (getSysfsValue(GPS_LAT, llat) != OK)
		return 99.9999;

	return ((double)llat)/ARDU_ROUND_FACTOR;
}

double
DeviceArdu::lon() {
	unsigned long llon = 0;

	if (getSysfsValue(GPS_LON, llon) != OK)
		return 999.9999;

	return ((double)llon)/ARDU_ROUND_FACTOR;
}



//----- DeviceOdo interface implementation -------------------------------------

double
DeviceArdu::odoSpeed(t_speedUnits unit) {
	unsigned long speed = 0;

	if (getSysfsValue(ODO_SPEED, speed) != OK)
		return 0;

	return ((double)speed)/ARDU_ROUND_FACTOR;

}

double
DeviceArdu::distance(t_distUnits unit) {
	unsigned long dist = 0;

	if (getSysfsValue(ODO_TOTM, dist) != OK)
		return 0;

	return ((double)dist);

}

double
DeviceArdu::speedAlarm(t_speedUnits unit) {
	unsigned long sa;

	if (getSysfsValue(ODO_SPEEDALARM, sa) != OK)
		return 0;

	return ((double)sa)/ARDU_ROUND_FACTOR;
}

double
DeviceArdu::emergencyBreakAlarm(t_speedUnits unit) {
	unsigned long emergencyBreak;

	if (getSysfsValue(ODO_BREAKALARM, emergencyBreak) != OK)
		return 0;

	return ((double)emergencyBreak)/ARDU_ROUND_FACTOR;
}

exitCode
DeviceArdu::setDistance(double dist, t_distUnits unit) {
	return OK;
}

exitCode
DeviceArdu::setSpeedAlarm(double speed, t_speedUnits unit) {
	return OK;
}

exitCode
DeviceArdu::setEmergencyBreakAlarm(double speed, t_speedUnits unit) {
	return OK;
}


//----- DeviceGPS interface implementation -------------------------------------

unsigned
DeviceArdu::fixStatus() {
	unsigned long fix;

	if (getSysfsValue(GPS_FIX, fix) != OK)
		return 0;

	return fix;
}

string
DeviceArdu::latitude(bool iso) {
	double llat = lat();
	char buff[10];

	if (iso) {
		sprintf(buff, "%+06.4f", llat);
		return string(buff);
	}

	if (llat>=0) {
		sprintf(buff, "%06.4f E", llat);
	} else {
		sprintf(buff, "%06.4f W", -llat);
	}

	return string(buff);
}

string
DeviceArdu::longitude(bool iso) {
	double llon = lon();
	char buff[11];

	if (iso) {
		sprintf(buff, "%+07.4f", llon);
		return string(buff);
	}

	if (llon>=0) {
		sprintf(buff, "%07.4f N", llon);
	} else {
		sprintf(buff, "%07.4f S", -llon);
	}

	return string(buff);
}

double
DeviceArdu::gpsSpeed(DeviceGPS::t_measureSystems system) {
	unsigned long speed;

	switch (system) {
	case DEVICEGPS_UMS_ISO:
		if (getSysfsValue(GPS_KMH, speed) != OK)
			speed = 0;
		break;
	case DEVICEGPS_UMS_MARINE:
		if (getSysfsValue(GPS_KNOTS, speed) != OK)
			speed = 0;
		break;
	}
	return ((double)speed)/ARDU_ROUND_FACTOR;
};

unsigned
DeviceArdu::course(DeviceGPS::t_courseType type) {
	unsigned long dir;

	if (getSysfsValue(GPS_DIR, dir) != OK)
		return 0;

	return dir;
}

string
DeviceArdu::time(bool utc) {
	return "";
}



//----- Event management -------------------------------------------------------

exitCode
DeviceArdu::notifyOdoEvent(unsigned short event) {
	comsys::Command * cOdoEvent;
	int eventCode;
	DeviceOdometer::t_cmdType cmdType;

	switch (event) {
	case MOVE:
		cmdType = DeviceOdometer::ODOMETER_EVENT_MOVE;
		eventCode = 0x0;
		return ARDU_EVENT_DISABLED;
		break;
	case STOP:
		cmdType = DeviceOdometer::ODOMETER_EVENT_STOP;
		eventCode = 0x0;
		return ARDU_EVENT_DISABLED;
		break;
	case OVER_SPEED:
		cmdType = DeviceOdometer::ODOMETER_EVENT_OVER_SPEED;
		eventCode = 0x17;
		cOdoEvent->setParam( "value", odoSpeed() );
		break;
	case EMERGENCY_BREAK:
		cmdType = DeviceOdometer::ODOMETER_EVENT_EMERGENCY_BREAK;
		eventCode = 0x23;
		cOdoEvent->setParam( "value", 0 );
		break;
	default:
		LOG4CPP_DEBUG(log, "Not an ODO event");
		return ARDU_EVENT_UNDEFINED;
	}

	if ( event <= EMERGENCY_BREAK) {
		cOdoEvent = comsys::Command::getCommand(cmdType,
				Device::DEVICE_ODO, "DEVICE_ODO",
				log.getName());
	} else {
		//TODO here goes the code for GPS events notification
	}

	if ( !cOdoEvent ) {
		LOG4CPP_FATAL(log, "Unable to build a new Command");
		return OUT_OF_MEMORY;
	}
	cOdoEvent->setParam( "timestamp", d_time->time() );
	cOdoEvent->setParam( "event",   eventCode);

	// Notifying command
	notify(cOdoEvent);

	return OK;

}

exitCode
DeviceArdu::checkAlarms(void) {
	unsigned long reg;
	unsigned short eventIdx;

	if (getSysfsValue(REG_EVENT, reg) != OK) {
		LOG4CPP_ERROR(log, "Reading Ardu event register failed");
		return ARDU_PORT_READ_ERROR;
	}

	LOG4CPP_DEBUG(log, "Ardu event register [0x%X]", reg);
	for (eventIdx=0x1; reg; eventIdx<<1) {
		LOG4CPP_DEBUG(log, "Checking for event []");
		if (reg & eventIdx) {
			notifyOdoEvent(eventIdx);
			reg &= ~eventIdx;
		}
	}

	return OK;
}

exitCode
DeviceArdu::getSysfsValue(t_sysfsValue idx, unsigned long & value) {
	std::ostringstream path("");
	int fd;
	char svalue[ARDU_SYSFS_ATTRIB_BUFSIZE];

	path << d_sysfspath << "/" << d_sysfsParams[idx];

	fd = ::open(path.str().c_str(), O_RDONLY);
	if (fd == -1) {
		LOG4CPP_ERROR(log, "Failed to open attrib [%s]", path.str().c_str());
		return ARDU_PORT_READ_ERROR;
	}

	::read(fd, svalue, ARDU_SYSFS_ATTRIB_BUFSIZE);
	errno = 0;
	value = ::strtoul(svalue, (char **)NULL, 10);
	if (errno) {
		LOG4CPP_ERROR(log, "string value [%s] conversion failed, %s",
				svalue, strerror(errno));
		::close(fd);
		return ARDU_VALUE_CONVERSION_FAILED;
	}

	LOG4CPP_DEBUG(log, "Reading attribute [%s = %lu]", path.str().c_str(), value);

	::close(fd);
	return OK;
}

void DeviceArdu::signalNotify(void) {

}

void DeviceArdu::run (void) {

	LOG4CPP_INFO(log, "working thread started");

	// Registering signal
	sigInstall(SIGPOLL);
	d_signals->registerHandler((DeviceSignals::t_interrupt)d_intrLine, this, name().c_str());

	while (true) {

		LOG4CPP_DEBUG(log, "Waiting for interrupt...");
		waitSignal(SIGPOLL);

		LOG4CPP_DEBUG(log, "Interrupt received [%d]", ++d_notifies);
		checkAlarms();

	}
}

}// namespace device
}// namespace controlbox
