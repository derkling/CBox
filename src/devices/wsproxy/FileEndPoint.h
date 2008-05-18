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


#ifndef _FILEENDPOINT_H
#define _FILEENDPOINT_H

#include "EndPoint.h"

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/PatternLayout.hh>

#define DEFAULT_FILEENDPOINT_FILENAME	"./wsuploads.log"
#define DEFAULT_FILEENDPOINT_LAYOUT	"%d{%Y-%m-%d %H:%M:%S,%l} - %m%n"
#define DEFAULT_FILEENDPOINT_APPEND	"yes"

namespace controlbox {
namespace device {

/// Class defining an File EndPoint.
/// An EndPoint encapsulate a WebService Porxie, providing
/// the methods needed to upload a message to the associated WebService.<br>
/// @note This class provide a simple way to log webservice uploaded messages
///	in a local logfile in order to implement a backup facility<br>
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>fileEndPoint_category</b> - <i>Default: DEFAULT_FILEENDPOINT_CATEGORY</i><br>
///		The catogery<br>
///		Size: [size]
///	</li>
/// </ul>
/// @see EndPoint
class FileEndPoint : public EndPoint {

protected:

    /// File Logger.
    /// The category referenced is named by the fileEndPoint_category and
    /// configured by the used log4cpp configuration file.
    log4cpp::Category * d_fepCategory;

    /// The FileAppender to witch dump Commands
    log4cpp::FileAppender * d_fepAppender;

    /// The dump Layout: define the layout of each command line
    log4cpp::PatternLayout * d_fepLayout;

//     log4cpp::Category * file;

public:
    /// @param paramBase the prefix for this EndPoint confiugration params lables
    /// @param logName the base logname to witch will be appended
    ///		the endPoint identifier 'FileEndPoint'
    FileEndPoint(std::string const & paramBase, std::string const & logName);

    ~FileEndPoint();

    exitCode upload(unsigned int & epEnabledQueues, std::string const & msg, EndPoint::t_epRespList &respList);

};

}// namespace device
}// namespace controlbox
#endif
