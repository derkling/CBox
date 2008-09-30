//*********************************************************************
//*************  Copyright (C) 2006        DARICOM  ********************
//*********************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** DARICOM The programs may be used and/or copied only with the
//** written permission from DARICOM or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//*********************************************************************
//******************** Module information *****************************
//**
//** Project:       ProjectName (ProjectCode/Version)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Developer
//** Creation date:  21/06/2006
//**
//*********************************************************************
//******************** Revision history *******************************
//** Revision date       Comments                           Responsible
//** -------- ---------- ---------------------------------- -----------
//** TODO: Implementare l'autosensing del tipo di testata
//**
//*********************************************************************


#ifndef _DEVICETE_H
#define _DEVICETE_H

#include <list>
#include <cc++/serial.h>
#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/base/comsys/CommandGenerator.h>
#include <controlbox/devices/DeviceSerial.h>
#include <controlbox/devices/DeviceTime.h>

/// Default TE model (this should match a valid value of t_teModels)
#define DEVICETE_DEFAULT_MODEL   		"0"
#define DEVICETE_DEFAULT_TTY			"/dev/ttyUSB0:9600,8,n,1"
#define DEVICETE_DEFAULT_POLLING_DELAY		"60000"
#define DEVICETE_DEFAULT_RETRY			"0"
#define DEVICETE_DEFAULT_RESPONCE_DELAY		"500"
#define DEVICETE_DEFAULT_LINE_TERMINATOR	"13"
#define DEVICETE_FORCERAW			"1"
#define DEVICETE_DUPLICATE_RAW			"1"
#define DEVICETE_FORCE				"1"


namespace controlbox {
namespace device {

/// A DeviceTE is a CommandGenerator for TE device management.
/// This class provide access to TE devices connected to
/// a TTY (serial) port. The class provide initialization
/// code.<br>
/// <br>
/// <h5>Configuration params availables for this class:</h5>
/// <ul>
///	<li>
///		<b>device_te_model</b> - <i>Default: DEVICETE_DEFAULT_MODEL</i><br>
///		The TE model used. Availables models are exported by t_teModels enumeration.
///		This params must be a t_teModels valid entry number.
///		@see t_teModels
///		<br>
///	</li>
///	<li>
///		<b>device_te_tty_port</b> - <i>Default: DEVICETE_DEFAULT_TTY</i><br>
///		The TTY port to witch the TE device is attached.
///		Using this param one can pass initial serial device parameters immediately
///		following the device name in a single string,
///		as in "/dev/tty3a:9600,7,e,1", as an example.<br>
///	</li>
///	<li>
///		<b>device_te_polling_delay</b> - <i>Default: DEVICETE_DEFAULT_POLLING_DELAY</i><br>
///		The required delay between data download from the TE (in milliseconds)<br>
///	</li>
///	<li>
///		<b>device_te_responce_delay</b> - <i>Default: DEVICETE_DEFAULT_RESPONCE_DELAY</i><br>
///		The required delay between an AT command send ad a responce get from TTY (in milliseconds)<br>
///	</li>
///	<li>
///		<b>device_te_events</b> - <i>Default: DEVICETE_DEFAULT_EVENTS</i><br>
///		A string defining the event of interest. This string could be composed
///		of any char in the set {A,S,C,E}. Each char correspond to the initial
///		letter of an event of interest. Availables events are those defined
///		by the t_teEvents enum.
///	</li>
///	<li>
///		<b>device_te_line_terminator</b> - <i>Default: DEVICETE_DEFAULT_LINE_TERMINATOR</i><br>
///		The line terminator used for messages send/received by the TE<br>
///	</li>
///	<li>
///		<b>device_te_force</b> - <i>Default: DEVICETE_FORCE</i><br>
///		Set to "1" to force events download from the TE without checksum
///		control. This params is considered <u>only when running in
///		debug mode</u>, otherwise it will be ignored.<br>
///	</li>
/// </ul>
/// @see CommandHandler
class DeviceTE : public comsys::CommandGenerator, public Device  {

  public:

	/// DEVICE_TE Messages
	enum cmdType {
		SEND_TE_EVENT = (DEVICE_TE*SERVICES_RANGE)+1,
		SEND_DAY_START,
		SEND_DAY_END,
		SEND_LOAD,
		SEND_DOWNLOAD,
		SEND_GENERIC_SAMPI500,
		SEND_GENERIC_SAMPI550,
		SEND_GENERIC_VEGAII,
		SEND_GENERIC_ISOIL,
		SEND_GENERIC_SOMEFI,
	};
	typedef enum cmdType t_cmdType;

	/// The TE device model.
	enum teModels {
		SAMPI500 = 0,
		SAMPI550,
		VEGAII,
		ISOIL,
		SOMEFI,
	};
	typedef enum teModels t_teModels;

	enum eventType {
		POWERUP = 0,
		SHUTDOWN,
		LOAD,
		DOWNLOAD,
		UNDEF
	};
	typedef enum eventType t_eventType;

	/// Type of product.
	enum productTipology {
		DIESEL = 0,
		BLUE_DIESEL,
		FULE,
		BLUE_FULE,
		GPL,
		BITUMEN
	};
	typedef enum productTipology t_productTipology;

	/// An event downloaded form the TE.
	/// This struct contatin an event downloaded from a TE
	/// being removed both the initial and final record markers
	/// and the eventtually checksum.
	struct event {
		t_eventType type;	///< the event
		std::string event;	///< the event string (plain information without
					///< any additional communication control token)
	};
	typedef struct event t_event;

	typedef list<t_event *> t_eventList;

  protected:

	static char const * eventDescr[UNDEF+1];

	/// The instance
	static DeviceTE * d_instance;

	/// Set to true once we want to terminate the polling thread and
	///	destroying the class
	bool d_doExit;

	/// The Configurator to use for getting configuration params
	Configurator & d_config;

	/// TE module type
	t_teModels d_model;

	/// The TTY port used
	DeviceSerial * d_tty;

	/// The Time Device to use
	DeviceTime * d_time;

	/// The polling time [ms]
	unsigned int d_pollInterval;

	/// Number of times to retry read on "device not responding"
	unsigned short d_retry;

	/// The list of last downloaded events.
	t_eventList d_eventsToNotify;

	/// Set true to force the upload of RAW messages only
	bool d_forceRawUpload;

	/// Set true to upload formatted messages as RAW messages too
	bool d_dupRawUpload;

#ifdef DARICOMDEBUG
	/// Force events download.
	bool d_forceDownload;
#endif

	/// The logger to use locally.
	log4cpp::Category & log;

  public:

	/// Return a new DeviceTE of the specified model.
	/// The returned device is initially disabled, in order to enable it
	/// a Dispatcher must be defined.
	/// @return a valid DeviceTE pointer on success, 0 on failure or
	///		unsupported model required.
	static DeviceTE * getInstance();

	/// Class destructor.
	~DeviceTE();

  protected:

	/// Create a new DeviceTE initially disabled.
	/// In order to enable it you need to attach a Dispatcher.
	/// @param model the TE model device to build
	/// @param logName the log category, this name is prepended by the
	///		class namespace "daricom.comlibs."
	DeviceTE(t_teModels model, std::string const & logName)
		throw (exceptions::SerialDeviceException*);

	/// Load TE configuration params.
	/// @return helpers::OK on success,
	inline exitCode teConfig();

	/// Setup a TE connection.
	/// @return helpers::OK on success,
	///	helpers::TE_CONNECT_FAILURE connection problems
	inline exitCode teConnect();

	/// Download events from the TE.
	/// This method should be implemented by sub-classes to download
	/// all new events from the TE device and it's expected to store them
	/// into the provided  list.
	virtual exitCode downloadEvents(t_eventList & eventList) = 0;

	/// Format an event.
	/// This method should parse the event string and fill some of the command
	/// parameters; this method is expected to return at least those parameters:
	/// <ul>
	///	<li>timestamp</li>
	///	<li>dist_event</li>
	///	<li>dist_msgType</li>
	/// </ul>
	virtual exitCode formatEvent(t_event const & event, comsys::Command & cmd) = 0;

	/// Return the command type associated to a generic TE event.
	/// This method should be implemented by sub-classes simply returning
	/// the command type corresponding to the specific device.
	virtual DeviceTE::t_cmdType cmdTypeGenericCode() = 0;

	/// Map event type co command type.
	inline DeviceTE::t_cmdType mapEvent2cmdType(t_eventType type);

	inline exitCode notifyEvents(void);

	/// The TE data polling cycle.
	/// This method it's a simple end-less cycle that:
	/// sleep for a while and after wake-up querying
	/// the TE for new data to be downloaded.
	void   run (void);

};

}// namespace device
}// namespace controlbox
#endif

