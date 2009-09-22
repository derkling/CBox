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





#include "EventDispatcher.ih"

namespace controlbox {
namespace comsys {


EventDispatcher::EventDispatcher(Handler * handler, bool suspended, std::string const & logName) :
        Object("comlibs."+logName),
        Dispatcher(),
        d_suspended(suspended),
        d_waiting(0),
        d_handler(handler) {

    LOG4CPP_DEBUG(log, "EventDispatcher::EventDispatcher(Handler * handler, bool suspended, std::string const & logName)");

}

EventDispatcher::~EventDispatcher() {

    LOG4CPP_DEBUG(log, "~EventDispatcher()");

}


exitCode EventDispatcher::setHandler(Handler * handler, bool suspended) {

    LOG4CPP_DEBUG(log, "EventDispatcher::setHandler(Handler * handler, bool suspended)");

    d_handler = handler;
    d_suspended = suspended;

    return OK;

}


exitCode EventDispatcher::resume(bool discard) {

    LOG4CPP_DEBUG(log, "EventDispatcher::resume(bool discard)");

    d_suspended = false;

    if ( !discard ) {

        LOG4CPP_INFO(log, "Disaptching [%d] queued events", d_waiting);
        while ( d_waiting-- && !d_suspended ) {
            d_handler->notifyEvent();
        }

    } else {
        LOG4CPP_WARN(log, "Flushing [%d] queued events", d_waiting);
    }

    // ATTENZIONE a sospensione mentre in resume...
    // si perde il contatore dei messaggi in coda...
    // ... magari ci vuole in MUTEX!?

    d_waiting = 0;

    if ( d_suspended ) {
        LOG4CPP_INFO(log, "Suspending just resumed Dispatcher");
        return DIS_SUSPENDED;
    }

    return OK;

}


exitCode EventDispatcher::suspend() {

    LOG4CPP_DEBUG(log, "EventDispatcher::suspend()");
    d_suspended = true;
    LOG4CPP_INFO(log, "EventDispatcher suspended");

    return OK;

}


exitCode EventDispatcher::dispatchEvent(bool clean) {

    LOG4CPP_DEBUG(log, "EventDispatcher::dispatch()");

    if ( !d_suspended ) {
        LOG4CPP_INFO(log, "Disaptching new event");
        d_handler->notifyEvent();
        return OK;
    } else {
        d_waiting++;
        LOG4CPP_INFO(log, "Disaptcher suspended; [%d] events queued for delayed dispatching", d_waiting);
        return DIS_SUSPENDED;
    }

}


} //namespace comsys
} //namespace controlbox
