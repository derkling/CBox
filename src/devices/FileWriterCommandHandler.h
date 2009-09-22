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
//** -------- ---------- ---------------------------------------------------- -----------------------------
//**
//**
//***************************************************************************************


#ifndef _FILEWRITERCOMMANDHANDLER_H
#define _FILEWRITERCOMMANDHANDLER_H


#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/CommandHandler.h>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <string>


/// @todo Features and extensions:
/// <ul>
///	<li> customize the dump string format (passing a custom Layout)
/// </ul>

namespace controlbox {
namespace device {

/// A FileWriterCommandHandler is a simple CommandHandler that save the Commands
/// it receive into a file.
/// @see CommandHandler
class FileWriterCommandHandler : public comsys::CommandHandler, public Device  {

protected:

    /// The Category to use for Commands dump
    log4cpp::Category * d_fwcategory;

    /// The FileAppender to witch dump Commands
    log4cpp::FileAppender * d_fwappender;

    /// The dump Layout: define the layout of each command line
    log4cpp::PatternLayout * d_fwlayout;

    /// The number of notify received
    unsigned int d_cnotify;

    /// The logger to use locally.
    log4cpp::Category & log;


public:

    /// Create a new FileWriterCommandHandler.
    /// @param fileName the name of the file to which the Appender has to log
    /// @param append   whether to truncate the file or just append to it if it
    ///		already exists. Defaults to 'true'.
    FileWriterCommandHandler(const std::string & fileName,
                             std::string const & logName = "XMLFileWriter",
                             bool append = true);

    /// Class destructor
    ~FileWriterCommandHandler();

    /// The default notify routine
    /// The implementation in that class does nothing, just generate
    /// log if level is <= DEBUG
    exitCode notifyEvent();

    /// Command notify routine
    /// The specified command's content will be dumped into the
    /// associated file.
    /// @param command the Command to process
    /// @return OK on success Command's dumping
    /// @throw exceptions::IllegalCommandException never throwed by this
    ///			implementation.
    exitCode notifyCommand(comsys::Command * command)
    throw (exceptions::IllegalCommandException);


};

} //namespace device
} //namespace controlbox
#endif
