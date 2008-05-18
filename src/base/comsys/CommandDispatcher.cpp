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


#include "CommandDispatcher.ih"

namespace controlbox {
namespace comsys {


CommandDispatcher::CommandDispatcher(Handler * handler, bool suspended):
        EventDispatcher(handler, suspended, "cd"),
        command(0) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::CommandDispatcher(Handler * handler, bool suspended)");

}


CommandDispatcher::~CommandDispatcher() {

    LOG4CPP_DEBUG(log, "~CommandDispatcher()");

    // ATTENZIONE: impedire l'accodamento durante lo shutdown...
    suspended = true;
    flushQueue(true);
    if (command)
        delete command;

}

exitCode CommandDispatcher::setDefaultCommand(Command * command) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::setDefaultCommand(Command * command)");

    if ( command != this->command ) {
        LOG4CPP_WARN(log, "Associated default command updated");
        LOG4CPP_DEBUG(log, "Previous command overwritten but NOT released: ensure to release any provious command's memory!");
        this->command = command;
    }

    return OK;

}

exitCode CommandDispatcher::resume(bool discard) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::resume(bool discard=%d)", discard);
    suspended = false;

    return flushQueue();

    // ATTENZIONE a sospensione mentre in resume...
    // si perde il contatore dei messaggi in coda...
    // ... magari ci vuole in MUTEX!?

}


exitCode CommandDispatcher::suspend() {

    LOG4CPP_DEBUG(log, "CommandDispatcher::suspend()");
    suspended = true;

    return OK;
}


exitCode CommandDispatcher::dispatch()
throw (exceptions::InitializationException,
       exceptions::OutOfMemoryException) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::dispatch()");

    if ( !command ) {
        LOG4CPP_ERROR(log, "Unable to dispatch Default command: NOT yet defined!");
        throw new exceptions::InitializationException("Default command not defined");
    }

    LOG4CPP_INFO(log, "Default command disaptching");
    return dispatch(command);

}


exitCode CommandDispatcher::dispatch(Command * command)
throw (exceptions::OutOfMemoryException) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::dispatch(Command * command)");

    // If disabled...
    if ( suspended ) {
        // Queuing command for handler notify
        LOG4CPP_INFO(log, "Disaptcher suspended; queuing new Command for delayed dispatching");
        queueCommand(command);
        return DIS_SUSPENDED;
    }

    // Dispatching command immediatly
    return handler->notify(command);

}


exitCode CommandDispatcher::queueCommand(Command * command)
throw (exceptions::OutOfMemoryException) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::queueCommand(Command * command)");

    // Queuing command for handler notify
    queuedCommands.push(command);

    // ATTENZIONE: come ci si accorge di problemi di
    // allocazione, tipo memoria esaurita?!?
    // In caso di problemi di memoria, lanciare una
    // exceptions::OutOfMemoryException

    return OK;
}

exitCode CommandDispatcher::flushQueue(bool discard) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::flushQueue(bool discard=%d)", discard);


    if ( discard ) {

        LOG4CPP_WARN(log, "Flushing Command Queue: Discarding all commands");

        while ( !suspended && !queuedCommands.empty() ) {
            queuedCommands.pop(); // needed to effectively REMOVE the element from the queue
        }

    } else {

        LOG4CPP_INFO(log, "Flushing Command Queue: Notifying all commands");

        while ( !suspended && !queuedCommands.empty() ) {
            handler->notify( (Command *)queuedCommands.front() );
            queuedCommands.pop(); // needed to effectively REMOVE the element from the queue
        }

    }

    return OK;

} //namespace comsys
} //namespace controlbox


}
