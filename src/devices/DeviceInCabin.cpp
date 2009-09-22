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


#include "DeviceInCabin.ih"

namespace controlbox {
namespace device {

DeviceInCabin::DeviceInCabin(std::string const & logName, t_interface interface) :
        comsys::CommandGenerator(logName),
        Device(Device::DEVICE_IC, interface, logName),
        d_interface(interface),
        d_hR(new handlerRegistry<DeviceInCabin>(this)),
        log(Device::log) {
    d_df = DeviceFactory::getInstance();

    LOG4CPP_DEBUG(log, "DeviceInCabin(logName=%s, interface=%d)", logName.c_str(), interface);

    DeviceInCabin::exportQuery();
    d_time = d_df->getDeviceTime();

    dbReg();

}

DeviceInCabin::~DeviceInCabin () {

    delete d_hR;

}


inline
exitCode DeviceInCabin::exportQuery() {

    LOG4CPP_DEBUG(log, "DeviceInCabin::exportQuery()");

    //--- "Invio dato generico da palmare"
    //d_hR->setHandler(SEND_GENERIC_DATA, &DeviceInCabin::qh_SendGenericData);
    //LOG4CPP_DEBUG(log, "Handler defined");
    //Querible::registerQuery(SEND_GENERIC_DATA, "SGD", "Send generic data", "String max 200 char", QST_WO);
    //LOG4CPP_DEBUG(log, "Query exported");



    EXPORT_QUERY(LIST_KNOWEN_COMMANDS, &DeviceInCabin::qh_ListKnowenCommands, "LKC", "List knowed commands", "[Read only]", QST_RO);

    //-----[ InCabin ]
    EXPORT_QUERY(DeviceInCabin::SEND_GENERIC_DATA, &DeviceInCabin::qh_SendGenericData, "SGD", "Send generic data", "String max 200 char", QST_WO);
    EXPORT_QUERY(DeviceInCabin::SEND_CODED_EVENT, &DeviceInCabin::qh_SendCodedEvent, "SCE", "Send coded event", "Send a pre-defined event", QST_WO);
    EXPORT_QUERY(DeviceInCabin::IS_CONC_CON, &DeviceInCabin::qh_IsConcConn, "CCON", "Return '1' if 'Concentratore' is connected", "[Read only]", QST_RO);

    //-----[ GPRS ]
    EXPORT_QUERY(DeviceGPRS::GPRS_STATUS_UPDATE, &DeviceInCabin::qh_GPRS_LinkStatusUpdate, "LSU", "Network link status update", "Define first the link of interest", QST_RW);

    return OK;
}



//-----[ Query handlers ]-------------------------------------------------------

// Lisk knowed commands
exitCode DeviceInCabin::qh_ListKnowenCommands (t_query & p_query) {
    QueryRegistry * qR = QueryRegistry::getInstance();

    LOG4CPP_DEBUG(log, "DeviceInCabin::qh_ListKnowenCommands(type=%d)", p_query.type);

    switch ( p_query.type ) {

    case QM_QUERY:
        LOG4CPP_DEBUG(log, "QUERY[CCON]", p_query.value.c_str());
        RETURN_VALUE(p_query, "Availables AT Commands:%s", (qR->printRegistry()).c_str());
        break;
    case QM_VALUES:
        RETURN_VALUE(p_query, "Return the list of availables AT commands\r\nFormat: AT+%s\n\r", p_query.descr->name.c_str());
        break;
    case QM_SET:
        RETURN_VALUE(p_query, "Read is the only mode supported by this query\n\r");
        return HR_QUERYMODE_NOT_SUPPORTED;

    }

    return OK;

}

// Is 'Concentratore' Connected?
exitCode DeviceInCabin::qh_IsConcConn (t_query & p_query) {

    LOG4CPP_DEBUG(log, "QUERY[CCON]");
    RETURN_VALUE(p_query, "1");
    return OK;

}

// Send Generic Data
exitCode DeviceInCabin::qh_SendGenericData (t_query & p_query) {
    comsys::Command * cSgd;
    std::ostringstream buf("");

    switch ( p_query.type ) {

    case QM_QUERY:
        RETURN_VALUE(p_query, "Read mode not supported for this query\n\r");
        return HR_QUERYMODE_NOT_SUPPORTED;

    case QM_VALUES:
        RETURN_VALUE(p_query, "Send Generic Data\r\nFormat: AT+%s=<message to send>\n\r", p_query.descr->name.c_str());
        return OK;

    case QM_SET:
        LOG4CPP_DEBUG(log, "AT+%s=%s", p_query.descr->name.c_str(), p_query.value.c_str());

        cSgd = comsys::Command::getCommand(SEND_GENERIC_DATA, Device::DEVICE_IC, "DEVICE_IC", Utils::strFormat("IC_%s", p_query.descr->name.c_str()));
        if ( !cSgd ) {
            LOG4CPP_FATAL(log, "Unable to build a new Command");
            RETURN_VALUE(p_query, "OUT_OF_MEMORY");
            return OUT_OF_MEMORY;
        }

        buf.str("");
        buf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x09;
        cSgd->setParam( "dist_evtType", buf.str());
//         cSgd->setParam( "dist_evtType", 0x09 );
        cSgd->setParam( "dist_evtData", p_query.value );
        cSgd->setParam( "timestamp", d_time->time() );

        // Notifying command
        notifyCommand(cSgd);
    }

    return OK;

}

// Send Coded Event
exitCode DeviceInCabin::qh_SendCodedEvent (t_query & p_query) {
    comsys::Command * cSgd;
    std::string strToReturn("");
    std::ostringstream buf("");

    switch ( p_query.type ) {

    case QM_QUERY:
        RETURN_VALUE(p_query, "Read mode not supported for this query\n\r");
        return HR_QUERYMODE_NOT_SUPPORTED;

    case QM_VALUES:
        APPEND_STRING(strToReturn, "%s\r\nFormat: AT+%s=<event code>\n\r", p_query.descr->description.c_str(), p_query.descr->name.c_str());
        APPEND_STRING(strToReturn, " Availables code:\n\r");
        APPEND_STRING(strToReturn, " 01 - Traffico regolare\n\r");
        APPEND_STRING(strToReturn, " 02 - Traffico intenssconnessoo\n\r");
        APPEND_STRING(strToReturn, " 03 - Traffico bloccato\n\r");
        APPEND_STRING(strToReturn, " 04 - Fondo stradale asciutto\n\r");
        APPEND_STRING(strToReturn, " 05 - Fondo stradale bagnato\n\r");
        APPEND_STRING(strToReturn, " 06 - Fondo stradale ghiacciato\n\r");
        APPEND_STRING(strToReturn, " 07 - Fondo stradale innevato\n\r");
        APPEND_STRING(strToReturn, " 08 - Fondo stradale sconnesso\n\r");
        APPEND_STRING(strToReturn, " 09 - Tempo sereno\n\r");
        APPEND_STRING(strToReturn, " 0A - Pioggia lieve\n\r");
        APPEND_STRING(strToReturn, " 0B - Pioggia media\n\r");
        APPEND_STRING(strToReturn, " 0C - Pioggia intensa\n\r");
        APPEND_STRING(strToReturn, " 0D - Grandine\n\r");
        APPEND_STRING(strToReturn, " 0E - Nebbia lieve\n\r");
        APPEND_STRING(strToReturn, " 0F - Nebbia media\n\r");
        APPEND_STRING(strToReturn, " 10 - Nebbia intensa\n\r");
        APPEND_STRING(strToReturn, " 11 - Incidente\n\r");
        APPEND_STRING(strToReturn, " 12 - Lavori in corso\n\r");
        RETURN_VALUE(p_query, strToReturn.c_str());
        return OK;

    case QM_SET:
        LOG4CPP_DEBUG(log, "AT+%s=%s", p_query.descr->name.c_str(), p_query.value.c_str());

        cSgd = comsys::Command::getCommand(SEND_CODED_EVENT, Device::DEVICE_IC, "DEVICE_IC", Utils::strFormat("IC_%s", p_query.descr->name.c_str()));
        if ( !cSgd ) {
            LOG4CPP_FATAL(log, "Unable to build a new Command");
            RETURN_VALUE(p_query, "OUT_OF_MEMORY");
            return OUT_OF_MEMORY;
        }

        buf.str("");
        buf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x0A;
        cSgd->setParam( "dist_evtType", buf.str());
// 	cSgd->setParam( "dist_evtType", 0x0A );
        buf.str("");
        buf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << p_query.value;
        cSgd->setParam( "dist_evtData", buf.str() );
        cSgd->setParam( "timestamp", d_time->time() );

        // Notifying command
        notifyCommand(cSgd);

    }

    return OK;

}

// Link Status Update
exitCode DeviceInCabin::qh_GPRS_LinkStatusUpdate (t_query & p_query) {
// 	QueryRegistry * qR = QueryRegistry::getInstance();
	DeviceGPRS * gprs;
	unsigned short state;
	exitCode result;

	LOG4CPP_DEBUG(log, "DeviceGPRS::qh_LinkStatusUpdate(type=%d)", p_query.type);

	gprs = d_df->getDeviceGPRS();

	switch ( p_query.type ) {

	case QM_QUERY:
		LOG4CPP_DEBUG(log, "QUERY[%s]", p_query.value.c_str());
		RETURN_VALUE(p_query, "%s", DeviceGPRS::d_netStatusStr[gprs->status()]);
		break;
	case QM_VALUES:
		RETURN_VALUE(p_query, "Return the GPRS link state\r\n"
					"Format: AT+%s=<state>\n\r"
					"  <state>: 0=DOWN, 1=UP\n\r", p_query.descr->name.c_str());
		break;
	case QM_SET:
		LOG4CPP_DEBUG(log, "AT+%s=%s", p_query.descr->name.c_str(), p_query.value.c_str());
		//FIXME the linkname MUST be defined as a parameter
		sscanf(p_query.value.c_str(), "%1hu", &state);
		switch(state) {
		case 0:
			result = gprs->disconnect();
			if (result == OK) {
				LOG4CPP_DEBUG(log, "GPRS disconnect... OK");
			} else {
				LOG4CPP_DEBUG(log, "GPRS disconnect... FAILED [%d]", result);
			}
			break;
		case 1:
			result = gprs->connect("tinlink");
			if (result == OK) {
				LOG4CPP_DEBUG(log, "GPRS connect... OK");
			} else {
				LOG4CPP_DEBUG(log, "GPRS connect... FAILED [%d]", result);
			}
			break;
		default:
			LOG4CPP_ERROR(log, "Unknowed GPRS Link Status Update command [%d]", state);
		}
	return OK;

	}

	return OK;
}


} //namespace device
} //namespace controlbox
