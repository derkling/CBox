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
//** Filename:      DeviceSignal.cpp
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

#include "DeviceSignalsAxis.ih"

namespace controlbox {
namespace device {

DeviceSignalsAxis * DeviceSignalsAxis::d_instance = 0;

DeviceSignalsAxis::DeviceSignalsAxis(std::string const & logName) :
	DeviceSignal(logName) {
	t_intMask curMask;
	int err;

/*
#ifdef CONTROLBOX_CRIS

	// Opening input port
	if ((fd_dev = open(d_devpath.c_str(), O_RDWR))<0) {
		LOG4CPP_ERROR(log, "failed to open device [%s], %s\n", d_devpath.c_str(), strerror(errno));
		fd_dev = 0;
		return INT_DEV_FAILED;
	}

	// Configuring OUTPUT Pins
	curMask = 0x0C;
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_SETGET_OUTPUT), &curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set output lines to [0x%02X], %s", curMask, strerror(errno));
	}
	LOG4CPP_DEBUG(log, "Outputs configuration [0x%02X]", curMask);

	// Configuring INPUT Pins
	curMask = 0xF3;
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_SETGET_INPUT), &curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set input lines to [0x%02X], %s", curMask, strerror(errno));
	}
	LOG4CPP_DEBUG(log, "Inputs configuration [0x%02X]", curMask);

	// Reading current levels state
	getLevels(d_levels);

	// Resetting all interrupts (especially from LED1, LED2 and SW)
	curMask = 0xFF;
	LOG4CPP_DEBUG(log, "Resetting alarms configuration for lines [0x%02X]", curMask);
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_CLRALARM), curMask );
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to clear input lines alarms, %s", strerror(errno));
	}
	LOG4CPP_DEBUG(log, "All alarms have been cleared [0x%02X]", curMask);

	// Initializing LIGHT interrupt
	// NOTE LIGHT interrupt line is LOW when dark (aka device box is closed)
	d_isDeviceOpen = ( d_levels & (0x1 << DEVICESIGNALS_DEVICEOPEN_LINE) );
	LOG4CPP_INFO(log, "cBox device %s", d_isDeviceOpen ? "OPENED" : "CLOSED" );
	LOG4CPP_DEBUG(log, "Configuring LIGHT interrupt on PA%d", DEVICESIGNALS_DEVICEOPEN_LINE);
	handlersNumber[INT_DEVICEOPEN] = 1;
	confInterrupt( (1<<DEVICESIGNALS_DEVICEOPEN_LINE), d_isDeviceOpen);

// 	// Initializing ODO-GPS interrupt
// 	// NOTE ODO-GPS interrupt line is LOW when DIGITAL state is changed
// 	d_isOdoGpsAlarm = !( d_levels & (0x1 << DEVICESIGNALS_ODOGPS_LINE) );
// 	confInterrupt( (1<<DEVICESIGNALS_ODOGPS_LINE), !d_isOdoGpsAlarm);
// 	LOG4CPP_DEBUG(log, "Configuring ODO-GPS interrupt on PA%d", DEVICESIGNALS_ODOGPS_LINE);
	handlersNumber[INT_ODOGPS] = 0;
	d_isOdoGpsAlarm = false;

	// Initializing BATTERY interrupt
	// NOTE battery interrupt line is LOW when running on battery
	d_isBatteryPowered = !( d_levels & (0x1 << DEVICESIGNALS_BATTERY_LINE) );
	LOG4CPP_INFO(log, "cBox %s powered", d_isBatteryPowered ? "BATTERY" : "AC" );
	LOG4CPP_DEBUG(log, "Configuring BATTERY interrupt on PA%d", DEVICESIGNALS_BATTERY_LINE);
	handlersNumber[INT_BATTERY] = 1;
	confInterrupt( (1<<DEVICESIGNALS_BATTERY_LINE), !d_isBatteryPowered);

	// Initializing MMC interrupt
	// NOTE MMC interrupt line is LOW when MMC is present
	d_isMmcPresent = !( d_levels & (0x1 << DEVICESIGNALS_MMC_LINE) );
	LOG4CPP_INFO(log, "Memory is %sPRESENT", d_isMmcPresent ? "" : "NOT ");
	LOG4CPP_DEBUG(log, "Configuring MMC interrupt on PA%d", DEVICESIGNALS_MMC_LINE);
	handlersNumber[INT_MMC] = 1;
	confInterrupt( (1<<DEVICESIGNALS_MMC_LINE), !d_isMmcPresent);

// 	// Initializing GPIO interrupt
// 	// NOTE GPIO interrupt line is LOW when GPIO changed states
// 	d_isGpioAlarm = !( d_levels & (0x1 << DEVICESIGNALS_GPIO_LINE) );
// 	confInterrupt( (1<<DEVICESIGNALS_GPIO_LINE), !d_isGpioAlarm);
// 	LOG4CPP_DEBUG(log, "Configuring DS interrupt on PA%d", DEVICESIGNALS_GPIO_LINE);
	handlersNumber[INT_GPIO] = 0;
	d_isGpioAlarm = false;

#endif

	// Start the working thread
	this->start();

*/

}



DeviceSignalsAxis::~DeviceSignalsAxis() {
	t_hMap::iterator anH;

	::close(fd_dev);

}

DeviceSignals * DeviceSignalsAxis::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceSignalsAxis(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceSignalsAxis::getInstance()");
	return d_instance;

}



// NOTE using boLines require to switch the interrupt configuration each times
//	the input change!!!... otehrwise we will get an endless interrupt storm!
exitCode DeviceSignalsAxis::configureInterrupts(void) {

/*	
	int err = 0;
	t_intMask curMask = 0x0;

	LOG4CPP_DEBUG(log, "Updating interrupts - Lo [0x%02X], Hi [0x%02X]",
				d_curMask.loLines,
				d_curMask.hiLines);

#ifdef CONTROLBOX_CRIS

	curMask = d_curMask.loLines | d_curMask.hiLines | d_curMask.boLines;
	LOG4CPP_DEBUG(log, "Resetting alarms configuration for lines [0x%02X]", curMask);
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_CLRALARM), curMask );
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to clear input lines alarms, %s", strerror(errno));
		return INT_DEV_FAILED;
	}

	curMask = d_curMask.hiLines;
	LOG4CPP_DEBUG(log, "Configuring HIGH alarms for lines [0x%02X]", curMask);
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_HIGHALARM), curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set high alarms, %s", strerror(errno));
		return INT_DEV_FAILED;
	}

	curMask = d_curMask.loLines;
	LOG4CPP_DEBUG(log, "Configuring LOW alarms for lines [0x%02X]", curMask);
	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_LOWALARM), curMask);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to set low alarms, %s", strerror(errno));
		return INT_DEV_FAILED;
	}

#endif
*/
	return OK;

}

exitCode DeviceSignalsAxis::updateSignalStatus(t_signalMask & status) {
/*
	int err = 0;

	LOG4CPP_DEBUG(log, "Reading current input levels...");

#ifdef CONTROLBOX_CRIS
	if (!fd_dev) {
		LOG4CPP_ERROR(log, "Device not opened");
		return INT_DEV_FAILED;
	}

	err = ioctl(fd_dev, _IO(ETRAXGPIO_IOCTYPE, IO_READ_INBITS), &levels);
	if (err == -1) {
		LOG4CPP_ERROR(log, "Failed to read input lines, %s", strerror(errno));
		return INT_DEV_FAILED;
	}
	LOG4CPP_ERROR(log, "Inputs levels [0x%02X]", levels);
#else
# warning Using WaitForInterrupt DUMMY implementation
	LOG4CPP_DEBUG(log, "Dummy getLevels return [0x00]");
	levels = 0x00;
#endif
*/
	return OK;

}

exitCode DeviceSignalsAxis::waitInterrupt(t_signalMask & status) {
/*
exitCode DeviceSignals::waitForIntr(t_intMask & levels) {
	fd_set set;
	int err = 0;
	exitCode result = OK;

	LOG4CPP_DEBUG(log, "Waiting for interrupts...");

#ifdef CONTROLBOX_CRIS
	if (!fd_dev) {
		LOG4CPP_ERROR(log, "Device not opened");
		return INT_WFI_FAILED;
	}

	FD_ZERO(&set);
	FD_SET(fd_dev, &set);
	err = select(fd_dev+1, &set, NULL, NULL, NULL);
	if ( err < 0) {
		// An EINTR is received every time an handler is added
		// this should not result in an warning
		LOG4CPP_DEBUG(log, "select failed, %s", strerror(errno));
		return INT_SELECT_FAILED;
	}

	result = getLevels(levels);
	if ( result == OK ) {
		d_changedBits = d_levels ^ levels;
		d_levels = levels;
	}

//----- Reconfiguring interrupt levels


	return result;

#else
# warning Using WaitForInterrupt DUMMY implementation
	::sleep(60);
	LOG4CPP_DEBUG(log, "Dummy WaitForInterrupt tirggered");
	return OK;
#endif
*/

}


}// namespace device
}// namespace controlbox
