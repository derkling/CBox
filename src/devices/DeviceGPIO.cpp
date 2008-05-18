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


#include "DeviceGPIO.ih"


// Axis implementation of the DeviceGPIO interface
#ifdef CONTROLBOX_CRIS
# define PORT_GPRS1_PWR 		PORTG
# define PIN_GPRS1_PWR 			PG2
# define PORT_GPRS1_STATE 		PORTG
# define PIN_GPRS1_STATE 		PG2
# define PORT_GPRS1_RST 		PORTG
# define PIN_GPRS1_RST 			PG0
# define PORT_GPRS2_PWR 		PORTG
# define PIN_GPRS2_PWR 			PG5
# define PORT_GPRS2_STATE 		PORTG
# define PIN_GPRS2_STATE 		PG5
# define PORT_GPRS2_RST 		PORTG
# define PIN_GPRS2_RST 			PG24
# define PORT_TTY1_MUX1 		PORTG
# define PIN_TTY1_MUX1 			PG28
# define PORT_TTY1_MUX2			PORTG
# define PIN_TTY1_MUX2 			PG29
# define PORT_TTY2_MUX1			PORTG
# define PIN_TTY2_MUX1 			PG28
# define PORT_TTY2_MUX2			PORTG
# define PIN_TTY2_MUX2 			PG29
#endif


namespace controlbox {
namespace device {

DeviceGPIO * DeviceGPIO::d_instance = 0;

DeviceGPIO * DeviceGPIO::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceGPIO(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceFactory::getInstance()");

	return d_instance;
}

DeviceGPIO::DeviceGPIO(std::string const & logName) :
	Device(Device::DEVICE_GPIO, 0, logName),
	d_config(Configurator::getInstance()),
	log(Device::log) {
	DeviceFactory * df = DeviceFactory::getInstance();

	LOG4CPP_DEBUG(log, "DeviceGPIO(const std::string &, bool)");

	// Configuring pin directions
#ifdef CONTROLBOX_CRIS
	// GPRS 1
	gpiosetdir(PORT_GPRS1_PWR, DIROUT, PIN_GPRS1_PWR);
	gpiosetdir(PORT_GPRS2_PWR, DIROUT, PIN_GPRS2_PWR);
	// GPRS 2
	gpiosetdir(PORT_GPRS1_STATE, DIRIN, PIN_GPRS1_STATE);
	gpiosetdir(PORT_GPRS2_STATE, DIRIN, PIN_GPRS2_STATE);
	// TTY1 mux
	gpiosetdir(PORT_TTY1_MUX1, DIROUT, PIN_TTY1_MUX1);
	gpiosetdir(PORT_TTY1_MUX2, DIROUT, PIN_TTY1_MUX2);
	// TTY2 mux
	gpiosetdir(PORT_TTY2_MUX1, DIROUT, PIN_TTY2_MUX1);
	gpiosetdir(PORT_TTY2_MUX2, DIROUT, PIN_TTY2_MUX2);
#endif


	// Registering device into the DeviceDB
	dbReg();

//	// Initialize sensors samples by reading inputs
//	d_sysfsbase = d_config.param("DigitalSensor_sysfsbase", DS_DEFAULT_SYSFSBASE);
//	LOG4CPP_INFO(log, "Using sysfsbase [%s]", d_sysfsbase.c_str());

}

DeviceGPIO::~DeviceGPIO() {
}

exitCode DeviceGPIO::gprsPower(unsigned short gprs, t_gpioState state) {
	bool gprsSwitch = false;
	t_gpioLine pinState, pinPower;
	t_gpioState curState;

	switch(gprs) {
	case GPRS1:
		pinState = GPIO_GPRS1_STATE;
		pinPower = GPIO_GPRS1_PWR;
	break;
	case GPRS2:
		pinState = GPIO_GPRS2_STATE;
		pinPower = GPIO_GPRS2_PWR;
	break;
	default:
		return GENERIC_ERROR;
	}

	curState = gpioRead(pinState);

	LOG4CPP_DEBUG(log, "Checking GPRS %s state: %s [%d]",
				  (gprs==GPRS1) ? "1" : "2",
				  (curState==GPIO_ON) ? "ON" : "OFF",
				  curState);

	switch(state) {
		case GPIO_OFF:
			if (curState == GPIO_ON) {
				gprsSwitch = true;
			}
			break;
		case GPIO_ON:
			if (curState == GPIO_OFF) {
				gprsSwitch = true;
			}
			break;
		default:
			LOG4CPP_WARN(log, "Pin state is unknowed");
	}

	if (gprsSwitch) {

		LOG4CPP_DEBUG(log, "Switching %s GPRS %s",
				  (state==GPIO_ON) ? "ON" : "OFF",
				  (gprs==GPRS1) ? "1" : "2");

		gpioWrite(GPIO_SET, pinPower);
		::sleep(1);
		gpioWrite(GPIO_CLEAR, pinPower);

		if (state==GPIO_ON) {
			// At power-on: waiting GPRS AT interface to startup
			::sleep(10);
			LOG4CPP_DEBUG(log, "GPRS %s powered up",
								(gprs==GPRS1) ? "1" : "2");
		}

	}

	return OK;
}

exitCode DeviceGPIO::gprsReset(unsigned short gprs) {
	t_gpioLine pinReset;
	t_gpioState curState;

	LOG4CPP_DEBUG(log, "Resetting GPRS %s", (gprs==GPRS1) ? "1" : "2");

	switch(gprs) {
	case GPRS1:
		pinReset = GPIO_GPRS1_RST;
	break;
	case GPRS2:
		pinReset = GPIO_GPRS2_RST;
	break;
	default:
		return GENERIC_ERROR;
	}

	gpioWrite(GPIO_SET, pinReset);
	::sleep(1);
	gpioWrite(GPIO_CLEAR, pinReset);
	::sleep(5);

	// Ensuring modem is powered up
	gprsPower(gprs, GPIO_ON);


#if 0
	if ( gprsIsPowered(gprs) ) {
		gprsPower(gprs, GPIO_OFF);
		::sleep(5);
	}
	gprsPower(gprs, GPIO_ON);
#endif
	return OK;
}

bool DeviceGPIO::gprsIsPowered(unsigned short gprs) {
	t_gpioState state = GPIO_OFF;

	switch(gprs) {
		case GPRS1:
			state = gpioRead(GPIO_GPRS1_STATE);
		break;
		case GPRS2:
			state = gpioRead(GPIO_GPRS2_STATE);
		break;
	}

	if ( state == GPIO_ON)
		return true;

	return false;
}


exitCode DeviceGPIO::ttyLock(unsigned short port) {

	if (port==TTY_SINGLE)
		return OK;

	// Acquiring mux lock to satefly use the port
	if (port & 0x8) {
		LOG4CPP_DEBUG(log, "Entering TTY2 mutex lock...");
		d_ttyLock[TTY2].enterMutex ();
		LOG4CPP_DEBUG(log, "TTY2 mutex lock acquired");
	} else {
		LOG4CPP_DEBUG(log, "Entering TTY1 mutex lock...");
		d_ttyLock[TTY1].enterMutex ();
		LOG4CPP_DEBUG(log, "TTY1 mutex lock acquired");
	}

	// Switching MUX to te required port
	ttySelect (port);

	return OK;
}

exitCode DeviceGPIO::ttyUnLock(unsigned short port) {

	if (port==TTY_SINGLE)
		return OK;

	if (port & 0x8) {
		LOG4CPP_DEBUG(log, "Releasing TTY2 mutex lock");
		d_ttyLock[TTY2].leaveMutex ();
	} else {
		LOG4CPP_DEBUG(log, "Releasing TTY1 mutex lock");
		d_ttyLock[TTY1].leaveMutex ();
	}

	return OK;
}

exitCode DeviceGPIO::ttySelect(unsigned short port) {
	t_gpioLine s0, s1;

	if (port==TTY_SINGLE)
		return OK;

	if (port & 0x8) {
		s0 = GPIO_TTY1_MUX1;
		s1 = GPIO_TTY1_MUX2;
	} else {
		s0 = GPIO_TTY2_MUX1;
		s1 = GPIO_TTY2_MUX2;
	}

	if (port & 0x2) {
		gpioWrite(GPIO_SET, s0);
	} else {
		gpioWrite(GPIO_CLEAR, s0);
	}

	if (port & 0x4) {
		gpioWrite(GPIO_SET, s1);
	} else {
		gpioWrite(GPIO_CLEAR, s1);
	}

	// Wait few time to ensure proper switching
	ost::Thread::sleep(200);

	return OK;
}

exitCode DeviceGPIO::gpioWrite(t_gpioOperation op, t_gpioLine gpio) {
	unsigned char port;
	unsigned int pin;

 	switch(gpio) {
		case GPIO_GPRS1_PWR:
			port = PORT_GPRS1_PWR;
			pin = PIN_GPRS1_PWR;
			break;
		case GPIO_GPRS2_PWR:
			port = PORT_GPRS2_PWR;
			pin = PIN_GPRS2_PWR;
			break;
		case GPIO_TTY1_MUX1:
			port = PORT_TTY1_MUX1;
			pin = PIN_TTY1_MUX1;
			break;
		case GPIO_TTY1_MUX2:
			port = PORT_TTY1_MUX2;
			pin = PIN_TTY1_MUX2;
			break;
		case GPIO_TTY2_MUX1:
			port = PORT_TTY2_MUX1;
			pin = PIN_TTY2_MUX1;
			break;
		case GPIO_TTY2_MUX2:
			port = PORT_TTY2_MUX2;
			pin = PIN_TTY2_MUX2;
			break;
		default:
				 return GENERIC_ERROR;
	}

	switch(op) {
		case GPIO_SET:
			LOG4CPP_DEBUG(log, "Setting pin %u on port %hu",
						  pin, port);
			gpiosetbits(port, pin);
			break;
		case GPIO_CLEAR:
			LOG4CPP_DEBUG(log, "Resetting pin %u on port %hu",
						  pin, port);
			gpioclearbits(port, pin);
			break;
		case GPIO_TOGGLE:
			LOG4CPP_DEBUG(log, "Toggling pin %u on port %hu",
						  pin, port);
			gpiotogglebit(port, pin);
			break;
		default:
				 return GENERIC_ERROR;
	}

	return OK;
}


DeviceGPIO::t_gpioState DeviceGPIO::gpioRead(t_gpioLine gpio) {
	unsigned char port;
	unsigned int pin;
	unsigned int level;

 	switch(gpio) {
		case GPIO_GPRS1_STATE:
		 	LOG4CPP_DEBUG(log, "Reading GPIO_GPRS1_STATE");
			port = PORT_GPRS1_PWR;
			pin = PIN_GPRS1_PWR;
			break;
		case GPIO_GPRS2_STATE:
		 	LOG4CPP_DEBUG(log, "Reading GPIO_GPRS2_STATE");
			port = PORT_GPRS2_PWR;
			pin = PIN_GPRS2_PWR;
			break;
		case GPIO_TTY1_MUX1:
		 	LOG4CPP_DEBUG(log, "Reading GPIO_TTY1_MUX1");
			port = PORT_TTY1_MUX1;
			pin = PIN_TTY1_MUX1;
			break;
		case GPIO_TTY1_MUX2:
		 	LOG4CPP_DEBUG(log, "Reading GPIO_TTY1_MUX2");
			port = PORT_TTY1_MUX2;
			pin = PIN_TTY1_MUX2;
		break;
		case GPIO_TTY2_MUX1:
		 	LOG4CPP_DEBUG(log, "Reading GPIO_TTY2_MUX1");
			port = PORT_TTY2_MUX1;
			pin = PIN_TTY2_MUX1;
			break;
		case GPIO_TTY2_MUX2:
		 	LOG4CPP_DEBUG(log, "Reading GPIO_TTY2_MUX2");
			port = PORT_TTY2_MUX2;
			pin = PIN_TTY2_MUX2;
			break;
		default:
		 	LOG4CPP_DEBUG(log, "Undefined GPIO line to read [%d]", gpio);
			return GPIO_UNDEF;
	}

	if (gpiogetbits(port, pin))
		return GPIO_ON;

	return GPIO_OFF;
}

}// namespace device
}// namespace controlbox
