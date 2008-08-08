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
# define PIN_GPRS1_STATE 		PG0
# define PORT_GPRS1_RST 		PORTG
# define PIN_GPRS1_RST 			PG0

# define PORT_GPRS2_PWR 		PORTG
# define PIN_GPRS2_PWR 			PG5
# define PORT_GPRS2_STATE 		PORTG
# define PIN_GPRS2_STATE 		PG24
# define PORT_GPRS2_RST 		PORTG
# define PIN_GPRS2_RST 			PG24

# define PORT_MUX1			PORTG
# define PIN_MUX1_BIT1 			PG28
# define PIN_MUX1_BIT2			PG29
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
	// GPRS control logic:
	// - normally input, OUTPUT = LOW to switch on
	// - normally input reading state (HIGH=ON, LOW=OFF), OUTPUT = LOW to reset
	// GPRS 1
	gpiosetdir(PORT_GPRS1_PWR,   DIRIN, PIN_GPRS1_PWR);
	gpiosetdir(PORT_GPRS1_RST,   DIRIN, PIN_GPRS1_RST);
	gpiosetdir(PORT_GPRS1_STATE, DIRIN, PIN_GPRS1_STATE);
	// GPRS 2
	gpiosetdir(PORT_GPRS2_PWR,   DIRIN, PIN_GPRS2_PWR);
	gpiosetdir(PORT_GPRS2_RST,   DIRIN, PIN_GPRS2_RST);
	gpiosetdir(PORT_GPRS2_STATE, DIRIN, PIN_GPRS2_STATE);
	// MUX1
	gpiosetdir(PORT_MUX1, DIROUT, PIN_MUX1_BIT1);
	gpiosetdir(PORT_MUX1, DIROUT, PIN_MUX1_BIT2);
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
	unsigned char pinPowerPort;
	t_gpioLine pinState, pinPower;
	t_gpioState curState;

	switch(gprs) {
	case GPRS1:
		pinState = GPIO_GPRS1_STATE;
		pinPower = GPIO_GPRS1_PWR;
		pinPowerPort = PORT_GPRS1_PWR;
	break;
	case GPRS2:
		pinState = GPIO_GPRS2_STATE;
		pinPower = GPIO_GPRS2_PWR;
		pinPowerPort = PORT_GPRS2_PWR;
	break;
	default:
		return GENERIC_ERROR;
	}

	curState = gpioRead(pinState);

	LOG4CPP_DEBUG(log, "Checking GPRS-%s state: %s [%d]",
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

		LOG4CPP_DEBUG(log, "Switching %s GPRS-%s",
				  (state==GPIO_ON) ? "ON" : "OFF",
				  (gprs==GPRS1) ? "1" : "2");

// NOTE on AXIS we use OG2... this could be configured only as OUTPUT
		gpioWrite(GPIO_SET, pinPower);
		::sleep(1);
		gpioWrite(GPIO_CLEAR, pinPower);


/* FOR USE WITHOUT TRANSISTOR AND I/O PORTS
		gpiosetdir(pinPowerPort, DIROUT, pinPower);
// 		gpioWrite(GPIO_SET, pinPower);
		//gpioWrite(GPIO_CLEAR, PIN_GPRS1_PWR); // -- if not using transistor
		::sleep(1);
// 		gpioWrite(GPIO_CLEAR, pinPower);
		gpiosetdir(pinPowerPort, DIRIN, pinPower);
*/
		if (state == GPIO_ON) {
			// At power-on: waiting GPRS AT interface to startup
			::sleep(10);

			if ( gprsIsPowered(gprs) ) {
				LOG4CPP_DEBUG(log, "GPRS %s powered up",
						(gprs == GPRS1) ? "1" : "2");
				return OK;
			}

			LOG4CPP_WARN(log, "Failed powering on GPRS-%s",
						(gprs == GPRS1) ? "1" : "2");
			return GPRS_DEVICE_POWER_ON_FAILURE;
		}

	}

	return OK;
}

exitCode DeviceGPIO::gprsReset(unsigned short gprs) {
	unsigned char port;
	unsigned int pin;

	if ( gprsIsPowered(gprs) ) {

		switch(gprs) {
		case GPRS1:
			port = PORT_GPRS1_RST;
			pin = PIN_GPRS1_RST;
			break;
		case GPRS2:
			port = PORT_GPRS2_RST;
			pin = PIN_GPRS2_RST;
			break;
		default:
			return GENERIC_ERROR;
		}

		LOG4CPP_INFO(log, "Resetting GPRS-%s [P%c%u]",
				(gprs==GPRS1) ? "1" : "2",
				port, pin-1);

		gpioclearbits(port, pin);
		gpiosetdir(port, DIROUT, pin);
		::sleep(2);
		gpiosetdir(port, DIRIN, pin);

	}

	// Ensuring modem is powered up
	return gprsPower(gprs, GPIO_ON);
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
	LOG4CPP_DEBUG(log, "Waiting for MUX1 lock...");

	d_ttyLock[MUX1].enterMutex ();
	LOG4CPP_DEBUG(log, "MUX1 lock acquired by port [%c]", 'A'-1+port);

	// Switching MUX to te required port
	ttySelect(port);

	return OK;
}

exitCode DeviceGPIO::ttyUnLock(unsigned short port) {

	if (port==TTY_SINGLE)
		return OK;

	d_ttyLock[MUX1].leaveMutex();
	LOG4CPP_DEBUG(log, "MUX1 lock released by port [%c]", 'A'-1+port);

	return OK;
}

exitCode DeviceGPIO::ttySelect(unsigned short port) {
	t_gpioLine s0, s1;

	if (port==TTY_SINGLE)
		return OK;

	// Mapping Port A[1] on configuration bits [00]
	port-=1;

	if (port & (unsigned short)0x1) {
		gpioWrite(GPIO_SET, GPIO_MUX1_BIT1);
	} else {
		gpioWrite(GPIO_CLEAR, GPIO_MUX1_BIT1);
	}

	if (port & (unsigned short)0x2) {
		gpioWrite(GPIO_SET, GPIO_MUX1_BIT2);
	} else {
		gpioWrite(GPIO_CLEAR, GPIO_MUX1_BIT2);
	}

	// Wait few time to ensure proper switching
	ost::Thread::sleep(200);

	LOG4CPP_INFO(log, "TTY mux switched to port [%c]", 'A'+port);

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
		case GPIO_GPRS1_RST:
			port = PORT_GPRS1_RST;
			pin = PIN_GPRS1_RST;
			break;
		case GPIO_GPRS2_PWR:
			port = PORT_GPRS2_PWR;
			pin = PIN_GPRS2_PWR;
			break;
		case GPIO_GPRS2_RST:
			port = PORT_GPRS1_RST;
			pin = PIN_GPRS1_RST;
			break;
		case GPIO_MUX1_BIT1:
			port = PORT_MUX1;
			pin = PIN_MUX1_BIT1;
			break;
		case GPIO_MUX1_BIT2:
			port = PORT_MUX1;
			pin = PIN_MUX1_BIT2;
			break;
		default:
			LOG4CPP_WARN(log, "Writing on unsupported pin");
			return GENERIC_ERROR;
	}

	switch(op) {
		case GPIO_SET:
			LOG4CPP_DEBUG(log, "Setting P%c%u",
						  port, pin-1);
			gpiosetbits(port, pin);
			break;
		case GPIO_CLEAR:
			LOG4CPP_DEBUG(log, "Resetting P%c%u",
						  port, pin-1);
			gpioclearbits(port, pin);
			break;
		case GPIO_TOGGLE:
			LOG4CPP_DEBUG(log, "Toggling P%c%u",
						  port, pin-1);
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
	unsigned short level;

 	switch(gpio) {
		case GPIO_GPRS1_STATE:
		 	LOG4CPP_DEBUG(log, "Reading GPIO_GPRS1_STATE");
			port = PORT_GPRS1_STATE;
			pin = PIN_GPRS1_STATE;
			break;
		case GPIO_GPRS2_STATE:
		 	LOG4CPP_DEBUG(log, "Reading GPIO_GPRS2_STATE");
			port = PORT_GPRS2_STATE;
			pin = PORT_GPRS2_STATE;
			break;
		case GPIO_MUX1_BIT1:
		 	LOG4CPP_DEBUG(log, "Reading PIN_MUX1_BIT1");
			port = PORT_MUX1;
			pin = PIN_MUX1_BIT1;
			break;
		case GPIO_MUX1_BIT2:
		 	LOG4CPP_DEBUG(log, "Reading PIN_MUX1_BIT2");
			port = PORT_MUX1;
			pin = PIN_MUX1_BIT2;
			break;
		default:
		 	LOG4CPP_DEBUG(log, "Undefined GPIO line to read [%d]", gpio);
			return GPIO_UNDEF;
	}

	level = gpiogetbits(port, pin);
	LOG4CPP_INFO(log, "Reading P%c%u [%hu]", port, pin-1, level);

	if (level) {
		return GPIO_ON;
	}

	return GPIO_OFF;
}

}// namespace device
}// namespace controlbox
