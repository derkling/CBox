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
#  include <controlbox/base/comsys/CommandGenerator.h>
#  include <controlbox/base/Device.h>
#  include <controlbox/base/Querible.h>

/// The maximun lenght for complete TTY device configurtation strings
#  define DEVICEGPRS_MAX_TTYCONFIG		64
/// The maximun lenght of a netlink name
# define DEVICEGPRS_MAX_NETLINK_NAMELEN		8
/// The maximin number of network links supported by each modem
#  define DEVICEGPRS_MAX_NETLINKS		2

#  define DEVICEGPRS_DEFAULT_GPRS_MODEL   	"2"
#  define DEVICEGPRS_DEFAULT_GPRS_DEVICE	"/dev/ttyUSB0:9600:8:n:1"
#  define DEVICEGPRS_MAX_FRIEND_SERVER		3

#  define DEVICEGPRS_DEFAULT_AT_RESPONCE_DELAY	"500"
#  define DEVICEGPRS_DEFAULT_AT_CMD_ESCAPE	"0xD"
#  define DEVICEGPRS_DEFAULT_AT_INIT_STRING	""

/// Timeout [ms] for non-blochking TTY reading
#  define DEVICEGPRS_DEFAULT_AT_RESPONCE_TIMEOUT 3000

#  define DEVICEGPRS_DEFAULT_PDP_CONTEXT	"AT+CGDCONT=1,\"IP\",\"ibox.tim.it\",,0,0"
#  define DEVICEGPRS_DEFAULT_DIAL		"ATD*99***1#"

#  define DEVICEGPRS_DEFAULT_PPPD_RUNSCRIPT	"/usr/sbin/pppd"
#  define DEVICEGPRS_DEFAULT_PPPD_PIDPATH	"/var/run"
#  define DEVICEGPRS_DEFAULT_PPPD_LINKNAME	"TIM"
#  define DEVICEGPRS_DEFAULT_PPPD_LATENCY	"1"
#  define DEVICEGPRS_DEFAULT_PPPD_LOGPATH	"/var"

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
///             <br>
///     </li>
///     <li>
///             <b>device_gprs_[N]_tty_port_default</b> - <i>Default: DEVICEGPRS_DEFAULT_GPRS_DEVICE</i><br>
///             The TTY port default configuration.<br>
///             Using this param one can pass initial serial device parameters immediately
///             following the device name in a single string,
///             as in "/dev/tty3a:9600,7,e,1", as an example.<br>
///     </li>
///     <li>
///             <b>device_gprs_[N]_tty_port_required</b> - <i>Default: DEVICEGPRS_DEFAULT_GPRS_DEVICE</i><br>
///             The TTY port required configuration.<br>
///             Using this param one can pass initial serial device parameters immediately
///             following the device name in a single string,
///             as in "/dev/tty3a:9600,7,e,1", as an example.<br>
///     </li>
///     <li>
///             <b>device_gprs_[N]_at_responce_delay</b> - <i>Default: DEVICEGPRS_DEFAULT_AT_RESPONCE_DELAY</i><br>
///             The required delay between an AT command send ad a responce get from TTY (in milliseconds)<br>
///     </li>
///     <li>
///             <b>device_gprs_[N]_linkname</b> - <i>Default: DEVICEGPRS_DEFAULT_PPPD_LINKNAME</i><br>
///             The link to use for GPRS connections using this module.
///             Specific link PDP context and dial string must also be specified for each linkname<br>
///     </li>
///     <li>
///             <b>device_gprs_[linkname]_PDP_Context</b> - <i>Default: DEVICEGPRS_DEFAULT_PDP_CONTEXT</i><br>
///             The PDP Context to activate for GPRS connection<br>
///     </li>
///     <li>
///             <b>device_gprs_[linkname]_dial</b> - <i>Default: DEVICEGPRS_DEFAULT_DIAL</i><br>
///             The dial AT command<br>
///     </li>
///     <li>
///             <b>device_gprs_pppd_runscript</b> - <i>Default: DEVICEGPRS_DEFAULT_PPPD_RUNSCRIPT</i><br>
///             The complete path of the PPP daemon launching script.<br>
///     </li>
///     <li>
///             <b>device_gprs_pppd_pidpath</b> - <i>Default: DEVICEGPRS_DEFAULT_PPPD_PIDPATH</i><br>
///             The path (without leading /) of the folder containing the pppd's PID file.
///             Expected PID files name are: ppp-[linkname].pid <br>
///     </li>
///     <li>
///             <b>device_gprs_pppd_latency</b> - <i>Default: DEVICEGPRS_DEFAULT_PPPD_LATENCY</i><br>
///             The maximun time interval to sleep (in second) waiting for PPP daemon to start (and create it's PID file)<br>
///     </li>
///     <li>
///             <b>device_gprs_pppd_logpath</b> - <i>Default: DEVICEGPRS_DEFAULT_PPPD_LOGPATH</i><br>
///             The path (without leading /) of the folder containing the PPP daemon's logfile.
///             Expected filename is: ppp-[linkname].log <br>
///             <i>N.B. This log-file <b>must</b> be a pipe</i><br><br>
///     </li>
/// </ul>
/// @see CommandHandler
class DeviceGPRS:public comsys::CommandGenerator, public Device {

      public:

	/// Generated Messages
	enum cmdType {
		GPRS_STATUS_UPDATE = (DEVICE_GPRS * SERVICES_RANGE) + 1,
	};
	typedef enum cmdType t_cmdType;

	/// The GPRS module model.
	enum gprs_models {
		DEVICEGPRS_MODEL_DUMMY = 0,
		DEVICEGPRS_MODEL_SONYERICSSON,
		DEVICEGPRS_MODEL_ENFORA
	};
	typedef enum gprs_models t_gprs_models;

	/// The running mode
	enum gprs_mode {
		DEVICEGPRS_MODE_COMMAND = 0,
		DEVICEGPRS_MODE_DATA,
		DEVICEGPRS_MODE_SMS,
		DEVICEGPRS_MODE_VOICE
	};
	typedef enum gprs_mode t_gprs_mode;

	/// NET operative conditions.
	/// Define the possible state of a net link
	enum netStatus {
		LINK_DOWN,
		LINK_GOING_DOWN,
		LINK_GOING_UP,
		LINK_UP
	};
	typedef enum netStatus t_netStatus;
	
#define LINK_STATUS(_vect_)\
	char *_vect_[] = {\
		"DOWN",\
		"GOING_DOWN",\
		"GOING_UP",\
		"UP"\
	}
	
	/// A linkname (i.e. an APN).
	typedef char t_linkname[DEVICEGPRS_MAX_NETLINK_NAMELEN];
	
	enum simNumber {
		SIM1 = 1,
		SIM2,
	};
	typedef enum simNumber t_simNuber;
	
	struct identification {
		std::string manufactor;
		std::string model;
		std::string revision;
		std::string imei;
		std::string imsi;
		std::string simNumber;
	};
	typedef struct identification t_identification;
	
	/// A generic list of strings.
	typedef vector < std::string > t_stringVector;
	
	/// A netlink configuration
	struct netlink {
		t_simNuber	sim;		///> the sim to use
		std::string	pdpContext; 	///> the PDP Context for GPRS connections
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
		char device[16];
		char interface[6];
		char speed[6];
		char ipLocal[16];
		char ipRemote[16];
		char dns1[16];
		char dns2[16];
	};
	typedef struct pppd_configuration t_pppd_configuration;


      protected:

	/// Serialize access to a TTY modem.
	/// This class serialize the access to a tty attacched modem
	/// on port opening. A mutex is associated to each tty controlled
	/// port and only one thread each time is allowed to successfully
	/// open a port. The mutex is released at port closing.
	/// Try open method are also exported.
	class ttymodem:public ost::ttystream {


	      protected:

		/// A reference to the GPRS device for access to
		///     its environment
		DeviceGPRS & d_gprs;

		/// Map each tty port to a Mutex.
		/// Each map list is a pair of (tty_device_name, mutex)
		/// i.e. (/dev/ttyUSB0, ttyUSB0_MUTEX)
		static std::map < std::string, ost::Mutex > d_ttyMutex;

		/// Set when a port has been successfully opened
		/// by entering the associated mutex
		bool d_opened;

		/// The controlled TTY device path filename.
		std::string d_device;


	      public:

		/// Create a new tty stream.
		/// The port must be opened befor being able to use it.
		/// @param ttyConfig the tty device to open with
		///     specified params
		ttymodem(DeviceGPRS & device);

		/// Release a tty stream.
		/// This method provide to eventually close the port and
		/// release the associated mutex
		~ttymodem();

		/// Open the required tty port.
		/// This method enter the mutex on the required port
		/// and open it with the specified params.
		/// @note: by default this is a blocking method,
		///     it will not return until the mutex is entered.
		/// @param ttyConfig the tty port configuration string.
		///     It is allowed to pass the initial serial device
		///     parameters immediately following the device name
		///     in a single string,
		///     as in <i>"/dev/tty3a:9600,7,e,1"</i>, as an example.
		/// @see DEVICEGPRS_MAX_TTYCONFIG
		/// @param blocking set false if the call must be non-blocking
		/// @return true on successfully open, false otherwise
		/// @see tryOpen
		bool open(std::string ttyConfig, bool blocking = true);


		/// Close the previously opened tty port.
		/// This method exit the mutex on the associated port
		/// releasing the access to other thread.
		/// @return true on successfully close.
		bool close();
		
		/// Reset modem to factory default.
		/// This method provide to shutdown the active connection and
		/// send an ATZ command. The modem will be left in Command Mode
		exitCode reset();

		/// Send an AT command to the modem.
		/// The specified AT command is sent to the modem and the
		/// wait for the specified responce.
		/// If the resp string is left empty, don't wait for responces.
		/// NOTE: this method preserve the Data/Commands Mode active at call
		///             time, eventually switching bethween them.
		/// NOTE: this method keep the original port
		///             open/closed state
		/// @param at_command the AT command to send (without the '\r' terminator)
		/// @param exitOn the responce string to catch to return with success
		/// @param resp a pointer to a t_stringVector that will contains
		///             all the command responce lines received by
		///             the modem
		/// @param cLines max number of '\n' terminated lines in the responce
		/// @param timeout the timeout for reading eventually responces (in ms)<br>
		///		<b>note:</b>setting to 0 could lead to an endless waiting (i.e. blocking call)
		/// @return OK on send ok,
		///     GPRS_TTY_OPEN_FAILURE on tty open failure
		///     GPRS_AT_NOT_SUPPORTED if the command is not supported by this module
		exitCode sendAT(std::string const &at_command = "AT",
				std::string const &exitOn = "OK",
				t_stringVector * resp = 0,
				short cLines = 2,
				unsigned long timeout = DEVICEGPRS_DEFAULT_AT_RESPONCE_TIMEOUT,
				bool escape = true);


		/// Expect a required str from the modem.
		/// Read lines produces by the modem expacting a required
		/// string. If the required str is not read within the specified
		/// number of received lines the call will fail.
		/// If specified return a list of all modem string
		/// received, even on case of failure.
		/// @param str the expected string
		/// @param timeout the timeout for reading eventually responces (in ms)<br>
		///		<b>note:</b>setting to 0 could lead to an endless waiting (i.e. blocking call)
		/// @param maxLines the maximun number of strings into witch we
		///             expect 'str', if maxLines<=0 wait indefinetly until
		///             'str' is received
		/// @param lines if not null, the list into witch save all the
		///             string received by the modem
		/// @return OK if 'str' is received within the specified
		///             number 'maxLines' of modem lines.
		exitCode readFromTTY(std::string const &str = "OK",
				     unsigned long timeout = DEVICEGPRS_DEFAULT_AT_RESPONCE_TIMEOUT,
				     short maxLines = 2,
				     t_stringVector * lines = 0);

	      protected:

		/// Set the port operating mode.
		/// If necessary try to switch the (previously opened)
		/// tty port to the required operating mode.
		/// @note the port must have been previsouly successfully
		///     opened.
		/// @return OK on success.
		///
		exitCode setMode(t_gprs_mode mode);


	};
	// Allowing inner class to access DeviceGPRS members
	friend class ttymodem;
	
	/// The connection call thread context.
	class netConnect: public PosixThread {
	      protected:
			/// A reference to the GPRS device for access to
			///     its environment
			DeviceGPRS & d_gprs;
		public:
			netConnect(DeviceGPRS & device) : d_gprs(device) {};
			void run() { d_gprs.doConnect(d_gprs.d_linkname); };
	};
	friend class netConnect;
	
	/// The disconnection call thread context.
	class netDisconnect: public PosixThread {
		protected:
			/// A reference to the GPRS device for access to
			///     its environment
			DeviceGPRS & d_gprs;
		public:
			netDisconnect(DeviceGPRS & device) : d_gprs(device) {};
			void run() { d_gprs.doDisconnect(); };
	};
	friend class netDisconnect;

      protected:

	/// Set to true once we want to terminate the parsing thread and
	/// destroying the class
	bool d_doExit;

	/// The Configurator to use for getting configuration params
	Configurator & d_config;

	/// The module number used to look for configuration params
	short d_module;

	/// GPRS module type
	t_gprs_models d_model;

	/// The TTY port configuration string
	std::string d_ttyConfig;
	
	t_identification d_ids;
	
	/// The network links (i.e. APN) supported by this modem
	/// @note each modem could support up to DEVICEGPRS_MAX_NETLINKS
	t_supportedLinks d_supportedLinks;
	
	/// The current netlink name
	const std::string * d_curNetlinkName;
	
	/// The current netlink configuration
	t_netlink * d_curNetlinkConf;

	/// The AT responce delay
	unsigned int d_atResponceDelay;
	
	/// The AT command escape
	unsigned int d_atCmdEscape;
	
	/// The modem initialization string
	std::string d_atInitString;
	
	/// The SIM number
	std::string d_simNumber;

	/// The current module mode
	t_gprs_mode d_mode;

	/// The PPP daemon launching script
	std::string d_pppdRunScript;

	/// The PID of the forked PPP daemon
	pid_t d_pppdPid;

	/// The PPP linkname for GPRS connection
	std::string d_linkname;

	/// The PPP daemon folder for logfile
	std::string d_pppdLogFolder;
	
	/// The PPP daemon logfile
	std::string d_pppdLogFile;

	/// The PPP daemon folder for PID files
	std::string d_pppdPidFolder;
	
	/// The PPP daemon logfile
	std::string d_pppdPidFile;

	/// The PPP daemon startup latency (in seconds)
	short d_pppdLatency;

	/// The current NET link status
	t_netStatus d_netStatus;
	
	/// The PPP configuration
	t_pppd_configuration d_pppConf;
	
	/// Whatever the parser thread is running.
	/// When this is false, all public command methods will fails
	/// with an GPRS_PARSER_NOT_RUNNING exitCode.
	bool isParserRunning;
	
	/// The PPP command thread.
	ost::PosixThread * d_cmdThread;
	
	/// The PPP parser thread.
	ost::PosixThread * d_runThread;
	
	static t_mapLink2DeviceGPRS d_mapLink2DeviceGPRS;

	/// The logger to use locally.
	log4cpp::Category & log;

#  if CONTROLBOX_DEBUG > 1
	/// The calltrace logger to use locally.
	log4cpp::Category & ct;
#  endif

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
	~DeviceGPRS();

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
	exitCode gprsConnect(std::string const & linkname);

	/// Terminate the current GPRS session.
	exitCode gprsDisconnect();

	/// Get current NET link status
	/// @return the NET link status as defined by the DeviceGPRS::t_netStatus type.
	/// @see t_netStatus
	DeviceGPRS::t_netStatus status() const;

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
	std::string time(bool utc = false) const;

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
		   std::string const &logName = "DeviceGPRS");
	
	/// Clean up on object destruction.
	void cleanUp();
	
	/// Load device identifiers.
	/// Query the GPRS device for all d_ids identifiers
	/// @see t_identification
	exitCode getDeviceIds();
	
	/// Load supported netlinks configurations.
	inline exitCode loadNetLinks();
	
	/// Load an APN configuration.
	/// @param apnId the ID of the configuration to load, must be in [0..9]
	/// @param apnName the simbolic name of the loaded APN configuration
	/// @param conf the configuration loaded
	/// @return GPRS_NETLINK_NOT_SUPPORTED if the required configuration is
	///		not present, OK otherwise
	inline exitCode loadAPNConf(unsigned int apnId, std::string & apnName, t_netlink & conf);
	
	/// Return the device identifier.
	/// Each phisical device has a unique identifier (e.g. the IMEI of one
	/// of its modems)
	std::string getDeviceIdentifier();
	
	/// Suspend a command calling method.
	/// This method should be used by public methods in order to suspend
	/// the execution of the calling execution context until a wakeup
	/// condition will happens
	/// @return OK after wakeup
	exitCode suspendCaller(void);
	
	/// Resume (eventually) suspended callers.
	/// This method should be used by private/protected methods to resume
	/// an eventually suspended caller.
	exitCode notifyCaller(void);
	
	/// The connection routine.
	/// This method will connect to the current configured d_linkname
	exitCode doConnect(std::string const & linkname);
	
	/// The disconnection routine.
	exitCode doDisconnect(void);

	/// Start a PPP daemon.
	/// @return OK on PPP daemon successfully started
	///         GPRS_PPPD_FAILURE on error starting the daemon
	exitCode pppdSession(std::string device, std::string linkname);

	/// Terminate a PPP daemon.
	/// @return OK on success,
	/// GPRS_PPPD_NOT_RUNNING if ther's not a daemon to terminate
	///         GPRS_PPPD_FAILURE on error stopping the daemon
	exitCode pppdTerminate();
	
	exitCode checkPipe(std::string const & pipe);

	/// Parse the PPP daemon log file for event generation.
	/// This method generate net link events that recognize
	/// parsing the PPP daemon log file.
	exitCode pppdMonitor();

	/// Parse a PPP daemon log sentence for event generation.
	/// This method generate net link events if the given sentence has been
	/// recognized.
	exitCode pppdParseLog(const char *logline);

	/// Retrive the ppp daemon's PID.
	/// Given a linkname return the PID of the ppp daemon
	/// menaging this GPRS's link.
	/// @return the PID of the ppp daemon for this GPRS's link,
	/// 0 if ther's not a running ppp daemon for that device
	/// (i.e. we are on LINK_DOWN state)
	pid_t getSessionPid();
	
	/// Suspend the PPP daemon.
	/// The eventually running instance of pppd is suspended.
	exitCode suspendPppDaemon();

	/// Resume the PPP daemon.
	/// Resume a previsously suspended pppd instance.
	exitCode resumePppDaemon();


	/// Return the tty device.
	/// Given a tty configuration string retrive and return
	/// the tty device filepath
	inline std::string ttyDevice(std::string ttyConfig);


	/// Get the PPP daemon logfile.
	/// This method update also the d_pppdLogFile attribute
	inline std::string pppDaemonLogfile(char *logFile = 0, unsigned int size = 0);

	/// Get the PPP daemon pidfile.
	/// This method update also the d_pppdPidFile attribute
	inline std::string pppDaemonPidfile(char *pidFile = 0, unsigned int size = 0);

	///
	exitCode sendSMS() {
	};
	
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


	/// Update the GPRS status
	/// 
	exitCode updateState(t_netStatus state);

	/// The thread cycle.
	/// This method, hinerited from Thread class, it's a simple
	/// thread body that: normally sleep waiting for data from the GPS device, update
	/// the current GPS status, eventually notify a FIX status change (if enabled),
	/// and so on in an endless loop.
	void run(void);


//------------------------------------------------------------------------------
//              --- GPRS-Modem's Specific Features ---
// The following virtual methods provide only a dummy general implementation
//      and shuld be re-defined in DeviceGPRS's derived classes
//------------------------------------------------------------------------------

	/// Setup a TTY port.
	/// Setup a tty port with the (configuration) specified
	/// params.
	/// @return OK on success, the modem has been switched
	/// into Data Mode,
	/// GPRS_TTY_OPEN_FAILURE on tty open failure
	/// @note once configured, the tty is closed.
	virtual exitCode initDeviceGPRS() = 0;

	/// Switch off Data Mode.
	/// If a switch from data mode to command mode is supported by the
	/// modem device, provide to switch to the required mode.
	/// @param ttymodem the tty to switch
	/// @param forse set true to force a command mode switch
	/// (by terminating pppd)
	/// @return OK on switch success
	/// @note this method suppose that Data Mode is the current operating mode
	/// and that the tty is already open
	virtual exitCode switchToCommandMode(ttymodem & modem, bool force =
					     false);

	/// Switch-back to Data Mode.
	/// If a switch from command mode to data mode is supported by the
	/// modem device, provide to switch to the required mode.
	/// @return OK on switch success
	/// @note this method suppose that Command Mode is the current one
	virtual exitCode switchToDataMode(ttymodem & modem);

	/// Notify friend server about IP change.
	/// If some firend servers have been defined for the current netlink
	/// this method provide to notify them about an IP change.
	/// Modem-specific implementations are allowed in order to exploit
	/// modem specific update support.
	virtual exitCode notifyFriends();
	
//------------------------------------------------------------------------------



};

}// namespace gprs
}// namespace controlbox
#endif
