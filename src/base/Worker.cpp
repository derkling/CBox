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


#include "Worker.ih"

namespace controlbox {

Worker::Worker(log4cpp::Category & log, char const *name, int pri) :
	d_doExit(false),
	d_tid(0),
	log(log) {

	LOG4CPP_DEBUG(log, "Worker(%s, %d)", name, pri);

	setName(name);

}

void Worker::suspendWorker(void) {
	d_notice.wait();
}

void Worker::pollWorker(unsigned int msec) {
	d_notice.wait(msec);
}

void Worker::signalWorker(void) {
	d_notice.signal(false);
}

void Worker::terminateWorker(void) {
	d_doExit = true;
	d_notice.signal(false);	
	join();
}

void Worker::initial(void) {
	ThreadDB *l_tdb = ThreadDB::getInstance();
	exitCode result;

	// Saving the LWP identifier
	d_tid = syscall(SYS_gettid);

	// NOTE use "ps -eLf" to check for thread started
	prctl(PR_SET_NAME, getName(), 01, 01, 01);

	result = l_tdb->registerThread(this, d_tid);

	LOG4CPP_INFO(log, "Thread [%s (%d)] started", getName(), d_tid);

}

void Worker::final(void) {
	ThreadDB *l_tdb = ThreadDB::getInstance();
	exitCode result;

	if ( d_tid ) {
		LOG4CPP_WARN(log, "Thread [%s (%d)] terminated", getName(), d_tid);
		result = l_tdb->unregisterThread(this);
	}

}

} //namespace controlbox

