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

#ifndef _DEVICETIME_H
#define _DEVICETIME_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/CommandGenerator.h>
#include <controlbox/base/comsys/Dispatcher.h>
#include <controlbox/devices/DeviceGPS.h>
#include <controlbox/devices/gprs/DeviceGPRS.h>

namespace controlbox {
namespace device {

/// A DeviceTime.
class DeviceTime : public comsys::CommandGenerator, public Device {

public:


protected:

    static DeviceTime * d_instance;

    /// The logger to use locally.
    log4cpp::Category & log;

    /// The DeviceGPS
    DeviceGPS * d_devGPS;

    /// The DeviceGPRS
    DeviceGPRS * d_devGPRS;

public:

    /// Get a parser instance.
    /// @param logName the log category
    static DeviceTime * getInstance(std::string const & logName = "DeviceTime");

    /// Get current Time in ISO 8601 format.
    /// Complete date plus hours, minutes, seconds and a decimal fraction of a
    /// second:<br>
    /// <center>YYYY-MM-DDThh:mm:ssTZD <i>(eg 1997-07-16T19:20:30+01:00)</i></center><br>
    /// where:<br>
    /// <ul>
    ///      <li>YYYY = four-digit year</li>
    ///      <li>MM   = two-digit month (01=January, etc.)</li>
    ///      <li>DD   = two-digit day of month (01 through 31)</li>
    ///      <li>hh   = two digits of hour (00 through 23) (am/pm NOT allowed)</li>
    ///      <li>mm   = two digits of minute (00 through 59)</li>
    ///      <li>ss   = two digits of second (00 through 59)</li>
    ///      <li>TZD  = time zone designator (Z or +hh:mm or -hh:mm)</li>
    /// </ul>
    /// Times are expressed in UTC (Coordinated Universal Time),
    /// with a special UTC designator ("Z").
    /// @param UTC set true to use UTC time zone designator
    /// @note the returned string is a fixed length array of
    ///	20 or 25 char, depending on using utc or not
    ///	(by default: no utc, 25 char)
    string time(bool utc = false) const;


protected:


    /// Create a new DeviceTime.
    /// In order to get a valid instance of that class
    /// the builder method shuld be used.
    /// @param logName the log category.
    /// @see getInstance
    DeviceTime(std::string const & logName);

    /// Parsing thread body.
    /// This method provide to configure the parser and actually start it.
    /// @see doConfigure - for parsing configuration
    /// @see doParse - for parsing routine
    void run(void);

};

} //namespace device
} //namespace controlbox
#endif
