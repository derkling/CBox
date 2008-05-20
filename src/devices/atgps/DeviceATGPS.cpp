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


#include "DeviceATGPS.ih"

namespace controlbox {
namespace device {

DeviceATGPS * DeviceATGPS::d_instance = 0;

// NOTE these values must match t_atgpsCmds enum definition
char *DeviceATGPS::d_atgpsCmds[] = {
	"+QER\0",
	"+QCM\0",
	"+QEC\0",
	"\0",
	"+OCP\0",
	"\0",
	"+OFP\0",
	"\0",
	"+ASL\0",
	"+AEB\0",
	"+GLO\0",
	"+GLA\0",
	"\0",		// gps_utc
	"\0",		// gps_val
	"+GGS\0",
	"+GTD\0",	// gps_dir
	"+GFV\0",
	"\0",		// gps_pdop
	"\0",		// gps_hdop
	"\0",		// gps_vdop
	"\0",		// gps_date
	"\0",		// gps_knots
	"\0",		// gps_var
};


DeviceATGPS * DeviceATGPS::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceATGPS(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceATGPS::getInstance()");
	return d_instance;
}

DeviceATGPS::DeviceATGPS(std::string const & logName) :
	CommandGenerator(logName),
	Device(Device::DEVICE_ATGPS, 0, logName),
	d_config(Configurator::getInstance()),
	d_tty(0),
	log(Device::log) {

	DeviceFactory * df = DeviceFactory::getInstance();
	std::string buf;

	LOG4CPP_DEBUG(log, "DeviceATGPS(const std::string &, bool)");

	// Registering device and exported interfaces into the DeviceDB
	dbReg();
	dbRegInterface(Device::DEVICE_GPS);
	dbRegInterface(Device::DEVICE_ODO);

	d_signals = df->getDeviceSignals();	// Needed to sens device alarms
	d_time = df->getDeviceTime();		// Needed to send alarm messages

	d_tty = new DeviceSerial(std::string("device_atgps"));
	if (!d_tty) {
		LOG4CPP_FATAL(log, "Failed to build a DeviceSerial");
		throw new exceptions::SerialDeviceException("Unable to build a SerialDevice");
	}

	d_intrLine = atoi(d_config.param("device_atgps_intr_PA_pin", ATGPS_DEFAULT_PA_INTRLINE).c_str());
	LOG4CPP_DEBUG(log, "Using PA interrupt line [%d]", d_intrLine);

	d_ppm = atoi(d_config.param("device_atgps_ppm", ATGPS_DEFAULT_PPM).c_str());
	LOG4CPP_INFO(log, "PPM [%d]", d_ppm);

	d_highSpeedAlarm = (unsigned)strtoul(
		d_config.param("device_atgps_hiSpeedAlarm", ATGPS_DEFAULT_HIGHSPEED_ALARM).c_str(),
		(char **)NULL, 10);
	LOG4CPP_INFO(log, "HighSpeedAlarm [%d m/s]", d_highSpeedAlarm);

	d_emergencyBreakAlarm = (unsigned)strtoul(
		d_config.param("device_atgps_emergencyBreak", ATGPS_DEFAULT_EMERGENCY_BREAK).c_str(),
		(char **)NULL, 10);
	LOG4CPP_INFO(log, "EmergencyBreakAlarm [%d m/s^2]", d_emergencyBreakAlarm);

	d_initDistance = strtoul(
		d_config.param("device_atgps_initDistance", ATGPS_DEFAULT_INIT_DISTANCE).c_str(),
		(char **)NULL, 10);
	LOG4CPP_INFO(log, "InitDistance [%d m]", d_initDistance);

	initDevice();

}

DeviceATGPS::~DeviceATGPS() {

	d_tty->closeSerial();
	delete(d_tty);

	LOG4CPP_INFO(log, "Stopping DeviceATGPS");

}

// inline exitCode
// DeviceATGPS::openDevice(void) {
//
// 	if (!d_tty->isOpen()) {
// 		LOG4CPP_DEBUG(log, "Device already open");
// 		return OK;
// 	}
//
// 	// Opening TTY port
// 	LOG4CPP_DEBUG(log, "Opening device...");
// 	d_tty->openSerial(false);
// 	if ( !d_tty->isOpen() ) {
// 		LOG4CPP_FATAL(log, "Failed opening TTY port");
// 		throw new exceptions::SerialDeviceException("Unable to open TTY port");
// 	}
//
// 	return OK;
// }

inline exitCode
DeviceATGPS::initDevice(void) {
	exitCode result;
	char buf[DEVICE_ATGPS_RXTX_BUFFER];
	int len;
	unsigned long events;

	// Opening TTY port
	LOG4CPP_DEBUG(log, "Opening device...");
	result = d_tty->openSerial(false);
	if (result)
		return result;
// 	if ( !d_tty->isOpen() ) {
// 		LOG4CPP_FATAL(log, "Failed opening TTY port");
// 		throw new exceptions::SerialDeviceException("Unable to open TTY port");
// 	}

	//FIXME we should use a flush method
	// Reading all pending output...
	//d_tty->readSerial(buf, len);

// snprintf(buf, DEVICE_ATGPS_RXTX_BUFFER,
// 	"+QCM=0+OCP=%lu+ASL=%lu+AEB=%lu+QER");


	// Disabling TTY detached mode to optimize port usage
	d_tty->detachedMode(false);

	// Disabling continuous monitoring
	setDeviceValue(REG_MONITOR, "0\0");

	//TODO Disabling command echo
	setDeviceValue(REG_ECHO, "0\0");

	// Configuring initial distance
	snprintf(buf, DEVICE_ATGPS_RXTX_BUFFER, "%lu\0", d_initDistance*d_ppm);
	setDeviceValue(ODO_PCOUNT, buf);

	// Configuring Alarms triggers
	snprintf(buf, DEVICE_ATGPS_RXTX_BUFFER, "%lu\0", d_highSpeedAlarm*d_ppm);
	setDeviceValue(ODO_SPEEDALARM, buf);

	snprintf(buf, DEVICE_ATGPS_RXTX_BUFFER, "%lu\0", d_emergencyBreakAlarm*d_ppm);
	setDeviceValue(ODO_BREAKALARM, buf);

	// Enabling Odometer

	// Resetting event register
	if (getDeviceValue(REG_EVENT, events) != OK) {
		LOG4CPP_ERROR(log, "Reading event register failed");
		return ATGPS_PORT_READ_ERROR;
	}

	// Recovering TTY detached mode
	d_tty->detachedMode();

	LOG4CPP_DEBUG(log, "ATGPS event register [0x%X]", events);

	return OK;

}

double
DeviceATGPS::lat() {
	double lat = 0;
	long deg;
	float min;
#ifdef CONTROLBOX_DEBUG
	char buf[256];
#endif

	if (getDeviceValue(GPS_LAT, lat) != OK) {
		LOG4CPP_DEBUG(log, "Lat Not Available");
		return 99.9999;
	}

	return lat;

// 	deg = (long)floor(lat/100);
// 	min = (lat-(deg*100))/60;
//
// #ifdef CONTROLBOX_DEBUG
// 	snprintf(buf, 256, "NMEA: %g, deg: %ld, min: %f", lat, deg, min);
// 	LOG4CPP_DEBUG(log, "%s", buf);
// #endif
//
// 	return deg + min;
}

double
DeviceATGPS::lon() {
	double lon = 0;
	long deg;
	float min;
#ifdef CONTROLBOX_DEBUG
	char buf[256];
#endif

	if (getDeviceValue(GPS_LON, lon) != OK) {
		LOG4CPP_DEBUG(log, "Lon Not Available");
		return 999.9999;
	}

	return lon;

// 	deg = (long)floor(lon/100);
// 	min = (lon-(deg*100))/60;
//
// #ifdef CONTROLBOX_DEBUG
// 	snprintf(buf, 256, "NMEA: %g, deg: %ld, min: %f", lon, deg, min);
// 	LOG4CPP_DEBUG(log, "%s", buf);
// #endif
//
// 	return deg + min;
}



//----- DeviceOdo interface implementation -------------------------------------

double
DeviceATGPS::odoSpeed(t_speedUnits unit) {
	unsigned long speed = 0;

	if (getDeviceValue(ODO_SPEED, speed) != OK)
		return 0;

	return speed;

}

double
DeviceATGPS::distance(t_distUnits unit) {
	unsigned long distance = 0;

	if (getDeviceValue(ODO_TOTM, distance) != OK)
		return 0;

	return distance;

}

double
DeviceATGPS::speedAlarm(t_speedUnits unit) {
// 	unsigned long speedAlarm;
//
// 	if (getDeviceValue(ODO_SPEEDALARM, speedAlarm) != OK)
// 		return 0;
//
// 	return ((double)speedAlarm)/ATGPS_ROUND_FACTOR;
}

double
DeviceATGPS::emergencyBreakAlarm(t_speedUnits unit) {
// 	unsigned long emergencyBreak;
//
// 	if (getSysfsValue(ODO_BREAKALARM, emergencyBreak) != OK)
		return 0;

// 	return ((double)emergencyBreak)/ATGPS_ROUND_FACTOR;
}

exitCode
DeviceATGPS::setDistance(double distance, t_distUnits unit) {
	return OK;
}

exitCode
DeviceATGPS::setSpeedAlarm(double speed, t_speedUnits unit) {
	return OK;
}

exitCode
DeviceATGPS::setEmergencyBreakAlarm(double speed, t_speedUnits unit) {
	return OK;
}


//----- DeviceGPS interface implementation -------------------------------------

DeviceGPS::t_fixStatus
DeviceATGPS::fixStatus() {
	unsigned long fix;

	if (getDeviceValue(GPS_FIX, fix) != OK)
		return DeviceGPS::DEVICEGPS_FIX_NA;

	switch(fix) {
	case 1:
		return DeviceGPS::DEVICEGPS_FIX_ASSIST;
	case 2:
		return DeviceGPS::DEVICEGPS_FIX_2D;
	case 3:
		return DeviceGPS::DEVICEGPS_FIX_3D;
	}

	return DeviceGPS::DEVICEGPS_FIX_NA;
}

string
DeviceATGPS::latitude(bool iso) {
	double latitude = lat();
	char buff[] = "99.9999 E\0";

	if (iso) {
		sprintf(buff, "%+08.4f", latitude);
		LOG4CPP_DEBUG(log, "===> LAT: [%s]", buff);
		return string(buff);
	}

	if (latitude>=0) {
		sprintf(buff, "%7.4f E", latitude);
	} else {
		sprintf(buff, "%7.4f W", -latitude);
	}

	return string(buff);
}

string
DeviceATGPS::longitude(bool iso) {
	double longitude = lon();
	char buff[] = "999.9999 N\0";

	if (iso) {
		sprintf(buff, "%+09.4f", longitude);
		LOG4CPP_DEBUG(log, "===> LON: [%s]", buff);
		return string(buff);
	}

	if (longitude>=0) {
		sprintf(buff, "%9.4f N", longitude);
	} else {
		sprintf(buff, "%9.4f S", -longitude);
	}

	return string(buff);
}

double
DeviceATGPS::gpsSpeed(DeviceGPS::t_measureSystems system) {
	double speed = 0;

	if (getDeviceValue(GPS_KMH, speed) != OK)
		return 0;

	return speed;
};

unsigned
DeviceATGPS::course(DeviceGPS::t_courseType type) {
	unsigned long dir = 0;

//TODO update DERKGPS firmware
LOG4CPP_WARN(log, "GPS Direction: update derkgps firwmare");
	return 0;

	if (getDeviceValue(GPS_DIR, dir) != OK)
		return 0;

	return (unsigned)dir;
}

string
DeviceATGPS::time(bool utc) {
	return "";
}

//----- Device Access Methods --------------------------------------------------

inline exitCode
DeviceATGPS::getDeviceLocalValue(t_atgpsCmds idx, char * buf, int & len) {
	exitCode result = OK;
	unsigned long val;

	switch(idx) {
	case ODO_PPM:
		len = snprintf(buf, len, "%d", d_ppm);
		break;

	case ODO_TOTM:
		result = getDeviceValue(ODO_PCOUNT, val);
		if ( result != OK )
			return result;

		len = snprintf(buf, len, "%lu", val/d_ppm);
		break;

	case ODO_SPEED:
		result = getDeviceValue(ODO_FREQ, val);
		if ( result != OK )
			return result;

		len = snprintf(buf, len, "%lu", val/d_ppm);
		break;
	default:
		LOG4CPP_WARN(log, "Command not supported [%d]", idx);
		return ATGPS_CMD_UNDEF;
	}

	return result;
}

// inline exitCode
// DeviceATGPS::getDeviceRemoteValue(t_atgpsCmds idx, char * buf, int & len) {
// 	exitCode result = OK;
// 	DeviceSerial::t_stringVector resp;
//
// 	LOG4CPP_DEBUG(log, "SEND [%s]", d_atgpsCmds[idx]);
// 	result = d_tty->sendSerial(d_atgpsCmds[idx], &resp);
// 	if ( result != OK ) {
// 		LOG4CPP_WARN(log, "TTY communication error");
// 		buf[0] = 0; len = 0;
// 		return result;
// 	}
//
// #if 0
// 	d_tty->sendSerial(d_atgpsCmds[idx], strlen(d_atgpsCmds[idx]));
//
// 	//NOTE if len==0: nothing readed or device not responding...
// 	result = d_tty->readSerial(buf, len);
// 	if ( result != OK ) {
// 		LOG4CPP_WARN(log, "TTY communication error");
// 		buf[0] = 0; len = 0;
// 		return result;
// 	}
// #endif
//
// 	snprintf(buf, len, "%s\0", resp[0].c_str());
//
// // 	if ( strlen(resp[0].c_str()) < len )
// // 		len = strlen(resp[0].c_str());
// //
// // 	if (len == 0) {
// // 		buf[0] = 0;
// // 	} else {
// // 		snprintf(buf, len, "%s\0" resp[0].c_str());
// // 	}
// 	LOG4CPP_DEBUG(log, "READ [%s]", buf);
//
// 	return result;
// }
inline exitCode
DeviceATGPS::getDeviceRemoteValue(t_atgpsCmds idx, char * buf, int & len) {
	exitCode result = OK;
	char cmd[8];


	LOG4CPP_DEBUG(log, "===> [%s]", d_atgpsCmds[idx]);

	d_tty->detachedMode(false);
	d_tty->sendSerial(d_atgpsCmds[idx], strlen(d_atgpsCmds[idx]));
	d_tty->readSerial(buf, len);
	d_tty->detachedMode();

	LOG4CPP_DEBUG(log, "<=== [%s]", buf);

	return result;
}


exitCode
DeviceATGPS::getDeviceValue(t_atgpsCmds idx, char * buf, int & len) {
	exitCode result;

	if ( d_atgpsCmds[idx][0] == 0 ) {
		// Local command
		return getDeviceLocalValue(idx, buf, len);
	}

	// Device command
	return getDeviceRemoteValue(idx, buf, len);

}

exitCode
DeviceATGPS::getDeviceValue(t_atgpsCmds idx, int & value) {
	char buf[DEVICE_ATGPS_RXTX_BUFFER];
	exitCode result;
	int len = DEVICE_ATGPS_RXTX_BUFFER;

	result = getDeviceValue(idx, buf, len);
	if (result != OK )
		return result;

	if ( sscanf(buf, "%d", &value) == 0) {
		// The returned string is not a number
		return CONVERSION_ERROR;
	}

	return OK;
}

exitCode
DeviceATGPS::getDeviceValue(t_atgpsCmds idx, unsigned long & value) {
	char buf[DEVICE_ATGPS_RXTX_BUFFER];
	exitCode result;
	int len = DEVICE_ATGPS_RXTX_BUFFER;

	result = getDeviceValue(idx, buf, len);
	if (result != OK )
		return result;

	if ( sscanf(buf, "%lu", &value) == 0) {
		// The returned string is not a number
		return CONVERSION_ERROR;
	}

	return OK;
}

exitCode
DeviceATGPS::getDeviceValue(t_atgpsCmds idx, double & value) {
	char buf[DEVICE_ATGPS_RXTX_BUFFER];
	exitCode result;
	int len = DEVICE_ATGPS_RXTX_BUFFER;

	result = getDeviceValue(idx, buf, len);
	if (result != OK )
		return result;

	len = sscanf(buf, "%lf", &value);
	if ( len == 0 ||
		len == EOF) {
		LOG4CPP_DEBUG(log, "Conversion error, %s", strerror(errno));
		// The returned string is not a number
		return CONVERSION_ERROR;
	}

	return OK;
}

//NOTE we suppose a maximum param len of 9 bytes + the '='
exitCode
DeviceATGPS::setDeviceValue(t_atgpsCmds idx, const char * value) {
	char buf[DEVICE_ATGPS_RXTX_BUFFER];
	int len;
	DeviceSerial::t_stringVector resp;
	exitCode result;


	len = snprintf(buf, DEVICE_ATGPS_RXTX_BUFFER, "%s", d_atgpsCmds[idx]);
	if (len<0 ||
		len > (DEVICE_ATGPS_RXTX_BUFFER-10) ) {
		LOG4CPP_ERROR(log, "Buffers too small @ %s:%d - %s", __FILE__, __LINE__, __FUNCTION__);
		return BUFFER_OVERFLOW;
	}

	// Appending "=" and the buf content
	if ( snprintf(buf+len, 10, "=%s", value) <= 0 ) {
		LOG4CPP_ERROR(log, "Error on command building");
		return BUFFER_OVERFLOW;
	}

	len += strlen(value)+1;
	if (len >= DEVICE_ATGPS_RXTX_BUFFER) {
		LOG4CPP_ERROR(log, "Error on command building");
		return BUFFER_OVERFLOW;
	}

	buf[len] = 0;

	LOG4CPP_DEBUG(log, "Sending AT command [%s]", buf);
	d_tty->sendSerial(buf, len);
	d_tty->readSerial(buf, len);


// 	d_tty->sendSerial(buf, &resp);



	return OK;
}

//----- Event Management Methods -----------------------------------------------

exitCode
DeviceATGPS::notifyEvent(unsigned short event) {
	comsys::Command * cSgd;
	int eventCode;
	std::ostringstream sbuf("");

	switch (event) {
	case MOVE:
		LOG4CPP_DEBUG(log, "MOVE event is DISABLED");
		return ATGPS_EVENT_DISABLED;
	case STOP:
		LOG4CPP_DEBUG(log, "STOP event is DISABLED");
		return ATGPS_EVENT_DISABLED;
	case OVER_SPEED:
		eventCode = 0x17;
		sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << odoSpeed();
		break;
	case EMERGENCY_BREAK:
		eventCode = 0x23;
		sbuf << "0";
		break;
	default:
		LOG4CPP_DEBUG(log, "Not an ODO event");
		return ATGPS_EVENT_UNDEFINED;
	}

	if ( event <= EMERGENCY_BREAK) {
		cSgd = comsys::Command::getCommand(DeviceOdometer::ODOMETER_EVENT,
				Device::DEVICE_ODO, "DEVICE_ODO",
				log.getName());
	} else {
		//TODO here goes the code for GPS events notification
	}

	if ( !cSgd ) {
		LOG4CPP_FATAL(log, "Unable to build a new Command");
		return OUT_OF_MEMORY;
	}

	cSgd->setParam("dist_evtData", sbuf.str() );

	sbuf.str("");
	sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)eventCode;
	cSgd->setParam( "dist_evtType", sbuf.str());
// 	cmd->setParam("dist_evtType", eventCode);

	cSgd->setParam("timestamp", d_time->time() );

	// Notifying command
	notify(cSgd);

	return OK;

}

exitCode
DeviceATGPS::checkAlarms(bool notify) {
	unsigned long reg = 0x0;
	unsigned short eventIdx;

	if (getDeviceValue(REG_EVENT, reg) != OK) {
		LOG4CPP_ERROR(log, "Reading event register failed");
		return ATGPS_PORT_READ_ERROR;
	}

	LOG4CPP_DEBUG(log, "ATGPS event register [0x%04X]", reg);

	if (!notify) {
		LOG4CPP_DEBUG(log, "Notification disabled");
		return OK;
	}

	for (eventIdx=0x1; reg; eventIdx<<1) {
		LOG4CPP_DEBUG(log, "Checking for event [0x%08X]", eventIdx);
		if (reg & eventIdx) {
			notifyEvent(eventIdx);
			reg &= ~eventIdx;
		}
	}

	return OK;
}

void DeviceATGPS::run (void) {
	pid_t tid;
	int notifies = 0;

	tid = (long) syscall(SYS_gettid);
	LOG4CPP_DEBUG(log, "working thread [%lu=>%lu] started", tid, pthread_self());

	// Registering signal
	//setSignal(SIGCONT,true);
	sigInstall(SIGCONT);
	d_signals->registerHandler((DeviceSignals::t_interrupt)d_intrLine, this, SIGCONT, name().c_str());

	checkAlarms(false);

	while (true) {

		LOG4CPP_DEBUG(log, "Waiting for interrupt...");
		waitSignal(SIGCONT);

		LOG4CPP_DEBUG(log, "Interrupt received [%d]", ++notifies);
		checkAlarms();

	}
}

}// namespace device
}// namespace controlbox
