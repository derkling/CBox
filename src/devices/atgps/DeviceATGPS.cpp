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
	// 0
	"+QER\0",	// reg_event_register
	"+QCM\0",	// reg_continuous_mode
	"\0",
	"+OCP\0",	// odo_count_pulses
	"\0",
	// 5
	"+OFP\0",	// odo_freq_pulses
	"\0",
	"+ASL\0",	// odo_speed_limit
	"+AEB\0",	// odo_emergency_break
	"+GLO\0",	// gps_lon
	// 10
	"+GLA\0",	// gps_lat
	"\0",		// gps_utc
	"\0",		// gps_val
	"+GGS\0",	// gps_speed
	"+GTD\0",	// gps_dir
	// 15
	"+GFV\0",	// gps_speed_value
	"\0",		// gps_pdop
	"\0",		// gps_hdop
	"\0",		// gps_vdop
	"\0",		// gps_date
	// 20
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

	LOG4CPP_DEBUG(log, "Stopping DeviceATGPS");

}

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

	// Disabling TTY detached mode to optimize port usage
	d_tty->detachedMode(false);

	// Disabling continuous monitoring
	setDeviceValue(REG_MONITOR, "0\0");

	//TODO Disabling command echo
// 	setDeviceValue(REG_ECHO, "0\0");

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
	checkAlarms(false);

	// Recovering TTY detached mode
	d_tty->detachedMode();

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

}



//----- DeviceOdo interface implementation -------------------------------------

double
DeviceATGPS::odoSpeed(t_speedUnits unit) {
	unsigned long speed = 0;

	if (getDeviceValue(ODO_SPEED, speed) != OK)
		return 0;

	if ( unit == DeviceOdometer::KMH ) {
		return (speed*3.6);
	}

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

unsigned
DeviceATGPS::fixStatus() {
	unsigned long fix;

	if (getDeviceValue(GPS_FIX, fix) != OK)
		return 0;

	return fix;
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

// //TODO update DERKGPS firmware
// #warning "GPS Direction: update derkgps firwmare"
// LOG4CPP_DEBUG(log, "!!!!!!!!!GPS Direction: update derkgps firwmare!!!!!!!!!!");
// 	return 0;

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
		LOG4CPP_DEBUG(log, "ODO_PPM [%s]", buf);
		break;

	case ODO_TOTM:
		result = getDeviceValue(ODO_PCOUNT, val);
		if ( result != OK )
			return result;

		len = snprintf(buf, len, "%lu", val/d_ppm);
		LOG4CPP_DEBUG(log, "ODO_TOTM [%s]", buf);
		break;

	case ODO_SPEED:
		result = getDeviceValue(ODO_FREQ, val);
		if ( result != OK )
			return result;

		len = snprintf(buf, len, "%lu", val/d_ppm);
		LOG4CPP_DEBUG(log, "ODO_SPEED [%s]", buf);
		break;
	default:
		LOG4CPP_WARN(log, "Command not supported [%d]", idx);
		return ATGPS_CMD_UNDEF;
	}

	return result;
}

inline exitCode
DeviceATGPS::getDeviceRemoteValue(t_atgpsCmds idx, char * buf, int & len) {
	exitCode result = OK;
	char cmd[8];

	LOG4CPP_DEBUG(log, "===> [%s]", d_atgpsCmds[idx]);

	LOG4CPP_DEBUG(log, "Entering mutex lock...");
	d_lock.enterMutex ();

	d_tty->detachedMode(false);
	d_tty->sendSerial(d_atgpsCmds[idx], strlen(d_atgpsCmds[idx]));
	d_tty->readSerial(buf, len);
	d_tty->detachedMode();

	LOG4CPP_DEBUG(log, "Releasing mutex lock");
	d_lock.leaveMutex ();

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
	comsys::Command::t_cmdType cmdType;
	comsys::Command * cSgd;
	int eventCode = 0x00;
	std::ostringstream sbuf("");

	switch (event) {
	case MOVE:
		cmdType = ODOMETER_EVENT_MOVE;
		LOG4CPP_DEBUG(log, "MOVE event");
		break;
	case STOP:
		cmdType = ODOMETER_EVENT_STOP;
		LOG4CPP_DEBUG(log, "STOP event");
		break;
	case OVER_SPEED:
		cmdType = ODOMETER_EVENT_OVER_SPEED;
		eventCode = 0x17;
		sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << odoSpeed();
		LOG4CPP_DEBUG(log, "OVER_SPEED Event");
		break;
	case EMERGENCY_BREAK:
		cmdType = ODOMETER_EVENT_EMERGENCY_BREAK;
		eventCode = 0x23;
		sbuf << "0";
		LOG4CPP_DEBUG(log, "EMERGENCY_BREAK Event");
		break;
	case SAFE_SPEED:
		cmdType = ODOMETER_EVENT_SAFE_SPEED;
		LOG4CPP_DEBUG(log, "SAFE_SPEED event");
		break;
	case FIX_GET:
		cmdType = GPS_EVENT_FIX_GET;
		LOG4CPP_DEBUG(log, "FIX_GET event");
		break;
	case FIX_LOSE:
		cmdType = GPS_EVENT_FIX_LOSE;
		LOG4CPP_DEBUG(log, "FIX_LOSE event");
		break;
	default:
		LOG4CPP_DEBUG(log, "Not an ODO event");
		return ATGPS_EVENT_UNDEFINED;
	}

	if ( event <= EMERGENCY_BREAK) {
		cSgd = comsys::Command::getCommand(cmdType,
				Device::DEVICE_ODO, "DEVICE_ODO",
				log.getName());
	} else {
		cSgd = comsys::Command::getCommand(cmdType,
				Device::DEVICE_GPS, "DEVICE_GPS",
				log.getName());
	}

	if ( !cSgd ) {
		LOG4CPP_FATAL(log, "Unable to build a new Command");
		return OUT_OF_MEMORY;
	}

	cSgd->setParam("timestamp", d_time->time() );

	if ( eventCode ) {
		// This is a DIST event
		cSgd->setParam("dist_evtData", sbuf.str() );
		sbuf.str("");
		sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)eventCode;
		cSgd->setParam("dist_evtType", sbuf.str());
	}

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

	if (reg==0x0) {
		LOG4CPP_DEBUG(log, "No new events for this device");
		return ATGPS_NO_NEW_EVENTS;
	}

	LOG4CPP_INFO(log, "Event register [0x%04X]", reg);

	if (!notify) {
		LOG4CPP_DEBUG(log, "Notification disabled");
		return OK;
	}

	for (eventIdx=0x1; reg; eventIdx<<=1) {
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
	exitCode result;

	tid = (long) syscall(SYS_gettid);
	LOG4CPP_DEBUG(log, "working thread [%lu=>%lu] started", tid, pthread_self());

	// Registering signal
	//setSignal(SIGCONT,true);
	sigInstall(SIGCONT);
	d_signals->registerHandler((DeviceSignals::t_interrupt)d_intrLine, this, SIGCONT, name().c_str(), DeviceSignals::INTERRUPT_ON_LOW);

	while (true) {

		LOG4CPP_DEBUG(log, "Waiting for interrupt...");
		waitSignal(SIGCONT);

		LOG4CPP_DEBUG(log, "Interrupt received [%d]", ++notifies);
/*
		do {*/
			result = checkAlarms();
			d_signals->ackSignal();
/*
			// This loop is required to handle repeated events...
			::sleep(1);
			LOG4CPP_DEBUG(log, "Re-Checking for LOST interrupts...");
		} while (result!=ATGPS_NO_NEW_EVENTS);*/
	}
}

}// namespace device
}// namespace controlbox
