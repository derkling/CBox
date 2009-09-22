//******************************************************************************
//*************  Copyright (C) 2006 - Patrick Bellasi **************************
//******************************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** Patrick Bellasi. The programs may be used and/or copied only with the
//** written permission from the author or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//******************************************************************************
//******************** Module information **************************************
//**
//** Project:       ControlBox (0.1)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Patrick Bellasi
//** Creation date:  21/06/2006
//**
//******************************************************************************
//******************** Revision history ****************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------- --------------------
//**
//**
//******************************************************************************


#ifndef _THREADDB_H
#define _THREADDB_H

#include <controlbox/base/Object.h>
#include <controlbox/base/Utility.h>
#include <cc++/thread.h>
#include <list>

namespace controlbox {


/// This interface
class ThreadDB : public ost::PosixThread {

private:

	struct threadData {
		ost::PosixThread* thread;
		int tid;
	};
	typedef struct threadData t_threadData;

	typedef std::list<t_threadData> t_threadDB;

	static ThreadDB * d_instance;

	t_threadDB d_threadDB;

	bool d_doExit;

	log4cpp::Category & log;

public:

	/// Get an instance of ThreadDB
	/// ThreadDB is a singleton class, this method provide
	/// a pointer to the (eventually just created) only one instance.
	static ThreadDB * getInstance();

	/// Default destructor
	virtual ~ThreadDB();

	/// Register a thread into DB
	/// The pointer to a Thread is saved into the DB and
	/// associated to the specified lable.
	/// @param owerride set TRUE to override any eventually already saved
	///		Device's pointer; default FALSE
	exitCode registerThread(ost::PosixThread * p_thread, int p_tid);

	/// Unregister a device from the DB
	exitCode unregisterThread(ost::PosixThread * p_thread);

	/// Print a log with current registerd thread and their status
	std::string printDB();

protected:

	/// Create a new DeviceDB
	ThreadDB(std::string const & logName = "ThreadDB");

	/// The monitor thread
	void run();


};

}// controlbox namespace

#endif
