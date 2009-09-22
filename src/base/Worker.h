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
//** -------- ---------- ---------------------------------- -----------------------------
//**
//**
//***************************************************************************************


#ifndef _WORKER_H
#define _WORKER_H

#include <controlbox/base/Utility.h>
#include <cc++/thread.h>

namespace controlbox {

///And EventGenerator is a simple Generator that could be used to notify a Dispatcher about someting happening.
class Worker : protected ost::PosixThread {

protected:

    /// The conditional to send a singla to this worker
    ost::Conditional d_notice;

    /// Set true when the worker should terminate
    bool d_doExit;
    
    /// The Thread ID
    int d_tid;

private:

    /// We don't want to pullate the derived classes namespace: this logger is
    /// for the base class only
    //std::string d_name;

    /// The logger to used
    log4cpp::Category & log;

public:

    /// The constructor
    Worker(log4cpp::Category & log, char const *name, int prio);

    /// Sleep the current worker until a signal is received.
    /// The current worker is suspended until a signal is received.
    void suspendWorker(void);

    /// Sleep the current worker for the specified time.
    /// The current worked is suspended for the specified time or until a
    /// signal is received.
    /// @param msec period in milliseconds to wait
    void pollWorker(unsigned int msec);

    /// Signal the worker about an event.
    /// This method should be used to wakeup a sleeping worker once there is
    /// new work to do.
    void signalWorker(void);

    /// Start the execution of the worker task
    void runWorker(void);

    /// Signal the worker that must terminate and wait for it to exit.
    /// Note: this method return only once the worker stop working.
    void terminateWorker(void);

protected:

    /// The worker thread initialization method
    /// This method is invoked by a newly created thread when it starts execution
    void initial(void);

    /// The worker thread finalization method
    /// This method is called by a thread that is self terminating
    void final(void);

};

} //namespace controlbox
#endif
