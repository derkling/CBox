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
        d_command(0) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::CommandDispatcher(Handler * handler, bool suspended)");

}


CommandDispatcher::~CommandDispatcher() {

	LOG4CPP_DEBUG(log, "~CommandDispatcher()");

	// ATTENZIONE: impedire l'accodamento durante lo shutdown...
	d_suspended = true;
	flushQueue(true);
	if (d_command) {
		delete d_command;
	}

}

exitCode CommandDispatcher::setDefaultCommand(Command * command) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::setDefaultCommand(Command * command)");

    if ( d_command != command ) {
        LOG4CPP_WARN(log, "Associated default command updated");
        LOG4CPP_DEBUG(log, "Previous command overwritten but NOT released: ensure to release any provious command's memory!");
        d_command = command;
    }

    return OK;

}

exitCode CommandDispatcher::resume(bool discard) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::resume(bool discard=%d)", discard);
    d_suspended = false;

    return flushQueue();

    // ATTENZIONE a sospensione mentre in resume...
    // si perde il contatore dei messaggi in coda...
    // ... magari ci vuole in MUTEX!?

}


exitCode CommandDispatcher::suspend() {

    LOG4CPP_DEBUG(log, "CommandDispatcher::suspend()");
    d_suspended = true;

    return OK;
}


exitCode CommandDispatcher::dispatchCommand(bool clean)
throw (exceptions::InitializationException,
       exceptions::OutOfMemoryException) {

	LOG4CPP_DEBUG(log, "CommandDispatcher::dispatch()");

	if ( !d_command ) {
		LOG4CPP_ERROR(log, "Unable to dispatch Default command: NOT yet defined!");
		return CS_DISPATCH_FAILURE;
	}

	LOG4CPP_INFO(log, "Default command disaptching");
	dispatchCommand(d_command, clean);

	return OK;
}


exitCode CommandDispatcher::dispatchCommand(Command * command, bool clean)
throw (exceptions::OutOfMemoryException) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::dispatch(Command * command)");

    // If disabled...
    if ( d_suspended ) {
        // Queuing command for handler notify
        LOG4CPP_INFO(log, "Disaptcher suspended; queuing new Command for delayed dispatching");
        queueCommand(command);
        return DIS_SUSPENDED;
    }

    // Dispatching command immediatly
    d_handler->notifyCommand(command);
    if ( clean ) {
    	delete command;
    }

    return OK;

}


exitCode CommandDispatcher::queueCommand(Command * command)
throw (exceptions::OutOfMemoryException) {

    LOG4CPP_DEBUG(log, "CommandDispatcher::queueCommand(Command * command)");

    // Queuing command for handler notify
    d_queuedCommands.push(command);

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

		while ( !d_queuedCommands.empty() && !d_suspended ) {
			d_queuedCommands.pop();

		}

	} else {

		LOG4CPP_INFO(log, "Flushing Command Queue: Notifying all commands");

		while ( !d_queuedCommands.empty() && !d_suspended ) {
			d_handler->notifyCommand( d_queuedCommands.front() );
			d_queuedCommands.pop();
		}
	}

	return OK;

} //namespace comsys
} //namespace controlbox


}
