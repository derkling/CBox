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


#include "ThreadDB.ih"


namespace controlbox {

ThreadDB * ThreadDB::d_instance = 0;


ThreadDB::ThreadDB(std::string const & logName) :
	d_doExit(false),
	log(log4cpp::Category::getInstance("controlbox."+logName)) {
}

ThreadDB * ThreadDB::getInstance() {

	if ( !d_instance ) {
		d_instance = new ThreadDB("ThreadDB");
	}

	LOG4CPP_DEBUG(d_instance->log, "ThreadDB::getInstance()");

	return d_instance;
}

ThreadDB::~ThreadDB() {

	LOG4CPP_DEBUG(log, "ThreadDB::~ThreadDB()");

	d_doExit = true;
		if ( isRunning() ) {
			this->resume();
		}

	d_threadDB.clear();

}

exitCode ThreadDB::registerThread(ost::PosixThread * p_thread, unsigned short p_pid) {
	ThreadDB::t_threadData l_thread;
	t_threadDB::iterator l_it;

	LOG4CPP_DEBUG(log, "ThreadDB::registerThread(ost::PosixThread * thread)");

	l_it = d_threadDB.begin();
	while ( l_it != d_threadDB.end() ) {
		if ( l_it->thread == p_thread ) {
			break;
		}
		l_it++;
	}
	if ( l_it != d_threadDB.end() ) {
		LOG4CPP_WARN(log, "Thread [%hu:%s] already registerd", l_it->pid, p_thread->getName());
		return OK;
	}

	l_thread.thread = p_thread;
	l_thread.pid = p_pid;

	d_threadDB.push_back(l_thread);
	LOG4CPP_INFO(log, "Registered new thread [%hu:%s]", p_pid, p_thread->getName());

	return OK;

}


exitCode ThreadDB::unregisterThread(ost::PosixThread * p_thread) {
	t_threadDB::iterator l_it;
	unsigned short l_pid;

	LOG4CPP_DEBUG(log, "ThreadDB::unregisterThread(ost::PosixThread * p_thread)");

	l_it = d_threadDB.begin();
	while ( l_it != d_threadDB.end() ) {
		if ( l_it->thread == p_thread ) {
			break;
		}
		l_it++;
	}
	if ( l_it == d_threadDB.end() ) {
		LOG4CPP_WARN(log, "Trying to unregister a thread [%s] not registerd", p_thread->getName());
		return OK;
	}

	l_pid = l_it->pid;

	d_threadDB.erase(l_it);
	LOG4CPP_WARN(log, "Unregistered thread [%hu:%s]", l_pid, p_thread->getName());

	return OK;

}

// Tot: N - Name1(pid1,S), Name2(pid2,S), Name3(pid3,S)...
std::string ThreadDB::printDB() {
	t_threadDB::iterator l_it;
	std::ostringstream l_dump("");
	bool l_isRunning;

	l_dump << d_threadDB.size() << " threads; ";

	l_it = d_threadDB.begin();
	while ( l_it != d_threadDB.end() ) {
		l_dump << (l_it->thread)->getName();
		l_dump << "(" << l_it->pid;

		l_isRunning = (l_it->thread)->isRunning();
		if ( l_isRunning ) {
			l_dump << ",R";
		} else {
			l_dump << ",T";
		}

		l_dump << ") ";

		l_it++;
	}

	return l_dump.str();
}

void ThreadDB::run() {
	unsigned short l_pid;

	l_pid = getpid();

	LOG4CPP_INFO(log, "Thread [%s (%u)] started", "UQ", l_pid);

	this->setName("TM");
	this->registerThread(this, l_pid);


	LOG4CPP_DEBUG(log, "Thread monitor started");
	while ( !d_doExit ) {
		LOG4CPP_INFO(log, "%s", printDB().c_str() );
		::sleep(60);
	}
	LOG4CPP_DEBUG(log, "Thread monitor terminated");

	LOG4CPP_WARN(log, "Thread [%s (%u)] terminated", this->getName(), l_pid);
	this->unregisterThread(this);

}

}
