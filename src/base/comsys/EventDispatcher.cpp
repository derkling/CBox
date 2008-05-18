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
        suspended(suspended),
        waiting(0),
        handler(handler) {

    LOG4CPP_DEBUG(log, "EventDispatcher::EventDispatcher(Handler * handler, bool suspended, std::string const & logName)");

}

EventDispatcher::~EventDispatcher() {

    LOG4CPP_DEBUG(log, "~EventDispatcher()");

}


exitCode EventDispatcher::setHandler(Handler * handler, bool suspended) {

    LOG4CPP_DEBUG(log, "EventDispatcher::setHandler(Handler * handler, bool suspended)");

    this->handler = handler;
    this->suspended = suspended;
}


exitCode EventDispatcher::resume(bool discard) {

    LOG4CPP_DEBUG(log, "EventDispatcher::resume(bool discard)");

    suspended = false;

    if ( !discard ) {

        LOG4CPP_INFO(log, "Disaptching [%d] queued events", waiting);
        while ( waiting-- && !suspended ) {
            handler->notify();
        }

    } else {
        LOG4CPP_WARN(log, "Flushing [%d] queued events", waiting);
    }

    // ATTENZIONE a sospensione mentre in resume...
    // si perde il contatore dei messaggi in coda...
    // ... magari ci vuole in MUTEX!?

    waiting = 0;

    if ( suspended ) {
        LOG4CPP_INFO(log, "Suspending just resumed Dispatcher");
        return DIS_SUSPENDED;
    }

    return OK;

}


exitCode EventDispatcher::suspend() {

    LOG4CPP_DEBUG(log, "EventDispatcher::suspend()");
    suspended = true;
    LOG4CPP_INFO(log, "EventDispatcher suspended");

}


exitCode EventDispatcher::dispatch() {

    LOG4CPP_DEBUG(log, "EventDispatcher::dispatch()");

    if ( !suspended ) {
        LOG4CPP_INFO(log, "Disaptching new event");
        handler->notify();
        return OK;
    } else {
        waiting++;
        LOG4CPP_INFO(log, "Disaptcher suspended; [%d] events queued for delayed dispatching", waiting);
        return DIS_SUSPENDED;
    }

}


} //namespace comsys
} //namespace controlbox
