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


#include "DeviceAnalogSensors.ih"

namespace controlbox {
namespace device {

DeviceAnalogSensors * DeviceAnalogSensors::d_instance = 0;

DeviceAnalogSensors * DeviceAnalogSensors::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceAnalogSensors(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceFactory::getInstance()");

	return d_instance;
}

DeviceAnalogSensors::DeviceAnalogSensors(std::string const & logName) :
	CommandGenerator(logName),
	Device(Device::DEVICE_AS, 0, logName),
	d_config(Configurator::getInstance()),
	loadI2C(false),
	alarmEnables(false),
	log(Device::log) {

// 	short sensor;
	std::ostringstream asCfgLable("");
	std::string asCfg;
// 	t_analogSensor * curSensor;

	DeviceFactory * df = DeviceFactory::getInstance();

	LOG4CPP_DEBUG(log, "DeviceAnalogSensors(const std::string &, bool)");

	// Registering device into the DeviceDB
	dbReg();

	d_time = df->getDeviceTime();
	d_i2cBus = df->getDeviceI2CBus();

	loadSensorConfiguration();
	LOG4CPP_INFO(log, "Successfully loaded %d analog sensors configurations", count());

	// Initialize sensors samples by reading inputs
	d_sysfsbase = d_config.param("DigitalSensor_sysfsbase", AS_DEFAULT_SYSFSBASE);
	LOG4CPP_DEBUG(log, "Using sysfsbase [%s]", d_sysfsbase.c_str());

	// Initialize sensors samples by reading inputs
	refresSensors();

}

DeviceAnalogSensors::~DeviceAnalogSensors() {
	t_asMap::iterator aSensor;
	DeviceAnalogSensors::t_analogSensor * pAs;

	LOG4CPP_INFO(log, "Stopping DeviceAnalogSensors");

	// Destroing analog sensors map
	aSensor = analogSensors.begin();
	while ( aSensor != analogSensors.end()) {
		pAs = aSensor->second;
		delete pAs->monitor;
		delete pAs;
		aSensor++;
	}

	analogSensors.clear();

}


inline exitCode
DeviceAnalogSensors::loadI2Cbus () {
	std::string device;
	int openError;

	device = d_config.param("AnalogSensor_i2cDevice", AS_DEFAULT_DEVICE);
	fdI2C = open(device.c_str(), O_RDWR);
	openError = errno;
	if (fdI2C <= 0) {
		LOG4CPP_ERROR(log, "Unable to open I2C bus device [%s] (errno %d)", device.c_str(), openError);
		LOG4CPP_ERROR(log, "\t%s", strerror(openError));
		fdI2C = 0;
		return AS_I2C_OPEN_FAILED;
	}

	return OK;

}

inline exitCode
DeviceAnalogSensors::loadSensorConfiguration(void) {
	short sensor;
	std::ostringstream dsCfgLable("");
	std::string dsCfg;
	t_analogSensor * curSensor;

	// Load Sensor Configuration
	sensor = AS_FIRST_ID;
	dsCfgLable << AS_CONF_BASE << sensor;
	dsCfg = d_config.param(dsCfgLable.str(), "");

	while ( dsCfg.size() ) {

		curSensor = parseCfgString(dsCfg);
		if ( curSensor ) {
			analogSensors[string(curSensor->id)] = curSensor;
			logSensorParams(curSensor);
		} else {
			LOG4CPP_WARN(log, "Error on loading an Analog Sensor configuration");
		}

		// Looking for next sensor definition
		sensor++; dsCfgLable.str("");
		dsCfgLable << AS_CONF_BASE << sensor;
		dsCfg = d_config.param(dsCfgLable.str(), "");
	};

	return OK;

}

inline
DeviceAnalogSensors::t_analogSensor * DeviceAnalogSensors::parseCfgString(std::string const & asCfg) {
	t_analogSensor * pAs = 0;
	std::ostringstream cfgTemplate("");
	int enabled;
// 	int alarmEnabled;
	float downLimit, upperLimit;
	char param[2];
	char description[AS_DESC_START+1];
	char * descStart;

	LOG4CPP_DEBUG(log, "parseCfgString(asCfg=%s)", asCfg.c_str());

	pAs = new t_analogSensor();

	// TODO: Error handling on pattern matching...

	// Input configuration string template:
	// - for i2c protocol:
	// id proto address reg minSample maxSample minValue maxValue unit enabled downLimit upperLimit alarmEnabled description
	cfgTemplate << "%" << AS_MAX_ID_LENGTH << "s %u";
	DLOG4CPP_DEBUG(log, "Format string: [%s]", cfgTemplate.str().c_str());
	sscanf(asCfg.c_str(), cfgTemplate.str().c_str(), pAs->id, &pAs->proto);

	DLOG4CPP_DEBUG(log, "Id: [%s] Proto: [%d]", pAs->id, pAs->proto);

	switch ( pAs->proto ) {
		case AS_PROTO_I2C:
			// Reformatting parsing template by:
			cfgTemplate.str("");
			// - throw away already parsed substrings
			cfgTemplate << "%*" << AS_MAX_ID_LENGTH << "s %*u";
			// - select Protocol-Specifics substrings...
			cfgTemplate << " %i %i";
			//LOG4CPP_DEBUG(log, "Format string: [%s]", cfgTemplate.str().c_str());
			sscanf(asCfg.c_str(), cfgTemplate.str().c_str(),
				&pAs->address.i2cAddress.chipAddress,
				&pAs->address.i2cAddress.reg);

			//LOG4CPP_DEBUG(log, "Chip: [%d] Reg: [%d]", pAs->address.i2cAddress.chipAddress, pAs->address.i2cAddress.reg);

			// Reformatting parsing string to throw away parsed substrings
			cfgTemplate.str("");
			cfgTemplate << "%*" << AS_MAX_ID_LENGTH << "s %*u %*i %*i";
		break;

		case AS_PROTO_SYSFS:
			// Reformatting parsing template by
			cfgTemplate.str("");
			// - throw away already parsed substrings
			cfgTemplate << "%*" << AS_MAX_ID_LENGTH << "s %*u";
			// - select Protocol-Specifics substrings...
			cfgTemplate << " %i %i %d";
			DLOG4CPP_DEBUG(log, "Format string: [%s]", cfgTemplate.str().c_str());
			sscanf(asCfg.c_str(), cfgTemplate.str().c_str(),
				&pAs->address.sysfsAddress.bus,
				&pAs->address.sysfsAddress.address,
				&pAs->address.sysfsAddress.channel);

			DLOG4CPP_DEBUG(log, "Bus: [0x%X] Address: [0x%X]"
				" Port: [%d] Bit: [%d]",
				pAs->address.sysfsAddress.bus,
				pAs->address.sysfsAddress.address,
				pAs->address.sysfsAddress.channel);

			// Build channel filename
			sprintf(pAs->sysfsPath, "%1d-%04x/in%1d_input",
				pAs->address.sysfsAddress.bus,
				pAs->address.sysfsAddress.address,
				pAs->address.sysfsAddress.channel);

			// Reformatting parsing string to throw away parsed substrings
			cfgTemplate.str("");
			cfgTemplate << "%*" << AS_MAX_ID_LENGTH << "s %*u %*i %*i %*d";
		break;

		default:
			LOG4CPP_WARN(log, "Unsupported Protocol Type for Analog Sensor [%s]", pAs->id);
			return 0;

	}

	// Parsing the remaining Protocol-Independant" params...
	cfgTemplate << " %d %d %f %f %" << AS_MAX_UNIT_LENGTH << "s %d %f %f %" << AS_MAX_VALUE_LENGTH << "s %" << AS_MAX_VALUE_LENGTH << "s %i %c %u %" << AS_DESC_START << "s";
	DLOG4CPP_DEBUG(log, "Format string: [%s]", cfgTemplate.str().c_str());
	sscanf(asCfg.c_str(), cfgTemplate.str().c_str(),
			&pAs->minSample, &pAs->maxSample,
			&pAs->minValue, &pAs->maxValue,
			pAs->unit, &enabled,
			&downLimit, &upperLimit,
			pAs->lvalue, pAs->hvalue,
			&pAs->event, &param,
			&pAs->alarmPollTime, description);

	pAs->enabled = enabled ? true : false;

	errno = 0;
	if (param[0] == '-') {
		pAs->hasParam = false;
	} else {
		pAs->hasParam = true;
		param[1] = 0;
		pAs->param = strtol((const char*)param, NULL, 10);
		if (errno == EINVAL) {
			// FIXME this never match: better handle definitions of no params sensors!!!
			LOG4CPP_DEBUG(log, "Param not defined: resetting to -1 default value");
			pAs->hasParam = false;
			pAs->param = 0;
		}
	}

	pAs->downLimit = valueToSample(pAs, downLimit);
	pAs->upperLimit = valueToSample(pAs, upperLimit);

	// Retriving complete description (by jumping initial ID string)
	description[AS_DESC_START+1] = 0;
	descStart = strstr(asCfg.c_str()+strlen(pAs->id), description);
	if ( descStart ) {
		strncpy(pAs->description, descStart, AS_MAX_DESC_LENGTH);
	}

	if ( validateParams(pAs) ) {
		return pAs;
	} else {
		return 0;
	}

}

inline bool
DeviceAnalogSensors::validateParams(DeviceAnalogSensors::t_analogSensor * pAs) {

	//TODO: params sanity checks!!!
	// Checking for id duplication
	if (findById(pAs->id)) {
		LOG4CPP_WARN(log, "Duplicated analog sensor id [%s]", pAs->id);
		LOG4CPP_ERROR(log, "Sensor definitions must have unique ID");
		return false;
	}

	// sampleRange MUST be NON zero!!!
	// Checking Polltime limit
	if (pAs->alarmPollTime && (pAs->alarmPollTime < AS_MIN_POLL_TIME)) {
		LOG4CPP_WARN(log, "Poll time for sensor [%s] could not be less then %ums! Resetting poll time to this value", pAs->id, AS_MIN_POLL_TIME);
		pAs->alarmPollTime = AS_MIN_POLL_TIME;
	}

	// Initialize lastSample
	pAs->lastSample = pAs->minSample;

	if ( !loadI2C && (pAs->proto == AS_PROTO_I2C) ) {
		loadI2C = true;
	}

	return true;
}

inline void
DeviceAnalogSensors::logSensorParams(DeviceAnalogSensors::t_analogSensor * pAs) {

#ifdef CONTROLBOX_DEBUG
	std::ostringstream params("");

	LOG4CPP_DEBUG(log, "ID: %-*s - %s", AS_MAX_ID_LENGTH, pAs->id, pAs->description);
	LOG4CPP_DEBUG(log, "\tMin Sample:   %5d\t\tMax Sample:   %5d", pAs->minSample, pAs->maxSample);

// TODO: check why on cris log4cpp doesn't handle float conversione!
/*
	LOG4CPP_INFO(log, "\tMin Value:  %7.3f %-*s\tMax Value:  %7.3f %-*s",
				pAs->minValue, AS_MAX_UNIT_LENGTH, pAs->unit,
				pAs->maxValue, AS_MAX_UNIT_LENGTH, pAs->unit);
	LOG4CPP_INFO(log, "\tLow Alarm:  %7.3f %-*s\tHigh Alarm: %7.3f %-*s",
				pAs->downLimit, AS_MAX_UNIT_LENGTH, pAs->unit,
				pAs->upperLimit, AS_MAX_UNIT_LENGTH, pAs->unit);
*/
//	The followinf code result in a segfault... we need to use the less
//	optimized followin code:
	params.str("");
	params << "Min Value:  " << setw(7) << std::setprecision(3) << pAs->minValue;
	params << " " << left << setw(AS_MAX_UNIT_LENGTH) << pAs->unit << right;
	params << "\tMax Value:  " << setw(7) << std::setprecision(3) << pAs->maxValue;
	params << " " << left << setw(AS_MAX_UNIT_LENGTH) << pAs->unit << right;
	LOG4CPP_DEBUG(log, "\t%s", params.str().c_str());

	params.str("");
	params << "Low Alarm:  " << setw(7) << std::setprecision(3) << sampleToValue(pAs, pAs->downLimit);
	params << " " << left << setw(AS_MAX_UNIT_LENGTH) << pAs->unit << right;
	params << "\tHigh Alarm:  " << setw(7) << std::setprecision(3) << sampleToValue(pAs, pAs->upperLimit);
	params << " " << left << setw(AS_MAX_UNIT_LENGTH) << pAs->unit;
	LOG4CPP_DEBUG(log, "\t%s", params.str().c_str());


	LOG4CPP_DEBUG(log, "\tSensor enabled:\t%s", pAs->enabled ? "YES" : "NO" );
	if (pAs->event) {
		LOG4CPP_DEBUG(log, "\tAlarm enabled:\tYES, Event: %u - Poll time: %dms", pAs->event, pAs->alarmPollTime );
	} else {
		LOG4CPP_DEBUG(log, "\tAlarm enabled:\tNO");
	}
	switch (pAs->proto) {
	case AS_PROTO_I2C:
		break;
	case AS_PROTO_SYSFS:
		//FIXME Don't use the SYSFS implementation for the GPIO based one
	case AS_PROTO_GPIO:
		LOG4CPP_DEBUG(log, "\tDevice:\t%s", pAs->sysfsPath);
		break;

	}
#endif

}

inline exitCode
DeviceAnalogSensors::refresSensors() {
	t_asMap::iterator aSensor;
	DeviceAnalogSensors::t_analogSensor * pAs;
	std::ostringstream values;

	LOG4CPP_DEBUG(log, "DeviceAnalogSensors::refresSensors()");

	values.str("");
	aSensor = analogSensors.begin();
	while ( aSensor != analogSensors.end()) {
		pAs = aSensor->second;
		updateSensor(pAs);
		values << "[" << setw(10) << fixed << setprecision(5) << sampleToValue(pAs) << "] ";
		aSensor++;
	}

	LOG4CPP_DEBUG(log, "%s", values.str().c_str());
	return OK;
}

exitCode
DeviceAnalogSensors::read(std::string asId, float & value, bool update) {
	DeviceAnalogSensors::t_analogSensor * pAs;

	pAs = findById(asId);
	if ( !pAs ) {
		LOG4CPP_WARN(log, "Checking an undefined sensor [%s]", asId.c_str());
		return AS_UNDEFINED_SENSOR;
	}

	if (update) {
		updateSensor(pAs);
	}

	value = sampleToValue(pAs);
	return OK;
}

inline DeviceAnalogSensors::t_channelValue
DeviceAnalogSensors::updateSensor(DeviceAnalogSensors::t_analogSensor * pAs) {
	exitCode result;

#ifdef CONTROLBOX_CRIS
	switch ( pAs->proto ) {
		case AS_PROTO_I2C:
			result = readI2C(pAs, pAs->lastSample);
		break;
		case AS_PROTO_SYSFS:
			result = readSysfs(pAs, pAs->lastSample);
		break;
		default:
			LOG4CPP_ERROR(log, "Unsupported Protocol Type for Analog Sensor [%s]", pAs->id);
			return 0;
	}

	if ( result != OK ) {
		//FIXME do a better error handling
		LOG4CPP_ERROR(log, "Error on sensor sampling");
	}

#else
	// On NON-CRIS architectures we simulate a read that return a prefedined value
	pAs->lastSample =  (unsigned)round((pAs->maxSample-pAs->minSample)/2);
#endif

	return pAs->lastSample;

}

inline exitCode
DeviceAnalogSensors::readI2C(DeviceAnalogSensors::t_analogSensor * pAs, t_channelValue & value) {
	I2C_DATA i2cdata;

	i2cdata.length = 1;
	i2cdata.slave = pAs->address.i2cAddress.chipAddress;
	i2cdata.reg = pAs->address.i2cAddress.reg;

	LOG4CPP_DEBUG(log, "readI2C(pAs=%s, slave=%d, reg=%d)", pAs->id, i2cdata.slave, i2cdata.reg);

	//NOTE: this code will be called only if compiled for the target CRIS platform
#ifdef CONTROLBOX_CRIS
	// Performing IOCTL on CRIS architecture
	ioctl(fdI2C, _IO(ETRAXI2C_IOCTYPE, I2C_READREG_N), &i2cdata);
	value = (i2cdata.data[0])/10;
#endif

	return OK;
}


inline exitCode
DeviceAnalogSensors::readSysfs(DeviceAnalogSensors::t_analogSensor * pAs, t_channelValue & value) {
#if 0
	std::ostringstream path("");
	int fd;
	char svalue[4];

	path << d_sysfsbase << "/" << pAs->sysfsPath;
	fd = ::open(path.str().c_str(), O_RDONLY);
	if (fd == -1) {
		LOG4CPP_ERROR(log, "Failed to open attrib [%s]", path.str().c_str());
		return DS_PORT_READ_ERROR;
	}

	::read(fd, svalue, 3);
	errno = 0;
	//NOTE the driver return an appended 0!!!
	value = strtol(svalue, (char **)NULL, 10) / 10;
	if (errno) {
		LOG4CPP_ERROR(log, "string value [%s] conversion failed, %s",
				svalue, strerror(errno));
		::close(fd);
		return DS_VALUE_CONVERSION_FAILED;
	}

	::close(fd);
#endif

	exitCode result;
	int addr = pAs->address.sysfsAddress.address;
	DeviceI2CBus::t_i2cCommand regs[2] = {
		pAs->address.sysfsAddress.channel,
		pAs->address.sysfsAddress.channel};
	DeviceI2CBus::t_i2cReg val[2];
	short len = 2;

	result = d_i2cBus->read(addr, regs, len, val, len);
	if ( result != OK ) {
		LOG4CPP_ERROR(log, "Update sensors failed!");
		return result;
	}
	value = val[1];

	LOG4CPP_DEBUG(log, "Readed value: %02X", value);

	return OK;
}

inline float
DeviceAnalogSensors::sampleToValue(DeviceAnalogSensors::t_analogSensor * pAs, int theSample) {
	unsigned p_sample = (theSample < 0) ? pAs->lastSample : theSample;
	float valueRange;
	unsigned sampleRange;
	unsigned curSampleRange;

	valueRange = pAs->maxValue - pAs->minValue;
	sampleRange = pAs->maxSample - pAs->minSample;
	curSampleRange = p_sample - pAs->minSample;

	return ((valueRange*curSampleRange)/sampleRange)+pAs->minValue;
}

inline unsigned
DeviceAnalogSensors::valueToSample(DeviceAnalogSensors::t_analogSensor * pAs, float value) {

	float valueRange;
	unsigned sampleRange;
	float curValueRange;

	valueRange = pAs->maxValue - pAs->minValue;
	sampleRange = pAs->maxSample - pAs->minSample;
	curValueRange = value - pAs->minValue;

	return (unsigned)round(((sampleRange*curValueRange)/valueRange)+pAs->minSample);

}

exitCode
DeviceAnalogSensors::startMonitors() {
	t_asMap::iterator aSensor;
	DeviceAnalogSensors::t_analogSensor * pAs;

	LOG4CPP_DEBUG(log, "Starting monitors threads...");

	aSensor = analogSensors.begin();
	while ( aSensor != analogSensors.end()) {
		pAs = aSensor->second;
		LOG4CPP_DEBUG(log, "Checking if sensor [%s] - Event [%u], PoolTime [%d]...",
				pAs->id, pAs->event, pAs->alarmPollTime);
		if ( pAs->event && pAs->alarmPollTime ) {
			pAs->monitor = new DeviceAnalogSensors::Monitor(this, pAs, pAs->alarmPollTime);
			(pAs->monitor)->start();
			LOG4CPP_INFO(log, "Monitoring sensor [%s] with poll time [%d]",
						pAs->id, pAs->alarmPollTime);
		}
		aSensor++;
	}
	return OK;
}

DeviceAnalogSensors::Monitor::Monitor(DeviceAnalogSensors * device, DeviceAnalogSensors::t_analogSensor * pAs, timeout_t pollTime) :
	d_device(device),
	d_pollTime(pollTime),
	d_pAs(pAs) {

}

void
DeviceAnalogSensors::Monitor::run (void) {

	while (true) {
		sleep(d_pollTime);
		d_device->checkSafety(d_pAs, true);
	}

}

inline bool
DeviceAnalogSensors::checkSafety(std::string asId) {
	t_analogSensor * pAs;

	LOG4CPP_DEBUG(log, "DeviceAnalogSensors::checkSafety(asId=%s)", asId.c_str());

	pAs = findById(asId);
	if ( pAs ) {
		return checkSafety(pAs);
	}

	LOG4CPP_WARN(log, "Checking an undefined sensor [%s]", asId.c_str());
	return true;

}

bool
DeviceAnalogSensors::checkSafety(DeviceAnalogSensors::t_analogSensor * pAs, bool p_notify) {
	unsigned lastSample;
	bool alarm = false;
	bool sendNotify = false;

	LOG4CPP_DEBUG(log, "DeviceAnalogSensors::checkSafety(pAs=%p, notify=%s)", pAs, p_notify ? "YES" : "NO");

	lastSample = updateSensor(pAs);

	// Fix the reading for sensors that are not attached:
	// in this case the input is floating and so we read 0V
	// => report this as the minimum configured value
	if (lastSample<pAs->minSample)
		lastSample=pAs->minSample;

	alarm = (lastSample < pAs->downLimit) || (lastSample > pAs->upperLimit);

	if ( alarm ) {
		if ( (lastSample < pAs->downLimit) && (pAs->alarmState != LOW_ALARM) ) {
			pAs->alarmState = LOW_ALARM;
			sendNotify = true;
		} else if ( (lastSample > pAs->upperLimit) && (pAs->alarmState != HIGH_ALARM) ) {
			pAs->alarmState = HIGH_ALARM;
			sendNotify = true;
		}
	} else {
		if ( pAs->alarmState != NO_ALARM ) {
			pAs->alarmState = NO_ALARM;
			sendNotify = true;
		}
	}

	if ( p_notify && sendNotify ) {
		notifyAlarm(pAs);
		return alarm;
	}

	return alarm;

}

void
DeviceAnalogSensors::notifyAlarm(DeviceAnalogSensors::t_analogSensor * pAs) {
	std::ostringstream alarmStr("");

	alarmStr << "Sensor [" << pAs->id << " = " << sampleToValue(pAs) << "] ";
	alarmStr << "out of safety range [" << setw(7) << setprecision(3) << sampleToValue(pAs, pAs->downLimit);
	alarmStr << " - " << sampleToValue(pAs, pAs->upperLimit) << "]";

	LOG4CPP_WARN(log, "%s", alarmStr.str().c_str());

	DLOG4CPP_WARN(log, "Sensor [%s = %f] out of safety range [%f - %f]",
				pAs->id, sampleToValue(pAs),
				sampleToValue(pAs, pAs->downLimit),
				sampleToValue(pAs, pAs->upperLimit)
			);

	notifySensorEvent(*pAs);

}

inline exitCode
DeviceAnalogSensors::notifySensorEvent(t_analogSensor & aSensor) {
	comsys::Command * cSgd;

	cSgd = comsys::Command::getCommand(ANALOG_SENSORS_EVENT,
			Device::DEVICE_AS, "DEVICE_AS",
			aSensor.id);
	if ( !cSgd ) {
		LOG4CPP_FATAL(log, "Unable to build a new Command");
		return OUT_OF_MEMORY;
	}
	cSgd->setParam( "timestamp", d_time->time() );
	cSgd->setParam( "event",   aSensor.event);
	if (aSensor.hasParam) {
		cSgd->setParam( "param",  aSensor.param);
	}
	cSgd->setParam( "value",  sampleToValue(&aSensor, aSensor.lastSample));
	cSgd->setParam( "sevent", getAlarmStrEvent(aSensor));

	// Notifying command
	notify(cSgd);

	return OK;
}

inline std::string
DeviceAnalogSensors::getAlarmStrEvent(t_analogSensor & aSensor) {

	if (aSensor.lastSample <= aSensor.downLimit)
		return aSensor.lvalue;

	if (aSensor.lastSample >= aSensor.upperLimit)
		return aSensor.hvalue;

	return "RangeOk";

}

inline
DeviceAnalogSensors::t_analogSensor * DeviceAnalogSensors::findById (std::string asId) {
	t_asMap::iterator aSensor;

	aSensor = analogSensors.begin();
	while ( aSensor != analogSensors.end()) {
		if ( aSensor->first ==  asId ) {
			return aSensor->second;
		}
		aSensor++;
	}
	return 0;
}

inline unsigned
DeviceAnalogSensors::count() {
	return analogSensors.size();
}

unsigned
DeviceAnalogSensors::listId(t_asIdList & asIdList) {
	unsigned cId;
	t_asMap::iterator aSensor;

	asIdList.clear();
	cId = 0;
	aSensor = analogSensors.begin();
	while ( aSensor != analogSensors.end()) {
		asIdList.push_back(aSensor->first);
		aSensor++; cId++;
	}

	return cId;

}



#if 0
inline
int DeviceAnalogSensors::sample(t_analogSensor * theSensor, bool update) {

	LOG4CPP_DEBUG(log, "DeviceAnalogSensors::sample(theSensor=%p, update=%s)", theSensor, update ? "YES" : "NO");

	switch ( theSensor->proto ) {
		case AS_PROTO_I2C:
#ifdef CONTROLBOX_CRIS
			if ( !fdI2C ) {
				return -1;
			}
#endif
			return readI2C(theSensor);
		break;
		default:
			LOG4CPP_WARN(log, "Unsupported Protocol Type for Analog Sensor [%s]", theSensor->id);
			return 0;

	}

}


int DeviceAnalogSensors::sample(std::string asId, bool update) {
	t_analogSensor * theSensor;

	LOG4CPP_DEBUG(log, "DeviceAnalogSensors::sample(asId=%s, update=%s)", asId.c_str(), update ? "YES" : "NO");

	theSensor = findById (asId);
	if ( ! theSensor ) {
		LOG4CPP_ERROR(log, "Required analog sensor ID [%d] not found", asId.c_str());
		return 0;
	}

	// If it's not required an update from sensor return the last available sample
	// NOTE: if the sensor is monitored we suppose that the polling time is
	//	reasonalbe updating interval for the sensor value: thus we
	//	ignore the update request and simply return the last readed
	//	value
	if ( theSensor->alarmPollTime || !update ) {
		return theSensor->lastSample;
	}

	return sample(theSensor, update);

}

float DeviceAnalogSensors::value(std::string asId, bool update) {
	int theSample;

	LOG4CPP_DEBUG(log, "DeviceAnalogSensors::value(asId=%s, update=%s)", asId.c_str(), update ? "YES" : "NO");

	theSample = sample(asId, update);
	if ( theSample == -1 ) {
#ifdef NAN
		return NAN;
#else
		//TODO: better error handling...
		return pAs->minValue-1000;
#endif
	}

	return sampleToValue(findById (asId));

}
#endif


void DeviceAnalogSensors::run(void) {

	d_pid = getpid();
	LOG4CPP_INFO(log, "DeviceAS thread (%u) started", d_pid);

	// Eventually start the monitor threads (that could generat events)
	startMonitors();

}


}// namespace device
}// namespace controlbox
