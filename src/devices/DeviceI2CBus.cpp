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
//** Filename:      DeviceI2CBus.cpp
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

#include "DeviceI2CBus.ih"

namespace controlbox {
namespace device {

DeviceI2CBus * DeviceI2CBus::d_instance = 0;

DeviceI2CBus::DeviceI2CBus(std::string const & logName) :
	Device(Device::DEVICE_SINGALS, logName, logName),
	d_config(Configurator::getInstance()),
	d_devAddr(0x00),
	log(Device::log) {

	LOG4CPP_DEBUG(log, "DeviceI2CBus(logName=%s)", logName.c_str());

	// Registering device into the DeviceDB
	dbReg();

	d_devpath = d_config.param("bus_i2c_devpath", DEFAULT_BUS_I2C_DEVPATH);
	LOG4CPP_INFO(log, "Using I2C adapter device [%s]", d_devpath.c_str());

	initDevice();

}

exitCode DeviceI2CBus::initDevice() {

	if ((fd_dev = open(d_devpath.c_str(), O_RDWR))<0) {
		LOG4CPP_ERROR(log, "failed to open device [%s], %s\n", d_devpath.c_str(), strerror(errno));
		fd_dev = 0;
		return I2C_DEV_FAILED;
	}

	return OK;

}

DeviceI2CBus::~DeviceI2CBus() {

	LOG4CPP_INFO(log, "Stopping DeviceI2CBus");
	::close(fd_dev);

}

DeviceI2CBus * DeviceI2CBus::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceI2CBus(logName);
	}
	return d_instance;

}

exitCode DeviceI2CBus::selectDevice(int addr) {
	int result = 0;

	if (!fd_dev) {
		LOG4CPP_ERROR(log, "Device not open");
		return I2C_DEV_NOT_OPEN;
	}

	LOG4CPP_DEBUG(log, "Select device [0x%02X]", addr);

	if ( addr == d_devAddr )
		return OK;

#ifdef CONTROLBOX_CRIS
	result = ioctl(fd_dev, I2C_SLAVE_FORCE, addr);
#endif
	if ( result < 0) {
		LOG4CPP_ERROR(log, "Slave 0x%02X access error: %s\n", addr, strerror(errno));
		return I2C_DEV_ACCESS_ERROR;
	}

	d_devAddr = addr;

	return OK;
}

exitCode DeviceI2CBus::read(int addr, t_i2cCommand * cmd, short & cmdLen, t_i2cReg * reg, short regLen) {
	short i;
	int result;

	d_busLock.enterMutex();

	selectDevice(addr);

	i = 0;
	while ( (i < cmdLen) && (i < regLen) ) {

		result = ::write(fd_dev, &cmd[i], 1);
		if ( result != 1) {
			LOG4CPP_ERROR(log, "Write cmd [0x%02X] failed: %s\n", reg[i], strerror(errno));
			cmdLen = i;
			return I2C_DEV_ACCESS_ERROR;
		}

		result = ::read(fd_dev, &reg[i], 1);
		if ( result != 1) {
			LOG4CPP_ERROR(log, "Read cmd [0x%02X] failed: %s\n", reg[i], strerror(errno));
			cmdLen = i;
			return I2C_DEV_ACCESS_ERROR;
		}

		i++;

	}

	d_busLock.leaveMutex();

	return OK;
}

}// namespace device
}// namespace controlbox
