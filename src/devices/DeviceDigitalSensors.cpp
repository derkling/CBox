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


#include "DeviceDigitalSensors.ih"

namespace controlbox {
namespace device {

DeviceDigitalSensors * DeviceDigitalSensors::d_instance = 0;

DeviceDigitalSensors * DeviceDigitalSensors::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceDigitalSensors(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceFactory::getInstance()");

	return d_instance;
}

DeviceDigitalSensors::DeviceDigitalSensors(std::string const & logName) :
	CommandGenerator(logName),
	Device(Device::DEVICE_DS, 0, logName),
	d_config(Configurator::getInstance()),
	d_sysfsbase(""),
	log(Device::log) {
	DeviceFactory * df = DeviceFactory::getInstance();

	LOG4CPP_DEBUG(log, "DeviceDigitalSensors(const std::string &, bool)");

	// Registering device into the DeviceDB
	dbReg();

	d_signals = df->getDeviceSignals();
	d_time = df->getDeviceTime();

	loadSensorConfiguration();
	LOG4CPP_INFO(log, "Successfully loaded %d digital sensors configurations", count());

	// Initialize sensors samples by reading inputs
	d_sysfsbase = d_config.param("DigitalSensor_sysfsbase", DS_DEFAULT_SYSFSBASE);
	LOG4CPP_DEBUG(log, "Using sysfsbase [%s]", d_sysfsbase.c_str());
	initSensors();

	d_intrLine = atoi(d_config.param("DigitalSensor_PA_intrline", DS_DEFAULT_PA_INTRLINE).c_str());
	LOG4CPP_DEBUG(log, "Using PA interrupt line [%d]", d_intrLine);

}


DeviceDigitalSensors::~DeviceDigitalSensors() {
	t_dsMap::iterator aSensor;
	t_attrMap::iterator anAttr;

	LOG4CPP_INFO(log, "Stopping DeviceDigitalSensors");

	// Destroing digital sensors map
	aSensor = sensors.begin();
	while ( aSensor != sensors.end()) {
		delete aSensor->second;
		aSensor++;
	}
	sensors.clear();

	// Destroing digital sensors attrbutes map
	anAttr = attrbutes.begin();
	while ( anAttr != attrbutes.end()) {
		anAttr->second->bits.clear();
		delete anAttr->second;
		anAttr++;
	}
	attrbutes.clear();

	// Closing I2C device
// 	if ( fdI2C ) {
//		std::close(fdI2C);
// 	}

}

inline exitCode
DeviceDigitalSensors::loadSensorConfiguration(void) {
	short sensor;
	std::ostringstream dsCfgLable("");
	std::string dsCfg;
	t_digitalSensor * curSensor;
	t_attribute * curAttr;

	// Load Sensor Configuration
	sensor = DS_FIRST_ID;
	dsCfgLable << DS_CONF_BASE << sensor;
	dsCfg = d_config.param(dsCfgLable.str(), "");

	while ( dsCfg.size() ) {

		curSensor = parseCfgString(dsCfg);
		if ( curSensor ) {
			curAttr = confAttribute(curSensor);
			if (!curAttr) {
				LOG4CPP_ERROR(log, "Error on configuring a Digital Sensor attribute");
				delete curSensor;
			} else {
				sensors[string(curSensor->id)] = curSensor;
				logSensorParams(curSensor);
			}
		} else {
			LOG4CPP_WARN(log, "Error on loading a Digital Sensor configuration");
		}

		// Looking for next sensor definition
		sensor++; dsCfgLable.str("");
		dsCfgLable << DS_CONF_BASE << sensor;
		dsCfg = d_config.param(dsCfgLable.str(), "");
	};

	return OK;

}

inline DeviceDigitalSensors::t_digitalSensor *
DeviceDigitalSensors::parseCfgString(std::string const & dsCfg) {
	t_digitalSensor * pDs = 0;
	std::ostringstream cfgTemplate("");
	int enabled, alarmEnabled;
	float downLimit, upperLimit;
	char param[2];
	char description[DS_DESC_START+1];
	char * descStart;

	LOG4CPP_DEBUG(log, "parseCfgString(dsCfg=%s)", dsCfg.c_str());

	pDs = new t_digitalSensor();

	//TODO Error handling on patterna matching...

	//NOTE Input configuration string template:
	// - for sysfs protocol:
	// DigitalSensor_N id proto bus address port bit| event param trig enable description

	cfgTemplate << "%" << DS_MAX_ID_LENGTH << "s %u";
DLOG4CPP_DEBUG(log, "Format string: [%s]", cfgTemplate.str().c_str());
	sscanf(dsCfg.c_str(), cfgTemplate.str().c_str(), pDs->id, &pDs->proto);

DLOG4CPP_DEBUG(log, "Id: [%s] Proto: [%d]", pDs->id, pDs->proto);

	switch ( pDs->proto ) {
		case DS_PROTO_SYSFS:
			// Reformatting parsing template by:
			cfgTemplate.str("");
			// - throw away already parsed substrings
			cfgTemplate << "%*" << DS_MAX_ID_LENGTH << "s %*u";
			// - select Protocol-Specifics substrings...
			cfgTemplate << " %i %i %d %d";
DLOG4CPP_DEBUG(log, "Format string: [%s]", cfgTemplate.str().c_str());
			sscanf(dsCfg.c_str(), cfgTemplate.str().c_str(),
				&pDs->address.sysfsAddress.bus,
				&pDs->address.sysfsAddress.address,
				&pDs->address.sysfsAddress.port,
				&pDs->address.sysfsAddress.bit);

DLOG4CPP_DEBUG(log, "Bus: [0x%X] Address: [0x%X]"
				" Port: [%d] Bit: [%d]",
				pDs->address.sysfsAddress.bus,
				pDs->address.sysfsAddress.address,
				pDs->address.sysfsAddress.port,
				pDs->address.sysfsAddress.bit);

			// Reformatting parsing string to throw away parsed substrings
			cfgTemplate.str("");
			cfgTemplate << "%*" << DS_MAX_ID_LENGTH << "s %*u %*i %*i %*d %*d";
		break;

		default:
			LOG4CPP_WARN(log, "Unsupported Protocol Type for Digital Sensor [%s]", pDs->id);
			return 0;

	}

	// Parsing the remaining Protocol-Independant" params...
	//NOTE %s in scanf don't cross spaces... we should use a trick to recovery the whole description string
	cfgTemplate << " %i %" << DS_MAX_VALUE_LENGTH << "s %" << DS_MAX_VALUE_LENGTH << "s %c %i %d %" << DS_DESC_START << "s";
DLOG4CPP_DEBUG(log, "Format string: [%s]", cfgTemplate.str().c_str());
	sscanf(dsCfg.c_str(), cfgTemplate.str().c_str(),
			&pDs->event, pDs->lvalue,
			pDs->hvalue, param,
			&pDs->trigger, &enabled,
			description);

	pDs->enabled = enabled ? true : false;

	errno = 0;
	if (param[0] == '-') {
		pDs->hasParam = false;
	} else {
		pDs->hasParam = true;
		param[1] = 0;
		pDs->param = strtol((const char*)param, NULL, 10);
		if (errno == EINVAL) {
			// FIXME this never match: better handle definitions of no params sensors!!!
			LOG4CPP_DEBUG(log, "Param not defined: resetting to -1 default value");
			pDs->hasParam = false;
			pDs->param = 0;
		}
	}

	// Retriving complete description
	description[DS_DESC_START+1] = 0;
	descStart = strstr(dsCfg.c_str()+strlen(pDs->id), description);
	if ( descStart ) {
		strncpy(pDs->description, descStart, DS_MAX_DESC_LENGTH);
	}

LOG4CPP_DEBUG(log, "Event: [0x%X] Param: [%d]"
			" Trigger: [0x%X] Enabled: [%s]"
			" Description: [%s]",
				pDs->event,
				pDs->param,
				pDs->trigger,
				pDs->enabled ? "YES" : "NO",
				pDs->description);

	if ( validateParams(pDs) ) {
		return pDs;
	} else {
		LOG4CPP_WARN(log, "Params check failure for [%s]", dsCfg.c_str());
		delete pDs;
		return 0;
	}

}

inline DeviceDigitalSensors::t_attribute * DeviceDigitalSensors::confAttribute(DeviceDigitalSensors::t_digitalSensor * pDs) {
	char attrId[DS_MAX_ATTRIB_LENGTH];
	t_attribute * pAttr;
	t_digitalSensor * aDs;

	// Build attribute filename
	sprintf(attrId, "%1d-%04X/input%1d",
		pDs->address.sysfsAddress.bus,
		pDs->address.sysfsAddress.address,
		pDs->address.sysfsAddress.port);
	LOG4CPP_DEBUG(log, "Looking for attribute already defined [%s]...", attrId);

	// Search for that attrbute already defined OR create a new one
	pAttr = findAttrById(attrId);
	if (!pAttr) {
		LOG4CPP_DEBUG(log, "Creating a new attribute");
		pAttr = new t_attribute();
	} else {
		// Search for bit already binded => exit error
		if (lookUpBit(pAttr->bits, pDs->address.sysfsAddress.bit)) {
			LOG4CPP_ERROR(log, "Duplicated bit assignment");
			return 0;
		}
	}

	// Bind this new bit and add save the new attribute
	LOG4CPP_DEBUG(log, "Binding bit %d of %s to digital sensor [%s]",
			pDs->address.sysfsAddress.bit,
			attrId, pDs->description);
	pAttr->bits[pDs->address.sysfsAddress.bit] = pDs;
	strncpy(pAttr->id, attrId, DS_MAX_ATTRIB_LENGTH);
	attrbutes[attrId] = pAttr;

	// Return the current attribute
	return pAttr;

}

inline bool
DeviceDigitalSensors::validateParams(DeviceDigitalSensors::t_digitalSensor * pDs) {
	t_dsMap::iterator s;

	//TODO params sanity checks!!!

	// Checking for id duplication
	if (findById(pDs->id)) {
		LOG4CPP_WARN(log, "Duplicated digital sensor id [%s]", pDs->id);
		LOG4CPP_ERROR(log, "Sensor definitions must have unique ID");
		return false;
	}

	// Initialize lastState

	return true;
}

inline void
DeviceDigitalSensors::logSensorParams(DeviceDigitalSensors::t_digitalSensor * pDs) {
	std::ostringstream params("");
	char *trigger[] = { "None", "Going High", "Going Low", "Level Change" };

	LOG4CPP_INFO(log, "ID: %-*s %s", DS_MAX_ID_LENGTH, pDs->id, pDs->description);
	LOG4CPP_INFO(log, "\tChip: %1d-%04X,\tPort: %d,\tBit: %d",
			pDs->address.sysfsAddress.bus,
			pDs->address.sysfsAddress.address,
			pDs->address.sysfsAddress.port,
			pDs->address.sysfsAddress.bit);
	LOG4CPP_INFO(log, "\tEvent:\t\t0x%X", pDs->event);
	LOG4CPP_INFO(log, "\tWhen High:\t%s", pDs->hvalue);
	LOG4CPP_INFO(log, "\tWhen Low: \t%s", pDs->lvalue);
	if (pDs->hasParam) {
		LOG4CPP_INFO(log, "\tParam:\t\t%d", pDs->param);
	} else {
		LOG4CPP_INFO(log, "\tParam:\t\t-");
	}
	LOG4CPP_INFO(log, "\tTrigger:\t%s", trigger[pDs->trigger]);
	LOG4CPP_INFO(log, "\tSensor enabled:\t%s", pDs->enabled ? "YES" : "NO" );

}


inline unsigned
DeviceDigitalSensors::count() {
	return sensors.size();
}


inline DeviceDigitalSensors::t_digitalSensor *
DeviceDigitalSensors::findById (t_dsId dsId) {
	t_dsMap::iterator aSensor;

	aSensor = sensors.begin();
	while ( aSensor != sensors.end()) {
		if ( aSensor->first ==  dsId ) {
			return aSensor->second;
		}
		aSensor++;
	}

	return 0;

}

inline DeviceDigitalSensors::t_attribute *
DeviceDigitalSensors::findAttrById (t_attrId attrId) {
	t_attrMap::iterator anAttr;

	anAttr = attrbutes.begin();
	while ( anAttr != attrbutes.end()) {
		if ( anAttr->first ==  attrId ) {
			return anAttr->second;
		}
		anAttr++;
	}

	return 0;

}

inline DeviceDigitalSensors::t_digitalSensor *
DeviceDigitalSensors::lookUpBit (t_bitMap &map, t_bitOffset bit) {
	t_bitMap::iterator aBit;

	aBit = map.begin();
	while ( aBit != map.end()) {
		if ( aBit->first ==  bit ) {
			return aBit->second;
		}
		aBit++;
	}

	return 0;
}

inline exitCode
DeviceDigitalSensors::getPortStatus(t_attribute const & anAttr, t_portStatus & value) {
	std::ostringstream path("");
	int fd;
	char svalue[4];

	path << d_sysfsbase << "/" << anAttr.id;
	fd = ::open(path.str().c_str(), O_RDONLY);
	if (fd == -1) {
		LOG4CPP_ERROR(log, "Failed to open attrib [%s]", path.str().c_str());
		return DS_PORT_READ_ERROR;
	}

	read(fd, svalue, 3);
	errno = 0;
	value = strtol(svalue, (char **)NULL, 10);
	if (errno) {
		LOG4CPP_ERROR(log, "string value [%s] conversion failed, %s",
				svalue, strerror(errno));
		::close(fd);
		return DS_VALUE_CONVERSION_FAILED;
	}

	::close(fd);
	return OK;
}

inline exitCode
DeviceDigitalSensors::initSensors(void) {
	t_attrMap::iterator anAttr;
	t_portStatus value;

	anAttr = attrbutes.begin();
	while ( anAttr != attrbutes.end()) {

		if (getPortStatus(*(anAttr->second), value) == OK) {
			anAttr->second->status = value;
			LOG4CPP_DEBUG(log, "Current port [%s] status: 0x%X",
					anAttr->second->id,
					anAttr->second->status);
		} else {
			LOG4CPP_ERROR(log, "Unable to update port [%s] status", anAttr->second->id);
		}

		anAttr++;
	}

}

inline exitCode
DeviceDigitalSensors::notifySensorEvent(t_digitalSensor & aSensor) {
	comsys::Command * cSgd;
	std::ostringstream buf("");

	cSgd = comsys::Command::getCommand(DIGITAL_SENSORS_EVENT,
			Device::DEVICE_DS, "DEVICE_DS",
			aSensor.id);
	if ( !cSgd ) {
		LOG4CPP_FATAL(log, "Unable to build a new Command");
		return OUT_OF_MEMORY;
	}

	cSgd->setParam( "event", aSensor.event);
	cSgd->setParam( "value", aSensor.lastState);
	cSgd->setParam( "status", (aSensor.lastState) ? aSensor.hvalue : aSensor.lvalue);
	if (aSensor.hasParam) {
		cSgd->setParam( "param", aSensor.param);
	}

/*#define CE_TESTS
#ifdef CE_TESTS
	if (aSensor.event == 0xFF) {
		system("/mnt/flash/mmc/bin/reboot.sh");
	}
#endif*/

// TODO right here we could implement a "tasklet" for simila events grouping.
//	We receive singular events notification... but we could optimize network
//	trasmissions by collecting similar events togheter and deferring trasmission
//	for a while in order to build a single message for all same type event.
//	E.G. event 0x12 on DIST protocol 4.3

	// Formatting value for DIST protocol
	buf.str("");
	buf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)aSensor.event;
	cSgd->setParam( "dist_evtType", buf.str());
// 	cSgd->setParam( "dist_evtType", (int)aSensor.event );

	buf.str("");
	switch ( (unsigned)aSensor.event ) {
	case 0x12:
		LOG4CPP_DEBUG(log, "EVENT [%d], PARAM [%d], STATE [%d]", (unsigned)aSensor.event, aSensor.param, aSensor.lastState);
		buf << "01" << (unsigned)aSensor.param << (unsigned)aSensor.lastState;
		break;
	default:
		buf << (unsigned)aSensor.lastState;
		break;
	}
	cSgd->setParam( "dist_evtData", buf.str() );

// 	cSgd->setParam( "dist_event", buf.str());
	cSgd->setParam( "timestamp", d_time->time() );

	// Notifying command
	notify(cSgd);

}

inline exitCode
DeviceDigitalSensors::notifySensorChange(t_digitalSensor & aSensor, t_digitalSensorState state) {
	stateDescriptionArray(descr);

	if (!aSensor.enabled) {
		LOG4CPP_WARN(log, "Change on disabled sensor [%s], current status: %s",
				aSensor.id, descr[state]);
		return OK;
	}

	LOG4CPP_INFO(log, "Changes on sensor [%s], new state [%d=%s]",
			aSensor.id, state, descr[state]);

	aSensor.lastState = state;

	if (aSensor.event == DS_EVENT_NULL) {
		LOG4CPP_DEBUG(log, "Event reporting disabled");
		return OK;
	}

	switch (aSensor.trigger) {
	case DS_SIGNAL_ON_LOW:
		if (state == DS_STATE_LOW)
			notifySensorEvent(aSensor);
		break;
	case DS_SIGNAL_ON_HIGH:
		if (state == DS_STATE_HIGH)
			notifySensorEvent(aSensor);
		break;
	case DS_SIGNAL_ON_BOTH:
		notifySensorEvent(aSensor);
		break;
	default:
		LOG4CPP_INFO(log, "Notification disabled for this sensor");
	}

	return OK;

}


inline exitCode
DeviceDigitalSensors::checkPortStatusChange(t_attribute & anAttr) {
	t_portStatus value;
	t_portStatus changeBitMask;
	t_portStatus bitSelector;
	u8 bitPosition;
	t_digitalSensor * pDs;

	// Low-pass software filter to cut-off high-frequency spikes
#if DS_LOW_PASS_FILTER_DELAY>0
#warning Using a software LowPass Filter
	usleep(DS_LOW_PASS_FILTER_DELAY);
#endif

	if (getPortStatus(anAttr, value) != OK) {
		LOG4CPP_ERROR(log, "Unable to update port [%s] status", anAttr.id);
		return DS_PORT_UPDATE_FAILED;
	}

	changeBitMask = value ^ anAttr.status;
	if (!changeBitMask) {
		LOG4CPP_DEBUG(log, "No status changes on this port [%s]", anAttr.id);
		return OK;
	}

	// Checking all bits changes
	bitSelector = 0x1;
	bitPosition = 0;
	while ( bitPosition < 8 ) {

		if (changeBitMask & bitSelector ) {
			LOG4CPP_DEBUG(log, "bit [%d] changed", bitPosition);
			pDs = lookUpBit (anAttr.bits, bitPosition);
			if (pDs) {
				if (value & bitSelector) {
					notifySensorChange(*pDs, DS_STATE_HIGH);
				} else {
					notifySensorChange(*pDs, DS_STATE_LOW);
				}
			} else {
				LOG4CPP_WARN(log, "Change on not configured sensor line [%d], port [%s]",
						bitPosition, anAttr.id);
			}
		}

		bitSelector <<= 1;
		bitPosition++;
	}

	anAttr.status = value;
	LOG4CPP_DEBUG(log, "Updating this port [%s] status [0x%X]",
			anAttr.id,
			anAttr.status);

	return OK;

}

inline exitCode
DeviceDigitalSensors::updateSensors(void) {
	t_attrMap::iterator anAttr;
	t_portStatus value;

	anAttr = attrbutes.begin();
	while ( anAttr != attrbutes.end()) {
		checkPortStatusChange(*(anAttr->second));
		anAttr++;
	}

	return OK;

}

void   DeviceDigitalSensors::run (void) {
	int err;
	int signo;
	sigset_t my_sigs;
	pid_t tid;

	tid = (long) syscall(SYS_gettid);
	LOG4CPP_DEBUG(log, "working thread [%lu=>%lu] started", tid, pthread_self());

	// Registering signal
	//setSignal(SIGCONT,true);
	sigInstall(SIGCONT);
	d_signals->registerHandler((DeviceSignals::t_interrupt)d_intrLine, this, SIGCONT, name().c_str());

	int notifies = 0;
	while (true) {

		LOG4CPP_DEBUG(log, "Waiting for interrupt...");
		waitSignal(SIGCONT);

		LOG4CPP_DEBUG(log, "Interrupt received [%d]", ++notifies);
		updateSensors();

	}

}

}// namespace device
}// namespace controlbox
