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


#include "DeviceGPIOAxis.ih"

#ifdef CONTROLBOX_CRIS

// Axis implementation of the DeviceGPIO interface
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

#else

// DUMMY implementation of the DeviceGPIO interface
# define PORT_GPRS1_PWR 		0
# define PIN_GPRS1_PWR 			0
# define PORT_GPRS1_STATE 		0
# define PIN_GPRS1_STATE 		0
# define PORT_GPRS1_RST 		0
# define PIN_GPRS1_RST 			0

# define PORT_GPRS2_PWR 		0
# define PIN_GPRS2_PWR 			0
# define PORT_GPRS2_STATE 		0
# define PIN_GPRS2_STATE 		0
# define PORT_GPRS2_RST 		0
# define PIN_GPRS2_RST 			0

# define PORT_MUX1			0
# define PIN_MUX1_BIT1 			0
# define PIN_MUX1_BIT2			0

#endif


namespace controlbox {
namespace device {

DeviceGPIO * DeviceGPIOAxis::d_instance = 0;

DeviceGPIO * DeviceGPIOAxis::getInstance(std::string const & logName) {

	if ( !d_instance ) {
		d_instance = new DeviceGPIOAxis(logName);
	}

	LOG4CPP_DEBUG(d_instance->log, "DeviceFactory::getInstance()");

	return d_instance;
}

#ifdef CONTROLBOX_CRIS

DeviceGPIOAxis::DeviceGPIOAxis(std::string const & logName) :
	DeviceGPIO(logName) {

	LOG4CPP_DEBUG(log, "DeviceGPIOAxis(const std::string &, bool)");

	// Configuring pin directions
	// GPRS control logic:
	// - PWR: normally input, OUTPUT = LOW to switch on
	// - RST: normally input reading state (HIGH=ON, LOW=OFF), OUTPUT = LOW to reset
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

}

DeviceGPIOAxis::~DeviceGPIOAxis() {

}

exitCode DeviceGPIOAxis::gpioWrite(t_gpioOperation op, t_gpioLine gpio) {
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
		case GPIO_TTYMUX_B0:
			port = PORT_MUX1;
			pin = PIN_MUX1_BIT1;
			break;
		case GPIO_TTYMUX_B1:
			port = PORT_MUX1;
			pin = PIN_MUX1_BIT2;
			break;
		default:
			LOG4CPP_WARN(log, "Writing on unsupported pin");
			return GENERIC_ERROR;
	}

	switch(op) {
		case SET:
			LOG4CPP_DEBUG(log, "Setting P%c%u",
						  port, pin-1);
			gpiosetbits(port, pin);
			break;
		case CLEAR:
			LOG4CPP_DEBUG(log, "Resetting P%c%u",
						  port, pin-1);
			gpioclearbits(port, pin);
			break;
		case TOGGLE:
			LOG4CPP_DEBUG(log, "Toggling P%c%u",
						  port, pin-1);
			gpiotogglebit(port, pin);
			break;
		default:
			return GENERIC_ERROR;
	}

	return OK;
}


DeviceGPIOAxis::t_gpioState DeviceGPIOAxis::gpioRead(t_gpioLine gpio) {
	unsigned char port;
	unsigned int pin;
	unsigned short level = 0;

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
		case GPIO_TTYMUX_B0:
			LOG4CPP_DEBUG(log, "Reading GPIO_TTYMUX_B0");
			port = PORT_MUX1;
			pin = PIN_MUX1_BIT1;
			break;
		case GPIO_TTYMUX_B1:
			LOG4CPP_DEBUG(log, "Reading GPIO_TTYMUX_B1");
			port = PORT_MUX1;
			pin = PIN_MUX1_BIT2;
			break;
		default:
			LOG4CPP_DEBUG(log, "Undefined GPIO line to read [%d]", gpio);
			return UNDEF;
	}

	level = gpiogetbits(port, pin);
	LOG4CPP_INFO(log, "Reading P%c%u [%hu]", port, pin-1, level);

	if (level) {
		return HIGH;
	}

	return LOW;
}

#else

DeviceGPIOAxis::DeviceGPIOAxis(std::string const & logName) :
	DeviceGPIO(logName) {

	LOG4CPP_DEBUG(log, "DeviceGPIOAxis(const std::string &, bool)");
}

DeviceGPIOAxis::~DeviceGPIOAxis() {

}

exitCode DeviceGPIOAxis::gpioWrite(t_gpioOperation op, t_gpioLine gpio) {
	return OK;
}

DeviceGPIOAxis::t_gpioState DeviceGPIOAxis::gpioRead(t_gpioLine gpio) {
	return LOW;
}


#endif

exitCode DeviceGPIOAxis::gprsReset(unsigned short gprs) {
	unsigned char port;
	unsigned int pin;

	if ( gprsPowered(gprs) ) {

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

#ifdef CONTROLBOX_CRIS
		gpioclearbits(port, pin);
		gpiosetdir(port, DIROUT, pin);
		::sleep(2);
		gpiosetdir(port, DIRIN, pin);
#endif

	}

	// Ensuring modem is powered up
	return gprsPowerOn(gprs);
}

bool DeviceGPIOAxis::gprsPowered(unsigned short gprs) {
	t_gpioState state = LOW;

	switch(gprs) {
		case GPRS1:
			state = gpioRead(GPIO_GPRS1_STATE);
		break;
		case GPRS2:
			state = gpioRead(GPIO_GPRS2_STATE);
		break;
	}

	if ( state == HIGH)
		return true;

	return false;
}

exitCode DeviceGPIOAxis::gprsSwitch(unsigned short gprs) {
	unsigned char pinPowerPort;
	t_gpioLine pinPower;

	switch(gprs) {
	case GPRS1:
		pinPower = GPIO_GPRS1_PWR;
		pinPowerPort = PORT_GPRS1_PWR;
		break;
	case GPRS2:
		pinPower = GPIO_GPRS2_PWR;
		pinPowerPort = PORT_GPRS2_PWR;
		break;
	default:
		return GENERIC_ERROR;
	}

// NOTE on AXIS we use OG2... this could be configured only as OUTPUT
	gpioWrite(SET, pinPower);
	::sleep(1);
	gpioWrite(CLEAR, pinPower);

	return OK;
}


exitCode DeviceGPIOAxis::ttySelect(unsigned short port) {
// 	t_gpioLine s0, s1;

	if (port==TTYMUX_SINGLE)
		return OK;

	// Mapping Port A[1] on configuration bits [00]
	port-=1;

	if (port & (unsigned short)0x1) {
		gpioWrite(SET, GPIO_TTYMUX_B0);
	} else {
		gpioWrite(CLEAR, GPIO_TTYMUX_B0);
	}

	if (port & (unsigned short)0x2) {
		gpioWrite(SET, GPIO_TTYMUX_B1);
	} else {
		gpioWrite(CLEAR, GPIO_TTYMUX_B1);
	}

	// Wait few time to ensure proper switching
	ost::Thread::sleep(200);

	LOG4CPP_INFO(log, "TTY mux switched to port [%c]", 'A'+port);

	return OK;
}

exitCode DeviceGPIOAxis::adcSelect(unsigned short port) {
	return OK;
}


}// namespace device
}// namespace controlbox
