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
        d_gpsFixStatus(DeviceGPS::DEVICEGPS_FIX_NA),
        d_netStatus(DeviceGPRS::LINK_DOWN),
        d_wsAccess("wsAccessMtx"),
        d_doExit(false) {

    LOG4CPP_DEBUG(log, "WSProxyCommandHandler(const std::string &, bool)");

    // Initializing configuration parameters
    preloadParams();

    // Linking required devices
    linkDependencies();

    // Initializing Command Parsers
    initCommandParser();

    // Initializing Query interface
    exportQuery();

    // Starting the upload thread
    start();

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

    LOG4CPP_DEBUG(log, "Loading configuration params... ");

    // TODO: check for params correctness!!!
    d_configurator.param("idAutista", WSPROXY_DEFAULT_IDA, true);
    d_configurator.param("idMotrice", WSPROXY_DEFAULT_IDM, true);
    d_configurator.param("idSemirimorchio", WSPROXY_DEFAULT_IDS, true);
    d_configurator.param("CIM", WSPROXY_DEFAULT_CIM, true);

    dumpQueueFilePath = d_configurator.param("dumpQueueFilePath", DEFAULT_DUMP_QUEUE_FILEPATH);

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
	unsigned short epType;
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
exitCode WSProxyCommandHandler::linkDependencies() {
    DeviceFactory * df;
    EndPoint * ep;

    LOG4CPP_DEBUG(log, "Finding dependendant services... ");

    //FIXME check for correct initialization!!!

    df = DeviceFactory::getInstance();
    d_devTime = df->getDeviceTime();
    d_devGPS  = df->getDeviceGPS();
    d_devODO  = df->getDeviceODO();

    //TODO load configured endpoints's GPRS
    //d_devGPRS = df->getDeviceGPRS(DeviceGPRS::DEVICEGPRS_MODEL_ENFORA, 0);

    d_devAS = df->getDeviceAS();

    LOG4CPP_DEBUG(log, "Loading required EndPoints... ");
    loadEndPoints();
//     LOG4CPP_DEBUG(log, "Loading all supported EndPoints... ");
//     //--- Load FileEndPoint
//     ep = new FileEndPoint(log.getName());
//     d_endPoints.push_back(ep);
//     //--- Load DistEndPoint
//     ep = new DistEndPoint(log.getName());
//     d_endPoints.push_back(ep);
    LOG4CPP_INFO(log, "%d EndPoints loadeds: %s", d_endPoints.size(), listEndPoint().c_str());

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

inline
exitCode WSProxyCommandHandler::initCommandParser() {

    // TODO: Definire i CommandType... tipicamente sono associati
    //		ai devices che li generano.

    LOG4CPP_DEBUG(log, "Initializing command parsers... ");

    d_cmdParser[DeviceGPS::GPS_FIX_UPDATE] = &WSProxyCommandHandler::cp_gpsFixUpdate;
    d_cmdParser[DeviceGPRS::GPRS_STATUS_UPDATE] = &WSProxyCommandHandler::cp_gprsStatusUpdate;

    d_cmdParser[PollEventGenerator::SEND_POLL_DATA] = &WSProxyCommandHandler::cp_sendPollData;
    d_cmdParser[DeviceInCabin::SEND_GENERIC_DATA] = &WSProxyCommandHandler::cp_sendGenericData;
    d_cmdParser[DeviceInCabin::SEND_CODED_EVENT] = &WSProxyCommandHandler::cp_sendCodedEvent;
    d_cmdParser[DeviceDigitalSensors::DIGITAL_SENSORS_EVENT] = &WSProxyCommandHandler::cp_sendDSEvent;
    d_cmdParser[DeviceOdometer::ODOMETER_EVENT] = &WSProxyCommandHandler::cp_sendOdoEvent;
    d_cmdParser[DeviceTE::SEND_TE_EVENT] = &WSProxyCommandHandler::cp_sendTEEvent;


}

WSProxyCommandHandler::~WSProxyCommandHandler() {
    t_EndPoints::iterator it;

    LOG4CPP_INFO(log, "Stopping WSProxyCommandHandler: no more Commands will by sent to the WS");

    // Safely terminate the upload thread...
    d_doExit = true;
    if ( isRunning() ) {
        resume();
    }

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
    t_wsData * wsData;
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
    result = (this->*(it->second))(&wsData, *cmd);

    // Checking whatever the command is local adressed
    if ( result == WS_LOCAL_COMMAND ) {
        // no more elaboration needed
        return OK;
    }


    // Otherwise, we should fill-in the command common section
    if ( fillCommonSection(wsData, *cmd) != OK ) {
        //TODO: handle memory allocation error...
        wsDataRelease(wsData);
        return WS_MEM_FAILURE;
    }

    // Trying to upload the message or storing it for delayed upload
    // into the upload queue
    if ( uploadMsg(*wsData) == OK) {
        wsDataRelease(wsData);
    }

    LOG4CPP_DEBUG(log, "Notify COMPLETED");

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
exitCode WSProxyCommandHandler::fillCommonSection(t_wsData * wsData, comsys::Command & cmd) {

    LOG4CPP_DEBUG(log, "WSProxyCommandHandler::fillCommonSection(Command & cmd)");

    if ( !wsData ) {
        LOG4CPP_ERROR(log, "The wsData param has not been initialized");
        return WS_MEM_FAILURE;
    }

    // Initializing common data
    strncpy(wsData->tx_date, (d_devTime->time()).c_str(), 25);
    wsData->cmm.str("");
    wsData->cmm << d_configurator.param("idAutista", WSPROXY_DEFAULT_IDA) << ";";
    wsData->cmm << d_configurator.param("idMotrice", WSPROXY_DEFAULT_IDM) << ";";
    wsData->cmm << d_configurator.param("idSemirimorchio", WSPROXY_DEFAULT_IDS) << ";";
    wsData->cmm << getCIM();
    strncpy(wsData->rx_lat, d_devGPS->latitude().c_str(), 8);
    strncpy(wsData->rx_lon, d_devGPS->longitude().c_str(), 9);
    // Ensuring null termination of GPS coords
    wsData->rx_lat[8] = 0;
    wsData->rx_lon[9] = 0;
    return OK;

}

inline
exitCode WSProxyCommandHandler::wsDataRelease(t_wsData * wsData) {

    LOG4CPP_DEBUG(log, "purgeMsg(t_wsData * wsData)");

    //NO MORE NEEDED because endPoint are bitfileds and not a list! ;-)
    //(wsData->endPoint).clear();

    delete wsData;
    wsData = 0;

    return OK;

}


exitCode WSProxyCommandHandler::uploadMsg(t_wsData & wsData) {
    exitCode queueing;

    LOG4CPP_DEBUG(log, "uploadMsg(t_wsData & wsData)");

    queueing = queueMsg(wsData);
    if ( queueing == OK ) {

#if 0
#ifdef CONTROLBOX_USELAN
        LOG4CPP_DEBUG(log, "CONTROLBOX_USELAN");
        if ( 1 ) {
#else
	// FIXME we could have problems if we lost a LINK_UP notifications,
	// e.g. because at initialization time the DeviceGPRS is build before
	// the WSProxy and the link_up notification is thus lost
	// This not safe: it's better to check for GPRS status by direct query
        if (d_netStatus == DeviceGPRS::LINK_UP) {
        //if ( d_devGPRS->status() == DeviceGPRS::LINK_UP) {
#endif
            LOG4CPP_DEBUG(log, "UPLOAD QUEUE: net link is up, polling the upload thread");
            onPolling();
        }
#endif
	LOG4CPP_DEBUG(log, "UPLOAD QUEUE: Data successfully queued, polling endpoints...");
	onPolling();

        return WS_DATA_QUEUED;
    }

    return queueing;

}

exitCode WSProxyCommandHandler::queueMsg(t_wsData & wsData) {

    LOG4CPP_DEBUG(log, "queueMsg(t_wsData & wsData)");

    d_uploadList.push_back(&wsData);

    LOG4CPP_DEBUG(log, "%u data messages queued for delayed uplpoad", d_uploadList.size());

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
}

exitCode WSProxyCommandHandler::callEndPoints(t_wsData & wsData) {
    std::ostringstream msg("");
    t_EndPoints::iterator it;
    exitCode result = WS_UPLOAD_FAULT;

    LOG4CPP_DEBUG(log, "WSProxyCommandHandler::callEndPoints(t_wsData & wsData)");

    // NOTE: This code MUST acquire a MUTEX!!!
    d_wsAccess.enterMutex();

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

    LOG4CPP_DEBUG(log, "Mutex acquired");

    // Formatting the data for EndPoint processing
    msg << wsData.idSrc << ";";
    msg << d_devTime->time() << ";";
    msg << wsData.rx_date << ";";
    msg << wsData.cx_date << ";";
    msg << wsData.cmm.str() << ";";
    msg << getMTC() << ";";
    msg << wsData.rx_lat << ";";
    msg << wsData.rx_lon << ";";
    msg << wsData.msg.str();

    //LOG4CPP_DEBUG(log, "Msg:\n%s", msg.str().c_str());

    // Sending message to all EndPoints
    it = d_endPoints.begin();
    while ( it != d_endPoints.end() ) {
        LOG4CPP_DEBUG(log, "UPLOAD THREAD: Prosessing msg with EndPoint [%s]", ((*it)->name()).c_str() );

	LOG4CPP_INFO(log, "==> %d [0x%02X]", wsData.msgCount, wsData.endPoint);

        result = (*it)->process(wsData.msgCount, msg.str(), wsData.endPoint, wsData.respList);
		checkEpCommands(wsData.respList);
        it++;
    }

    LOG4CPP_DEBUG(log, "UPLOAD THREAD: all EndPoint have been processed");

    // Remove data ONLY if all endPoints have successfully completed
    // their processing
    if ( wsData.endPoint ) {
        LOG4CPP_DEBUG(log, "UPLOAD THREAD: upload NOT completed: rescheduling for future processing");
        result = WS_UPLOAD_FAULT;
    } else {
        LOG4CPP_DEBUG(log, "UPLOAD THREAD: data processing completed by all EndPoint");
        result = OK;
    }

    // Releasing the Mutex;
    d_wsAccess.leaveMutex();
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
	t_wsData * wsData;
	exitCode result;
	unsigned int queueCount;

	LOG4CPP_INFO(log, "Upload thread started");

	while ( !d_doExit ) {

// #ifdef CONTROLBOX_USELAN
//         while ( !d_uploadList.empty() ) {
// #else
//         while ( !d_uploadList.empty() && (d_netStatus == DeviceGPRS::LINK_UP)) {
// #endif

	it = d_uploadList.begin();
        while ( it != d_uploadList.end() ) {

		// Q1 to be implemented
		LOG4CPP_INFO(log, "Q0: 0, Q1: %d, UPLOADING...", d_uploadList.size());

// 		wsData = (t_wsData *) d_uploadList.front();
// 		result = callEndPoints( *wsData );
		result = callEndPoints( *(*it) );


#if 0
//             switch ( callEndPoints( *wsData ) ) {
//
//             case WS_UPLOAD_FAULT:
//                 // NOTE: this message couldn't be uploaded!
//                 // Shuld it be logged and discarded before proceiding with
//                 //		the next one!?!?!
//
//                 //FIXME right now we simply discard the message and continue...
//                 break;
// //                 continue;
//
//             case WS_INVALID_DATA:
//                 // NOTE: What to do if there is some params wrong?!?
//                 // NOW we just discard the wrong SOAP message (leaving some log)
//                 // ---- This solution MUST be reviewed!!! ----
//                 break;
//
//             case WS_LINK_DOWN:
//                 // NOTE: In this case we fall dowd to sleep and wait for a LINK_UP
//                 // event to be notified
//                 LOG4CPP_WARN(log, "UPLOAD THREAD: netlink down, suspending data upload thread");
//                 d_netStatus = DeviceGPRS::LINK_DOWN;
//                 suspend();
//                 break;
//             }
#endif

		// Removing the SOAP message from the upload queue;
// 		d_uploadList.pop_front();
		it = d_uploadList.erase(it);
		it++;
	}

	queueCount = d_uploadList.size();
	if (!queueCount) {
		// Q1 to be implemented
		LOG4CPP_INFO(log, "Q0: 0, Q1: %d, SUSPENDING", d_uploadList.size());
		suspend();
	}

	}

	// On doExit
	//terminate();

}


void WSProxyCommandHandler::onPolling(void) {

	LOG4CPP_DEBUG(log, "Resuming ready data messages's upload thread");

    if ( isRunning() ) {
        resume();
    }

}

void WSProxyCommandHandler::onHangup(void) {

    LOG4CPP_INFO(log, "SIGHUP recevied by ready SOAP messages's upload thread");

    d_doExit = true;
    if ( isRunning() ) {
        resume();
    }

}

void WSProxyCommandHandler::onException(void) {

    LOG4CPP_INFO(log, "SIGABRT recevied by ready SOAP messages's upload thread");

    d_doExit = true;
    if ( isRunning() ) {
        resume();
    }

}

inline
WSProxyCommandHandler::t_wsData * WSProxyCommandHandler::newWsData(t_idSource src) {
    t_wsData * wsData;

    LOG4CPP_DEBUG(log, "WSProxyCommandHandler::newWsData(t_idSource src)");

    wsData = new t_wsData();

    // DEFAULT initialization
    wsData->msgCount = ++d_msgCount;
    wsData->endPoint = EndPoint::getEndPointQueuesMask();
    wsData->idSrc = src;
    strncpy(wsData->rx_date, (d_devTime->time()).c_str(), WSPROXY_TIMESTAMP_SIZE);
    (wsData->rx_date)[WSPROXY_TIMESTAMP_SIZE] = 0;
    (wsData->cmm).str("");
    (wsData->msg).str("");

    LOG4CPP_DEBUG(log, "Created new wsData struct at %p", wsData);

    return wsData;

}

//------------------------------------------------------------------------------
//				Command Parsers
//------------------------------------------------------------------------------

//-----[ Local Commands ]-------------------------------------------------------

/// GPS fix status update
/// Command type: DeviceGPS::GPS_FIX_UPDATE<br>
/// Command params:
/// <ul>
///	<li>[int] fix: the fix status [NA=0, 2D=1, 3D=2]</li>
/// </ul>
exitCode WSProxyCommandHandler::cp_gpsFixUpdate(t_wsData ** wsData, comsys::Command & cmd) {
    DeviceGPS::t_fixStatus fix;

    LOG4CPP_DEBUG(log, "Parsing command GPS_FIX_UPDATE");

    // NO wsData: local command
    *wsData = 0;

    try {
        d_gpsFixStatus = (DeviceGPS::t_fixStatus)cmd.getIParam("fix");
    } catch (exceptions::UnknowedParamException upe) {
        LOG4CPP_ERROR(log, "Missing [fix] param on GPS_FIX_UPDATE command processing");
        return WS_MISSING_COMMAND_PARAM;
    }

    return OK;

}

/// NET link status update
/// Command type: DeviceGPRS::GPRS_STATUS_UPDATE<br>
/// Command params:
/// <ul>
///	<li>[int] status: the link status [DOWN=0, GOING_DOWN=1, GOING_UP=2, UP=3]</li>
/// </ul>
exitCode WSProxyCommandHandler::cp_gprsStatusUpdate(t_wsData ** wsData, comsys::Command & cmd) {
    DeviceGPS::t_fixStatus fix;

    LOG4CPP_DEBUG(log, "Parsing command GPRS_STATUS_UPDATE");

    // NO wsData: local command
    *wsData = 0;

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

    return OK;

}


//-----[ Query interface ]------------------------------------------------------


exitCode WSProxyCommandHandler::exportQuery() {
    QueryRegistry * registry;
    Querible::t_queryDescription * descr;

    registry = QueryRegistry::getInstance();
    if ( !registry ) {
        LOG4CPP_WARN(log, "Unable to export query: registry not found");
        return WS_REGISTRY_NOT_FOUND;
    }

}


exitCode  WSProxyCommandHandler::query(Querible::t_query & query) {
    comsys::Command * cmd;

    LOG4CPP_DEBUG(log, "Received new query [name: %s], [type: %d], [value: %s]", query.descr->name.c_str(), query.type, query.value.c_str());

    if ( query.descr->name == "SGD" ) {

        LOG4CPP_DEBUG(log, "SET[%s] = %s", query.descr->name.c_str(), query.value.c_str());
        /*
        		cmd = new comsys::Command(1,Device::WSPROXY, 0, "AT+SGD");
        		cmd->setParam("timestamp", "");
        		cmd->setParam( "msg", );
        */
    }

    LOG4CPP_WARN(log, "Query received [%s] not supported", query.descr->name.c_str());
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


exitCode WSProxyCommandHandler::formatDistEvent(t_wsData ** wsData, comsys::Command & cmd, t_idSource src) {

    // Building the new wsData element
    (*wsData) = newWsData(src);
    LOG4CPP_DEBUG(log, "Created new wsData struct at %p", wsData);

    strncpy((*wsData)->cx_date, cmd.param("timestamp").c_str(), WSPROXY_TIMESTAMP_SIZE);
    ((*wsData)->cx_date)[WSPROXY_TIMESTAMP_SIZE] = 0;

    // Appending event data
    (*wsData)->msg << setw(2) << setfill('0') << hex << cmd.param("dist_evtType");
    (*wsData)->msg << ";" << cmd.param("dist_evtData");

    return OK;
}


/// Dati a frequenza predefinita.
/// Command type: PollEventGenerator::SEND_POLL_DATA<br>
/// Variable Part Code: 01<br>
/// Command params: NONE
exitCode WSProxyCommandHandler::cp_sendPollData(t_wsData ** wsData, comsys::Command & cmd) {
    short count = 0;
    std::ostringstream strId("");
    std::ostringstream strData("");
    std::ostringstream tmp;
    float asValue;
    exitCode result;

    LOG4CPP_DEBUG(log, "Parsing command [01] SEND_POLL_DATA");

    (*wsData) = newWsData(WS_SRC_CONC);

    strncpy((*wsData)->cx_date, (d_devTime->time()).c_str(), WSPROXY_TIMESTAMP_SIZE);
    ((*wsData)->cx_date)[WSPROXY_TIMESTAMP_SIZE] = 0;


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
	strData << setw(4) << setfill('0') << hex << (unsigned)asValue;
	count++;
    }

    // Odo distance (in 1/8 of meters)
    strId << "04";
    strData << setw(8) << setfill('0') << hex << (unsigned)(d_devODO->distance()*8);
    count++;

    // Odo velocity
    strId << "05";
    strData << setw(2) << setfill('0') << hex << (unsigned)(d_devODO->odoSpeed());
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

    (*wsData)->msg << "01;" << setw(2) << setfill('0') << hex << count;
    (*wsData)->msg << strId.str();
    (*wsData)->msg << strData.str();

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
exitCode WSProxyCommandHandler::cp_sendGenericData(t_wsData ** wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing command [09] SEND_GENERIC_DATA");

	return formatDistEvent(wsData, cmd, WS_SRC_ICD);

}

/// Messaggio a struttura pre-codificata proveniente dall'autista.
/// Command type: DeviceInCabin::SEND_CODED_EVENT<br>
/// Variable Part Code: 0A<br>
/// Command params:
/// <ul>
///	<li>event: event code, 2 hex digit</li>
/// </ul>
exitCode WSProxyCommandHandler::cp_sendCodedEvent(t_wsData ** wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing command [0A] SEND_CODED_EVENT");

	return formatDistEvent(wsData, cmd, WS_SRC_ICD);

}

/// Eventi provenienti dai sensori digitali.
exitCode WSProxyCommandHandler::cp_sendDSEvent(t_wsData ** wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing DS Event");

	return formatDistEvent(wsData, cmd, WS_SRC_CONC);

}

/// Eventi provenienti dalla Testata Elettronica.
exitCode WSProxyCommandHandler::cp_sendTEEvent(t_wsData ** wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing TE Events");

	return formatDistEvent(wsData, cmd, WS_SRC_CONC);

}

/// Eventi provenienti dall'odometro.
exitCode WSProxyCommandHandler::cp_sendOdoEvent(t_wsData ** wsData, comsys::Command & cmd) {

	LOG4CPP_DEBUG(log, "Parsing ODO Event");

	return formatDistEvent(wsData, cmd, WS_SRC_CONC);

}



}// namespace device
}// namespace controlbox
