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


#ifndef _OBJECT_H
#define _OBJECT_H

#include <controlbox/base/Utility.h>

namespace controlbox {

/// The system base class.
/// This class is meaned to provide, to all other system objects, a set of basic
/// features as:
/// <ul>
///	<li> logging</li>
///	<li> reference counting</li>
/// </ul>
class Object {



//-----[ Types ]----------------------------------------------------------------

public:


protected:


//-----[ Members ]--------------------------------------------------------------

protected:
    /// Logger
    /// Use this logger reference, related to the 'log' category, to log your messages
    log4cpp::Category & log;

#if CONTROLBOX_DEBUG > 1
    /// Logger
    /// Logger reference used for call tracing
    log4cpp::Category & ct;
#endif

//-----[ Methods ]--------------------------------------------------------------

protected:

    /// Create a new Object.
    /// The new object is associated to a logger category
    /// whose name is prefixed by "controlbox."
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox."
    Object(std::string const & logName = "Object");

    /// Class destructor
    ~Object() {};

    /// Check for configuration params.
    /// This method should be used in Object contructors in order to
    /// check for required configuration params and (eventually) preload them
    /// @return it must return OK
    virtual exitCode preloadParams();


};

}
#endif
