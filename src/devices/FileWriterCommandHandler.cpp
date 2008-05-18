//***************************************************************************************
//*************  Copyright (C) 2006 - Patrick Bellasi ***********************************
//***************************************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** Patrick Bellasi. The programs may be used and/or copied only with the
//** written permission from the author or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//***************************************************************************************
//******************** Module information ***********************************************
//**
//** Project:       ControlBox (0.1)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Patrick Bellasi
//** Creation date:  21/06/2006
//**
//***************************************************************************************
//******************** Revision history *************************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------- --------------------
//**
//**
//***************************************************************************************


#include "FileWriterCommandHandler.ih"

namespace controlbox {
namespace device {


FileWriterCommandHandler::FileWriterCommandHandler(const std::string & fileName, std::string const & logName, bool append) :
        comsys::CommandHandler(logName),
        Device(Device::CH_FILEWRITER, fileName, logName),
        d_cnotify(0),
        log(Device::log) {

    LOG4CPP_DEBUG(log, "FileWriterCommandHandler(const std::string &, bool)");

    // Creating a simple plain layout for maximun output string control
    d_fwlayout = new log4cpp::PatternLayout();
    try {
        d_fwlayout->setConversionPattern(std::string("%m"));
    } catch (log4cpp::ConfigureFailure cf) {
        LOG4CPP_FATAL(log, "Invalid conversion pattern");
    }

    // Opening the file for Commands dumped
    d_fwappender = new log4cpp::FileAppender ("FileWriterXMLDumper", fileName, append);
    d_fwappender->setLayout (d_fwlayout);

    // Initializing a new Category for the FileWriter
    d_fwcategory = &log.getInstance("FileWriterXMLDumper");
    try {
        d_fwcategory->setPriority (log4cpp::Priority::INFO);
    } catch (std::invalid_argument) {
        LOG4CPP_ERROR(log, "Invalid priority level");
    }
    d_fwcategory->removeAllAppenders ();
    d_fwcategory->addAppender(*d_fwappender);

    // Registering device into the DeviceDB
    dbReg();

}

FileWriterCommandHandler::~FileWriterCommandHandler() {

    LOG4CPP_INFO(log, "Stopping FileWriterCommandHandler: no more Commands will by dumped");

    // Closing the logfile
    d_fwappender->close();


}


exitCode FileWriterCommandHandler::notify() {

    LOG4CPP_INFO(log, "\t --- Notify received [%d]", d_cnotify++ );
    return OK;

}


exitCode FileWriterCommandHandler::notify(comsys::Command * command)
throw (exceptions::IllegalCommandException) {
    std::string xmlCommand;

    LOG4CPP_DEBUG(log, "FileWriterCommandHandler::notify(Command * command)");

    command->xmlDump(xmlCommand);
    d_fwcategory->info("%s", xmlCommand.c_str());

    return OK;

}

} //namespace device
} //namespace controlbox
