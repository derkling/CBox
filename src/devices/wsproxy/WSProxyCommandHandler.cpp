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


#include "WSProxyCommandHandler.ih"

//#define CONTROLBOX_USELAN

namespace controlbox {
namespace device {

WSProxyCommandHandler * WSProxyCommandHandler::d_instance = 0;

//FIXME product index _MUST_ match the definition of t_idProduct
const WSProxyCommandHandler::t_product WSProxyCommandHandler::d_products[] = {
    	{1203, "Diesel"},
    	{1203, "BluDiesel"},
    	{1203, "Petrol"},
    	{1203, "BluSuper"},
    	{1203, "GPL"},
    	{1203, "Bitumen"},
    	{1203, "JetFeul"}
    };

WSProxyCommandHandler::WSProxyCommandHandler(std::string const & logName) :
        comsys::CommandHandler(logName),
        d_name(logName),
        d_msgCount(0),
        d_configurator(Configurator::getInstance()),
        d_devTime(0),
        d_devGPS(0),
        d_devODO(0),
        d_devAS(0),
        d_devPoll(0),
        d_pollCd(0),
        d_pollCmd(0),
        d_pollState(NOT_MOVING),
        d_nextPollTime(0),
        d_lastStopTime(0),
        d_gpsFixStatus(DeviceGPS::DEVICEGPS_FIX_NA),
        d_netStatus(DeviceGPRS::LINK_DOWN),
        d_queuesUpdated(false),
        d_uploadOldMessages(true),
//         d_wsAccess("wsAccessMtx"),
        d_uqMutex("uploadQueueMtx"),
        d_doExit(false),
        d_okToExit(false) {

    LOG4CPP_DEBUG(log, "WSProxyCommandHandler(const std::string &, bool)");

    // Initializing configuration parameters
    preloadParams();

    // Initializing Command Parsers
    setupCommandParser();

    // Setup upload queues
    initUploadQueues();

    // Initialize the poller uploader
    initPoller();


}

WSProxyCommandHandler * WSProxyCommandHandler::getInstance(std::string const & logName) {

    if ( !d_instance ) {
        d_instance = new WSProxyCommandHandler(logName);
    }

    LOG4CPP_DEBUG(d_instance->log, "WSProxyCommandHandler::getInstance()");

    return d_instance;
}

inline
exitCode WSProxyCommandHandler::preloadParams() {

    LOG4CPP_DEBUG(log, "Loading configuration params...");

    // TODO: check for params correctness!!!
    d_configurator.param("idAutista", WSPROXY_DEFAULT_IDA, true);
    d_configurator.param("idMotrice", WSPROXY_DEFAULT_IDM, true);
    d_configurator.param("idSemirimorchio", WSPROXY_DEFAULT_IDS, true);
    d_configurator.param("CIM", WSPROXY_DEFAULT_CIM, true);

    d_configurator.param("WSProxy_polltime_stopped", WSPROXY_POLLTIME_NOT_MOVING, true);
    d_configurator.param("WSProxy_polltime_moving", WSPROXY_POLLTIME_MOVE_NO_GPS, true);
    d_configurator.param("WSProxy_polltime_movingwgps", WSPROXY_POLLTIME_MOVE, true);

    d_configurator.param("WSProxy_polltime_min", WSPROXY_POLLTIME_MIN, true);

    dumpQueueFilePath = d_configurator.param("dumpQueueFilePath", DEFAULT_DUMP_QUEUE_FILEPATH);

    return OK;
}

inline
exitCode WSProxyCommandHandler::setupCommandParser() {

    // TODO: Definire i CommandType... tipicamente sono associati
    //		ai devices che li generano.

    LOG4CPP_DEBUG(log, "Initializing command parsers...");

    d_cmdParser[DeviceGPS::GPS_EVENT_FIX_GET] = &WSProxyCommandHandler::cp_gpsEvent;
    d_cmdParser[DeviceGPS::GPS_EVENT_FIX_LOSE] = &WSProxyCommandHandler::cp_gpsEvent;

    d_cmdParser[DeviceOdometer::ODOMETER_EVENT_MOVE] = &WSProxyCommandHandler::cp_sendOdoEvent;
    d_cmdParser[DeviceOdometer::ODOMETER_EVENT_STOP] = &WSProxyCommandHandler::cp_sendOdoEvent;
    d_cmdParser[DeviceOdometer::ODOMETER_EVENT_OVER_SPEED] = &WSProxyCommandHandler::cp_sendOdoEvent;
    d_cmdParser[DeviceOdometer::ODOMETER_EVENT_EMERGENCY_BREAK] = &WSProxyCommandHandler::cp_sendOdoEvent;

    d_cmdParser[DeviceGPRS::GPRS_STATUS_UPDATE] = &WSProxyCommandHandler::cp_gprsStatusUpdate;

    d_cmdParser[PollEventGenerator::SEND_POLL_DATA] = &WSProxyCommandHandler::cp_sendPollData;
    d_cmdParser[DeviceInCabin::SEND_GENERIC_DATA] = &WSProxyCommandHandler::cp_sendGenericData;
    d_cmdParser[DeviceInCabin::SEND_CODED_EVENT] = &WSProxyCommandHandler::cp_sendCodedEvent;
    d_cmdParser[DeviceDigitalSensors::DIGITAL_SENSORS_EVENT] = &WSProxyCommandHandler::cp_sendDSEvent;

    d_cmdParser[DeviceTE::SEND_TE_EVENT] = &WSProxyCommandHandler::cp_sendTEEvent;
    d_cmdParser[DeviceSignals::SYSTEM_EVENT] = &WSProxyCommandHandler::cp_sendSignalEvent;

    return OK;
}

inline
exitCode WSProxyCommandHandler::initUploadQueues() {

	LOG4CPP_DEBUG(log, "Loading upload queues...");
	// TODO load queues from file dump


	LOG4CPP_INFO(log, "Upload queues loaded (%d priority levels)",
				WSPROXY_UPLOAD_QUEUES);
	printQueuesStatus();

	return OK;

}

exitCode WSProxyCommandHandler::startUpload() {

    // Linking required devices
    linkDependencies();

    // Initializing Query interface
    exportQuery();

    // Starting the upload thread
    start();

    return OK;
}


#if 0
inline
unsigned int WSProxyCommandHandler::epEnabled() {
    std::string param;
    unsigned int epActives;

    param = d_configurator.param("epmasky", "", true);
    if ( !param.size() ) {
    	// By default we activate all the EndPoints
    	epActives = EndPoint::WS_EP_ALL;
    } else {
    	sscanf(param.c_str(), "%i", &epActives);
    }

    return epActives;

}
#endif

inline
exitCode WSProxyCommandHandler::loadEndPoints() {
	short epId;
	std::ostringstream epCfgLable("");
	std::string epCfg;
	int epType;
	EndPoint * ep;

	// Load EndPoint Configuration
	epId = WSPROXY_EP_FIRST_ID;
	while ( epId <= WSPROXY_EP_MAXNUM ) {

		epCfgLable.str("");
		epCfgLable << "WSProxy_EndPoint_" << epId; // << "_type";
		epCfg = d_configurator.param(epCfgLable.str(), "");

		if ( !epCfg.size() ) {
			break;
		}

		sscanf(epCfg.c_str(), "%i", &epType);

		ep = EndPoint::getEndPoint(epType, epCfgLable.str(), log.getName());
		if ( !ep ) {
			LOG4CPP_ERROR(log, "failed to load an EndPoint [%d]", epType);
		} else {
			LOG4CPP_DEBUG(log, "EndPoint [%s] successfully loaded", ep->name().c_str());
			d_endPoints.push_back(ep);
		}

		// Looking for next EndPoint definition
		epId++;
	};

	return OK;
}

inline
exitCode WSProxyCommandHandler::initPoller() {
// 	DeviceFactory * d_devFactory = DeviceFactory::getInstance();
// 	EndPoint * ep;
// 	unsigned int pollTime;

	LOG4CPP_DEBUG(log, "Initialize poller uploader... ");

	// Building the polling device
	d_pollCmd = controlbox::comsys::Command::getCommand(
					controlbox::device::PollEventGenerator::SEND_POLL_DATA,
					controlbox::Device::EG_POLLER,
					"SendPollData",
					"PollData");
	if ( !d_pollCmd ) {
		LOG4CPP_FATAL(log, "Failed building Poll Command");
	}
	d_pollCmd->setPrio(WSPROXY_DEFAULT_QUEUE);
	d_pollCd = new controlbox::comsys::CommandDispatcher(this, false);
	d_pollCd->setDefaultCommand(d_pollCmd);

	// NOTE if we set 0 here at startup, if we are not moving, the system
	// start immediately in "NOT_MOVING" state; otherwise, at least a
	// minimum stop time is required before moving in this state even if
	// we are alwasy stopped.
	//d_lastStopTime = std::time(0);
	d_lastStopTime = 0;
	updatePoller(false, true);

	return OK;
}

inline
exitCode WSProxyCommandHandler::linkDependencies() {
	DeviceFactory * d_devFactory = DeviceFactory::getInstance();
// 	EndPoint * ep;
// 	unsigned int pollTime;

	LOG4CPP_DEBUG(log, "Finding dependendant services... ");

	LOG4CPP_DEBUG(log, "Loading required EndPoints... ");
	//NOTE the EndPoint loader will initialize required GPRSs too
	loadEndPoints();
	LOG4CPP_INFO(log, "%d EndPoints loadeds: %s", d_endPoints.size(), listEndPoint().c_str());

	//FIXME check for correct initialization!!!
	d_devTime = d_devFactory->getDeviceTime();
	d_devAS = d_devFactory->getDeviceAS();
	d_devGPS  = d_devFactory->getDeviceGPS();
	d_devODO  = d_devFactory->getDeviceODO();

	return OK;
}

inline
std::string WSProxyCommandHandler::listEndPoint() {
    t_EndPoints::iterator it;
    std::ostringstream epList("");

    it = d_endPoints.begin();
    while ( it != d_endPoints.end() ) {
        epList << (*it)->name() << " ";
        it++;
    }

    return epList.str();

}

WSProxyCommandHandler::~WSProxyCommandHandler() {
    t_EndPoints::iterator it;

    LOG4CPP_INFO(log, "Stopping WSProxyCommandHandler, no more Commands will by uploaded");

    // Safely terminate the upload thread...
    d_doExit = true;
    if ( isRunning() ) {
        this->resume();
    }

    // Wait for upload thread to terminate
    ::sleep(1);

    // Releasing EndPoints...
    it = d_endPoints.begin();
    while (!d_endPoints.empty()) {
        delete d_endPoints.front();
        d_endPoints.pop_front();
    }

    //terminate();

}


exitCode WSProxyCommandHandler::notify() {

    LOG4CPP_DEBUG(log, "WSProxyCommandHandler::notify()");
    return OK;

}


exitCode WSProxyCommandHandler::notify(comsys::Command * cmd)
throw (exceptions::IllegalCommandException) {
    t_cmdParser::iterator it;
    t_wsData * l_wsData = 0;
    exitCode result;

    LOG4CPP_DEBUG(log, "%s:%d WSProxyCommandHandler::notify(Command * cmd)", __FILE__, __LINE__);

    // Checking if the command is supported (aka. a Command Parser has
    // been defined for the current command)
    it = d_cmdParser.find(cmd->type());
    if ( it == d_cmdParser.end() ) {
        LOG4CPP_DEBUG(log, "Received a command with type [%u] not supported by this class.", cmd->type());
        return DIS_COMMAND_NOT_SUPPORTED;
    }
    // At this time we have a valid commandProcessor...

    //Dispatching to the associated command parser for command specific
    // actions been taken.
    result = (this->*(it->second))(&l_wsData, *cmd);

    // Checking whatever the command is local adressed
    if ( result == WS_LOCAL_COMMAND ||
    		l_wsData == 0 ) {
        // no more elaboration needed
        LOG4CPP_DEBUG(log, "Completed processing LOCAL command");
        return OK;
    }


    // Otherwise, we should fill-in the command common section
    if ( fillCommonSection(l_wsData, *cmd) != OK ) {
        //TODO: handle memory allocation error...
        LOG4CPP_ERROR(log, "Failed collecting common section data");
        wsDataRelease(l_wsData);
        return WS_MEM_FAILURE;
    }

	// Setting message priority
	l_wsData->prio = cmd->getPrio();
	LOG4CPP_DEBUG(log, "Message prio [%hu]", l_wsData->prio);

	result = queueMsg(*l_wsData);
	if (result!=OK) {
		LOG4CPP_WARN(log, "Failed queuing message");
		return result;
	}

	LOG4CPP_DEBUG(log, "Notify COMPLETED");

	return OK;

}

unsigned int WSProxyCommandHandler::updatePollTime(void) {

	switch (d_pollState) {
	case NOT_MOVING:
		sscanf(d_configurator.param("WSProxy_polltime_stopped", WSPROXY_POLLTIME_NOT_MOVING).c_str(),
			"%u", &d_pollTime);
		LOG4CPP_DEBUG(log, "Configuring POLLTIME for NOT_MOVING");
		break;
	case MOVING_NO_GPS:
		sscanf(d_configurator.param("WSProxy_polltime_moving", WSPROXY_POLLTIME_MOVE_NO_GPS).c_str(),
			"%u", &d_pollTime);
		LOG4CPP_DEBUG(log, "Configuring POLLTIME for MOVE_NO_GPS");
		break;
	case MOVING_WITH_GPS:
		sscanf(d_configurator.param("WSProxy_polltime_movingwgps", WSPROXY_POLLTIME_MOVE).c_str(),
			"%u", &d_pollTime);
		LOG4CPP_DEBUG(log, "Configuring POLLTIME for MOVE");
		break;
	}

	LOG4CPP_INFO(log, "Polltime update to %ds (%s)", d_pollTime,
		(d_pollState==NOT_MOVING) ? "NOT_MOVING" :
			( (d_pollState==MOVING_NO_GPS) ? "MOVING_NO_GPS" :
				"MOVING_WITH_GPS"
			)
		);

	return d_pollTime;
}

exitCode WSProxyCommandHandler::updatePoller(bool notifyChange, bool force) {
	t_pollState state = MOVING_NO_GPS;
	unsigned long stopTime;
	unsigned minStopTime;
	unsigned fix = 0;
	DeviceFactory * d_devFactory;
	double odoSpeed = 0.0, gpsSpeed = 0.0;

	if (d_devGPS && d_devODO) {
		// Collecting needed data
		fix = d_devGPS->fixStatus();
		odoSpeed = d_devODO->odoSpeed();
		gpsSpeed = d_devGPS->gpsSpeed();
	} else {
		LOG4CPP_WARN(log, "GPS and ODO devices not availables");
	}

	LOG4CPP_DEBUG(log, "gpsFix [%u], odoSpeed [%f], gpsSpeed [%f]", fix, odoSpeed, gpsSpeed);

	// Updating current device state
	if (fix) {
		state = MOVING_WITH_GPS;
	} else {
		state = MOVING_NO_GPS;
	}
	if ( (odoSpeed==0) && (gpsSpeed==0) ) {
		if (force) {
			state = NOT_MOVING;
		} else {
			stopTime = (std::time(0) - d_lastStopTime);
			sscanf(d_configurator.param("WSProxy_polltime_minstoptime", WSPROXY_MIN_STOP_TIME).c_str(),
				"%u", &minStopTime);
			if ( stopTime > minStopTime ) {
				LOG4CPP_DEBUG(log, "More than [%d]s since last stop event", minStopTime);
				state = NOT_MOVING;
			} else {
				LOG4CPP_DEBUG(log, "Only [%d]s since last stop event", stopTime);
			}
		}
	}

	if (!force && (state == d_pollState)) {
		LOG4CPP_DEBUG(log, "Poll state not changed (%s)",
			(d_pollState==MOVING_WITH_GPS) ? "MOVING_WITH_GPS" :
				( (d_pollState==MOVING_NO_GPS) ? "MOVING_NO_GPS" : "NOT_MOVING" )
		);
		return WS_POLLER_UPDATE_NOT_NEEDED;
	}

	LOG4CPP_DEBUG(log, "Updating poll time (%s => %s)",
		(d_pollState==MOVING_WITH_GPS) ? "MOVING_WITH_GPS" :
			( (d_pollState==MOVING_NO_GPS) ? "MOVING_NO_GPS" : "NOT_MOVING" ),
		(state==MOVING_WITH_GPS) ? "MOVING_WITH_GPS" :
			( (state==MOVING_NO_GPS) ? "MOVING_NO_GPS" : "NOT_MOVING" ) );

	d_pollState = state;
	LOG4CPP_DEBUG(log, "Poll state changed to %d", d_pollState);

	updatePollTime();

	// Delete any previous used poller
	if (d_devPoll) {
		delete d_devPoll;
	}

	// Build a new poller using corrent defined poll time
	d_devFactory = DeviceFactory::getInstance();
	d_devPoll = d_devFactory->getDevicePoller(d_pollTime*1000, "SendDataPoller");
	d_devPoll->setDispatcher(d_pollCd);
	d_devPoll->enable();

	if ( notifyChange ) {
		// Sending a new poll message to represent the state changed
		notify(d_pollCmd);
	}

	LOG4CPP_DEBUG(log, "Built new DevicePoller(%d)", d_pollTime*1000);

	return OK;

}

inline
std::string WSProxyCommandHandler::getCIM(void) {
	//TODO the CIM is provided by the TIP and thus is automatically defined
	return d_configurator.param("CIM", WSPROXY_DEFAULT_CIM, true);
}

inline
std::string WSProxyCommandHandler::getMTC(void) {
	//FIXME the actual MTC should be returned here
	return std::string("0");
}


inline
exitCode WSProxyCommandHandler::fillCommonSection(t_wsData * p_wsData, comsys::Command & cmd) {

    if ( !p_wsData ) {
        LOG4CPP_DEBUG(log, "The wsData param has not been initialized");
        return WS_MEM_FAILURE;
    }

    // Initializing common data
    strncpy(p_wsData->tx_date, (d_devTime->time()).c_str(), 25);
    p_wsData->cmm.str("");
    p_wsData->cmm << d_configurator.param("idAutista", WSPROXY_DEFAULT_IDA) << ";";
    p_wsData->cmm << d_configurator.param("idMotrice", WSPROXY_DEFAULT_IDM) << ";";
    p_wsData->cmm << d_configurator.param("idSemirimorchio", WSPROXY_DEFAULT_IDS) << ";";
    p_wsData->cmm << getCIM();
    strncpy(p_wsData->rx_lat, d_devGPS->latitude().c_str(), 8);
    strncpy(p_wsData->rx_lon, d_devGPS->longitude().c_str(), 9);
    // Ensuring null termination of GPS coords
    p_wsData->rx_lat[8] = 0;
    p_wsData->rx_lon[9] = 0;

    return OK;

}

inline
exitCode WSProxyCommandHandler::wsDataRelease(t_wsData * p_wsData) {

    LOG4CPP_DEBUG(log, "purgeMsg(t_wsData * wsData)");

    //NO MORE NEEDED because endPoint are bitfileds and not a list! ;-)
    //(wsData->endPoint).clear();

    delete p_wsData;
    p_wsData = 0;

    return OK;

}


void WSProxyCommandHandler::printQueuesStatus(void) {
	std::ostringstream queueStatus("");
	unsigned short i;

	queueStatus << setw(3) << setfill('0') << d_uploadQueues[0].size();
	for (i=1; i<WSPROXY_UPLOAD_QUEUES; i++) {
		if (i==WSPROXY_QUEUING_ONLY_PRI) {
			queueStatus << " | ";
		} else {
			queueStatus << " ";
		}
		queueStatus << setw(3) << d_uploadQueues[i].size();
	}
	LOG4CPP_INFO(log, "Queues entries [%s]", queueStatus.str().c_str());

}

exitCode WSProxyCommandHandler::queueMsg(t_wsData & p_wsData) {
	std::ostringstream queueStatus("");
// 	unsigned short i;

	if ( p_wsData.prio >= WSPROXY_UPLOAD_QUEUES )
		p_wsData.prio = (WSPROXY_UPLOAD_QUEUES-1);

	LOG4CPP_DEBUG(log, "Queuing message with prio [%d]", p_wsData.prio);

LOG4CPP_WARN(log, "MUTEX, QN, W...");
	d_uqMutex.enterMutex();
LOG4CPP_WARN(log, "MUTEX, QN, A");

	d_uploadQueues[p_wsData.prio].push_front(&p_wsData);
	d_queuesUpdated = true;
	d_lastLoadedQueue = p_wsData.prio;

	d_uqMutex.leaveMutex();
LOG4CPP_WARN(log, "MUTEX, QN, R");

	LOG4CPP_INFO(log, "==> Q%u [%05d:%s]", p_wsData.prio, p_wsData.msgCount, getQueueMask(p_wsData.endPoint).c_str());

	printQueuesStatus();

	// Trigger upload thread only if this is not a queuing-only message
	if ( p_wsData.prio < WSPROXY_QUEUING_ONLY_PRI ) {
		LOG4CPP_DEBUG(log, "Polling upload thread...");
		onPolling();
	}

	return OK;

}


exitCode WSProxyCommandHandler::checkEpCommands(EndPoint::t_epRespList &respList) {
	EndPoint::t_epResp * epResp;
	EndPoint::t_epCmd * cmd;

	LOG4CPP_DEBUG(log, "Checking EndPoint commands");

	while (!respList.empty()) {
		epResp = respList.front();

		while (!(epResp->cmds).empty()) {
			cmd = (epResp->cmds).front();

			switch (cmd->code) {
			case EndPoint::EPCMD_CIM:
				LOG4CPP_INFO(log, "Setting CIM [%s]", (cmd->value).c_str());
				d_configurator.setParam("CIM", cmd->value);
				break;
			case EndPoint::EPCMD_FRQ:
				LOG4CPP_INFO(log, "Setting upload frequency [%d]", (cmd->value).c_str());
				d_configurator.setParam("uploadFreq", cmd->value);
				break;
			default:
				LOG4CPP_WARN(log, "EndPoint command [%d] with value [%s] not (yet) supported",
							cmd->code,
							(cmd->value).c_str()
						);
			}

			// Removing command from list and releasing memory
			delete cmd;
			(epResp->cmds).pop_front();
		}
		delete epResp;
		respList.pop_front();
	}

	return OK;
}

std::string WSProxyCommandHandler::getQueueMask(unsigned int queues) {
	unsigned int enabled;
	unsigned short i;
	char buf[] = "__________\n";


	enabled = EndPoint::getEndPointQueuesMask();

	for (i=0; i<10; i++) {
		if ( ! (enabled & (0x01<<i)) )
			break;
		buf[i] = (queues & (0x01<<i)) ? ('A'+i) : '-';
	}
	buf[i]=0;

	return buf;
}

exitCode WSProxyCommandHandler::callEndPoints(t_wsData & p_wsData, t_mgsType p_type) {
    std::ostringstream msg("");
    t_EndPoints::iterator it;
    exitCode result = WS_UPLOAD_FAULT;
    bool l_okToProcess = false;
    bool l_allRemoteFailed = true;
    bool l_disconnectNetwork = false;
    unsigned short l_failures = EP_WORKING;

    LOG4CPP_DEBUG(log, "WSProxyCommandHandler::callEndPoints(t_wsData & p_wsData)");

//     // NOTE: This code MUST acquire a MUTEX!!!
//     d_wsAccess.enterMutex();

// #ifdef CONTROLBOX_USELAN
//     if ( 0 ) {
// #else
//     if (d_netStatus == DeviceGPRS::LINK_DOWN ) {
// #endif
//         LOG4CPP_ERROR(log, "Data upload FAILURE - link DOWN");
//         // Releasing the Mutex;
//         d_wsAccess.leaveMutex();
//         return WS_LINK_DOWN;
//     }

    // Formatting the data for EndPoint processing
    msg << p_wsData.idSrc << ";";
    msg << d_devTime->time() << ";";
    msg << p_wsData.rx_date << ";";
    msg << p_wsData.cx_date << ";";
    msg << p_wsData.cmm.str() << ";";
    msg << getMTC() << ";";
    msg << p_wsData.rx_lat << ";";
    msg << p_wsData.rx_lon << ";";
    msg << p_wsData.msg.str();

    LOG4CPP_INFO(log, "Q%u [%05d:%s] ==>", p_wsData.prio, p_wsData.msgCount, getQueueMask(p_wsData.endPoint).c_str());

    // Sending message to all EndPoints
    it = d_endPoints.begin();
    while ( it != d_endPoints.end() ) {
	LOG4CPP_DEBUG(log, "UPLOAD THREAD: Prosessing msg with EndPoint [%s]", ((*it)->name()).c_str() );


//----- Verifying upload policy

	if ( (*it)->type() < EndPoint::EPTYPE_REMOTE ) {
		LOG4CPP_DEBUG(log, "Processing LOCAL EndPoint");
		l_okToProcess = true;
	} else {
		l_failures = (*it)->failures();
		if ( l_failures <= EP_WORKING ) {
			LOG4CPP_DEBUG(log, "Processing REMOTE-WORKING EndPoint");
			l_okToProcess = true;
		} else {
			if ( l_failures <= EP_TRYING ) {
				LOG4CPP_DEBUG(log, "Processing REMOTE-TRYING EndPoint");
				l_okToProcess = ( p_type == WS_MSG_NEW );
			}
		}
	}

//----- Uploading messages
	if ( l_okToProcess ) {
		LOG4CPP_DEBUG(log, "Porcessing ENABLED for EndPoint [%s]", (*it)->name().c_str());
		result = (*it)->process(p_wsData.msgCount, msg.str(), p_wsData.endPoint, p_wsData.respList);
		switch (result) {
		case OK:
			checkEpCommands(p_wsData.respList);
//----- Decreasing failures
			(*it)->setFailures(l_failures-1);
			l_allRemoteFailed = false;
			break;
		case WS_FORMAT_ERROR:
			LOG4CPP_WARN(log, "Discarding message due to format error");
			p_wsData.endPoint = 0x0;
			l_allRemoteFailed = false;
			break;
		default:
//----- Increasing failures
			(*it)->setFailures(l_failures+1);
			LOG4CPP_WARN(log, "UPLOAD THREAD: Unandeled return code");
		}
	} else {
		LOG4CPP_DEBUG(log, "Porcessing DISABLED for EndPoint [%s]", (*it)->name().c_str());
	}

        it++;

    }

    LOG4CPP_DEBUG(log, "UPLOAD THREAD: all EndPoint have been processed");

	if ( l_allRemoteFailed ) {
		LOG4CPP_WARN(log, "Remote endpoints NOT reachables");

		// Suppose all endpoint has MORE than EP_TRYING failures
		// and then the network connection should be recovered
		l_disconnectNetwork = true;

		// Suppose all endpoint has MORE than EP_WORKING failures
		// and then only new messages should be uploaded
		d_uploadOldMessages = false;

		// Checking if at least one endpoint has less than EP_TRYING
		// failures to disable network recovery
		// Checking if at least one endpoint has less than EP_WORKING
		// failures to enable queued messages upload
		it = d_endPoints.begin();
		while ( it != d_endPoints.end() ) {
			if ( (*it)->type() >= EndPoint::EPTYPE_REMOTE ) {
				l_failures = (*it)->failures();
				if ( l_failures <= EP_WORKING ) {
					d_uploadOldMessages = true;
				}
				if ( l_failures <= EP_TRYING ) {
					l_disconnectNetwork = false;
				}
			}
			it++;
		}
	}

	if ( l_disconnectNetwork ) {
		LOG4CPP_WARN(log, "Resetting network connections");

		it = d_endPoints.begin();
		while ( it != d_endPoints.end() ) {
			if ( (*it)->type() >= EndPoint::EPTYPE_REMOTE ) {
				(*it)->suspending();
			}
		}

	}


    // Remove data ONLY if all endPoints have successfully completed
    // their processing
    if ( p_wsData.endPoint ) {
        LOG4CPP_DEBUG(log, "==> Q%u [%05d:%s]", p_wsData.prio, p_wsData.msgCount, getQueueMask(p_wsData.endPoint).c_str());
        result = WS_UPLOAD_FAULT;
    } else {
        LOG4CPP_DEBUG(log, "UPLOAD THREAD: data processing completed by all EndPoint");
        result = OK;
    }

//     // Releasing the Mutex;
//     d_wsAccess.leaveMutex();
    return result;

}

exitCode WSProxyCommandHandler::notifyEndPoints(bool p_suspend) {
    std::ostringstream msg("");
    t_EndPoints::iterator it;
    exitCode result = WS_UPLOAD_FAULT;

    // Sending message to all EndPoints
    it = d_endPoints.begin();
    while ( it != d_endPoints.end() ) {
	LOG4CPP_DEBUG(log, "UPLOAD THREAD: Notifying EndPoint [%s]", ((*it)->name()).c_str() );

	if ( p_suspend ) {
		LOG4CPP_DEBUG(log, "Notifying SUSPEND to EndPoints... ");
		(*it)->suspending();
	} else {
		LOG4CPP_DEBUG(log, "Notifying RESUME to EndPoints... ");
		(*it)->resuming();
	}

        it++;
    }

    return result;

}

/*
exitCode WSProxyCommandHandler::flushUploadQueueToFile(std::string const & filepath) {

	LOG4CPP_DEBUG(log, "flushUploadQueueToFile(std::string const & filepath)");

}


exitCode WSProxyCommandHandler::loadUploadQueueFromFile(std::string const & filepath) {

	LOG4CPP_DEBUG(log, "loadUploadQueueFromFile(std::string const & filepath)");

}
*/


void WSProxyCommandHandler::run(void) {
	// A pointer to a gSOAP message to upload
	t_uploadList::iterator it;
	unsigned int qIndex;
	controlbox::ThreadDB *l_tdb = ThreadDB::getInstance();
	unsigned short l_pid;
	exitCode result;

	l_pid = getpid();

	LOG4CPP_INFO(log, "Thread [%s (%u)] started", "UQ", l_pid);

	this->setName("UQ");
	result = l_tdb->registerThread(this, l_pid);

	do { // While system is running...

		// Updating poll time (if needed)
		result = updatePoller(false);
		if ( result == WS_POLLER_UPDATE_NOT_NEEDED ) {
			// If the poller has been update we don't suspend
			// EndPoints because there could be a new message to
			// be uploaded...
			if ( d_pollState == NOT_MOVING ) {
				notifyEndPoints(true);
			}
		}

		LOG4CPP_WARN(log, "UPLOAD THREAD: SUSPENDING");
		this->suspend();

		// Notify EndPoints about resume...
		notifyEndPoints(false);

		do { // While new messages have been queued during upload...

			// Start serving queues from the higher priority ones
			d_queuesUpdated = false;
			LOG4CPP_WARN(log, "Queue update flag reset");

			// Upload the most recent message
			do {
				it = d_uploadQueues[d_lastLoadedQueue].begin();
				if ( it != d_uploadQueues[d_lastLoadedQueue].end() ) {
					result = callEndPoints( *(*it), WS_MSG_NEW );
					if (result == OK) {
LOG4CPP_WARN(log, "MUTEX, UN, W...");
	d_uqMutex.enterMutex();
LOG4CPP_WARN(log, "MUTEX, UN, A");
						// Removing the SOAP message from the upload queue;
						LOG4CPP_DEBUG(log, "UPLOAD THREAD: removing MOST RECENT message from queue");
						delete (*it);
						it = d_uploadQueues[qIndex].erase(it);
	d_uqMutex.leaveMutex();
LOG4CPP_WARN(log, "MUTEX, UN, R");
					}
					printQueuesStatus();
				}
			} while ( d_queuesUpdated && !d_doExit );

			if ( !d_uploadOldMessages ) {
				LOG4CPP_WARN(log, "Old messages upload DISABLED");
				continue;
			}

			// Upload each queue in higher priority order
			for (qIndex=0;
				qIndex<WSPROXY_UPLOAD_QUEUES &&
				!d_queuesUpdated && !d_doExit;
				qIndex++) {

				// Find the first higher priority queue not empty
				it = d_uploadQueues[qIndex].begin();
				if ( it == d_uploadQueues[qIndex].end() ) {
					continue;
				}

				LOG4CPP_DEBUG(log, "Serving queue Q%u...", qIndex);

				// While the queue is not empty and no new messages
				// has been queued, or the system is shutting down...
				while ( it != d_uploadQueues[qIndex].end() &&
					!d_queuesUpdated && !d_doExit ) {
					result = callEndPoints( *(*it), WS_MSG_QUEUED );
					if (result == OK) {
LOG4CPP_WARN(log, "MUTEX, UQ, W...");
	d_uqMutex.enterMutex();
LOG4CPP_WARN(log, "MUTEX, UQ, A");
						// Removing the SOAP message from the upload queue;
						LOG4CPP_DEBUG(log, "UPLOAD THREAD: removing message from queue");
						delete (*it);
						it = d_uploadQueues[qIndex].erase(it);
	d_uqMutex.leaveMutex();
LOG4CPP_WARN(log, "MUTEX, UQ, R");
					} else {
						// NOTE the erase already update the iterator to the
						// next entry! ;-)
						it++;
					}
					printQueuesStatus();
				}

			}

		} while (d_queuesUpdated && !d_doExit);

	} while ( !d_doExit );

	LOG4CPP_WARN(log, "Terminating upload thread...");

	// Terminating the Poll Event Generator
	delete d_devPoll;
	delete d_pollCd;
	delete d_pollCmd;

	// Uploading last-one HIGH-PRIORITY message
	it = d_uploadQueues[0].begin();
	if ( it != d_uploadQueues[0].end() ) {
		result = callEndPoints( *(*it) );
		if (result == OK) {
			// Removing the SOAP message from the upload queue;
			LOG4CPP_DEBUG(log, "UPLOAD THREAD: removing message from queue");
			it = d_uploadQueues[qIndex].erase(it);
		}
	}

	LOG4CPP_WARN(log, "Upload queue terminated");
	d_okToExit = true;


	LOG4CPP_WARN(log, "Thread [%s (%u)] terminated", this->getName(), l_pid);
	result = l_tdb->unregisterThread(this);

}

void WSProxyCommandHandler::onShutdown(void) {

	d_queuesUpdated = true;
	d_doExit = true;

	LOG4CPP_WARN(log, "Terminating the upload thread...");
	if ( isRunning() ) {
		this->resume();
	}

	// Wait for upload thread stopping
	LOG4CPP_DEBUG(log, "Waiting for upload thread to terminate");
	while ( !d_okToExit ) {
		LOG4CPP_DEBUG(log, "Waiting for upload thread to terminate");
		::sleep(1);
	}

	//TODO save upload queue contents


}

void WSProxyCommandHandler::onPolling(void) {
	LOG4CPP_DEBUG(log, "Resuming ready data messages's upload thread");
	if ( isRunning() ) {
		this->resume();
	}
}

// void WSProxyCommandHandler::onHangup(void) {
// 	LOG4CPP_WARN(log, "SIGHUP recevied");
// 	d_doExit = true;
// 	if ( isRunning() ) {
// 		resume();
// 	}
// }
//
// void WSProxyCommandHandler::onException(void) {
// 	LOG4CPP_WARN(log, "SIGABRT recevied");
// 	d_doExit = true;
// 	if ( isRunning() ) {
// 		resume();
// 	}
// }

inline
WSProxyCommandHandler::t_wsData * WSProxyCommandHandler::newWsData(t_idSource src) {
    t_wsData * l_wsData;

    LOG4CPP_DEBUG(log, "WSProxyCommandHandler::newWsData(t_idSource src)");

    l_wsData = new t_wsData();

    // DEFAULT initialization
    l_wsData->msgCount = ++d_msgCount;
    l_wsData->endPoint = EndPoint::getEndPointQueuesMask();
    l_wsData->prio = WSPROXY_DEFAULT_QUEUE;
    l_wsData->idSrc = src;
    strncpy(l_wsData->rx_date, (d_devTime->time()).c_str(), WSPROXY_TIMESTAMP_SIZE);
    (l_wsData->rx_date)[WSPROXY_TIMESTAMP_SIZE] = 0;
    (l_wsData->cmm).str("");
    (l_wsData->msg).str("");

    LOG4CPP_DEBUG(log, "Created new wsData struct at %p", l_wsData);

    return l_wsData;

}

//------------------------------------------------------------------------------
//				Command Parsers
//------------------------------------------------------------------------------

//-----[ Local Commands ]-------------------------------------------------------

/// GPS fix status update
/// Command type: DeviceGPS::GPS_EVENT<br>
/// Command params:
/// <ul>
///	<li>[int] fix: the fix status [NA=0, 2D=1, 3D=2]</li>
/// </ul>
exitCode WSProxyCommandHandler::cp_gpsEvent(t_wsData ** p_wsData, comsys::Command & cmd) {
	exitCode result = WS_LOCAL_COMMAND;

	LOG4CPP_DEBUG(log, "Parsing command GPS_EVENT");

	(*p_wsData) = 0; // NO wsData: local command

	switch (cmd.type()) {
	case DeviceGPS::GPS_EVENT_FIX_GET:
		LOG4CPP_INFO(log, "GPS FIX_GET event");
		updatePoller();
		break;
	case DeviceGPS::GPS_EVENT_FIX_LOSE:
		LOG4CPP_INFO(log, "GPS FIX_LOSE event");
		updatePoller();
		break;
	}

	return result;

}

/// NET link status update
/// Command type: DeviceGPRS::GPRS_STATUS_UPDATE<br>
/// Command params:
/// <ul>
///	<li>[int] status: the link status [DOWN=0, GOING_DOWN=1, GOING_UP=2, UP=3]</li>
/// </ul>
exitCode WSProxyCommandHandler::cp_gprsStatusUpdate(t_wsData ** p_wsData, comsys::Command & cmd) {
	exitCode result = WS_LOCAL_COMMAND;

	LOG4CPP_DEBUG(log, "Parsing command GPRS_STATUS_UPDATE");

	// NO wsData: local command
	(*p_wsData) = 0;

/*
    try {
        d_netStatus = (DeviceGPRS::t_netStatus)cmd.getIParam("state");
    } catch (exceptions::UnknowedParamException upe) {
        LOG4CPP_ERROR(log, "Missing [state] param on GPRS_STATUS_UPDATE command processing");
        return WS_MISSING_COMMAND_PARAM;
    }

    if ( d_netStatus == DeviceGPRS::LINK_UP) {
        LOG4CPP_INFO(log, "Network is UP, Resuming upload thread");
        onPolling();
    }

    return result;
*/

	return result;

}


//-----[ Query interface ]------------------------------------------------------


exitCode WSProxyCommandHandler::exportQuery() {
    QueryRegistry * registry;
//     Querible::t_queryDescription * descr;

    registry = QueryRegistry::getInstance();
    if ( !registry ) {
        LOG4CPP_WARN(log, "Unable to export query: registry not found");
        return WS_REGISTRY_NOT_FOUND;
    }

    return OK;
}


exitCode  WSProxyCommandHandler::query(Querible::t_query & p_query) {
//     comsys::Command * cmd;

    LOG4CPP_DEBUG(log, "Received new query [name: %s], [type: %d], [value: %s]", p_query.descr->name.c_str(), p_query.type, p_query.value.c_str());

    if ( p_query.descr->name == "SGD" ) {

        LOG4CPP_DEBUG(log, "SET[%s] = %s", p_query.descr->name.c_str(), p_query.value.c_str());
        /*
        		cmd = new comsys::Command(1,Device::WSPROXY, 0, "AT+SGD");
        		cmd->setParam("timestamp", "");
        		cmd->setParam( "msg", );
        */
    }

    LOG4CPP_WARN(log, "Query received [%s] not supported", p_query.descr->name.c_str());
    return OK;
}




//-----[ Remote Commands ]------------------------------------------------------

// TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
// Il protocollo di invio deve essere legato all'endpoint.
// Quindi:
//	- nei command_handler aggiungiamo dei parametri al Command
//	- passiamo il command al metodo process dell'EndPoint
//	- l'EndPoint estrae i parametri di interesse e li riformatta come più
//		opportuno per l'invio eggettivo.
//	- i command così arricchiti di parametri sono adatti per essere
//		serializzati e salvati su memoria in caso di shutdown
// TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO


exitCode WSProxyCommandHandler::formatDistEvent(t_wsData ** p_wsData, comsys::Command & cmd, t_idSource src) {


   LOG4CPP_DEBUG(log, "Building new DIST event");

    // Building the new wsData element
    (*p_wsData) = newWsData(src);

    strncpy((*p_wsData)->cx_date, cmd.param("timestamp").c_str(), WSPROXY_TIMESTAMP_SIZE);
    ((*p_wsData)->cx_date)[WSPROXY_TIMESTAMP_SIZE] = 0;

    // Appending event data
    (*p_wsData)->msg << setw(2) << setfill('0') << hex << cmd.param("dist_evtType");
    (*p_wsData)->msg << ";" << cmd.param("dist_evtData");

    return OK;
}


/// Dati a frequenza predefinita.
/// Command type: PollEventGenerator::SEND_POLL_DATA<br>
/// Variable Part Code: 01<br>
/// Command params: NONE
exitCode WSProxyCommandHandler::cp_sendPollData(t_wsData ** p_wsData, comsys::Command & cmd) {
    short count = 0;
    std::ostringstream strId("");
    std::ostringstream strData("");
    std::ostringstream tmp;
    float asValue;
    exitCode result;

    LOG4CPP_DEBUG(log, "Parsing command [01] SEND_POLL_DATA");


//     d_configurator.param("WSProxy_polltime_min", WSPROXY_POLLTIME_MIN, true);



    (*p_wsData) = newWsData(WS_SRC_CONC);
    //FIXME we should consider OUT_OF_MEMORY problems!!!

    strncpy((*p_wsData)->cx_date, (d_devTime->time()).c_str(), WSPROXY_TIMESTAMP_SIZE);
    ((*p_wsData)->cx_date)[WSPROXY_TIMESTAMP_SIZE] = 0;


    // Setting string FLAGS
    strId << std::uppercase;
    strData << std::uppercase;

    // GPS Velocity
    strId << "01";
    strData << setw(2) << setfill('0') << hex << (unsigned)d_devGPS->gpsSpeed();
    count++;

    // GPS Direction
    strId << "02";
    strData << setw(3) << setfill('0') << hex << (unsigned)d_devGPS->course();
    count++;

    // Pressure on "Sospensioni"
    result = d_devAS->read("04_APRES", asValue);
    if (result != OK) {
    	LOG4CPP_DEBUG(log, "Analog sensor 04_APRES not defined");
    } else {
	strId << "03";
	asValue = ((asValue) > 0x270F ) ? 0x270f : asValue;
	strData << setw(4) << setfill('0') << hex << (unsigned)asValue;
	count++;
    }

    // Odo distance (in 1/8 of meters)
    strId << "04";
    strData << setw(8) << setfill('0') << hex << (unsigned)(d_devODO->distance()*8);
    count++;

    // Odo velocity
    strId << "05";
    strData << setw(2) << setfill('0') << hex << (unsigned)(d_devODO->odoSpeed(DeviceOdometer::KMH));
    count++;

    // Longitudinal inclination
    result = d_devAS->read("00_INCL", asValue);
    if (result != OK) {
    	LOG4CPP_DEBUG(log, "Analog sensor 00_INCL not defined");
    } else {
	strId << "06";
	if (asValue<0) {
		// Negative values should be trasmitted as (8bit) 2-complement
		asValue = 256+asValue;
	}
	strData << setw(2) << setfill('0') << hex << (unsigned)asValue;
	count++;
    }

    // Trasversal inclination
    result = d_devAS->read("01_INCT", asValue);
    if (result != OK) {
    	LOG4CPP_DEBUG(log, "Analog sensor 00_INCT not defined");
    } else {
	strId << "07";
	if (asValue<0) {
		// Negative values should be trasmitted as (8bit) 2-complement
		asValue = 256+asValue;
	}
	strData << setw(2) << setfill('0') << hex << (unsigned)asValue;
	count++;
    }

    // Truk CAN data
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [0D] Truk CAN data");

    // Trailer CAN data
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [0E] Trailer CAN data");

    // Product quantities
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [0F] Product quantities");

    // Products pressure
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [10] Products pressure");

    // Product temperature
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [11] Product temperature");

    // Product density
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [12] Product density");

    // Water quantities
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [13] Water quantities");

    // Product type
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [14] Product type");

    // Explosive mixture levels
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [15] Explosive mixture levels");

    // Odo distance (raw data)
    // TODO:
    LOG4CPP_DEBUG(log, "TODO: [16] Odo distance (raw data)");

    (*p_wsData)->msg << "01;" << setw(2) << setfill('0') << hex << count;
    (*p_wsData)->msg << strId.str();
    (*p_wsData)->msg << strData.str();

    return OK;

}


/// Messaggio a struttura non pre-codificata proveniente dall'autista.
/// Command type: DeviceInCabin::SEND_GENERIC_DATA<br>
/// Variable Part Code: 09<br>
/// Command params:
/// <ul>
///	<li>timestamp: message creation data</li>
///	<li>msg: </li>
/// </ul>
exitCode WSProxyCommandHandler::cp_sendGenericData(t_wsData ** p_wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing command [09] SEND_GENERIC_DATA");

	return formatDistEvent(p_wsData, cmd, WS_SRC_ICD);

}

/// Messaggio a struttura pre-codificata proveniente dall'autista.
/// Command type: DeviceInCabin::SEND_CODED_EVENT<br>
/// Variable Part Code: 0A<br>
/// Command params:
/// <ul>
///	<li>event: event code, 2 hex digit</li>
/// </ul>
exitCode WSProxyCommandHandler::cp_sendCodedEvent(t_wsData ** p_wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing command [0A] SEND_CODED_EVENT");

	return formatDistEvent(p_wsData, cmd, WS_SRC_ICD);

}

/// Eventi provenienti dai sensori digitali.
exitCode WSProxyCommandHandler::cp_sendDSEvent(t_wsData ** p_wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing DS Event");

	return formatDistEvent(p_wsData, cmd, WS_SRC_CONC);

}

/// Eventi provenienti dalla Testata Elettronica.
exitCode WSProxyCommandHandler::cp_sendTEEvent(t_wsData ** p_wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing TE Events");

	return formatDistEvent(p_wsData, cmd, WS_SRC_CONC);

}

/// Eventi provenienti dall'odometro.
exitCode WSProxyCommandHandler::cp_sendOdoEvent(t_wsData ** p_wsData, comsys::Command & cmd) {
// 	int event;
	// NOTE if not otherwise specified the command is LOCALLY ADDRESSED
	exitCode result = WS_LOCAL_COMMAND;

	LOG4CPP_DEBUG(log, "Parsing ODO Event");


	switch (cmd.type()) {
	case DeviceOdometer::ODOMETER_EVENT_MOVE:
		LOG4CPP_DEBUG(log, "MOVE event");
		*p_wsData = 0; // NO wsData: local command
		updatePoller();
		break;
	case DeviceOdometer::ODOMETER_EVENT_STOP:
		LOG4CPP_DEBUG(log, "STOP event");
		*p_wsData = 0; // NO wsData: local command
		d_lastStopTime = std::time(0);
		updatePoller();
		break;
	case DeviceOdometer::ODOMETER_EVENT_OVER_SPEED:
		LOG4CPP_DEBUG(log, "OVER_SPEED event");
		// This events should be notified
		result = formatDistEvent(p_wsData, cmd, WS_SRC_CONC);
		break;
	case DeviceOdometer::ODOMETER_EVENT_EMERGENCY_BREAK:
		LOG4CPP_DEBUG(log, "EMERGENCY_BREAK event");
		// This events should be notified
		result = formatDistEvent(p_wsData, cmd, WS_SRC_CONC);
		break;
	case DeviceOdometer::ODOMETER_EVENT_SAFE_SPEED:
		*p_wsData = 0; // NO wsData: local command
		LOG4CPP_DEBUG(log, "SAFE SPEED event (ignored)");
		break;
	case DeviceOdometer::ODOMETER_EVENT_DIST_ALARM:
		*p_wsData = 0; // NO wsData: local command
		LOG4CPP_DEBUG(log, "DIST ALARM event");
		if ( d_devPoll ) {
			d_devPoll->trigger();
		}
		break;
	}

	return result;

}

/// Eventi provenienti dall'odometro.
exitCode WSProxyCommandHandler::cp_sendSignalEvent(t_wsData ** p_wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing Signal Event");

	return formatDistEvent(p_wsData, cmd, WS_SRC_CONC);

}

}// namespace device
}// namespace controlbox
