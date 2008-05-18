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

DeviceGPRS::DeviceGPRS(short module, t_gprs_models model, std::string const
		       &logName) :
			CommandGenerator(logName),
			Device(Device::DEVICE_GPRS, module, logName),
			d_doExit(false),
			d_config(Configurator::getInstance()),
			d_module(module),
			d_model(model),
			d_ttyConfig(""),
			d_curNetlinkName(0),
			d_curNetlinkConf(0),
			d_pppdPid(0),
			d_mode(DEVICEGPRS_MODE_COMMAND),
			d_netStatus(DeviceGPRS::LINK_DOWN),
			d_cmdThread(0),
			d_runThread(0),
			isParserRunning(false),
			#if CONTROLBOX_DEBUG > 1
			ct(Device::ct),
			#endif
			log(Device::log)
{

	LOG4CPP_DEBUG(log, "DeviceGPRS(std::string const &)");

	LOG4CPP_WARN(log,
		     "Created a DeviceGPRS without an associated Dispatcher");

	LOG4CPP_DEBUG(log, "Using GPRS module configuration number %d",
		      d_module);

	//----- Loading GPRS configuration params
	//TODO: Verify 'd_config' initialization
	
	// modem's params
	// Loading the TTY port configuration string
	d_ttyConfig = d_config.param(paramName("tty"), DEVICEGPRS_DEFAULT_GPRS_DEVICE);

	sscanf(d_config.param(paramName("atDelay"),
				DEVICEGPRS_DEFAULT_AT_RESPONCE_DELAY).c_str(),
		"%i", &d_atResponceDelay);

	sscanf(d_config.param(paramName("cmdEscape"),
				DEVICEGPRS_DEFAULT_AT_CMD_ESCAPE).c_str(),
		"%i", &d_atCmdEscape);

	d_atInitString = d_config.param(paramName("atInitString"),
				DEVICEGPRS_DEFAULT_AT_INIT_STRING);

#if 0
	if ( d_atCmdEscape != (unsigned)'\r') {
		LOG4CPP_FATAL(log, "ERRRRRRRRRRRRRRRORE: %u != %u",
					d_atCmdEscape,
					(unsigned)'\r');
	}
#endif

	// pppd's params
	d_pppdRunScript =
	    d_config.param(paramName("pppd", "runscript"),
			   DEVICEGPRS_DEFAULT_PPPD_RUNSCRIPT);

	d_pppdPidFolder =
	    d_config.param(paramName("pppd", "pidpath"),
			   DEVICEGPRS_DEFAULT_PPPD_PIDPATH);

	d_pppdLatency =
	    (short) atoi(d_config.
			 param(paramName
			       ("pppd", "latency"),
			       DEVICEGPRS_DEFAULT_PPPD_LATENCY).c_str());

	d_pppdLogFolder =
	    d_config.param(paramName("pppd", "logpath"),
			   DEVICEGPRS_DEFAULT_PPPD_LOGPATH);
	
	// loading supported linknames
	loadNetLinks();
	
	// preloading the default linkname
#if 0
	d_linkname =
	    d_config.param(paramName("linkname"),
			   DEVICEGPRS_DEFAULT_PPPD_LINKNAME);
	d_pdpContext =
	    d_config.param(paramName(d_linkname, "PDP_Context"),
			   DEVICEGPRS_DEFAULT_PDP_CONTEXT);
	d_AtDial =
	    d_config.param(paramName(d_linkname, "dial"),
			   DEVICEGPRS_DEFAULT_DIAL);
#endif

/*-----------------------------------------------------------------------------
    	if ( gprsConnect("TIM") != OK ) {
    		LOG4CPP_ERROR(log, "Connection problems");
    		return;
    	}

    	sleep(30000);
    	sendAT("AT+CSQ","ECCO IL BACO",0,20);

    	sleep(10000);
    	pppdTerminate();
//*---------------------------------------------------------------------------*/

	// Registering device into the DeviceDB
	dbReg();

}

exitCode
DeviceGPRS::getDeviceIds() {
	ttymodem modem((*this));	// The TTY port stream
	t_stringVector lresp;
	exitCode result;

	// Getting the SIM number
	// this command will return something like:
	// +CNUM: "Line 1","12125551212",145
	result = modem.sendAT("AT+CNUM", "OK", &lresp, 3);
	switch (result) {
	case OK:
		if ( lresp.empty() ) {
			LOG4CPP_WARN(log, "SIM number not returned");
		}
		//TODO parse out the sim number and place it into d_simNumber
		LOG4CPP_DEBUG(log, "SIM number is here [%s]",  lresp.front().c_str());
		break;
	default:
		LOG4CPP_WARN(log, "Unable to get SIM number");
	}
	if ( modem.sendAT("AT+CGMI", "OK", &lresp, 4) == OK ) {
		LOG4CPP_INFO(log, "Manufacturer ID [%s]", lresp[0].c_str());
		d_ids.manufactor = lresp[0];
	}
	lresp.clear();
	if ( modem.sendAT("AT+CGMM", "OK", &lresp, 4) == OK ) {
		LOG4CPP_INFO(log, "Model ID [%s]", lresp[0].c_str());
		d_ids.model = lresp[0];
	}
	lresp.clear();
	if ( modem.sendAT("AT+CGMR", "OK", &lresp, 4) == OK ) {
		LOG4CPP_INFO(log, "Revision ID [%s]", lresp[0].c_str());
		d_ids.revision = lresp[0];
	}
	lresp.clear();
	if ( modem.sendAT("AT+CGSN", "OK", &lresp, 4) == OK ) {
		LOG4CPP_INFO(log, "IMEM [%s]", lresp[0].c_str());
		d_ids.imei = lresp[0];
	}
	lresp.clear();
	if ( modem.sendAT("AT+CIMI", "OK", &lresp, 4) == OK ) {
		LOG4CPP_INFO(log, "IMSI [%s]", lresp[0].c_str());
		d_ids.imsi = lresp[0];
	}
	
	modem.close();
	return OK;

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

std::string
DeviceGPRS::getDeviceIdentifier() {
	//FIXME we should return the IMEI
	return d_simNumber;
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

	LOG4CPP_MARK(log);

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

	_logName << logName << "_" << module;
	try {
		switch (d_model) {
		case DEVICEGPRS_MODEL_SONYERICSSON:
			gprs = new SonyEricsson(module, _logName.str());
			break;
		case DEVICEGPRS_MODEL_ENFORA:
			gprs = new Enfora(module, _logName.str());
			break;
		default:
			// Failure: unsupported model
			LOG4CPP_FATAL(log, "Unsupported GPRS model required. Controlling device NOT built");
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

#undef USE_THREAD_SYNC
//#define USE_THREAD_SYNC

exitCode
DeviceGPRS::suspendCaller(void) {
#ifdef USE_THREAD_SYNC
	std::ostringstream tName;
	
	///FIXME we should consider the case of multiple caller to suspend...
	/// the same class puclic method could be called by multiples different threads
	/// we should suspend all them and notify all them once the wakeup condition
	/// happens
	/// By know we optimize for just ONE caller each time, exceding callers
	/// will do a busy waiting!
	if (d_cmdThread) {
		LOG4CPP_WARN(log, "Multiple thread waiting for run_%s",
			d_name.c_str());
#endif
		PosixThread::sleep(PPPD_SARTUP_POLLTIME);
#ifdef USE_THREAD_SYNC
		return OK;
	}
	
	tName << "cmd_" << d_name << "-" << d_module;
	PosixThread::setName(tName.str().c_str());
	d_cmdThread = this;
	
	LOG4CPP_DEBUG(log, "Suspending thread [%s] @ [%p]",
		PosixThread::getName(),
		d_cmdThread);
		
	suspend();
	
	LOG4CPP_DEBUG(log, "Resumed thread  [%s] @ [%p]",
		PosixThread::getName(),
		d_cmdThread);
		
	d_cmdThread = 0;
#endif
	return OK;

}

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
DeviceGPRS::gprsConnect(std::string const & linkname) {
	t_supportedLinks::iterator it;
	t_netlink * netlink;

#if 0
	netConnect cThread(*this);
	
	LOG4CPP_DEBUG(log, "Detaching a Connection thread");
	cThread.detach();
	join();
	
	return OK;
#endif
	// If the PPPD parser is not running it's not consistent to start the daemon
	if (!isParserRunning) {
		LOG4CPP_WARN(log, "GPRS Connect failed, PPPD parser not running");
		return GPRS_PARSER_NOT_RUNNING;
	}

	// Check if the required connection is already up
	if ( (d_netStatus >= LINK_GOING_UP) &&
		(d_curNetlinkName) &&
		(linkname == *d_curNetlinkName) ) {
		LOG4CPP_DEBUG(log, "The required netlink [%s] is already up", linkname.c_str());
		return OK;
	}
	
	// Check if the required connection is supported
	it = d_supportedLinks.find(linkname);
	if (it == d_supportedLinks.end()) {
		LOG4CPP_ERROR(log, "Netlink [%s] not supported by this GPRS device", linkname.c_str());
		return GPRS_NETLINK_NOT_SUPPORTED;
	}
	
	// If we are already connected... first we should disconnect
	if ( d_netStatus >= LINK_GOING_UP ) {
		LOG4CPP_DEBUG(log, "Switching GPRS connection from [%s] to [%s]",
				d_curNetlinkName->c_str(), linkname.c_str());
		gprsDisconnect();
	}
	
	// Configure params for the new netlink (SIM, PDPCTX, DIAL...)
	if ( !d_curNetlinkConf) {
		//TODO SIM Card initialization: we select as first that required
		//		by the firt connection we have to enable
		// Place here the code for SIM card activation
	} else {
		if (d_curNetlinkConf->sim != it->second->sim) {
			
			//TODO place here the code for SIM card swapping
			
			LOG4CPP_WARN(log, "SIM Card swapping NOT YET SUPPORTED, aborting!");
			return GENERIC_ERROR;
		}
	}
	d_curNetlinkName = &(it->first);
	d_curNetlinkConf = it->second;
	
	return doConnect(linkname);

}

exitCode
DeviceGPRS::doConnect(std::string const & linkname) {
	ttymodem modem((*this));	// The TTY port stream
	std::string pppdPid;
	std::string linkConf;
	pid_t pidByFile;
	exitCode error;
	unsigned int retry;


	LOG4CPP_DEBUG(log,
		      "DeviceGPRS::gprsConnect(std::string linkname)");
	
	//--- Opening the port to get modem MUTEX
	modem.open(d_ttyConfig);
	if (!modem) {
		LOG4CPP_FATAL(log, "Unable to open TTY port [%s]",
			      d_ttyConfig.c_str());
		return GPRS_TTY_OPEN_FAILURE;
	}
	
	// Resetting modem
	modem.reset();
	
	updateState(LINK_GOING_UP);
	
	LOG4CPP_DEBUG(log, "TTY Device Opened");

#if 0
	//--- Trying to load the required link configuration if different from the
	//     last used one
	if (linkname != d_linkname) {
		// Check if the configuration required is available
		d_linkname = d_config.param(paramName("linkname"), "");
		d_pdpContext =
		    d_config.param(paramName(d_linkname, "PDP_Context"),
				   "");
		d_AtDial =
		    d_config.param(paramName(d_linkname, "dial"), "");
			
		LOG4CPP_DEBUG(log,
			      "link: [%s], PDPctx: [%s], ATD: [%s]",
			      d_linkname.c_str(),
			      d_pdpContext.c_str(), d_AtDial.c_str());
			
		if (!(d_linkname.size() && d_pdpContext.size()
		      && d_AtDial.size())) {
			
			LOG4CPP_ERROR(log,
				      "Unable to load the required GPRS link configuration [%s]",
				      linkname.c_str());
			return GPRS_LINK_NOT_CONFIGURED;
		}
	}
#endif
	
	
	//TODO test for signal level... we shuld have at least with signal
	//          in order to try a connection! ;-)
	error = modem.sendAT("AT+CSQ", "OK", 0, 4);
	switch (error) {
	case OK:
		break;
	default:
		LOG4CPP_WARN(log, "Unable to get signal strength");
	}
	//TODO Implement a check: it shuld be a BUSY-WAIT?!?!
	
	//--- Configuring GPRS Connection
	error = modem.sendAT(d_curNetlinkConf->pdpContext);
	switch (error) {
	case OK:
		break;
	default:
		LOG4CPP_ERROR(log, "PDP Context configuration failed, connection aborted");
		updateState(LINK_DOWN);
		modem.reset();
		modem.close();
		return GPRS_PDPCTX_FAILED;
	}
	
	//--- Switching to Data mode
	error = modem.sendAT(d_curNetlinkConf->AtDial, "CONNECT", 0, 5);
	switch (error) {
	case OK:
		break;
	default:
		LOG4CPP_ERROR(log, "Dial failed, connection aborted");
		updateState(LINK_DOWN);
		modem.reset();
		modem.close();
		return GPRS_CONNECT_FAILURE;
	}
	
	// Updating run mode to Data Mode
	d_mode = DEVICEGPRS_MODE_DATA;
	
	// NOTE releasing the port: it's no more nedded since this moment
	// this is a safety point and MUST be done BEFORE calling
	// pppSession that will eventually fork off a PPP daemon
	// keeping the port occupied
	modem.close();
	
	// Small sleep to avoid "Serial line is looped back" problem
	sleep(1000);
	
	//--- Starting a PPP session
	if (pppdSession(ttyDevice(d_ttyConfig), linkname) != OK) {
		LOG4CPP_ERROR(log,
			      "Problems on starting the PPP daemon for [%s]",
			      linkname.c_str());
		updateState(LINK_DOWN);
		modem.reset();
		modem.close();
		return GPRS_PPPD_FAILURE;
	}
	
	//--- Waiting for PPP coming up
	// NOTE before returning OK we shuld wait for a confirmation from
	// logfiles
	// pppdSession return TRUE only on ppp daemon starting... but it can
	// crash or simply be unable to tear up a connection right
	// after it start and exit with error!!!
	LOG4CPP_INFO(log,
		     "PPP session [%s, PID: %d] is going up... ",
		     linkname.c_str(), d_pppdPid);
	
	// Waiting for PPP to be created
	sleep(3000);
	
	// Waiting for a link state change
	retry = 3;
	while (d_netStatus == LINK_GOING_UP && retry--) {
		LOG4CPP_DEBUG(log,
			      "Waiting for PPP daemon to start [d_netStatus>=%d]",
			      d_netStatus);
// 		sleep(PPPD_SARTUP_POLLTIME);
		//TODO set a time limit!
		//FIXME it whould be better to use an async notification!?!
		suspendCaller();
	}

	if ( !retry || d_netStatus == LINK_DOWN) {
		LOG4CPP_ERROR(log,
			      "PPP session [%s, PID: %d] failed to start",
			      linkname.c_str(), d_pppdPid);
		modem.reset();
		modem.close();
		return GPRS_PPPD_FAILURE;
	}
	
	// Trying to get PPP daemon PID
	pidByFile = getSessionPid();
	if (!pidByFile) {
		//NOTE Supposing that if the PID file is not created the PPP daemon is not running!
		LOG4CPP_FATAL(log,
			      "PID file not created: supposing PPP daemon for [%s] is not running!",
			      linkname.c_str());
		updateState(LINK_DOWN);
		modem.reset();
		modem.close();
		return GPRS_CONNECT_FAILURE;
	}
	d_pppdPid = pidByFile;
	modem.close();
	
	LOG4CPP_INFO(log,
		     "PPP daemon [%s, PID: %d] is up and running",
		     linkname.c_str(), d_pppdPid);
	
	// Notify firend servers about new IP address
	notifyFriends();
	
	return OK;
	
}

exitCode
DeviceGPRS::pppdSession(std::string device, std::string linkname) {
	std::string pppdPid;

	LOG4CPP_DEBUG(log,
		      "DeviceGPRS::pppdSession(device=%s, linkname=%s)",
		      device.c_str(), linkname.c_str());

	d_pppdPid = fork();

	if (d_pppdPid) {	// This is the parent
		//--------------------------------------------------------------
		//              This is the parent
		//--------------------------------------------------------------
		if (d_pppdPid == -1) {
			int errsv = errno;
			d_pppdPid = 0;
			//--- Notifying a LINK_DOWN
			d_netStatus = LINK_DOWN;
			//TODO Fork error handling
			LOG4CPP_FATAL(log, "Error on running PPP daemon");
			LOG4CPP_FATAL(log,
				      "PPP daemon fork failed with error code: %d",
				      errno);
			return GPRS_PPPD_FAILURE;
		}
		
		return OK;
		
	} else {
		//--------------------------------------------------------------
		//              PPPD Daemon Process
		//--------------------------------------------------------------
		// Referring to pppd(8) manpage:
		// 1. the "linkname name" option will
		//	generate a logfile named ppp-name.pid
		//	We use a logfile named ppp-gprsN.pid where N=d_module
		//	This way each modem has a single logfile for each netlink
		//	it will support.
		//	This file will be on /var/run (or /etc/ppp on some
		//	systems)
		// 2. the "ipparam string" option will pass the specified
		//	string as an extra parameter to the ip-up, ip-pre-up
		//	and ip-down scripts. The string supplied is given as
		//	the 6th parameter and is used to pass the filename
		//	of the PIPE that is parsed by the monitoring thread.
		char logFile[256];
		char pidFileLable[] = "gprs0";
		pidFileLable[4] += d_module%10;
		
		pppDaemonLogfile(logFile, 256);
		
		char *const argv[] = {
			(char *) d_pppdRunScript.c_str(),
			(char *) device.c_str(),
			"linkname",
			pidFileLable,
			"ipparam",
			logFile,
			"call",
			(char *) linkname.c_str(),
			0
		};
		char *const envp[] = {
			0
		};
		int errnoFork;

		// TODO: export the pppd launch script as configuration var
		LOG4CPP_DEBUG(log,
				"Starting PPPD [%s %s %s %s %s %s %s %s]",
				argv[0], argv[1], argv[2], argv[3],
				argv[4], argv[5], argv[6], argv[7]);
		if (execve(d_pppdRunScript.c_str(), argv, envp) == -1) {
			errnoFork = errno;
			//--- Notifying a LINK_DOWN
			d_netStatus = LINK_DOWN;
			LOG4CPP_DEBUG(log,
				      "Starting PPPD: execve() error [errno %d]",
				      errnoFork);
			LOG4CPP_ERROR(log, "       %s",
				      strerror(errnoFork));
			return GPRS_CONNECT_FAILURE;
		}
		//--------------------------------------------------------------
	}
}



exitCode
DeviceGPRS::gprsDisconnect() {
#if 0
	netDisconnect dThread(*this);
	
	LOG4CPP_DEBUG(log, "Detaching a Disconnection thread");
	dThread.detach();
	join();
	
	return OK;
#endif
	doDisconnect();
}

exitCode
DeviceGPRS::doDisconnect(void) {
	ttymodem modem((*this));	// The TTY port stream

	LOG4CPP_DEBUG(log, "DeviceGPRS::gprsDisconnect()");
	
	if (d_netStatus <= LINK_GOING_DOWN)
		return OK;
	
	cout << 1 << endl;
	
	//NOTE we should try to complete anyway the connection shutdown: so
	//	we signal only a warning and go haead...
	if (!isParserRunning) {
		LOG4CPP_WARN(log, "Unsafe GPRS Disconnect, PPPD parser not running");
	}

	if (d_netStatus < DeviceGPRS::LINK_GOING_DOWN) {
		LOG4CPP_WARN(log,
			     "Trying to shutdown a link already down");
		return GPRS_LINK_DOWN;
	}
	
	updateState(LINK_GOING_DOWN);
	
	cout << 2 << endl;
	
	// Stopping the PPP daemon before
	if (pppdTerminate() != OK) {
		LOG4CPP_ERROR(log, "Failed in terminating PPP daemon");
		return GPRS_PPPD_FAILURE;
	}
	
	cout << 3 << endl;
	
#if 0
	if (!modem.open(d_ttyConfig)) {
		LOG4CPP_FATAL(log, "Unable to open TTY port [%s]",
			      d_ttyConfig.c_str());
		return GPRS_TTY_OPEN_FAILURE;
	}
	// Ensure no other thread have changed the modem state now that
	//  we have entered the modem mutex
	if (d_netStatus == DeviceGPRS::LINK_DOWN) {
		return OK;
	}
	
	LOG4CPP_DEBUG(log, "TTY Device Opened");
#endif
	
	modem.reset();
	
	cout << 4 << endl;
	
	return OK;

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
		// TODO going to sleep waiting for a notify from the ppp's log parser
		sleep(PPPD_SHUTDOWN_LATENCY);
		return OK;
	}

	LOG4CPP_WARN(log, "Ther's NOT running PPP daemon to stop");
	return GPRS_PPPD_NOT_RUNNING;

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

	isParserRunning = true;

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
		
// 		// Checking if the logmessage is complete (i.e. a '\n' precede the final \0
// 		if (ifsLogLine[strl-1] == '\n') {
// 			ifsLogLine[strl-1] = 0;	// This will keep away the terminal '\n'
// 		} else {
// 			LOG4CPP_WARN(log, "PPPD sentence exceding buffer size [%d < %d]",
// 					DEVICEGPRS_MAX_PPPDLOGLINE, strl);
// 		}
		
		pppdParseLog(ifsLogLine);
		
		if (strl >= DEVICEGPRS_MAX_PPPDLOGLINE) {
			// Ignoring the remaining portion of a long message
			fgets(ifsLogLine, DEVICEGPRS_MAX_PPPDLOGLINE, d_fs);
		}
		
		
	} // While
	
	isParserRunning = false;
	return OK;
}

exitCode
DeviceGPRS::pppdParseLog(const char *logline) {
	char buf[10];
	
	
	//LOG4CPP_DEBUG(log, "PPPD: %s", logline);

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
			updateState(LINK_DOWN);
			notifyCaller();
			LOG4CPP_INFO(log, "GPRS link is DOWN");
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
			return OK;
		}
		break;

//--- Connection shutartup
	case 'b':
		// 'baud: baud rate of the tty device'
		if (!strncmp(logline, "baud", 4)) {
			sscanf(logline, "%*s %5s", d_pppConf.speed);
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
		if (!strncmp(logline, "pDNS", 4)) {
			sscanf(logline, "%*s %15s", d_pppConf.dns1);
			LOG4CPP_DEBUG(log, "Primary DNS [%s]", d_pppConf.dns1);
			return OK;
		}
		break;
	case 'r':
		// 'rIP: remote IP address'
		if (!strncmp(logline, "rIP", 3)) {
			sscanf(logline, "%*s %15s", d_pppConf.ipRemote);
			LOG4CPP_INFO(log, "Remote IP [%s]", d_pppConf.ipRemote);
			//--- Notifying a LINK_UP
			updateState(LINK_UP);
			notifyCaller();
			LOG4CPP_INFO(log, "GPRS link is UP");

			return OK;
		}
		break;
	case 's':
		// 'sDNS: secondary DNS address'
		if (!strncmp(logline, "sDNS", 4)) {
			sscanf(logline, "%*s %16s", d_pppConf.dns2);
			LOG4CPP_DEBUG(log, "Secondary DNS [%s]", d_pppConf.dns2);
			return OK;
		}
		break;


	default:
		LOG4CPP_DEBUG(log, "Unknowen PPPD log message: %s",
			      logline);
		return GPRS_PPPD_UNKNOWED_SENTENCE;
	}
	
	return GPRS_PPPD_UNKNOWED_SENTENCE;
}


pid_t
DeviceGPRS::getSessionPid() {
	std::string pidFile;
	FILE *fd;
	short started;		// non zero if the PID file is present
	pid_t pid = 0;
	
	pidFile = pppDaemonPidfile();
	started = (d_pppdLatency * 2) + 1;	// setting a 500ms wait cycle

	LOG4CPP_DEBUG(log, "Using PidFile: %s", pidFile.c_str());

	// Small loop cycle to wait for PID file to be created
	while (!(fd = fopen(pidFile.c_str(), "r"))
	       && --started) {
		LOG4CPP_DEBUG(log, "Sleep on PID file creation");
		sleep(500);
	}

	if (fd) {
		LOG4CPP_DEBUG(log, "PID file created");
		fclose(fd);
		std::ifstream ifs(pidFile.c_str());
		ifs >> pidFile;	// Reading the PID from the first line
		pid = (pid_t) atoi(pidFile.c_str());
		ifs.close();
	} else {
		LOG4CPP_DEBUG(log, "PID file NOT created");
	}

	return pid;

}

exitCode
DeviceGPRS::suspendPppDaemon() {
	
	if (d_pppdPid) {
		if ( kill(d_pppdPid, PPPD_SUSPEND_SIGNAL) ) {
			LOG4CPP_ERROR(log, "suspending pppd failed, %s", strerror(errno));
			return GPRS_PPPD_SUSPEND_FAILED;
		}
	} else {
		LOG4CPP_DEBUG(log, "pppd not running, failed to suspend");
	}
	return OK;
	
}

exitCode
DeviceGPRS::resumePppDaemon() {
	
	if ( d_pppdPid ) {
		if ( kill(d_pppdPid, PPPD_RESUME_SIGNAL) ) {
			LOG4CPP_ERROR(log, "resuming pppd failed, %s", strerror(errno));
			return GPRS_PPPD_RESUME_FAILED;
		}
	} else  {
		LOG4CPP_DEBUG(log, "pppd not running, failed to resume");
	}
	
	return OK;
}

inline std::string
DeviceGPRS::ttyDevice(std::string ttyConfig) {
	char params[DEVICEGPRS_MAX_TTYCONFIG + 1];

	LOG4CPP_DEBUG(log, "DeviceGPRS::ttyDevice(ttyConfig=%s",
		      ttyConfig.c_str());

	memset(params, 0, DEVICEGPRS_MAX_TTYCONFIG + 1);
	strncpy(params, (char *) ttyConfig.c_str(),
		DEVICEGPRS_MAX_TTYCONFIG);

	// Parsing param to get the TTY device filepath
	if (strtok(params, ":")) {
		LOG4CPP_DEBUG(log, "Null terminated device filepath [%s]",
			      params);
	}
	/*
	   sep = strchr(params, (int)":");
	   if ( sep ) {
	   // Null terminating the device filepath
	   LOG4CPP_DEBUG(d_gprs.log, "Null terminating the device filepath at '%s'", *sep );
	   (*sep) = 0;
	   }
	 */
	return string(params);

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

inline std::string
DeviceGPRS::pppDaemonPidfile(char *pidFile, unsigned int size) {
	
	char module[] = "gprs0";
	module[4] += d_module%10;
	d_pppdPidFile = string(d_pppdPidFolder + "/ppp-" + module + ".pid");
	
	if (size) {
		strncpy(pidFile, d_pppdPidFile.c_str(), size);
	}
	
	return d_pppdPidFile;
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
	
}


DeviceGPRS::~DeviceGPRS() {

	//TODO clean the d_supportedLinks map
	
	d_doExit = true;
	
	if (d_netStatus > LINK_DOWN) {
		gprsDisconnect();
	}
	
	cleanUp();

}


DeviceGPRS::t_netStatus
DeviceGPRS::status() const {

	LOG4CPP_DEBUG(log, "status()");

	return d_netStatus;

}

string
DeviceGPRS::time(bool utc) const {

	//TODO: To implement
	LOG4CPP_WARN(log, "GPRS Time not yet implemented");

	return string("");

}

exitCode
DeviceGPRS::updateState(t_netStatus state) {
	comsys::Command * cGprsState;
	//LINK_STATUS(sstate);
		
	LOG4CPP_MARKP(ct, "(state=%d)", state);
	
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
	//cGprsState->setParam( "descr",  sstate[state]);
	
	// Notifying command
	notify(cGprsState);
	
	return OK;
	
}

void
DeviceGPRS::run(void) {
	std::ostringstream tName;
	

	tName << "run_" << d_name << "-" << d_module;
	PosixThread::setName(tName.str().c_str());
	d_runThread = this;

	LOG4CPP_INFO(log, "Starting thread [%s]", PosixThread::getName());
	LOG4CPP_DEBUG(log, "Run thread [%s] is @ [%p]",
			PosixThread::getName(),
			d_runThread);
	
	pppdMonitor();

}


//------------------------------------------------------------------------------
//              --- GPRS-Modem's Specific Features ---
// The following virtual methods provide only a dummy general implementation
//      and shuld be re-defined in DeviceGPRS's derived classes
//------------------------------------------------------------------------------

// By default Switch-out from Data Mode is not supported
exitCode
DeviceGPRS::switchToCommandMode(ttymodem & modem, bool force) {

	if (!force) {
		LOG4CPP_DEBUG(log,
			      "Switching to Command Mode NOT supported by this module [%d] ",
			      d_module);
		return GPRS_AT_NOT_SUPPORTED;
	}
	// TODO: Forcing Command Mode by Connection Shutdown...
	return GPRS_AT_NOT_SUPPORTED;

}

	// By default Switch-out from Data Mode is not supported
exitCode
DeviceGPRS::switchToDataMode(ttymodem & modem) {

	LOG4CPP_DEBUG(log,
		      "Switching-back to Data Mode NOT supported by this module [%d] ",
		      d_module);
	return GPRS_AT_NOT_SUPPORTED;

}

exitCode
DeviceGPRS::notifyFriends() {

	//TODO implemente the default notification schema
	// we could use UDP packets: referring to the protocol used by Enfora
	// modems
	
	return OK;
	
}

//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
//                      class ttymodem implementation
//------------------------------------------------------------------------------

std::map < std::string, ost::Mutex > DeviceGPRS::ttymodem::d_ttyMutex;

DeviceGPRS::ttymodem::ttymodem(DeviceGPRS & device):
ttystream(), d_gprs(device), d_opened(false) {
	//TODO: init the locales!

	LOG4CPP_MARK(d_gprs.ct);
	
	// Using an Interactive TTY (i.e. no buffering)
// 	this->interactive(false);

}

DeviceGPRS::ttymodem::~ttymodem() {

	// Release a tty stream.
	// This method provide to eventually close the port and
	// release the associated mutex

	if (!d_opened) {
		//LOG4CPP_WARN (d_gprs.log, "Trying to close a not opened TTY port [%s]", d_device.c_str());
		return;
	}

	if (close()) {
		LOG4CPP_DEBUG(d_gprs.log, "TTY [%s] closed",
			      d_device.c_str());
	} else {
		LOG4CPP_DEBUG(d_gprs.log,
			      "Unable to close TTY port [%s]",
			      d_device.c_str());
	}

}

bool
DeviceGPRS::ttymodem::open(std::string ttyConfig, bool blocking) {
	//char params[DEVICEGPRS_MAX_TTYCONFIG+1];
	char *sep;

	LOG4CPP_MARKP(d_gprs.ct,
		      "(ttyConfig=[%s], blocking=[%s])",
		      ttyConfig.c_str(), blocking ? "True" : "False");

	if (d_opened) {
		LOG4CPP_DEBUG(d_gprs.log,
			      "Trying to open an already opened TTY port [%s]",
			      d_device.c_str());
		return true;
	}

	/*
	   memset(params, 0, DEVICEGPRS_MAX_TTYCONFIG+1);
	   strncpy(params, (char *)ttyConfig.c_str(), DEVICEGPRS_MAX_TTYCONFIG);
	   // Parsing param to get the TTY device filepath
	   if (strtok(params, ":") ) {
	   LOG4CPP_DEBUG(d_gprs.log, "Null terminated device filepath [%s]", params);
	   }
	   d_device = string(params);
	 */
	d_device = d_gprs.ttyDevice(ttyConfig);

	// Checking if the port has already an associated mutex
	if (d_ttyMutex.find(d_device) == d_ttyMutex.end()) {
		d_ttyMutex[d_device] = ost::Mutex();
	}
	// Waiting for the required mutex
	if (!blocking) {
		if (!d_ttyMutex[d_device].tryEnterMutex()) {
			LOG4CPP_DEBUG(d_gprs.log,
				      "MUTEX for TTY port [%s] LOCKED",
				      d_device.c_str());
			return false;
		}
	} else {
		d_ttyMutex[d_device].enter();
	}

	LOG4CPP_DEBUG(d_gprs.log,
		      "MUTEX entered for TTY port [%s]", d_device.c_str());

	// Opening the port
	ttystream::open(d_gprs.d_ttyConfig.c_str());

	// Checking for opening problems
	// using ttystream::operator!()
	if (ttystream::operator!()) {
		LOG4CPP_DEBUG(d_gprs.log,
			      "Failed to open TTY device [%s]",
			      d_device.c_str());
		d_ttyMutex[d_device].leave();
		return false;
	}

	d_opened = true;
	LOG4CPP_DEBUG(d_gprs.log, "TTY port [%s] OPENED",
		      d_device.c_str());


	return true;

}

bool
DeviceGPRS::ttymodem::close() {
	// Close the previously opened tty port.
	// This method exit the mutex on the associated port
	// releasing the access to other thread.
	// @return true on successfully close.

	LOG4CPP_DEBUG(d_gprs.log, "DeviceGPRS::ttymodem::close()");

	if (!d_opened) {
		LOG4CPP_WARN(d_gprs.log,
			     "Trying to close a NOT opened TTY port [%s]",
			     d_device.c_str());
		return true;
	}
	// Closing the TTY port
	ttystream::close();

	// Checking for closing problems
	// using ttystream::operator!()
	if (!(*this)) {

		d_opened = false;
		LOG4CPP_DEBUG(d_gprs.log, "TTY port [%s] CLOSED",
			      d_device.c_str());

		// Leaving the mutex
		d_ttyMutex[d_device].leave();
		LOG4CPP_DEBUG(d_gprs.log,
			      "MUTEX released for TTY port [%s]",
			      d_device.c_str());

		return true;
	}

	LOG4CPP_DEBUG(d_gprs.log,
		      "Failed to close TTY device [%s]", d_device.c_str());
	return false;

}

exitCode
DeviceGPRS::ttymodem::reset() {

	if ( d_gprs.d_netStatus >= LINK_GOING_UP) {
		LOG4CPP_DEBUG(d_gprs.log, "Killing active conncetion for modem reset");
	}
	this->sendAT("ATH", "OK", 0, 5);
	this->sendAT("ATZ");
	d_gprs.d_mode = DEVICEGPRS_MODE_COMMAND;
}

exitCode
DeviceGPRS::ttymodem::sendAT(std::
				 string const
				 &at_command,
				 std::
				 string const &exitOn,
				 t_stringVector * resp,
				 short cLines,
				 unsigned long timeout,
				 bool escape) {

	bool opened = !d_opened;	// To know if we have to close the port
	t_gprs_mode prevMode;		// The port mode before at_command
	std::ostringstream ostr("", ios::ate);	// A buffer for the responce string to witch append
	bool switched = false;		// Set to true when we have switched
	exitCode result;


	LOG4CPP_DEBUG(d_gprs.log,
		      "DeviceGPRS::ttymodem::sendAT([%s], [%s], %p, %d, %lu)",
		      at_command.c_str(), exitOn.c_str(), resp, cLines, timeout);

	// (if not yet) opening the port
	// NOTE here opened is logically inverted ;-)
	if (opened && !open(d_gprs.d_ttyConfig)) {
		LOG4CPP_FATAL(d_gprs.log,
			      "Unable to open TTY port [%s]",
			      d_gprs.d_ttyConfig.c_str());
		return GPRS_TTY_OPEN_FAILURE;
	}
	// If not yet in Command Mode
	// TODO handle mode switching from modes different tha Data Mode
	if (d_gprs.d_mode != DEVICEGPRS_MODE_COMMAND) {
		prevMode = d_gprs.d_mode;

		if (d_gprs.switchToCommandMode((*this)) != OK) {
			LOG4CPP_ERROR(d_gprs.log,
				      "Switching to Command Mode Failed");
			close();
			return GPRS_TTY_SWITCH_FAILED;
		}

		switched = true;
		d_gprs.d_mode = DEVICEGPRS_MODE_COMMAND;
	}
	// Sending the required AT command (if not empty)
	if (at_command.size()) {
		LOG4CPP_DEBUG(d_gprs.log,
			      "GPRS module[%d]:< %s",
			      d_gprs.d_module, at_command.c_str(),
			      exitOn.c_str());
		if (escape) {
			(*this) << at_command << (char)d_gprs.d_atCmdEscape;
			(*this).flush();
		} else {
			(*this) << at_command;
			(*this).flush();
		}
	}
	// If not responce required... returning immediatly
	if (!cLines) {
		if (opened) {
			close();
		}
		return OK;
	}
	// Doing a small wait between send AT command and responce read
	if (at_command.size() && escape) {
		sleep(d_gprs.d_atResponceDelay);
	}
	// Reading at_command responce
	result = readFromTTY(exitOn, timeout, cLines, resp);

	// Switch back to Data Mode
	if (switched) {
		d_gprs.switchToDataMode((*this));
	}

	if (opened) {
		close();
	}

	return result;

}

exitCode
DeviceGPRS::ttymodem::readFromTTY(std::string const &str,
				      unsigned long timeout,
				      short maxLines,
				      t_stringVector * lines) {
	const char *p_strRet;	// The return-sentence to wait for
	char l_strRet;		// The return-sentence length
	short inLines;		// The number of lines received from the modem
	bool blocking;		// If we have to wait indefinetly
	std::string modemin;	// The string read-in from the TTY	
	char p_modemin[256];

	LOG4CPP_DEBUG(d_gprs.log,
		      "DeviceGPRS::ttymodem::readFromTTY(%s, %lu, %d, %p)",
		      str.c_str(), timeout, maxLines, lines);

	if (!d_opened) {
		LOG4CPP_WARN(d_gprs.log,
			     "Trying to read from a not opened TTY port");
		return GPRS_TTY_NOT_OPENED;
	}
	// Initializing locales
	p_strRet = str.c_str();
	l_strRet = str.length();
	inLines = 0;
	blocking = (maxLines) ? false : true;

	// Reading at_command responce
	do {
	
#if 0
		//TODO shuld be useful a small sleep?!?!
		// Using non-blocking I/O by default
		if (timeout &&
			!(*this).isPending(ost::Serial::pendingInput, timeout)
			) {
			LOG4CPP_DEBUG(d_gprs.log, "Reading timedout");
			return GPRS_TTY_TIMEOUT;
		}
#endif

		(*this).getline(p_modemin, 256);
		if (strlen(p_modemin)==1)
			continue;
		p_modemin[strlen(p_modemin)-1] = 0; // Removing trailing '\n'
		
		LOG4CPP_DEBUG(d_gprs.log,
			      "TTY [%s [%s:%d]]:> [line %3d(%d)] %s",
			      d_device.c_str(), p_strRet,
			      l_strRet, ++inLines, strlen(p_modemin), p_modemin);

		// TODO Check the return while we are in Switch mode!

		// Check for exit conditions
		if (!strncmp(p_modemin, p_strRet, l_strRet)) {
			LOG4CPP_DEBUG(d_gprs.log, "Responce OK");
			return OK;
		}

		if (!strncmp(p_modemin, "ERROR", 5)) {
			LOG4CPP_DEBUG(d_gprs.log, "Responce KO");
			return GENERIC_ERROR;
		}
		// Saving modem lines if required
		if (lines) {
			lines->push_back( std::string(p_modemin) );
		}

	} while (blocking || --maxLines);

	LOG4CPP_DEBUG(d_gprs.log, "Responce KO");
	return GPRS_TTY_NOT_ARRIVED;

}


} // namespace gprs
} // namespace controlbox
