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
//**
//**
//*********************************************************************


#include "DeviceTE.ih"

namespace controlbox {
namespace device {

DeviceTE * DeviceTE::d_instance = 0;

char const * DeviceTE::eventDescr[] = {
	"PowerUp",
	"Shutdown",
	"Load",
	"Download",
	"Undefined"
};

DeviceTE::DeviceTE(t_teModels model, std::string const & logName)
	throw (exceptions::SerialDeviceException*) :
	CommandGenerator(logName),
	Device(Device::DEVICE_TE, model, logName),
	d_tty(0),
	d_doExit(false),
	d_config(Configurator::getInstance()),
	d_model(model),
#ifdef DARICOMDEBUG
	d_forceDownload(false);
#endif
	log(Device::log) {
	DeviceFactory * df = DeviceFactory::getInstance();
	exitCode result;

	LOG4CPP_DEBUG(log, "DeviceTE(std::string const &)");

	d_tty = new DeviceSerial(std::string("device_te"));
	if (!d_tty) {
		LOG4CPP_FATAL(log, "Failed to build a DeviceSerial");
		throw new exceptions::SerialDeviceException("Unable to build a SerialDevice");
	}

	// Loading configuration params
	sscanf(d_config.param("device_te_polling_delay",
				DEVICETE_DEFAULT_POLLING_DELAY).c_str(),
		"%i", &d_pollInterval, true);

	d_forceRawUpload = d_config.testParam("device_te_forceRawUpload", DEVICETE_FORCERAW);
	if ( d_forceRawUpload ) {
		LOG4CPP_INFO(log, "Forcing RAW messages upload");
	}

	d_dupRawUpload = d_config.testParam("device_te_dupRawUpload", DEVICETE_DUPLICATE_RAW);
	if ( d_forceRawUpload ) {
		LOG4CPP_INFO(log, "Enabling duplicate upload for RAW messages");
	}

#ifdef DARICOMDEBUG
	d_forceDownload = d_config.testParam("device_te_force", DEVICETE_FORCE);
	if ( d_forceDownload ) {
		LOG4CPP_WARN(log, "Checksum verification on TE record download is DISABLED");
	}
#endif

	d_time = df->getDeviceTime();

	// Opening connection to TE device
	// NOTE this code SHOULD trigger an exception on errors...
	// NOTE don't use an initialization string with TE devices...
	result = d_tty->openSerial(false);
	if (result!=OK) {
		LOG4CPP_FATAL(log, "Failed opening TTY port");
	}

	// Registering device into the DeviceDB
	dbReg();

}

DeviceTE * DeviceTE::getInstance() {
	Configurator & config = Configurator::getInstance();
	t_teModels model;

	if ( d_instance ) {
		return d_instance;
	}

	model = (t_teModels) atoi (config.param("device_te_model", DEVICETE_DEFAULT_MODEL).c_str());

	switch ( model ) {
		case SAMPI:
			d_instance = new SampiTE();
			break;
		default:
			return 0;
	}

	return d_instance;

}

DeviceTE::~DeviceTE() {
	d_tty->closeSerial();
	delete(d_tty);
}

inline
DeviceTE::t_cmdType DeviceTE::mapEvent2cmdType(t_eventType type) {

	switch ( type ) {
		case POWERUP:
			return SEND_DAY_START;
		case SHUTDOWN:
			return SEND_DAY_END;
		case LOAD:
			return SEND_LOAD;
		case DOWNLOAD:
			return SEND_DOWNLOAD;
		default:
			return cmdTypeGenericCode();
	}

}

inline
exitCode DeviceTE::notifyEvents(void) {
	t_event * event;
	comsys::Command * cSgd;
	bool rawNotify = false;

	while ( !d_eventsToNotify.empty() ) {

		event = d_eventsToNotify.front();

// Events that are knowen could be formatted in a specific notification.
// In this case we could decide to upload both the event-specific message or
// only the raw message too.
// This piece of code could run up to 2 time:
// - the first time, if the event is knowen (classified) a event-specific
//	formatter will be called. event.type define the formatter to call.
//	after the formatting and event notification the event.type will be UNDEF
//	thus triggering a RAW formatter too...
// - the second time the RAW formatter will be called
		do {

			if (rawNotify || d_forceRawUpload) {
				rawNotify = false;
				event->type = UNDEF;
			}

			// Build a new command
// 			cSgd = comsys::Command::getCommand(mapEvent2cmdType(event->type), Device::DEVICE_TE, "DEVICE_TE", name());
			cSgd = comsys::Command::getCommand(DeviceTE::SEND_TE_EVENT, Device::DEVICE_TE, "DEVICE_TE", name());
			if ( !cSgd ) {
				LOG4CPP_FATAL(log, "Unable to build a new Command");
				return OUT_OF_MEMORY;
			}
			// Setting event param
			cSgd->setParam( "type", eventDescr[event->type]);

			// Calling TE specific parser and event formatter
			formatEvent(*event, *cSgd);

			// Checking if a "timestamp" has been defined, otherwise we add a local timestamp
			if ( !cSgd->hasParam("timestamp") ) {
				LOG4CPP_DEBUG(log, "No timestamp parsed from TE event");
				// TODO: Should we set also a timestamp?
				cSgd->setParam( "timestamp", d_time->time() );
			}

			//cSgd->setParam( "event",  formatEvent(*event));

			// Notifying the command
			notify(cSgd);

			if (event->type != UNDEF) {
				rawNotify = true;
			} else {
				break;
			}

		} while(d_dupRawUpload);

		// Resetting raw notification
		rawNotify = false;

		// Removing command from the list
		d_eventsToNotify.pop_front();
		delete event;

	}
	return OK;
}

void DeviceTE::run (void) {
	exitCode downloadExitCode;

	while ( !d_doExit ) {
		// NOTE by setting d_pollInterval==0 we disable the TE polling query
		if (d_pollInterval) {

			downloadExitCode = downloadEvents(d_eventsToNotify);

			switch ( downloadExitCode ) {
				case OK:
					notifyEvents();
					break;
				case TE_NO_NEW_EVENTS:
					break;
				case TE_NOT_RESPONDING:
					LOG4CPP_DEBUG(log, "Device TE not responding");
					break;
				case TE_RESTART_DOWNLOAD:
					LOG4CPP_DEBUG(log, "Retrying download");
					continue;
			}

		}

		sleep(d_pollInterval);

	}

}


}// namespace device
}// namespace controlbox
