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


#ifndef _DEVICEGPRS_H
#  define _DEVICEGPRS_H

#  include <termios.h>
#  include <cc++/serial.h>
#  include <map>
#  include <controlbox/base/Utility.h>
#  include <controlbox/base/Configurator.h>
#  include <controlbox/base/Worker.h>
#  include <controlbox/base/Device.h>
#  include <controlbox/base/comsys/CommandGenerator.h>
#  include <controlbox/base/Querible.h>
#  include <controlbox/devices/DeviceSerial.h>

/// The maximun lenght for complete TTY device configurtation strings
#  define DEVICEGPRS_MAX_TTYCONFIG		64
/// The maximun lenght of a netlink name
#  define DEVICEGPRS_MAX_NETLINK_NAMELEN	8
/// The maximin number of network links supported by each modem
#  define DEVICEGPRS_MAX_NETLINKS		2

/// The default modem model (dummy modem: use ethernet connection)
#  define DEVICEGPRS_DEFAULT_GPRS_MODEL   	"0"
#  define DEVICEGPRS_DEFAULT_GPRS_DEVICE	"/dev/ttyUSB0:9600:8:n:1"
#  define DEVICEGPRS_MAX_FRIEND_SERVER		3
/// The default supported network APNs
#  define DEVICEGPRS_DEFAULT_NETLINKS		""


#  define DEVICEGPRS_DEFAULT_AT_RESPONCE_DELAY	"500"
#  define DEVICEGPRS_DEFAULT_AT_CMD_ESCAPE	"\r"
#  define DEVICEGPRS_DEFAULT_AT_INIT_STRING	""
/// Timeout [ms] for non-blochking TTY reading
#  define DEVICEGPRS_DEFAULT_AT_RESPONCE_TIMEOUT 3000


#  define DEVICEGPRS_DEFAULT_PPPD_LOGPATH	"/var"


/// The default SMS's service center (valid for the italian TIM network)
#   define DEVICEGPRS_DEFAULT_SMS_CSCA		"3359609600"

namespace controlbox {
namespace device {

/// A DeviceGPRS is a CommandGenerator for GPRS device management.
/// This class provide access to GSM/GPRS devices connected to
/// a TTY (serial) port. The class provide initialization
/// code and methods to tear up a GPRS connection and accessing
/// to others supported features as: modem state query, send SMS,
/// etc. <br>
/// <br>
/// <h5>Configuration params availables for this class:</h5>
///             Since we could use more than one GPRS module, in the following params [N] must be
///             a single digit number in the range [0-9]. The same digit must be used to delcare
///             configuration params for the same module.<br>
///             Multiple gprs link connections could be configured specifying needed AT params;
///             in the following params [linkname] shuld be substituted with a valid linkname
///             as specified by device_gprs_linkname params.
/// <ul>
///     <li>
///             <b>device_gprs_[N]_model</b> - <i>Default: DEVICEGPRS_DEFAULT_GPRS_MODEL</i><br>
///             The GPRS model used. Availables models are exported by t_gprs_models enumeration.
///             This params must be a t_gprs_models valid entry number.
///             @see t_gprs_models
///             @note by default when model=0 we use an ethernet connection.
///             <br>
///     </li>
///     <li>
///             <b>gprs_modem_[N]_links</b> - <i>Default: DEVICEGPRS_DEFAULT_NETLINKS</i><br>
///             The netlinks configurations supported by this modem.
///             This is a string with this format:<br>
///             <i>sim,linkid[:sim,linkid[...]]</i>
///     </li>
///     <li>
///             <b>gprs_modem_[N]_sms_csca</b> - <i>Default: DEVICEGPRS_DEFAULT_SMS_CSCA</i><br>
///             The numnber of GSM network operator's service center.<br>
///     </li>
///     <li>
///             this class use a DeviceSerial to control the TTY port; refer
///             to this class documentation for the list of availables configuration
///             parameters.
///             <br>
///     </li>
/// </ul>
/// @see CommandHandler
/// @see DeviceSerial
class DeviceGPRS:public comsys::CommandGenerator,
		public Device,
		public Worker {

      public:

	/// Generated Messages
	enum cmdType {
		GPRS_STATUS_UPDATE = (DEVICE_GPRS * SERVICES_RANGE) + 1,
	};
	typedef enum cmdType t_cmdType;

	/// The GPRS module model.
	enum gprs_models {
		DEVICEGPRS_MODEL_DUMMY = 0,	///> A dummy modem (using ethernet connection)
		DEVICEGPRS_MODEL_SONYERICSSON,	///> SonyEricsson Modem using AT command
		DEVICEGPRS_MODEL_ENFORA,	///> Enfora Enabler Modem using AT command interface
		DEVICEGPRS_MODEL_ENFORA_API,	///> Enfora Enabler Modem using propietary API interface
	};
	typedef enum gprs_models t_gprs_models;

	/// The running mode
	enum gprs_mode {
		DEVICEGPRS_MODE_COMMAND = 0,	///> AT Command mode
		DEVICEGPRS_MODE_DATA,		///> Data mode connection
		DEVICEGPRS_MODE_SMS,		///> Sending SMS
		DEVICEGPRS_MODE_VOICE,		///> Ongoing voice call
		DEVICEGPRS_MODE_API,		///> Using modem specific communication protocol
	};
	typedef enum gprs_mode t_gprs_mode;

	/// NET operative conditions.
	/// Define the possible state of a net link
	enum netStatus {
		LINK_DOWN = 0,		///> No network connection
		LINK_GOING_DOWN,	///> Terminating a network connection
		LINK_GOING_UP,		///> Starting a network connection
		LINK_UP			///> Active network connection
	};
	typedef enum netStatus t_netStatus;

// 	enum apiStatus {
// 		LINK_API_DOWN = 0,	///> The API interface is down
// 		LINK_API_GOING_DOWN,	///> The API interface is being actived
// 		LINK_API_GOING_UP,	///> The API interface is being disabled
// 		LINK_API_UP,		///> The API interface is Up
// 	};
// 	typedef enum apiStatus t_apiStatus;

	/// Network status description.
	/// @note this array entries should match the values of t_netStatus enum
	static const char *d_netStatusStr[];

	/// A linkname (i.e. an APN).
	typedef char t_linkname[DEVICEGPRS_MAX_NETLINK_NAMELEN];

	enum simSlot {
		SIM1 = 1,	///> first SIM slot
		SIM2,		///> second SIM slot
	};
	typedef enum simSlot t_simSlot;

	/// GPRS device identification numners
	struct identification {
		std::string manufactor;		///< Modem producer
		std::string model;		///< Modem model
		std::string revision;		///< Modem Hw revision number
		std::string imei;		///< IMEI code
		std::string imsi;		///< IMSI code
		std::string simSlot;		///< SIM's slot actually actived
	};
	typedef struct identification t_identification;

	/// A generic list of strings.
	typedef vector < std::string > t_stringVector;

	/// A netlink configuration
	struct netlink {
		t_simSlot	sim;		///> the sim to use
		std::string	pdpContext; 	///> the PDP Context for GPRS connections
		std::string	smsCsca;	///> SMS service center
		std::string	AtDial;		///> the Dial AT Command String
		t_stringVector	lserv;		///> the list of server to witch sync IP
	};
	typedef struct netlink t_netlink;

	/// Map linknames (APN) to their configuration
	typedef map<std::string, t_netlink*> t_supportedLinks;

	/// Map linknames (APN) to devices configured to handle that link
	typedef multimap<std::string, DeviceGPRS*> t_mapLink2DeviceGPRS;

	/// A ppp interface configuration
	struct pppd_configuration {
		t_linkname linkname;	///< The linkname
		char device[16];	///< the tty device used by the modem
		char interface[6];	///< the ppp network interface
		char speed[7];		///< the speed of the network link
		char ipLocal[16];	///< local IP address
		char ipRemote[16];	///< remote IP address
		char dns1[16];		///< first DNS server
		char dns2[16];		///< second DNS server
	};
	typedef struct pppd_configuration t_pppd_configuration;

      protected:

	/// The Configurator to use for getting configuration params
	Configurator & d_config;

	/// The module number.
	/// The module number is used to look for configuration params
	short d_module;

	/// GPRS module type
	t_gprs_models d_model;

	/// The DeviceSerial used to communicate with the modem
	DeviceSerial * d_tty;

	/// The GPRS module identification strings
	t_identification d_ids;

	/// The network links (i.e. APN) supported by this modem
	/// @note each modem could support up to DEVICEGPRS_MAX_NETLINKS
	t_supportedLinks d_supportedLinks;

	/// The current netlink name
	const std::string * d_curNetlinkName;

	/// The current netlink configuration
	t_netlink * d_curNetlinkConf;

	/// The currently used SIM's number
	std::string d_simNumber;

	/// The current module mode
	t_gprs_mode d_mode;

	/// The current NET link status
	t_netStatus d_netStatus;

	/// The PPP configuration
	t_pppd_configuration d_pppConf;

	/// Whatever the parser thread is running.
	/// When this is false, all public command methods will fails
	/// with an GPRS_PARSER_NOT_RUNNING exitCode.
	bool d_parserRunning;

	/// The PID of the PPP daemon
	pid_t d_pppdPid;

	/// The PPP daemon folder for logfile
	std::string d_pppdLogFolder;

	/// The PPP daemon logfile
	std::string d_pppdLogFile;

	/// Local IP address
	std::string d_localIP;

	/// Remote IP address
	std::string d_remoteIP;

	/// DNS1
	std::string d_dns1;

	/// DNS2
	std::string d_dns2;

	static t_mapLink2DeviceGPRS d_mapLink2DeviceGPRS;

	/// The logger to use locally.
	log4cpp::Category & log;

      public:

	/// Return a of the specified model-module DeviceGPRS.
	/// The returned device is initially disabled, in order to enable it
	/// a Dispatcher must be defined.
	/// @param module the module number, used to look for configuration
	///         params. It <b>must be<b> a single digit number in the
	///         range [0-9]
	/// @return a valid DeviceGPRS pointer on success, 0 on failure or
	///         unsupported model required.
	static DeviceGPRS *getInstance(unsigned short module,
					std::string const &logName =
				       "DeviceGPRS");

	/// Return an instante of a gprs modem supporting the specified linkname.
	/// @return 0 if the required linkname is not supported by the current
	///		configuration
	static DeviceGPRS *getInstance(std::string const & linkname,
					std::string const &logName =
				       "DeviceGPRS");

	/// Class destructor.
	virtual ~DeviceGPRS();

	/// PowerOn GPRS
	virtual exitCode powerOn(bool reset = false);

	/// PowerOff GPRS
	virtual exitCode powerOff();

	/// Setup a GPRS connection.
	/// Configure modem and switch to Data Mode for the
	/// specified linkname.
	/// @param linkname the name of the Data Mode to activate.
	/// This name is used as string prefix for the connection
	/// configuration params to be placed into
	/// the application configuration file.
	/// @return OK on success, the modem has been switched
	/// into Data Mode,
	/// GPRS_CONNECTION_FAILED on modem connection problems
	virtual exitCode connect(std::string const & linkname);

	/// Terminate the current GPRS session.
	virtual exitCode disconnect();


	/// Get current Time in ISO 8601 format.
	/// Complete date plus hours, minutes, seconds and a decimal fraction of a
	/// second:<br>
	/// <center>YYYY-MM-DDThh:mm:ssTZD <i>(eg 1997-07-16T19:20:30+01:00)</i></center><br>
	/// where:<br>
	/// <ul>
	///      <li>YYYY = four-digit year</li>
	///      <li>MM   = two-digit month (01=January, etc.)</li>
	///      <li>DD   = two-digit day of month (01 through 31)</li>
	///      <li>hh   = two digits of hour (00 through 23) (am/pm NOT allowed)</li>
	///      <li>mm   = two digits of minute (00 through 59)</li>
	///      <li>ss   = two digits of second (00 through 59)</li>
	///      <li>TZD  = time zone designator (Z or +hh:mm or -hh:mm)</li>
	/// </ul>
	/// Times are expressed in UTC (Coordinated Universal Time),
	/// with a special UTC designator ("Z").
	/// @param UTC set true to use UTC time zone designator (Z)
	virtual std::string time(bool utc = false) const;

	/// Send an SMS to the specified number.
	/// @param msg the string message to send; it should not exceed 160 chars
	///		otherwise it will be truncated
	/// @param number the cell number to witch the message should be sent
	/// @return OK on success, otherwise... <i>to be defined</i>
	virtual exitCode sendSMS(std::string number, std::string text);

	/// Get the current signal level.
	virtual exitCode signalLevel(unsigned short & level);

	/// Get GPRS registration status
	virtual exitCode gprsStatus(unsigned short & status);

	/// Get current NET link status
	/// @return the NET link status as defined by the DeviceGPRS::t_netStatus type.
	/// @see t_netStatus
	inline DeviceGPRS::t_netStatus status() const {
		return d_netStatus;
	};

      protected:

	/// Create a new DeviceGPRS initially disabled.
	/// In order to enable it you need to attach a Dispatcher.
	/// The new DeviceGPRS, as default logger category, has
	/// the class namespace "controlbox.comlibs.NETCommandGenerator"
	/// @param module the module number, used to look for configuration
	///         params. It <b>must be<b> a single digit number in the
	///         range [0-9]
	/// @param model the GPRS chip model
	/// @param logName the log category, this name is prepended by the
	///         class namespace "controlbox.comlibs."
	DeviceGPRS(short module, t_gprs_models model,
		   std::string const &logName = "DeviceGPRS")
		   throw (exceptions::SerialDeviceException*);

	/// Build a configuration param name.
	/// @param param the param name, valid values are configuration params
	///                 substring following param radix and type
	inline std::string paramName(std::string param) {
		std::ostringstream ostr("gprs_modem_", ios::ate);

		ostr << d_module << "_" << param;
		return ostr.str();
	}

	/// Build a configuration param name.
	/// @param type the param type, valid values are variables configuration
	///                 params substrings
	/// @param param the param name, valid values are configuration params
	///                 substring following param radix and type
	inline std::string paramName(std::string const &type,
				     std::string param) {
		std::ostringstream ostr("gprs_", ios::ate);

		ostr << type;
		if (param.size()) {
			ostr << "_" << param;
		}
		return ostr.str();
	}

	/// Retrive the PID
	/// @param update set true to read the PID from ppp daemon log file
	pid_t getPppdPid(bool update = true);

	/// Terminate a PPP daemon.
	/// @return OK on success,
	/// GPRS_PPPD_NOT_RUNNING if ther's not a daemon to terminate
	///         GPRS_PPPD_FAILURE on error stopping the daemon
	exitCode pppdTerminate();

	/// Load supported netlinks configurations.
	inline exitCode loadNetLinks();

	/// Load an APN configuration.
	/// @param apnId the ID of the configuration to load, must be in [0..9]
	/// @param apnName the simbolic name of the loaded APN configuration
	/// @param conf the configuration loaded
	/// @return GPRS_NETLINK_NOT_SUPPORTED if the required configuration is
	///		not present, OK otherwise
	inline exitCode loadAPNConf(unsigned int apnId, std::string & apnName, t_netlink & conf);

	exitCode getDeviceIds();

	/// Check if the modem is responding
	exitCode checkModem();

	/// Cleaning up before exit.
	void cleanUp();

	/// Notify a network status change.
	exitCode updateState(t_netStatus state);

	/// Callback to notify ppp daemon state change.
	/// A derived class could implement this method to get a notify on
	/// ppp daemon changing state.
	virtual void pppNotifyState(bool running);

	/// Resume (eventually) suspended callers.
	/// This method should be used by private/protected methods to resume
	/// an eventually suspended caller.
	exitCode notifyCaller(void);

	/// Ensure that the specified param is a pipe file.
	exitCode checkPipe(std::string const & pipe);

	/// Get the PPP daemon logfile.
	/// This method update also the d_pppdLogFile attribute
	inline std::string pppDaemonLogfile(char *logFile = 0, unsigned int size = 0);

	/// Parse a PPP daemon log sentence for event generation.
	/// This method generate net link events if the given sentence has been
	/// recognized.
	/// The current implementation of the log parser require the use
	/// of <i>ip-up</i> and <i>ip-down</i> scripts that pppd call at start and
	/// end of connections.
	/// Those scripts require a 6th parameter, in addition to those autolatically
	/// passed by pppd, that is used to define the the pipe filepath
	/// parsed by this code.
	/// In order to pass this 6th parameter the ppp daemon should be called
	/// by passing the <b>ipparam</b> option.
	exitCode pppdParseLog(const char *logline);

	/// The PPPD daemon monitoring thread
	void run(void);

};

}// namespace gprs
}// namespace controlbox
#endif
