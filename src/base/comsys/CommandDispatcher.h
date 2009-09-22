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


#ifndef _COMMANDDISPATCHER_H
#define _COMMANDDISPATCHER_H

#include <controlbox/base/comsys/EventDispatcher.h>
#include <controlbox/base/comsys/Command.h>
#include <controlbox/base/Utility.h>
#include <queue>


namespace controlbox {
namespace comsys {

/// A command dispatcher.
/// A CommandDispatcher is a Dispatcher that could Dispatch Command to a CommandHandler.
/// Each CommandDispatcher could have a default Command to dispatch or otherwiese could dispatch
/// Command provided by a Generator.
class CommandDispatcher : public EventDispatcher {

protected:

    /// The default command to send
    Command * d_command;

    /// The queue of commands waiting to be dispatched while in suspended state
    queue<Command *> d_queuedCommands;

public:

    /// Build a new suspended CommandDispatcher.
    /// The newly created CommandDispatcher is set in suspended state till it is
    /// attacched to an handler.
    CommandDispatcher(Handler * handler = 0, bool suspended = true);

    ///
    ~CommandDispatcher();

    /// Set the default Command to dispatch.
    /// The specified comand will the send to the associated (command) handler,
    /// each time disaptch() is called.
    /// @param command the default command to dispatch on notifyCommand() calls
    /// @note any proviously defined default command reference will be lost,
    ///		overwritten by that one, but the previous instance will NOT
    ///		be cleaned by that class: the client must provide to release
    ///		the memory associated with the previous command.
    exitCode setDefaultCommand(Command * command);

    /// Enable commands dispatching
    /// Enable the dispatching of new commands and, by default,
    /// dispatche all queued commands. Otherwise queued commands are flushed.
    /// @param discard set to true to discard queued commands while in suspended state (default flase)
    /// @return OK on success, Core::GEN_NO_HANDLER if there's not handlers defined
    exitCode resume(bool discard = false);

    /// Suspend events notifications
    /// New events are not dispetched but still counted for, eventually,
    /// delayed notification.
    /// @return OK
    exitCode suspend();

    /// Disaptch the default command to Handler.
    /// If not suspended, dispatch the default commands to the associated Handler.
    /// Otherwise the command is queued for future dispatching.
    /// @param clean when true the command is cleaned by this dispatcher before retunring
    /// @throw exceptions::InitializationException if ther's not a default command to disaptch
    /// @throw exceptions::OutOfMemoryException while in suspended state, if ther's not
    ///		enought memory to queue the command for delayed dispatching
    exitCode dispatchCommand(bool clean = true)
    throw (exceptions::InitializationException,
           exceptions::OutOfMemoryException);

    /// Dispatch a command to Handler.
    /// If not suspended, dispatch the default command (or the specified one) to the associated Handler.
    /// Otherwise the command is queued for future dispatching.
    /// The default command, if needed, must be built by the subclasses's constructors.
    /// @param the Command to dispatch, if NULL the default command will be dispatched (default NULL)
    /// @param clean when true the command is cleaned by this dispatcher before retunring
    /// @return helpers:OK on success, Core::DIS_SUSPENDED if the Command has been queued.
    /// @throw exceptions::OutOfMemoryException while in suspended state, if ther's not
    ///		enought memory to queue the command for delayed dispatching
    /// @note commands queued while the disaptcher is suspended are alwais cleaned
    ///		once have been notified after resume.
    exitCode dispatchCommand(Command * command, bool clean = true)
    throw (exceptions::OutOfMemoryException);

protected:

    /// Queue new commands.
    /// Queue the defined command for delayed dispatching
    /// @param command the command to queue
    /// @return OK on success
    /// @throw exceptions::OutOfMemoryException if ther's not enought memory to queue the command
    exitCode queueCommand(Command * command)
    throw (exceptions::OutOfMemoryException);


    /// Flush queude commands.
    /// Disaptch all Commands queued while in suspended state.
    /// @param discard set TRUE when queued messages shuld be discarded instead than notified
    /// @return OK on success
    exitCode flushQueue(bool discard = false);


};


} //namespace comsys
} //namespace controlbox
#endif
