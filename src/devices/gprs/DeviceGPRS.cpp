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

#include "DeviceGPRS.ih"

namespace controlbox {
namespace device {

DeviceGPRS::t_mapLink2DeviceGPRS DeviceGPRS::d_mapLink2DeviceGPRS;

const char *DeviceGPRS::d_netStatusStr[] = {
		"DOWN",
		"GOING_DOWN",
		"GOING_UP",
		"UP",
};

DeviceGPRS::DeviceGPRS(short module, t_gprs_models model, std::string const
		       &logName)
			throw (exceptions::SerialDeviceException*) :
			CommandGenerator(logName),
			Device(Device::DEVICE_GPRS, module, logName),
			d_doExit(false),
			d_config(Configurator::getInstance()),
			d_module(module),
			d_model(model),
			d_curNetlinkName(0),
			d_curNetlinkConf(0),
			d_pppdPid(0),
			d_runThread(0),
			d_parserRunning(false),
			d_mode(DEVICEGPRS_MODE_COMMAND),
			d_netStatus(DeviceGPRS::LINK_DOWN),
			log(Device::log) {
	char lable[] = "gprs_modem_0";

	LOG4CPP_INFO(log, "Loading GPRS module configuration [%d]", d_module);
	lable[11] += (d_module%10);

	//----- Loading GPRS configuration params
	//TODO: Verify 'd_config' initialization

	if (d_model!=DEVICEGPRS_MODEL_DUMMY) {

#if 0
// This code has been replaced by the usage of DeviceSerial
		// modem's params
		// Loading the TTY port configuration string
		d_ttyConfig = d_config.param(paramName("tty"), DEVICEGPRS_DEFAULT_GPRS_DEVICE);
	
		sscanf(d_config.param(paramName("atDelay"),
					DEVICEGPRS_DEFAULT_AT_RESPONCE_DELAY).c_str(),
			"%i", &d_atResponceDelay);
	
		d_atCmdEscape = d_config.param(paramName("cmdEscape"),
					DEVICEGPRS_DEFAULT_AT_CMD_ESCAPE);
	
		d_atInitString = d_config.param(paramName("atInitString"),
					DEVICEGPRS_DEFAULT_AT_INIT_STRING);
#else
		d_tty = new DeviceSerial(lable);
		if (!d_tty) {
			LOG4CPP_FATAL(log, "Failed to build a DeviceSerial");
			throw new exceptions::SerialDeviceException("Unable to build a SerialDevice");
		}
#endif
	
		d_pppdLogFolder = d_config.param(paramName("pppd", "logpath"),
			   DEVICEGPRS_DEFAULT_PPPD_LOGPATH);
	
		// loading supported linknames
		loadNetLinks();
		
	}

	// Registering device into the DeviceDB
	dbReg();

}


DeviceGPRS *
DeviceGPRS::getInstance(unsigned short module, std::string const &logName) {
	log4cpp::Category & log(log4cpp::Category::
				getInstance(std::
					    string
					    ("controlbox.device." +
					     logName)));
	std::ostringstream _logName("");
	char label[] = "gprs_modem_0_model";
	Configurator & d_config = Configurator::getInstance();
	t_gprs_models d_model;
	DeviceGPRS * gprs = 0;

	if (module > 9) {
		LOG4CPP_FATAL(log,
			      "Invalid module request [%d]. Module number must be in the range [0-9]",
			      module);
		return 0;
	}

	// Loading the GPRS model id
	label[11] += (module%10);
	d_model =
	    (DeviceGPRS::gprs_models) atoi(d_config.
	    		    param(label, DEVICEGPRS_DEFAULT_GPRS_MODEL).
	    		    c_str());

	LOG4CPP_DEBUG(log,
		      "Builing new GPRS device controller [%s, module: %d, model: %d]",
		      logName.c_str(), module, d_model);


	//TODO modem identifications should be done using AUTOSENSING,
	//	using device IDs... ;-)
	
	_logName << logName << "_" << module;
	try {
		switch (d_model) {
		case DEVICEGPRS_MODEL_DUMMY:
			LOG4CPP_WARN(log, "DUMMY configuration: using ethernet network link");
			gprs = new DeviceGPRS(module, DEVICEGPRS_MODEL_DUMMY, _logName.str());
			break;
		case DEVICEGPRS_MODEL_SONYERICSSON:
// 			gprs = new SonyEricsson(module, _logName.str());
			break;
		case DEVICEGPRS_MODEL_ENFORA:
// 			gprs = new Enfora(module, _logName.str());
			break;
		case DEVICEGPRS_MODEL_ENFORA_API:
			gprs = new EnforaAPI(module, _logName.str());
			break;
		default:
			// Failure: unsupported model
			LOG4CPP_FATAL(log, "Unsupported GPRS model required.");
			return 0;
		}
	} catch (exceptions::SerialConfigurationException * sce) {
		LOG4CPP_FATAL(log, "Access TTY port failed, %s", sce->getMessage().c_str());
	}

	return gprs;
	
}


DeviceGPRS *
DeviceGPRS::getInstance(std::string const & linkname, std::string const &logName) {
	log4cpp::Category & log(log4cpp::Category::
				getInstance(std::
					    string
					    ("controlbox.device." +
					     logName)));
	Configurator & d_config = Configurator::getInstance();
	t_mapLink2DeviceGPRS::iterator it;
	char apnName[] = "gprs_apn_0_name";
	char gprsAPN[] = "gprs_modem_0_links";
	char checkConf[] = ",0";
	short id;
	std::string buff;

	// Check within the DevDB for a modem supporting the required netlink
	it = d_mapLink2DeviceGPRS.find(linkname);
	if (it != d_mapLink2DeviceGPRS.end() ) {
		LOG4CPP_DEBUG(log, "Found a DeviceGPRS supporting the required APN [%s]");
		//FIXME what if there is more than a DeviceGPRS supporting this linkname?!?
		return (DeviceGPRS *)it->second;
	}
	
	//TODO If not found, look within configuration for a modem supporting the
	// required netlink, if any build it and return.
	LOG4CPP_WARN(log, "Building a new DeviceGPRS for the required APN [%s]",
				linkname.c_str());
	
	//TODO find a suitable configuration
	//Find APN id if any...
	id = 0;
	while (id<DEVICEGPRS_MAX_NETLINKS) {
		// Updating the apnId number
		apnName[9] += (id%10);
		
		buff = d_config.param(string(apnName), "");
		if (!buff.size()) {
			LOG4CPP_DEBUG(log, "No more APN configuration availables");
			id = -1;
			break;
		}
		
		if (buff == linkname) {
			LOG4CPP_DEBUG(log, "Found required APN [%s] id [%d]", buff.c_str(), id);
			break;
		}
		
		id++;
	}
	
	if (id<0 || id>=DEVICEGPRS_MAX_NETLINKS) {
		// By default return null if the required netlink is unsupported
		LOG4CPP_ERROR(log, "Unable to find a DeviceGPRS supporting the required APN [%s]",
					linkname.c_str());
		return 0;
	}
	
	// Find a modem supporting this APN if any...
	checkConf[1] += (id%10);
	id = 0;
	while (id<10) {
		// Updating the apnId number
		gprsAPN[11] += (id%10);
		
		buff = d_config.param(string(gprsAPN), "");
		if (!buff.size()) {
			LOG4CPP_DEBUG(log, "No more GPRS configuration availables");
			id = -1;
			break;
		}
		
		if ( string::npos !=
			buff.find((const char *)checkConf, 0, 2) ) {
			LOG4CPP_DEBUG(log, "Found GPRS device [%d] supporting required APN [%s]",
						id, linkname.c_str());
			return getInstance(id, logName);
		}
		
		id++;
	}
	
	// By default return null if the required netlink is unsupported
	LOG4CPP_ERROR(log, "Unable to find a DeviceGPRS supporting the required APN [%s]",
					linkname.c_str());
	return 0;
}


inline exitCode
DeviceGPRS::loadNetLinks() {
	std::string linkConf;
	short i;
	unsigned int simId, apnId;
	int error;
	t_netlink * newConf;
	std::string apnName;
	
	linkConf = d_config.param(paramName("links"), "");
	if (!linkConf.size()) {
		LOG4CPP_WARN(log, "Undefined APNs for GPRS module [%d]", d_module);
		return GPRS_NETLINK_UNDEFINED;
	}
	
	// Parsing the netlink configuration string, sintax is:
	// 	sim,linkid[:sim,linkid[...]]
	i = 0;
	while ( i < linkConf.size()) {
		error = sscanf(linkConf.c_str()+i, "%1u,%1u", &simId, &apnId);
		if (error == EOF) {
			LOG4CPP_ERROR(log, "Parsing GPRS's APN configuration failed: %s", strerror(errno));
			return GPRS_NETLINK_PARSE_ERROR;
		}
		i+=4; // ther's a leading ':' if another configuration follow
		
		newConf = new t_netlink();
		if (!newConf) {
			LOG4CPP_ERROR(log, "Allocation of a new APN failed");
			return OUT_OF_MEMORY;
		}
		
		LOG4CPP_DEBUG(log, "Loading netlink configuration [%d] on SIM [%d]", apnId, simId);
		
		switch(simId) {
		case 1:
			newConf->sim = SIM1;
			break;
		case 2:
			newConf->sim = SIM2;
			break;
		default:
			LOG4CPP_ERROR(log, "Unsupported SIM number [%d]", simId);
			return GPRS_LINK_NOT_CONFIGURED;
			delete newConf;
			continue;
		}
		
		error = loadAPNConf(apnId, apnName, *newConf);
		if (!error) {
			LOG4CPP_INFO(log, "Successfully loaded APN configuration for [%s] on SIM [%u]",
			apnName.c_str(), newConf->sim);
		} else {
			delete newConf;
			continue;
		}
		
		// Updating list of linknames supported by this DeviceGPRS
		d_supportedLinks[apnName] = newConf;
		
		// Updating list of DeviceGPRS supporting this linkname
		d_mapLink2DeviceGPRS.insert(pair<std::string, DeviceGPRS*>(apnName, (DeviceGPRS*)this) );
		
		//FIXME More than a DeviceGPRS could support the same linkname
		//	we should avoid this or at least have a way to define
		//	which modem to use with a link...
		
	}
	
	return OK;
	
}


inline exitCode
DeviceGPRS::loadAPNConf(unsigned int apnId, std::string & apnName, t_netlink & conf) {
	char baseLabel[] = "apn_0";
	std::string buf;
	const char *bptr, *ptr, *eptr, *next;
	unsigned int len;
	int count;
	std::string aServer;

	LOG4CPP_DEBUG(log, "Loading APN configuration [%u]", apnId);
	
	// Updating the apnId number
	baseLabel[4] += (apnId%10);
	
	apnName = d_config.param(paramName(baseLabel, "name"), "");
	if (!apnName.size()) {
		LOG4CPP_ERROR(log, "APN configuration [%u] loading failed", apnId);
		return GPRS_NETLINK_NOT_SUPPORTED;
	}
	conf.pdpContext = d_config.param(paramName(baseLabel, "PDPContext"), "");
	conf.smsCsca = d_config.param(paramName(baseLabel, "sms_csca"), "");
	conf.AtDial = d_config.param(paramName(baseLabel, "dial"), "");
	
	// Loading friend servers
	buf = d_config.param(paramName(baseLabel, "friendServers"), "");
	DLOG4CPP_DEBUG(log, "Firends [%s]", buf.c_str());
	if ( buf.length() ) {
		count = 0;
		bptr = ptr = buf.c_str();
		eptr = ptr + buf.length();
		do {
			DLOG4CPP_DEBUG(log, "bptr: %p, ptr: %p, eptr %p", bptr, ptr, eptr);
			next = strchr(ptr, ';');
			len = (next) ? ((unsigned)next-(unsigned)ptr) : (unsigned)eptr-(unsigned)ptr+1;
			
			aServer = buf.substr((unsigned)ptr-(unsigned)bptr, len);
			DLOG4CPP_DEBUG(log, "Find firend server [%s], from: %d, len: %d",
					buf.substr((unsigned)ptr-(unsigned)bptr, len).c_str(),
					(unsigned)ptr-(unsigned)bptr, len);
			if (aServer.length()) {
				// This avoid empty server loading if ther's a leading ';'
				conf.lserv.push_back( aServer );
			}
			ptr = next+1; count++;
		} while (next && count < DEVICEGPRS_MAX_FRIEND_SERVER);
		
		LOG4CPP_INFO(log, "Loaded %d friend server", count);
		//TODO format list of friend server loaded
	}
	
	return OK;
}


#if 0
// This code has been replaced by the usage of DeviceSerial
exitCode
DeviceGPRS::openSerial() {

	if (!d_ttyConfig.size()) {
		LOG4CPP_ERROR(log, "Failed opening serial: missing configuration");
		return GPRS_TTY_OPEN_FAILURE;
	}
	
	LOG4CPP_INFO(log, "Opening serial port [%s]", d_ttyConfig.c_str());
	
	d_tty.open(d_ttyConfig.c_str());
	if (!d_tty) {
		LOG4CPP_ERROR(log, "Failed opening serial []", d_ttyConfig.c_str());
		return GPRS_TTY_OPEN_FAILURE;
	}
	// Disabling TTY buffers
	d_tty.interactive(true);
	
	return OK;
}

exitCode
DeviceGPRS::closeSerial(bool sync) {

	if (!d_tty)
		return OK;

	if (sync)
		d_tty.sync();

	d_tty.close();
	return OK;
}


exitCode
DeviceGPRS::sendSerial(std::string cmd, t_stringVector * resp) {

	d_tty << cmd.c_str() << "\r";//d_atCmdEscape.c_str();
	d_tty.sync();

	LOG4CPP_DEBUG(log, "TTY_SEND: %s",
				cmd.c_str(), d_ttyConfig.c_str());

	// At least ONE line is expected in order to have a successfully read
	if (!d_tty.isPending(ost::Serial::pendingInput, 3000)) {
		LOG4CPP_WARN(log, "Modem not responding");
		return  GPRS_TTY_MODEM_NOT_RESPONDING;
	}

	if (!resp)
		return OK;

	return readSerial(resp);

}


exitCode
DeviceGPRS::readSerial(t_stringVector * resp) {
	char cbuf[256];
	exitCode result = GPRS_TTY_MODEM_NOT_RESPONDING;

	result = OK;
	do {
		cbuf[0] = 0;
		d_tty.getline(cbuf, 256);
		if (cbuf[1]) { // This will throw away empty lines
			// Delating terminating "\r"
			cbuf[strlen(cbuf)-1] = 0;
			if (resp) { // Save only if required...
				resp->push_back( std::string(cbuf) );
			} // while debugging is always allowed
			LOG4CPP_DEBUG(log, "TTY_READ: %s", cbuf);
		}
	} while ( d_tty.isPending(ost::Serial::pendingInput, 100) );

	return result;

}
#endif

exitCode
DeviceGPRS::getDeviceIds() {
	t_stringVector resp;
	exitCode result;

	if ( d_tty->sendSerial("AT+CNUM", &resp) == OK ) {
		LOG4CPP_INFO(log, "SIM number [%s]", resp[0].c_str());
		//TODO parse the output to retrive the SIM number...
		// this command will return something like:
		// +CNUM: "Line 1","12125551212",145
	}

	resp.clear();
	if ( d_tty->sendSerial("AT+CGMI", &resp) == OK ) {
		LOG4CPP_INFO(log, "Manufacturer ID [%s]", resp[0].c_str());
		d_ids.manufactor = resp[0];
	}

	resp.clear();
	if ( d_tty->sendSerial("AT+CGMM", &resp) == OK ) {
		LOG4CPP_INFO(log, "Model ID [%s]", resp[0].c_str());
		d_ids.model = resp[0];
	}

	resp.clear();
	if ( d_tty->sendSerial("AT+CGMR", &resp) == OK ) {
		LOG4CPP_INFO(log, "Revision ID [%s]", resp[0].c_str());
		d_ids.revision = resp[0];
	}

	resp.clear();
	if ( d_tty->sendSerial("AT+CGSN", &resp) == OK ) {
		LOG4CPP_INFO(log, "IMEM [%s]", resp[0].c_str());
		d_ids.imei = resp[0];
	}

	resp.clear();
	if ( d_tty->sendSerial("AT+CIMI", &resp) == OK ) {
		LOG4CPP_INFO(log, "IMSI [%s]", resp[0].c_str());
		d_ids.imsi = resp[0];
	}

	return OK;

}

exitCode
DeviceGPRS::powerOn(bool reset) {
	LOG4CPP_DEBUG(log, "Dummy power-on implementation: do noting");
	return OK;
}

exitCode
DeviceGPRS::powerOff() {
		LOG4CPP_DEBUG(log, "Dummy power-off implementation: do noting");
	return OK;
}

exitCode
DeviceGPRS::connect(std::string const & linkname) {
	LOG4CPP_DEBUG(log, "DUMMY connect");
	d_netStatus = LINK_UP;
	return OK;
}

exitCode
DeviceGPRS::disconnect() {
	LOG4CPP_DEBUG(log, "DUMMY disconnect");
	d_netStatus = LINK_DOWN;
	return OK;
}


std::string
DeviceGPRS::time(bool utc) const {

	//TODO: To implement
	LOG4CPP_WARN(log, "GPRS Time not yet implemented");

	return string("");

}


exitCode
DeviceGPRS::sendSMS(std::string number, std::string text) {

	LOG4CPP_WARN(log, "SMS sending failed: "
		"functionality not supportd by DUMMY GPRS configuration");
	
	return GPRS_SMS_SEND_FAILED;
}

exitCode
DeviceGPRS::signalLevel(unsigned short & level) {
	LOG4CPP_WARN(log, "Getting signal level failed: "
		"functionality not supportd by DUMMY GPRS configuration");
		
	return GPRS_SIGNAL_LEVEL_FAILED;
}


exitCode
DeviceGPRS::gprsStatus(unsigned short & status) {
	LOG4CPP_WARN(log, "Getting GPRS status failed: "
		"functionality not supportd by DUMMY GPRS configuration");
		
	return GPRS_GPRS_STATUS_FAILED;
}

exitCode
DeviceGPRS::pppdTerminate() {
	int errnoKill;

	if (d_pppdPid) {
		LOG4CPP_INFO(log,
			     "Stopping PPP daemon [%d] with signal [%d]",
			     d_pppdPid, PPPD_SIGKILL);
		if (kill(d_pppdPid, PPPD_SIGKILL)) {
			errnoKill = errno;
			LOG4CPP_ERROR(log,
				      "Failed to stop the pppd daemon [pid %d], errno %d]",
				      d_pppdPid, errnoKill);
			LOG4CPP_ERROR(log, "       %s",
				      strerror(errnoKill));
			return GPRS_PPPD_KILL_FAILED;
		}
		
		// Wait for PPP Daemon to completely shut down
		//sleep(PPPD_SHUTDOWN_LATENCY);
		return OK;
	}

	LOG4CPP_WARN(log, "Ther is NOT running PPP daemon to stop");
	return GPRS_PPPD_NOT_RUNNING;

}


pid_t
DeviceGPRS::getPppdPid(bool update) {
	char module[] = "gprs0";
	std::string pidFile;
	FILE *fd;
	short retry = 4;
	
	if (!update)
		return d_pppdPid;

	module[4] += (d_module%10);
	pidFile = string("/var/run/ppp-") + module + ".pid";

	LOG4CPP_DEBUG(log, "Using PidFile: %s", pidFile.c_str());

	// Small loop cycle to wait for PID file to be created
	while ( !(fd = fopen(pidFile.c_str(), "r") )
	       && --retry) {
		LOG4CPP_DEBUG(log, "Waiting for PPPD pid file [%s] creation...",
				pidFile.c_str());
		sleep(500);
	}

	if (fd) {
		LOG4CPP_DEBUG(log, "PPPD's pid file created");
		fclose(fd);
		std::ifstream ifs(pidFile.c_str());

		// Reading the PID from the first line
		ifs >> pidFile;
		d_pppdPid = (pid_t) atoi(pidFile.c_str());

		ifs.close();
	} else {
		LOG4CPP_DEBUG(log, "PPPD's pid file NOT created");
		return -1;
	}

	return d_pppdPid;

}

exitCode
DeviceGPRS::checkModem() {
	t_stringVector resp;
	
	LOG4CPP_DEBUG(log, "Checking if modem is responding... ");
	d_tty->sendSerial("AT", &resp);
	
	if (resp[0]!="OK")
		return GPRS_AT_RESPONCE_KO;
	
	return OK;
	
}

void
DeviceGPRS::cleanUp() {
	t_supportedLinks::iterator it;
	
	LOG4CPP_MARK(log);
	
	// Releasing netlink configurations
	d_curNetlinkName = 0;
	d_curNetlinkConf = 0;
	it = d_supportedLinks.begin();
	while ( it != d_supportedLinks.end() ) {
		delete (it->second);
		it++;
	}
	d_supportedLinks.clear();
	
	delete(d_tty);
	
}


DeviceGPRS::~DeviceGPRS() {

	//TODO clean the d_supportedLinks map
	
	if (d_netStatus > LINK_DOWN) {
		disconnect();
	}
	
	cleanUp();

}


exitCode
DeviceGPRS::updateState(t_netStatus state) {
	comsys::Command * cGprsState;
	
	if ( state == d_netStatus) {
		return OK;
	}
	
	d_netStatus = state;
	
	//Notify a status change
	cGprsState = comsys::Command::getCommand(GPRS_STATUS_UPDATE,
		Device::DEVICE_GPRS, "DEVICE_GPRS", name());
	if ( !cGprsState ) {
		LOG4CPP_FATAL(log, "Unable to build a new Command for GPRS state update");
		return OUT_OF_MEMORY;
	}
	cGprsState->setParam( "state",  state);
	cGprsState->setParam( "descr",  d_netStatusStr[state]);
	
	// Notifying command
	notify(cGprsState);
	
	return OK;
	
}

void
DeviceGPRS::pppNotifyState(bool running) {
	// Default empty implementation
	LOG4CPP_DEBUG(log, "PPP notify default (empty) implementation");
}


//----- [ PPPD monitoring ]-----------------------------------------------------

exitCode
DeviceGPRS::notifyCaller(void) {
#ifdef USE_THREAD_SYNC
	if (d_cmdThread) {
		LOG4CPP_DEBUG(log, "Resuming thread [%s] @ [%p]...",
			d_cmdThread->getName(),
			d_cmdThread);
		d_cmdThread->resume();
	}
#endif
	return OK;
}

exitCode
DeviceGPRS::checkPipe(std::string const & pipe) {
	
	unlink(pipe.c_str());
	if (mknod(pipe.c_str(), S_IFIFO|0644, 0)) {
		LOG4CPP_FATAL(log, "Error on creating the PPPD log pipe [%s]",
				pipe.c_str());
		return GPRS_PPPD_PIPE_FAILURE;
	}
	
	return OK;
}

inline std::string
DeviceGPRS::pppDaemonLogfile(char *logFile, unsigned int size) {
	char module[] = "gprs0";
	
	module[4] += d_module%10;
	d_pppdLogFile = string(d_pppdLogFolder + "/ppp-" + module + ".log");
	
	if (size) {
		strncpy(logFile, d_pppdLogFile.c_str(), size);
	}
	
	return d_pppdLogFile;
}

exitCode
DeviceGPRS::pppdParseLog(const char *logline) {
	char buf[10];
	
// 	LOG4CPP_DEBUG(log, "PPPD: %s", logline);

	// The current implementation of the log parser require the use
	//	of 'ip-up' and 'ip-down' scripts that pppd call at start and
	//	end of connections.
	//	Those scripts require a 6th parameter used to define the
	//	the pipe filepath parsed by this code

	switch (logline[0]) {

//--- Connection shutdown
	case 'C':
		// 'Connect time'
		if (!strncmp(logline, "CTime", 5)) {
			sscanf(logline, "%*s %9s", buf);
			LOG4CPP_INFO(log, "Connection time [%s]", buf);
			
			//--- Notifying a LINK_DOWN
			// NOTE when an API interface is down, the GPRS connection
			//	is down too
			updateState(LINK_DOWN);
			pppNotifyState(false);
			notifyCaller();
			LOG4CPP_INFO(log, "PPPD daemon is DOWN");
			return OK;
		}
		break;
	case 'L':
		// 'LName: link name'
		if (!strncmp(logline, "LName", 5)) {
			sscanf(logline, "%*s %9s", buf);
			LOG4CPP_DEBUG(log, "Link name [%s]", buf);
			return OK;
		}
		break;
	case 'R':
		// 'RByte'
		if (!strncmp(logline, "RByte", 5)) {
			sscanf(logline, "%*s %9s", buf);
			LOG4CPP_DEBUG(log, "Received bytes [%s]", buf);
			return OK;
		}
		break;
	case 'S':
		// 'SByte'
		if (!strncmp(logline, "SByte", 5)) {
			sscanf(logline, "%*s %9s", buf);
			LOG4CPP_DEBUG(log, "Sent bytes [%s]", buf);
			return OK;
		}
		break;
	case 'T':
		// 'Terminating'
		if (!strncmp(logline, "Terminating", 11)) {
			LOG4CPP_DEBUG(log, "Terminating PPP...");
			//--- Notifying a LINK_GOING_DOWN
			updateState(LINK_GOING_DOWN);
			pppNotifyState(false);
			return OK;
		}
		break;

//--- Connection startup
	case 'b':
		// 'baud: baud rate of the tty device'
		if (!strncmp(logline, "baud", 4)) {
			sscanf(logline, "%*s %6s", d_pppConf.speed);
			LOG4CPP_DEBUG(log, "Baud rate [%s]", d_pppConf.speed);
			return OK;
		}
		break;
	case 'd':
		// 'dName: Device Name'
		if (!strncmp(logline, "dName", 5)) {
			sscanf(logline, "%*s %15s", d_pppConf.device);
			LOG4CPP_DEBUG(log, "PPP Device [%s]", d_pppConf.device);
			return OK;
		}
		break;
	case 'i':
		// 'iName: Interface Name'
		if (!strncmp(logline, "iName", 5)) {
			sscanf(logline, "%*s %5s", d_pppConf.interface);
			LOG4CPP_DEBUG(log, "PPP Interface [%s]", d_pppConf.interface);
			return OK;
		}
		break;
	case 'l':
		// 'lIP: local IP address'
		if (!strncmp(logline, "lIP", 3)) {
			sscanf(logline, "%*s %15s", d_pppConf.ipLocal);
			LOG4CPP_INFO(log, "Local IP [%s]", d_pppConf.ipLocal);
			return OK;
		}
		break;
	case 'p':
		// 'pDNS: primary DNS address'
// 		if (!strncmp(logline, "pDNS", 4)) {
// 			sscanf(logline, "%*s %15s", d_pppConf.dns1);
// 			LOG4CPP_DEBUG(log, "Primary DNS [%s]", d_pppConf.dns1);
// 			return OK;
// 		}
		break;
	case 'r':
		// 'rIP: remote IP address'
		if (!strncmp(logline, "rIP", 3)) {
			sscanf(logline, "%*s %15s", d_pppConf.ipRemote);
			LOG4CPP_INFO(log, "Remote IP [%s]", d_pppConf.ipRemote);
			
			//--- Notifying a LINK_UP
			// NOTE when an API interface is UP, the GPRS connection
			//	may NOT be up
			updateState(LINK_API_UP);
			pppNotifyState(true);
			notifyCaller();
			LOG4CPP_INFO(log, "PPPD daemon UP");

			return OK;
		}
		break;
	case 's':
		// 'sDNS: secondary DNS address'
// 		if (!strncmp(logline, "sDNS", 4)) {
// 			sscanf(logline, "%*s %16s", d_pppConf.dns2);
// 			LOG4CPP_DEBUG(log, "Secondary DNS [%s]", d_pppConf.dns2);
// 			return OK;
// 		}
		break;


	default:
		LOG4CPP_DEBUG(log, "Unknowen PPPD log message: %s",
			      logline);
		return GPRS_PPPD_UNKNOWED_SENTENCE;
	}
	
	return GPRS_PPPD_UNKNOWED_SENTENCE;
}

exitCode
DeviceGPRS::pppdMonitor() {
	std::string logFile = pppDaemonLogfile();
	std::ifstream ifsLog;
	int d_fd;
	FILE *d_fs;
	char ifsLogLine[DEVICEGPRS_MAX_PPPDLOGLINE];
	struct pollfd plog;
	int strl;


	LOG4CPP_DEBUG(log, "monitoring pppd logfile [%s]", logFile.c_str());

	//FIXME We should check that logFile is a pipe: otherwise we have issues on
	// reading without blocking... may we should check that the required log is
	// a fifo, otherwise we have to rebuild it
	if (checkPipe(logFile.c_str())) {
		LOG4CPP_FATAL(log, "error on monitoring pppd process");
		return GPRS_PPPD_PIPE_FAILURE;
	}

	d_parserRunning = true;

OPENFIFO:
	// Opening the log _fifo_
	d_fd = open(logFile.c_str(), O_RDONLY | O_NONBLOCK);
	if (!d_fd) {
		LOG4CPP_ERROR(log,
			      "Trick failed! Unable to open PPPD (piped) logfile [%s], %s",
			      logFile.c_str(), strerror(errno));
		return GPRS_PPPD_LOGFILE_FAILURE;
	}
	// Building the log file stream to be used by fgets
	d_fs = fdopen(d_fd, "r");
	if (!d_fs) {
		LOG4CPP_ERROR(log, "Failed to buil file stream, %s",
			      strerror(errno));
		return GPRS_PPPD_LOGFILE_FAILURE;
	}
	// Configuring for poll syscall
	plog.fd = d_fd;
	plog.events = POLLIN;

	LOG4CPP_INFO(log, "Monitoring PPP daemon logfile [%s]",
		     logFile.c_str());

	while (!d_doExit) {
		
		if (poll(&plog, 1, -1) == -1) {
			LOG4CPP_ERROR(log, "poll on logfile failed, %s",
				      strerror(errno));
			sleep(1000);
			continue;
		}

		//NOTE return NULL on error or when end of file occurs while no
		//	characters have been read.
		if (!fgets(ifsLogLine, DEVICEGPRS_MAX_PPPDLOGLINE, d_fs)) {
			if (feof(d_fs)) {
				// if we are at EOF => the pipe has been closed!
				LOG4CPP_DEBUG(log, "EOF on reading file stream");
				clearerr(d_fs);
				fclose(d_fs);
				::close(d_fd);
				goto OPENFIFO;
			}
			LOG4CPP_ERROR(log, "Error on reading file stream");
			sleep(1000);
			continue;
		}
		// Ignoring empty lines (for next test be valid)
		strl = strlen(ifsLogLine);
		if (strl==1) { // there is only the final '\n'
			continue;
		}
		
		pppdParseLog(ifsLogLine);
		
		if (strl >= DEVICEGPRS_MAX_PPPDLOGLINE) {
			// Ignoring the remaining portion of a long message
			fgets(ifsLogLine, DEVICEGPRS_MAX_PPPDLOGLINE, d_fs);
		}
		
		
	} // While
	
	d_parserRunning = false;
	return OK;
}

exitCode
DeviceGPRS::runParser() {
	LOG4CPP_DEBUG(log, "Running DUMMY GPRS configuration");
	return OK;
};

void
DeviceGPRS::run(void) {
	LOG4CPP_DEBUG(log, "Running DUMMY GPRS configuration");
}


// void
// PppLogParserThread::run(void) {
// // 	std::ostringstream tName;
// // 	
// // 	
// // 	tName << "run_" << d_name << "-" << d_module;
// // 	PosixThread::setName(tName.str().c_str());
// // 	d_runThread = this;
// // 	
// // 	LOG4CPP_INFO(log, "Starting thread [%s]", PosixThread::getName());
// // 	LOG4CPP_DEBUG(log, "Run thread [%s] is @ [%p]",
// // 			PosixThread::getName(),
// // 			d_runThread);
// 	
// 	pppdMonitor();
// 
// }


} // namespace gprs
} // namespace controlbox
