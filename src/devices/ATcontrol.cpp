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

#include "ATcontrol.ih"


namespace controlbox {
namespace device {

ATcontrol * ATcontrol::d_instance = 0;


ATcontrol::ATcontrol(std::string const & logName) :
        DeviceInCabin(logName, DeviceInCabin::IC_ATCONTROL),
	Worker(Device::log, "cbw_AT", 0),
        d_configurator(Configurator::getInstance()),
	log(Device::log) {

    initATcontrol();

    ATcontrol::exportQuery();

}

ATcontrol * ATcontrol::getInstance() {

    if ( !d_instance ) {
        d_instance = new ATcontrol();
    }

    LOG4CPP_DEBUG(d_instance->log, "ATcontrol::getInstance()");

    return d_instance;
}

ATcontrol::~ATcontrol() {

    LOG4CPP_DEBUG(log, "ATcontrol::~ATcontrol()");

}


inline
void ATcontrol::initATcontrol() {

    LOG4CPP_DEBUG(log, "ATcontrol::initATcontrol()");

    d_controlAt = d_configurator.param("control_AT_tty_port", ATCONTROL_DEFAULT_DEVICE);
    d_delimiter = d_configurator.param("control_AT_delimiter", ATCONTROL_DEFAULT_DELIMITER).c_str()[0];
    d_doEcho = !d_configurator.testParam("control_AT_tty_doEcho", ATCONTROL_DEFAULT_ECHOMODE);
    d_sendExitCode = !d_configurator.testParam("control_AT_tty_sendExitCode", ATCONTROL_DEFAULT_EXITCODE);
    d_isParsing = false;

}

inline
exitCode ATcontrol::doConfigure(void) {

    LOG4CPP_DEBUG(log,"ATcontrol::doConfigure()");

    // TODO Read from configuration file the tty port params
    //	and accordingly configure the port.
    // IF error => AT_CONFIGURATION_FAILURE


    // Trying to open the device
    d_tty.open(d_controlAt.c_str());
    if ( !d_tty ) {
        LOG4CPP_WARN(log, "Unable to open device [%s]", d_controlAt.c_str());
        return ATCONTROL_TTY_OPEN_FAILURE;
    }


    LOG4CPP_INFO(log, "TTY device opened for AT command parsing");

    return OK;

}

inline
exitCode ATcontrol::exportQuery() {

    LOG4CPP_DEBUG(log, "ATcontrol::exportQuery()");

    //--- "Invio dato generico da palmare"
    //EXPORT_QUERY(SEND_GENERIC_DATA, &DeviceInCabin::qh_SendGenericData, "SGD", "Send generic data", "String max 200 char", QST_WO);

    return OK;
}


exitCode ATcontrol::query(t_query & p_query) {
    exitCode queryResult;

    LOG4CPP_DEBUG(log, "Received new query id:[%d] name:[%s], type:[%d], value:[%s]",
                  p_query.descr->id, p_query.descr->name.c_str(), p_query.type, p_query.value.c_str());

    // Query dispatching and error
    queryResult = d_hR->call(p_query.descr->id, p_query);
    if ( queryResult != OK ) {
        LOG4CPP_WARN(log, "Query [%s] FAILED: %s", p_query.descr->name.c_str(), p_query.value.c_str());
        return queryResult;
    }

    // Sending responce only if required
    if ( p_query.responce ) {
        LOG4CPP_DEBUG(log, "Query [%s] responce: %s%c", p_query.descr->name.c_str(), p_query.value.c_str(), d_delimiter);
        d_tty << p_query.value.c_str() << d_delimiter << flush;
    }

    return OK;

}


void ATcontrol::doParse (void) {
    std::string sentence;
//     int line = 0;
    Querible * querible;
    Querible::t_query theQuery;
    exitCode queryResult;

    // Buffer for sentence parsing
//	char sentenceToParse[ATCONTROL_SENTENCE_MAXLENGTH+1];
//	short sentenceLength;


    LOG4CPP_DEBUG(log,"ATcontrol::doParse (), using delimiter [%d]", d_delimiter);

    while ( ! d_tty.eof() ) {
        getline(d_tty, sentence, d_delimiter);

        /*
        		sentenceLength = sentence.length();
        		if ( ! sentenceLength ) {
        			continue;
        		}

        		if ( sentenceLength > ATCONTROL_SENTENCE_MAXLENGTH ) {
        			LOG4CPP_WARN(log, "AT command out-of-range: the sentence is more than %d chars length", ATCONTROL_SENTENCE_MAXLENGTH);
        			sentenceLength = ATCONTROL_SENTENCE_MAXLENGTH;
        		}
        */

        LOG4CPP_DEBUG(log, "Received new AT Command [%s]", sentence.c_str());

        // Parsing command
        querible = parseQuery(sentence, theQuery);
        if ( !querible ) {
            LOG4CPP_DEBUG(log, "Invalid AT command received");
            d_tty << "Not supported AT Command" << d_delimiter << flush;
            queryResult = GENERIC_ERROR;
        } else {
            queryResult = querible->query(theQuery);
        }

        if ( d_sendExitCode ) {
            LOG4CPP_DEBUG(log, "Sending exitCode... ");
            if ( queryResult == OK ) {
                d_tty << "OK" << d_delimiter << flush;
            } else {
                d_tty << "ERROR" << d_delimiter << flush;
            }
        }

        // wait a while before parse the next sentence
        sleep(ATCONTROL_SENTENCE_DELAY);
    }

}

Querible * ATcontrol::parseQuery (std::string const & atCommand, Querible::t_query & theQuery) {
    unsigned short commandEndIndex;
    const char * atStr = atCommand.c_str();
    char queryName[ATCONTROL_COMMAND_MAXLENGTH];
    QueryRegistry * qR;
    Querible * querible;

    LOG4CPP_DEBUG(log, "ATcontrol::parseQuery (atCommand=%s)", atCommand.c_str());

    if ( ! ( !strncmp("AT+", atStr, 3) ||
             !strncmp("at+", atStr, 3) ) ) {
        LOG4CPP_WARN(log, "Received invalid AT command [%s]", atStr);
        return 0;
    }

    qR = QueryRegistry::getInstance();

    // Looking for AT command name delimiter:
    commandEndIndex = 3;
    theQuery.type = QM_QUERY;
    while ( commandEndIndex < atCommand.size() ) {
        if ( isspace(atStr[commandEndIndex]) ) {
            // - Simple command [AT+<CMD>]
            break;
        }
        if ( !strncmp(atStr+commandEndIndex, "?", 1) ) {
            // - Query value command [AT+<CMD>?]
            theQuery.type = QM_VALUES;
            break;
        }
        if ( !strncmp(atStr+commandEndIndex, "=", 1) ) {
            // - Set value command [AT+<CMD>=<value>]
            theQuery.type = QM_SET;
            theQuery.value = string(atStr+commandEndIndex+1);
            break;
        }
        commandEndIndex++;
    }

    commandEndIndex = ((commandEndIndex-3) < ATCONTROL_COMMAND_MAXLENGTH-1) ? commandEndIndex-3 : ATCONTROL_COMMAND_MAXLENGTH-1;

    // now 'commandEndIndex' point to the end of the command:
    // we could look for the querible associated
    strncpy(queryName, atStr+3, commandEndIndex);
    queryName[commandEndIndex] = 0;
    LOG4CPP_DEBUG(log, "Command name: [%s]", queryName);

    querible = qR->getQuerible(queryName);

    if ( !querible ) {
        LOG4CPP_WARN(log, "AT command [%s] not supported", queryName);
        return 0;
    }

    theQuery.descr = qR->getQueryDescriptor(queryName, querible);
    theQuery.responce = false;

    return querible;


}


void ATcontrol::run(void)  {

    // NOTE: Handle the case of EOF from input...
    // on that cases shuld be correct to destroy restart the parsing
    // so that the input channel will be reopened...
    // TODO TODO TODO

    // Configuring the Parser
    if ( (! d_isParsing ) && (doConfigure() == OK) ) {
        d_isParsing = true;
        // Starting to parse AT commands
        doParse();

    } else {
        LOG4CPP_ERROR(log, "Configuration failure! Sentences parsing rutine NOT started");
    }

    d_isParsing = false;

}

}// namespace device
}// namespace controlbox
