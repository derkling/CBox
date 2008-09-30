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


#ifndef _SIGNALHANDLER_H
#define _SIGNALHANDLER_H

namespace controlbox {
namespace device {

/// A Signal Handler.
/// A signal handler is an entity that could register to a DeviceSignal in order
/// to be notified once a signal happens.
class SignalHandler {

public:

    /// SignalHandler destructor
    virtual ~SignalHandler() {};

    /// The default signal notify routine
    virtual void signalNotify(void) = 0;

};

} //namespace device
} //namespace controlbox
#endif
