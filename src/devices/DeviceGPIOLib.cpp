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


#include "DeviceGPIOLib.ih"

#ifdef CONTROLBOX_ARM

#else

#endif


namespace controlbox {
namespace device {

DeviceGPIOLib *DeviceGPIOLib::d_instance = 0;

DeviceGPIOLib::t_attrData DeviceGPIOLib::d_attr[ATTR_COUNT] = {
	{"gpio124", "GPRS_PWR", ATTR_VAL|ATTR_DIR, 0, 0},
	{"gpio123", "GPRS_RST", ATTR_VAL|ATTR_DIR, 0, 0},
	{"gpio122", "GPRS_IO",  ATTR_VAL|ATTR_DIR, 0, 0},
	{"gpio80",  "TTY_MUX0", ATTR_VAL, 0, 0},
	{"gpio81",  "TTY_MUX1", ATTR_VAL, 0, 0},
	{"gpio118", "ADC_MUX0", ATTR_VAL, 0, 0},
	{"gpio119", "ADC_MUX1", ATTR_VAL, 0, 0},
	{"gpio120", "ADC_MUX2", ATTR_VAL, 0, 0},
	{"gpio121", "ADC_MUX3", ATTR_VAL, 0, 0},
};


DeviceGPIO * DeviceGPIOLib::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceGPIOLib(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceFactory::getInstance()");

	return d_instance;
}


#ifdef CONTROLBOX_ARM

DeviceGPIOLib::DeviceGPIOLib(std::string const & logName) :
	DeviceGPIO(logName){

	int result;
	char path[SYSFS_PATH_MAX];
	int mntlen;
	int i;
	struct sysfs_attribute *attr;

	LOG4CPP_DEBUG(log, "DeviceGPIOLib(const std::string &, bool)");


	// Initializing libsysfs
	memset(d_mntPath, 0, SYSFS_PATH_MAX);
	result = sysfs_get_mnt_path(d_mntPath, SYSFS_PATH_MAX);
	if ( result ) {
		LOG4CPP_ERROR(log, "unable to find sysfs mount point");
		return;
	}
	mntlen = snprintf(path, SYSFS_PATH_MAX, "%s", d_mntPath);
	LOG4CPP_DEBUG(log, "sysfs interface [%s] initialized", d_mntPath);

	mntlen += snprintf(path+mntlen, SYSFS_PATH_MAX-mntlen,
				"/class/gpio/");

	// Opening all sysfs attributes
	for (i=0; i<ATTR_COUNT; i++) {

		LOG4CPP_DEBUG(log, "Opening attributes for %s (%s)",
				d_attr[i].descr, d_attr[i].name);


		if ( d_attr[i].mask & ATTR_DIR ) {

			snprintf(path+mntlen, SYSFS_PATH_MAX-mntlen,
					"%s/direction",
					d_attr[i].name);
			d_attr[i].dir = sysfs_open_attribute(path);
			if ( !d_attr[i].dir ) {
				LOG4CPP_ERROR(log, "unable to open %s (%s) direction attrribute",
						d_attr[i].descr, d_attr[i].name);
			}

		}

		if ( d_attr[i].mask & ATTR_VAL ) {

			snprintf(path+mntlen, SYSFS_PATH_MAX-mntlen,
					"%s/value",
					d_attr[i].name);
			d_attr[i].val = sysfs_open_attribute(path);
			if ( !d_attr[i].val ) {
				LOG4CPP_ERROR(log, "unable to open %s (%s) value attrribute");
			}

		}

	}

}

DeviceGPIOLib::~DeviceGPIOLib() {
	int i;

	// Closing all sysfs attributes
	for (i=0; i<ATTR_COUNT; i++) {

		LOG4CPP_DEBUG(log, "Closing attributes for %s (%s)",
				d_attr[i].descr, d_attr[i].name);

		if ( d_attr[i].mask & ATTR_DIR )
			sysfs_close_attribute(d_attr[i].dir);
		if ( d_attr[i].mask & ATTR_VAL )
			sysfs_close_attribute(d_attr[i].val);

	}

}

exitCode DeviceGPIOLib::getGpioDir(t_attrType type, DeviceGPIO::t_gpioDir & dir) {
	int result;

	// Reading gpio direction
	if ( !(d_attr[type].mask & ATTR_DIR) ||
			!d_attr[type].dir ) {
		return GPIO_ATTR_NODEV;
	}

	result = sysfs_read_attribute(d_attr[type].dir);
	if ( result ) {
		LOG4CPP_ERROR(log, "unable to read dir for attribute [%s (%s)]",
				d_attr[type].descr, d_attr[type].name);
		return GPIO_ATTR_READ_FAILURE;
	}

	if ( strcmp(d_attr[type].dir->value, "in") )
		dir = DIR_IN;
	else
		dir = DIR_OUT;

	LOG4CPP_DEBUG(log, "[%s (%s)] configured as %s",
			d_attr[type].descr, d_attr[type].name,
			(dir == DIR_IN) ? "INPUT" : "OUTPUT" );

	return OK;

}

exitCode DeviceGPIOLib::setGpioDir(t_attrType type, DeviceGPIO::t_gpioDir dir) {
	int result;
	const char *attr[] = {"out", "in"};
	const short len[] = {3, 2};

	// Reading gpio direction
	if ( ! (d_attr[type].mask & ATTR_DIR) ||
			!d_attr[type].dir ) {
		return GPIO_ATTR_WRITE_NOAUTH;
	}

	result = sysfs_write_attribute(d_attr[type].dir,
                                attr[dir], len[dir]);
	if ( result ) {
		LOG4CPP_ERROR(log, "unable to write dir for attribute %s (%s)",
				d_attr[type].descr, d_attr[type].name);
		return GPIO_ATTR_WRITE_FAILURE;
	}

	LOG4CPP_DEBUG(log, "%s [%s] configured as %s",
			d_attr[type].descr, d_attr[type].name,
			(dir == DIR_IN) ? "INPUT" : "OUTPUT" );

	return OK;

}

exitCode DeviceGPIOLib::getGpioVal(t_attrType type, int & val) {
	int result;

	// Reading gpio direction
	if ( ! (d_attr[type].mask & ATTR_VAL) ||
			!d_attr[type].val ) {
		return GPIO_ATTR_NODEV;
	}

	result = sysfs_read_attribute(d_attr[type].val);
	if ( result ) {
		LOG4CPP_ERROR(log, "unable to read val for attribute %s (%s)",
				d_attr[type].descr, d_attr[type].name);
		return GPIO_ATTR_READ_FAILURE;
	}

	result = sscanf(d_attr[type].val->value, "%d", &val);
	if ( !result ) {
		LOG4CPP_ERROR(log, "unable to parse val (%s) for attribute %s (%s)",
				val, d_attr[type].descr, d_attr[type].name);
		return GPIO_ATTR_READ_FAILURE;
	}

	LOG4CPP_DEBUG(log, "value of [%s (%s)] is %d",
			d_attr[type].descr, d_attr[type].name,
			val);

	return OK;

}

exitCode DeviceGPIOLib::setGpioVal(t_attrType type, int val) {
	int result;
	char buff[8];

	// Reading gpio direction
	if ( ! (d_attr[type].mask & ATTR_VAL) ||
			!d_attr[type].val ) {
		return GPIO_ATTR_NODEV;
	}

	result = snprintf(buff, 8, "%d", val);
	if ( result<0 ) {
		LOG4CPP_ERROR(log, "setting attribute buffer value failed");
		return GPIO_ATTR_WRITE_FAILURE;
	}
	result = sysfs_write_attribute(d_attr[type].val,
                                buff, result);
	if ( result ) {
		LOG4CPP_ERROR(log, "unable to write val for attribute %s (%s)",
				d_attr[type].descr, d_attr[type].name);
		return GPIO_ATTR_WRITE_FAILURE;
	}

	LOG4CPP_DEBUG(log, "value of %s [%s] is %d",
			d_attr[type].descr, d_attr[type].name,
			val);

	return OK;

}
#else

DeviceGPIOLib::DeviceGPIOLib(std::string const & logName) :
	DeviceGPIO(logName){

	LOG4CPP_DEBUG(log, "DeviceGPIOLib(const std::string &, bool)");
}

DeviceGPIOLib::~DeviceGPIOLib() {
}

exitCode DeviceGPIOLib::getGpioDir(t_attrType type, DeviceGPIO::t_gpioDir & dir) {
	dir = DIR_IN;
	return OK;
}

exitCode DeviceGPIOLib::setGpioDir(t_attrType type, DeviceGPIO::t_gpioDir dir) {
	return OK;
}

exitCode DeviceGPIOLib::getGpioVal(t_attrType type, int & val) {
	val = 0;
	return OK;
}

exitCode DeviceGPIOLib::setGpioVal(t_attrType type, int val) {
	return OK;
}

#endif

exitCode DeviceGPIOLib::gprsReset(unsigned short gprs) {
	int result;

	if ( gprsPowered(gprs) ) {

		LOG4CPP_INFO(log, "Resetting GPRS-%s",
				(gprs==GPRS1) ? "1" : "2");

		result = setGpioVal(GPRS_RST, 0);
		if ( result ) {
			LOG4CPP_ERROR(log, "resetting GPRS failed (set val 0)");
			return GPRS_DEVICE_RESET_FAILURE;
		}
		result = setGpioDir(GPRS_RST, DIR_OUT);
		if ( result ) {
			LOG4CPP_ERROR(log, "resetting GPRS failed (set dir out)");
			return GPRS_DEVICE_RESET_FAILURE;
		}
		::sleep(2);
		result = setGpioDir(GPRS_RST, DIR_IN);
		if ( result ) {
			LOG4CPP_ERROR(log, "resetting GPRS failed (set dir in)");
			return GPRS_DEVICE_RESET_FAILURE;
		}

	}

	// Ensuring modem is powered up
	return gprsPowerOn(gprs);

}

bool DeviceGPIOLib::gprsPowered(unsigned short gprs) {
	int result;
	int val;

	// Reading power state
	result = setGpioDir(GPRS_RST, DIR_IN);
	if ( result ) {
		LOG4CPP_ERROR(log, "reading GPRS state failed (set dir)");
		return true;
	}
	result = getGpioVal(GPRS_RST, val);
	if ( result ) {
		LOG4CPP_ERROR(log, "reading GPRS state failed (read val)");
		return true;
	}

	LOG4CPP_DEBUG(log, "Checking GPRS state: %s [%d]",
			val ? "ON" : "OFF", val);

	if ( val )
		return true;

	return false;

}

exitCode DeviceGPIOLib::gprsSwitch(unsigned short gprs) {
	int result;

	result = setGpioDir(GPRS_PWR, DIR_OUT);
	if ( result )
		return GPRS_DEVICE_POWER_ON_FAILURE;

	result = setGpioVal(GPRS_PWR, LOW);
	if ( result )
		return GENERIC_ERROR;
	::sleep(1);

	result = setGpioDir(GPRS_PWR, DIR_IN);
	if ( result )
		return GPRS_DEVICE_POWER_ON_FAILURE;

	return OK;

}

exitCode DeviceGPIOLib::ttySelect(unsigned short port) {
	int val;
	exitCode result;

	if (port==TTYMUX_SINGLE)
		return OK;

	// Mapping Port A[1] on configuration bits [00]
	port-=1;

	val = (port & (unsigned short)0x1) ? 1 : 0;
	result = setGpioVal(TTY_MUX0, val);
	if ( result )
		return result;

	val = (port & (unsigned short)0x2) ? 1 : 0;
	result = setGpioVal(TTY_MUX1, val);
	if ( result )
		return result;

	// Wait few time to ensure proper switching
	ost::Thread::sleep(200);

	LOG4CPP_INFO(log, "TTY mux switched to channel [%c]", 'A'+port);

	return OK;
}

exitCode DeviceGPIOLib::adcSelect(unsigned short port) {
	int val;
	exitCode result;

	if (port==ACDMUX_SINGLE)
		return OK;

	// Mapping PORT1 on configuration bits [0000]
	port-=1;

	val = (port & (unsigned short)0x1) ? 1 : 0;
	result = setGpioVal(ADC_MUX0, val);
	if ( result )
		return result;

	val = (port & (unsigned short)0x2) ? 1 : 0;
	result = setGpioVal(ADC_MUX1, val);
	if ( result )
		return result;

	val = (port & (unsigned short)0x4) ? 1 : 0;
	result = setGpioVal(ADC_MUX2, val);
	if ( result )
		return result;

	val = (port & (unsigned short)0x8) ? 1 : 0;
	result = setGpioVal(ADC_MUX3, val);
	if ( result )
		return result;

	// Wait few time to ensure proper switching
	ost::Thread::sleep(20);

	LOG4CPP_INFO(log, "ADC mux switched to channel [%c]", 'A'+port);


	return OK;
}

}// namespace device
}// namespace controlbox
