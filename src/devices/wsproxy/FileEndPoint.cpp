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

#include "FileEndPoint.ih"


namespace controlbox {
namespace device {

FileEndPoint::FileEndPoint(std::string const & paramBase, std::string const & logName) :
        EndPoint(WS_EP_FILE, EPTYPE_MMC, paramBase, logName+".FileEndPoint"),
        d_fepCategory(0) {
	std::ostringstream lable("");
	std::string layout;
	bool append;

	lable.str("");
	lable << paramBase.c_str() << "_layout";
	layout = d_configurator.param(lable.str().c_str(), DEFAULT_FILEENDPOINT_LAYOUT);

	lable.str("");
	lable << paramBase.c_str() << "_filename";
	d_filename = d_configurator.param(lable.str().c_str(), DEFAULT_FILEENDPOINT_FILENAME);

	lable.str("");
	lable << paramBase.c_str() << "_append";
	append = d_configurator.testParam(lable.str().c_str(), DEFAULT_FILEENDPOINT_APPEND);

	// Creating a simple plain layout for maximun output string control
	d_fepLayout = new log4cpp::PatternLayout();
	try {
		d_fepLayout->setConversionPattern(layout);
	} catch (log4cpp::ConfigureFailure cf) {
		LOG4CPP_FATAL(log, "Invalid conversion pattern");
	}

	// Opening the file for Commands dumped
	d_fepAppender = new log4cpp::FileAppender ("FileWriterXMLDumper", d_filename, append);
	d_fepAppender->setLayout (d_fepLayout);

	// Initializing a new Category for the FileWriter
	d_fepCategory = &log.getInstance("FileEndPoint");
	try {
		d_fepCategory->setPriority (log4cpp::Priority::INFO);
		// Avoiding msg duplication on root category
		d_fepCategory->setAdditivity(false);
	} catch (std::invalid_argument) {
		LOG4CPP_ERROR(log, "Invalid priority level");
	}
	d_fepCategory->removeAllAppenders();
	d_fepCategory->addAppender(*d_fepAppender);

}

FileEndPoint::~FileEndPoint() {

    LOG4CPP_DEBUG(log, "Stopping FileEndPoint: no more messages will by dumped");
    // Closing the logfile
    d_fepAppender->close();

//     if (d_fepAppender)
//     	(*d_fepAppender).shutdown();

}

exitCode FileEndPoint::upload(unsigned int & epEnabledQueues, std::string const & msg, EndPoint::t_epRespList &respList) {
	unsigned int l_isEnabled = 0x0;

	// Checking if this File EndPoint queue is enabled
	l_isEnabled = epEnabledQueues && d_epQueueMask;
	if ( l_isEnabled == 0x0 ) {
		LOG4CPP_DEBUG(log, "    [%c(%hu) - %s] is DISABLED",
			getQueueLable(d_epQueueMask),
			d_failures, d_name.c_str() );
	}


	if (d_fepCategory) {
		LOG4CPP_DEBUG(log, "Dumping message to journal [%s]", msg.c_str());
// 		LOG4CPP_INFO((*d_fepCategory), "%s", msg.c_str());
		d_fepCategory->log(::log4cpp::Priority::INFO, "%s", msg.c_str());
	}

	LOG4CPP_INFO(log, "    [%c(%hu) - %s]",
		getQueueLable(d_epQueueMask),
		d_failures, d_name.c_str() );

	// Resetting this File EndPoint queue
	epEnabledQueues ^= d_epQueueMask;

	return OK;

}

}// namespace device
}// namespace controlbox
