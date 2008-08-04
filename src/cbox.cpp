//******************************************************************************
//**             Copyright (C) 2006 by Patrick Bellasi                        **
//******************************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** Patrick Bellasi. The programs may be used and/or copied only with the
//** written permission from the author or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//******************************************************************************
//**                   Module information                                     **
//**
//** Project:       ControlBox (0.1)
//** Description:   Main program
//**
//** Filename:      cBox
//** Owner:         Patrick Bellasi
//** Creation date:  01/08/2007
//**
//**
//******************************************************************************
//**                   Revision history                                       **
//**
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------- --------------------
//**
//**
//******************************************************************************

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <iomanip>

#include <cc++/thread.h>

#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/BasicConfigurator.hh>


#include "controlbox/base/Utility.h"
#include "controlbox/base/QueryRegistry.h"
#include "controlbox/devices/DeviceFactory.h"
#include "controlbox/devices/ATcontrol.h"
#include "controlbox/devices/DeviceGPS.h"
#include "controlbox/devices/gprs/DeviceGPRS.h"
#include "controlbox/base/comsys/CommandDispatcher.h"
#include "controlbox/devices/PollEventGenerator.h"
#include "controlbox/devices/FileWriterCommandHandler.h"
#include "controlbox/devices/wsproxy/WSProxyCommandHandler.h"

#define GCC_SPLIT_BLOCK __asm__ ("");

log4cpp::Category & logger = log4cpp::Category::getInstance("controlbox");

void shandler(int sig) {

	logger.info("Received signal [%d], shutting down cBox", sig);

}

int cBoxMain(std::string const & conf,
		std::string const & cmdlog) {
	sigset_t mask;
	struct sigaction act;
	int status;

	controlbox::device::DeviceFactory * df;
	controlbox::QueryRegistry * qr = 0;
// 	controlbox::comsys::CommandDispatcher * stateCd = 0;
// 	controlbox::device::DeviceGPRS * devGPRS = 0;
// 	controlbox::device::DeviceATGPS * devATGPS = 0;
// 	controlbox::device::ATcontrol * devAT = 0;
	controlbox::device::DeviceSignals * devSig = 0;
// 	controlbox::comsys::CommandDispatcher * pollDataCd = 0;
// 	controlbox::comsys::Command * cmdPollData = 0;
// 	controlbox::device::PollEventGenerator * peg = 0;
	controlbox::device::WSProxyCommandHandler * ws = 0;

	// Preloading the configuration options
	controlbox::Configurator::getInstance(conf);

	qr = controlbox::QueryRegistry::getInstance();

	// We use a DeviceFatory to build all others dependencies
	// and link them to the WSProxy
	df = controlbox::device::DeviceFactory::getInstance();

	//--- Installing Handlers
	ws = df->getWSProxy();
	if ( !ws ) {
		logger.error("Proxy initialization FAILED");
		return -1;
	}

// 	logger.info("Initializing poll generator for periodic data collection... ");
// 	cmdPollData = controlbox::comsys::Command::getCommand(
// 				controlbox::device::PollEventGenerator::SEND_POLL_DATA,
// 				controlbox::Device::EG_POLLER,
// 				"SendPollData",
// 				"PollData");
// 	pollDataCd = new controlbox::comsys::CommandDispatcher(ws, false);
// 	pollDataCd->setDefaultCommand(cmdPollData);
// 	//FIXME the polling time should be, runtime configurable!
// 	peg = df->getDevicePoller((600*1000)/10, "SendDataPoller");
// 	peg->setDispatcher(pollDataCd);
// 	peg->enable();

// 	stateCd = new controlbox::comsys::CommandDispatcher(ws, false);
//
// 	logger.info("Initializing GPRS... ");
// 	devGPRS = df->getDeviceGPRS();
// 	if ( !devGPRS ) {
// 		logger.error("GPRS initialization FAILED");
// 	} else {
// 		devGPRS->setDispatcher(stateCd);
// 		devGPRS->enable();
// 	}

// 	logger.info("Initializing GPS/ODO/MEMS... ");
// 	devATGPS = df->getDeviceATGPS();
// 	if ( !devATGPS ) {
// 		logger.error("GPS/ODO/MEMS initialization FAILED");
// 	} else {
// 		devATGPS->setDispatcher(stateCd);
// 		devATGPS->enable();
// 	}

// NOTE disable console before use the AT device!!!
// Disable this to avoid ttyS0 conflict with console...
// 	logger.info("Initializing AT control interface... ");
// 	devAT = controlbox::device::ATcontrol::getInstance();
// 	if ( !devAT ) {
// 		logger.error("AT control interface initialization FAILED");
// 	} else {
// 		devAT->setDispatcher(stateCd);
// 		devAT->enable();
// 	}

	sleep(2);

	sigemptyset(&mask);
	act.sa_handler = shandler;
	act.sa_mask = mask;
	act.sa_flags = 0;

	logger.info("Configuring system signals... ");
	if ( sigaction(SIGTERM, &act, 0) == -1 ) {
		logger.error("signal handler installation FAILED");
	}

	logger.info("System Up and Running");
	sleep(2);

	logger.info("Sending PowerOn Event... ");
	devSig = df->getDeviceSignals();
	if (devSig) {
		devSig->powerOn();
	}

	// Suspending and waiting for system suthdown...
	sigsuspend(&mask);

	logger.info("Sending PowerOff Event... ");
	if (devSig) {
		devSig->powerOn(false);
	}

	// Wait few moments for message (possible) uploading...
	sleep(10);

shutdown:
// 	delete devGPRS;
// 	delete devATGPS;
// 	delete devAT;
	delete ws;

	return 0;

}

/// Print the Help menu
void print_usage(char * progname) {

	cout << "cBox ver. " << VERSION << endl;
	cout << "\tUsage: " << progname << " [options]" << endl;
	cout << "\tOptions:" << endl;
	cout << "\t -c, --configuration        Configuration file" << endl;
	cout << "\t -d, --cmdlog               Command logger filepath" << endl;
	cout << "\t -l, --loggeer              Logger configuration filepath" << endl;
	cout << "\t -v, --verbose              Enable verbose output" << endl;
	cout << "\t -h, --help                 Print this help" << endl;

	cout << "\nby Patrick Bellasi - derkling@gmail.com\n" << endl;

}

int main (int argc, char *argv[]) {

	// Command line parsing
	int this_option_optind;
	int option_index;
	static struct option long_options[] = {
			{"configuration", required_argument, 0, 'c'},
			{"cmdlog", required_argument, 0, 'd'},
			{"logconf", required_argument, 0, 'l'},
			{"verbose", no_argument, 0, 'v'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};
	static char * optstring = "c:d:l:vh";
	int c;
	bool silent = true;

	// cBox configuration
	std::string cboxConfiguration = "/etc/cbox/cbox.conf";
	// Logger configuration
	std::string loggerConfiguration = "/etc/cbox/cbox.conf";
	// Command logfile
	std::string cmdLogfile = "/var/tmp/cboxcmd.log";

#ifdef CONTROLBOX_DEBUG
	// Forcinf unbuffered stdout
	setlinebuf(stdout);
#endif

	// Command line parameter parsing
	while (1) {
		this_option_optind = optind ? optind : 1;
		option_index = 0;

		c = getopt_long (argc, argv, optstring, long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'c':
				if (optarg) {
					cout << optarg << endl;
					cboxConfiguration = optarg;
				} else {
					cout << "Missing Configuration option parameter!" << endl;
					print_usage(argv[0]);
					return EXIT_FAILURE;
				}
				break;
			case 'd':
				if (optarg) {
					cout << optarg << endl;
					cmdLogfile = optarg;
				} else {
					cout << "Missing Configuration option parameter!" << endl;
					print_usage(argv[0]);
					return EXIT_FAILURE;
				}
				break;
			case 'l':
				if (optarg) {
					cout << optarg << endl;
					loggerConfiguration = optarg;
				} else {
					cout << "Missing Configuration option parameter!" << endl;
					print_usage(argv[0]);
					return EXIT_FAILURE;
				}
				break;
			case 'h':
				print_usage(argv[0]);
				return EXIT_SUCCESS;
			case 'v':
				silent = false;
			default:
				cout << "Unknowen option: " << c << endl;
		}
	}

//-------------------------- MAIN START HERE -----------------------------------

	cout << endl << "\t\t       cBox ver. " << VERSION << endl;
	cout <<         "\t\tby Patrick Bellasi - derkling@gmail.com" << endl << endl;

	// Logger initialization
	try {
		std::cout << "Using logger configuration: " << loggerConfiguration << endl;
		log4cpp::PropertyConfigurator::configure(loggerConfiguration);
	} catch(log4cpp::ConfigureFailure& f) {
		std::cout << "Configuration problem " << f.what() << std::endl;
		return EXIT_FAILURE;
	}

	logger.debug("Logger correctly initialized");

	if (silent) {
		logger.setPriority(log4cpp::Priority::INFO);
	}

	std::cout << "Using system configuration: " << cboxConfiguration << endl;
	std::cout << "Command dump: " << cmdLogfile << endl << endl;
	cBoxMain(cboxConfiguration, cmdLogfile);

	log4cpp::Category::shutdown();

	return EXIT_SUCCESS;

}

