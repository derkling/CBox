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


#ifndef _DEVICESERIAL_H
#  define _DEVICESERIAL_H

#  include <termios.h>
#  include <cc++/serial.h>
#  include <cc++/thread.h>
#  include <controlbox/base/Utility.h>
#  include <controlbox/base/Configurator.h>
#  include <controlbox/devices/DeviceGPIO.h>

/// The default device to open
#  define DEVICESERIAL_DEFAULT_DEVICE		"/dev/ttyS0:9600:8:n:1"
/// The default line terminator
#  define DEVICESERIAL_DEFAULT_LINE_TERMINATOR	"\r"
/// The default initialization string
#  define DEVICESERIAL_DEFAULT_INIT_STRING	""
/// The default minimum time [ms] to wait between a write and the following read
#  define DEVICESERIAL_DEFAULT_RESPONCE_DELAY	"500"
/// Timeout [ms] for non-blochking TTY reading
#  define DEVICESERIAL_DEFAULT_RESPONCE_TIMEOUT "3000"

namespace controlbox {
namespace device {

/// A DeviceSerial provide methods to handle communication using TTY ports.
/// Since we could use more than one TTY port, in the following params [BASE] must be
/// a string defining the root of each configuration parameter,<br>
/// Each module using this class must define the [base] to use in order
/// to load the correct parameters values.<br>
/// <h5>Configuration params availables for this class:</h5>
/// <ul>
///     <li>
///             <b>[BASE]_tty_device</b> - <i>Default: DEVICESERIAL_DEFAULT_DEVICE</i><br>
///             The device to use and the initial parameters immediately following
///             the device name in a single string, as in "/dev/ttyS0:9600,7,e,1",
///             as an example.
///             <br>
///     </li>
//TODO implement the usage of tty_device_required
///     <li>
///             <b>[BASE]_tty_device_required</b> - <i>Default: DEVICESERIAL_DEFAULT_DEVICE</i><br>
///             The TTY port required configuration, to be used in case the initial device
///             port configuration does not match that required by the application.<br>
///             Using this param one can pass initial serial device parameters immediately
///             following the device name in a single string,
///             as in "/dev/tty3a:9600,7,e,1", as an example.<br>
///     </li>
///     <li>
///             <b>[BASE]_tty_lineTerminator</b> - <i>Default: DEVICESERIAL_DEFAULT_LINE_TERMINATOR</i><br>
///             The line terminator (max. 2 characters) to use.<br>
///     </li>
///     <li>
///             <b>[BASE]_tty_initString</b> - <i>Default: DEVICESERIAL_DEFAULT_INIT_STRING</i><br>
///             The init string to send on port open.<br>
///     </li>
///     <li>
///             <b>[BASE]_tty_respDelay</b> - <i>Default: DEVICESERIAL_DEFAULT_RESPONCE_DELAY</i><br>
///             The minimum required delay [ms] between a write and the following read<br>
///     </li>
///     <li>
///             <b>[BASE]_tty_respTimeout</b> - <i>Default: DEVICESERIAL_DEFAULT_RESPONCE_TIMEOUT</i><br>
///             The maximum time [ms] to wait for a read.
///     </li>
/// </ul>
class DeviceSerial : public ost::ttystream {

      public:

	/// A generic list of strings.
	typedef vector < std::string > t_stringVector;

      protected:

	/// The Configurator to use for getting configuration params.
	Configurator & d_config;

	/// The TTY port configuration string.
	std::string d_ttyConfig;
		
	/// The TTY virtual port (multiplexd phisical port)
	unsigned short d_ttyMuxPort;
		
	/// The GPIO device used to handle multiplexed TTY ports
	DeviceGPIO * d_gpio;
		
	/// True when detached mode is enabled
	bool d_detachedMode;
	
	/// The line terminator.
	char d_lineTerminator[3];
	
	/// The initialization string.
	std::string d_initString;

	/// The minimum responce delay.
	unsigned int d_respDelay;

	/// The responce timeout.
	unsigned int d_respTimeout;
	
	/// The [BASE] to use for configuration parameters.
	std::string d_base;

	/// The port status (true if port is open)
	bool d_isOpen;

	/// The logger to use locally.
	log4cpp::Category & log;

      public:

	/// Create a new DeviceSerial.
	/// @param base the [BASE] to use for configuration parameters
	/// @param logName the log category, this name is prepended by the
	///         class namespace "controlbox.device."
	DeviceSerial(std::string const & base, std::string const & logName = "controlbox.device.DeviceSerial");

	/// Class destructor.
	~DeviceSerial();

	/// Open the serial port.
	/// @param init set true to send initialization string.
	/// @return OK on success.
	exitCode openSerial(bool init = false, t_stringVector * resp = 0);
	
	bool isOpen();

	/// Close the serial port.
	/// @param sync set true to force a flush of input/output buffers before
	///	closeing the device.
	/// @return OK on success
	exitCode closeSerial(bool sync = false);
		
	/// Configure detached operation mode
	/// In detached operation mode the port will be opened/closed on each
	/// read/write operation. This mode is disabled by default unless the port
	/// is multimplexed. In this case the port will operate by default in detached
	/// mode and this method could be used to switch temporareli into attached mode
	/// in order to optimize a sequence of port data exchange.
	exitCode detachedMode(bool enable = true);

	/// Write a string to a TTY port.
	/// @param cmd the command to send
	/// @param resp if specified, it will return the modem responce;
	///		there will be a vector entry for each reponce line.
	/// @note emtpy lines are discarded from responce lines
	/// @return OK on success, GPRS_TTY_MODEM_NOT_RESPONDING otherwise.
	exitCode sendSerial(std::string str, t_stringVector * resp = 0);
	
	/// Write a byte buffer to a TTY port.
	/// @param cmd the buffer to write
	/// @param len the number of bytes within buf
	/// @param resp (an optional) buffer for storing responce messages,
	///		by default we don't read anithing from the buffer
	/// @return OK on no error
	exitCode sendSerial(const char * buf, const int len);

	/// Read one (ore more) string from TTY port.
	exitCode readSerial(t_stringVector * resp = 0, bool blocking = false);

	/// Read a byte buffer from a TTY port up to d_lineTerminator or buffer full
	/// @param resp the buffer for the responce
	/// @param len the size of the buffer, at return the number of bytes received
	/// @param blocking set true for a bloching call
	///		(return once end has been received or the buffer is full)
	/// @return OK on success, an error otherwise
	exitCode readSerial(char * resp, int & len, bool blocking = false);

	void printHexBuf(const char * buf, unsigned len, char * str, unsigned size);

	void printBuf(const char * buf, unsigned len, char * str, unsigned size);

      protected:

	/// Build a configuration param name.
	/// @param param the param name, valid values are configuration params
	///                 substring following param radix and type
	inline std::string paramName(std::string param) {
		std::ostringstream ostr("");
		
		ostr << d_base << "_" << param;
		return ostr.str();
	}
	
};

}// namespace gprs
}// namespace controlbox
#endif
