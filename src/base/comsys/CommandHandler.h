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


#ifndef _COMMANDHANDLER_H
#define _COMMANDHANDLER_H


#include <controlbox/base/comsys/EventHandler.h>

namespace controlbox {
namespace comsys {

/// A command handler.
/// A CommandHandler is an Handler that could elaborate Command.
/// CommandHandlers should define the set of command types they are able to process.
/// The creation and inizialization of Command requesting services exported by it could be
/// requested to that same CommandHandler.
/// Once receiving a command, CommandHandler take care to check the Command is related to
/// a services exported by them, an if that's the case they provide to satisfy the requeste, eventually
/// by means of a CommandProcessor for complex tasks.
///
class CommandHandler : public EventHandler {

public:

    /// Create a new CommandHandler with an associated logName.
    /// The new CommandHandler, as default logger category, has
    /// the class namespace "controlbox.comlibs.ch"
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    CommandHandler(std::string const & logName = "ch");

    /// Class destructor
    ~CommandHandler();

};

} //namespace comsys
} //namespace controlbox
#endif
