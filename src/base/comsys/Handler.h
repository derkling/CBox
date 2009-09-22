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


#ifndef _HANDLER_H
#define _HANDLER_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/Command.h>

namespace controlbox {
namespace comsys {

/// A message Handler.
/// An Handler in an entity that could menage messages coming from dispatchers.
class Handler {

public:

    /// Handler destructor
    /// This method expoit polimorphism to call the correctly run-time
    /// Handler's derived class type destructor.
    virtual ~Handler() {};

    /// The default notify routine
    virtual exitCode notifyEvent() = 0;

    /// Command notify routine
    /// Process the specified command if related to an exported service.
    /// Exported services are defined by the services enum that derived
    /// concrete class MUST define.
    /// Concrete implementations of this method shuld use the checkService
    /// method to verify if the class is able to manage this kind of
    /// Commands
    /// @param command the Command to process
    /// @return OK on success Command's processing
    /// @throw exceptions::IllegalCommandException if the concrete class
    ///		is not eligible for handling the Command required
    /// @see checkService
    virtual exitCode notifyCommand(Command * command)
    throw(exceptions::IllegalCommandException) = 0;

};

} //namespace comsys
} //namespace controlbox
#endif
