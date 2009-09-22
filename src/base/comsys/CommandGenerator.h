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


#ifndef _COMMANDGENERATOR_H
#define _COMMANDGENERATOR_H



#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/EventGenerator.h>


namespace controlbox {
namespace comsys {


/// A CommandGenerator is a more elaborate Generator that could send custom Commands to a Dispatcher
/// and wait for elaboration results.
class CommandGenerator : public EventGenerator {


public:

    /// Create a new CommandGenerator initially disabled.
    /// In order to enable it you need to attach a Dispatcher.
    /// The new CommandGenerator, as default logger category, has
    /// the class namespace "controlbox.comlibs.cg"
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    CommandGenerator(std::string const & logName = "CommandGenerator");

    /// Create a new CommandGenerator associated to the specified Dispatcher
    /// The new CommandGenerator, as default logger category, has
    /// the class namespace "controlbox.comlibs.cg"
    /// @param dispatcher the dispatcher to with send events notifications
    /// @param enabled set false if the generator has to be initially disabled (default true)
    /// @param logName the log category, this name is prepended by the
    ///		class namespace "controlbox.comlibs."
    CommandGenerator(Dispatcher * dispatcher, bool enabled = true,  std::string const & logName = "CommandGenerator");

    /// Class destructor
    ~CommandGenerator() {};


    exitCode notifyCommand(Command * command, bool clean = true);

};


} //namespace comsys
} //namespace controlbox
#endif
