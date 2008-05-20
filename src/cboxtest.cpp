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
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Patrick Bellasi
//** Creation date:  21/06/2006
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

#include "controlbox/base/Utility.h"
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/BasicConfigurator.hh>


#define __PROVA__



#include "controlbox/base/comsys/EventDispatcher.h"
#include "controlbox/base/comsys/CommandDispatcher.h"

#include "controlbox/devices/DeviceFactory.h"
#include "controlbox/devices/PollEventGenerator.h"
#include "controlbox/devices/FileWriterCommandHandler.h"
#include "controlbox/base/comsys/Command.h"
#include "controlbox/base/DeviceDB.h"
#include "controlbox/devices/DeviceFactory.h"

#include "controlbox/devices/DeviceGPIO.h"
#include "controlbox/devices/DeviceGPS.h"
#include "controlbox/devices/DeviceOdometer.h"
#include "controlbox/devices/arduino/DeviceArdu.h"
#include "controlbox/devices/gprs/DeviceGPRS.h"
#include "controlbox/devices/DeviceAnalogSensors.h"
#include "controlbox/devices/DeviceDigitalSensors.h"
#include "controlbox/devices/DeviceOdometer.h"
#include "controlbox/devices/te/DeviceTE.h"
#include "controlbox/devices/wsproxy/WSProxyCommandHandler.h"

#include "controlbox/base/QueryRegistry.h"
#include "controlbox/devices/ATcontrol.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <iomanip>

#define GCC_SPLIT_BLOCK __asm__ ("");

using namespace controlbox;

void print_usage(char * progname);
int test_comlibs(log4cpp::Category & logger);
int test_devicedb(log4cpp::Category & logger);
int test_command(log4cpp::Category & logger);
int test_wsproxy(log4cpp::Category & logger);
// int test_nmeaparser(log4cpp::Category & logger);
int test_devicegprs(log4cpp::Category & logger, int sleeptime);
int test_deviceas(log4cpp::Category & logger);
int test_devicegpio(log4cpp::Category & logger);
int test_deviceds(log4cpp::Category & logger);
// int test_deviceodo(log4cpp::Category & logger);
// int test_deviceardu(log4cpp::Category & logger);
int test_deviceatgps(log4cpp::Category & logger);
int test_devicete(log4cpp::Category & logger);
int test_atinterface(log4cpp::Category & logger);

unsigned sleeptime = 0;
unsigned cycles = 0;

int main (int argc, char *argv[]) {

	// Command line parsing
	int this_option_optind;
	int option_index;
	static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"release", no_argument, 0, 'r'},
			{"sleep", required_argument, 0, 's'},
			{"conf", required_argument, 0, 'C'},
			{"cycles", required_argument, 0, 'c'},
			{"comlibstest", no_argument, 0, 'l'},
			{"devdbtest", no_argument, 0, 'e'},
			{"commandtest", no_argument, 0, 'd'},
			{"wsproxytest", no_argument, 0, 'w'},
			{"gprstest", no_argument, 0, 'n'},
			{"astest", no_argument, 0, 'a'},
			{"gpiotest", no_argument, 0, 'o'},
			{"dstest", no_argument, 0, 'b'},
			{"atgpstest", no_argument, 0, 'g'},
			{"tetest", no_argument, 0, 'i'},
			{"attest", no_argument, 0, 't'},
			{0, 0, 0, 0}
		};
	static char * optstring = "abC:c:dehgilnors:tw";
	int c;
	bool silent = false;

	// TESTS to do
	bool printHelp = true;
	bool testComlibs = false;
	bool testDeviceDB = false;
	bool testDaricomCommand = false;
	bool testWSProxyCommandHandler = false;
// 	bool testDeviceGPS = false;
	bool testDeviceGPRS = false;
	bool testDeviceAS = false;
	bool testDeviceGPIO = false;
	bool testDeviceDS = false;
// 	bool testDeviceOdo = false;
	bool testDeviceATGPS = false;
	bool testDeviceTE = false;
	bool testATinterface = false;

	// Logger configuration
	std::string confFilename = "/etc/cbox/cbox.conf";

#ifdef CONTROLBOX_DEBUG
	// Forcinf unbuffered stdout
	setlinebuf(stdout);
#endif

	// Command line parameter parsing
	if (argc < 2) {
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	while (1) {
		this_option_optind = optind ? optind : 1;
		option_index = 0;

		c = getopt_long (argc, argv, optstring, long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 's':
				if (optarg) {
					sleeptime = atoi(optarg);
				} else {
					cout << "Missing Sleeptime option parameter!" << endl;
					print_usage(argv[0]);
					return EXIT_FAILURE;
				}
				cout << "Sleep time configuration: " << sleeptime << endl;
				break;
			case 'c':
				if (optarg) {
					cycles = atoi(optarg);
				} else {
					cout << "Missing Cycles option parameter!" << endl;
					print_usage(argv[0]);
					return EXIT_FAILURE;
				}
				cout << "Cycles count configuration: " << cycles << endl;
				break;
			case 'C':
				if (optarg) {
					confFilename = optarg;
				} else {
					cout << "Missing Configuration option parameter!" << endl;
					print_usage(argv[0]);
					return EXIT_FAILURE;
				}
				break;
			case 'l':
				testComlibs = true;
				printHelp = false;
				break;
			case 'e':
				testDeviceDB = true;
				printHelp = false;
				break;
			case 'd':
				testDaricomCommand = true;
				printHelp = false;
				break;
			case 'w':
				testWSProxyCommandHandler = true;
				printHelp = false;
				break;
			case 'g':
				testDeviceATGPS = true;
				printHelp = false;
				break;
			case 'n':
				testDeviceGPRS = true;
				printHelp = false;
				break;
			case 'a':
				testDeviceAS = true;
				printHelp = false;
				break;
			case 'b':
				testDeviceDS = true;
				printHelp = false;
				break;
			case 'o':
				testDeviceGPIO = true;
				printHelp = false;
				break;
			case 't':
				testATinterface = true;
				printHelp = false;
				break;
			case 'i':
				testDeviceTE = true;
				printHelp = false;
				break;
			case 'r':
				silent = true;
				break;
			case 'h':
				print_usage(argv[0]);
				return EXIT_SUCCESS;
			default:
				cout << "Unknowen test required: " << c << endl;
		}
	}

	// Print help if any test hasb been required
	if ( printHelp ) {
		print_usage(argv[0]);
		return EXIT_SUCCESS;
	}

	// Checking required params and setting default values
	if (testDeviceGPRS) {
		sleeptime = sleeptime ? sleeptime : 30;
	}
	if (testDeviceAS) {
		sleeptime = sleeptime ? sleeptime : 6;
		cycles = cycles ? cycles : 10;
	}
	if (testWSProxyCommandHandler) {
		sleeptime = sleeptime ? sleeptime : 30;
	}

//-------------------------------------------- MAIN START HERE --------------------------------------------

	cout << " " << endl;
	cout << "\t\t++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
	cout << "\t\t+ cBox v0.9   -   by Patrick Bellasi <derkling@gmail.com>      +" << endl;
	cout << "\t\t++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;

	// Initializing Configurator
	cout << endl;
	cout << "Using configuration: " << confFilename << "\n" << endl;
	controlbox::Configurator::getInstance(confFilename);
	try {
		log4cpp::PropertyConfigurator::configure(confFilename);
	} catch(log4cpp::ConfigureFailure& f) {
		std::cout << "Configuration problem " << f.what() << std::endl;
		return EXIT_FAILURE;
	}

	log4cpp::Category & logger = log4cpp::Category::getInstance("cboxtest");
	logger.debug("Logger correctly initialized");

	logger.debug("Sleep time: %d", sleeptime);
	logger.debug("Cycles:     %d", cycles);

	if (silent) {
		logger.setPriority(log4cpp::Priority::INFO);
	}

	logger.debug("Runnig tests...");

	if (testComlibs) {
		logger.debug("----------- Testing comlibs ---");
		test_comlibs(logger);
	}
	if (testDeviceDB) {
		logger.debug("----------- Testing DeviceDB ---");
		test_devicedb(logger);
	}
	if (testDaricomCommand) {
		logger.debug("----------- Testing DaricomCommand ---");
		test_command(logger);
	}
	if (testWSProxyCommandHandler) {
		logger.debug("----------- Testing WSProxyCommandHandler ---");
		test_wsproxy(logger);
	}
	if (testDeviceGPRS) {
		logger.debug("----------- Testing DeviceGPRS ---");
		test_devicegprs(logger, sleeptime);
	}
	if (testDeviceAS) {
		logger.debug("----------- Testing DeviceAS ---");
		test_deviceas(logger);
	}
	if (testDeviceGPIO) {
		logger.debug("----------- Testing DeviceGPIO ---");
		test_devicegpio(logger);
	}
	if (testDeviceDS) {
		logger.debug("----------- Testing DeviceDS ---");
		test_deviceds(logger);
	}
	if (testDeviceATGPS) {
		logger.debug("----------- Testing DeviceATGPS ---");
		test_deviceatgps(logger);
	}
	if (testDeviceTE) {
		logger.debug("----------- Testing DeviceTE ---");
		test_devicete(logger);
	}
	if (testATinterface) {
		logger.debug("----------- Testing AT Interface ---");
		test_atinterface(logger);
	}

	logger.debug("----------- Tests Completed ---\n");
	log4cpp::Category::shutdown();

	return EXIT_SUCCESS;

}

/// Print the Help menu
void print_usage(char * progname) {

	cout << "Usage: " << progname << " [options]" << endl;
	cout << "Options:" << endl;
	cout << "-h, --help                 Print this help" << endl;
	cout << "-r, --release              Force don't print debug messages" << endl;
	cout << "-s, --sleep <seconds>      Seconds to wait between cycles" << endl;
	cout << "-C, --configuration        Configuration file path" << endl;
	cout << "-c, --cycles <count>       Test cycles number" << endl;
	cout << "-l, --comlibstest          Do a Test on comlibs" << endl;
	cout << "-e, --devdbtest            Do a Test on DeviceDB" << endl;
	cout << "-d, --commandtest          Do a Test on DaricomCommand" << endl;
	cout << "-w, --wsproxytest          Do a Test on WSProxyCommandHandler" << endl;
	cout << "-g, --atgpstest            Do a Test on DeviceATGPS" << endl;
	cout << "-o, --gpiotest             Do a Test on DeviceGPIO" << endl;
	cout << "-n, --gprstest             Do a Test on DeviceGPRS" << endl;
	cout << "-a, --astest               Do a Test on DeviceAS" << endl;
	cout << "-b, --bstest               Do a Test on DeviceDS" << endl;
	cout << "-i, --tetest               Do a Test on DeviceTE" << endl;
	cout << "-t, --attest               Do a Test on the AT interface" << endl;

	cout << "\nPatrick Bellasi - derkling@gmail.com\n" << endl;

}


/// Comlibs TEST case
int test_comlibs(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::EventDispatcher * ed = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::PollEventGenerator * peg = 0;
	controlbox::comsys::Command * command = 0;


	logger.info("Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("/tmp/filewriter.log");
	logger.info("DONE!");

	logger.info("Initializing a Command... ");
		command = controlbox::comsys::Command::getCommand(1, Device::EG_POLLER, "PollEventGenerator", "PollEventGenerator");
	command->setParam( "StringParam", "ParamValue" );
	command->setParam( "IntParam", 2 );
	command->setParam( "FloatParam",  1.23 );
	logger.info("DONE!");

	logger.info("Initializing an CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	cd->setDefaultCommand(command);

	logger.info("Initializing a PollEventGenerator... ");
	peg = new controlbox::device::PollEventGenerator(2000, cd);
	logger.info("DONE!");


	sleep(5); 		// print 2 messages
	peg->disable();
	sleep(5); 		// lose 3 messages
	peg->enable();
	sleep(2); 		// print 1 messages
	cd->suspend();
	sleep(6); 		// queue 3 messages
	cd->resume(); 		// => immediatly forward 3 messages
	sleep(2); 		// print 1 messages
	cd->suspend();
	sleep(6); 		// queue 3 messages
	cd->resume(true);	// => flush 3 messages
	sleep(2); 		// print 1 messages


	logger.info("Shutting down the PollEventGenerator... ");
	peg->disable();
	delete peg;
	logger.info("DONE!");

	logger.info("Shutting down the EventDispatcher... ");
	delete ed;
	logger.info("DONE!");

	logger.info("Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}


/// DeviceDB and DeviceFactory TEST case
int test_devicedb(log4cpp::Category & logger) {

	controlbox::device::DeviceFactory * df = 0;
	controlbox::DeviceDB * db = 0;
	controlbox::device::FileWriterCommandHandler * fw1 = 0;
	controlbox::device::FileWriterCommandHandler * fw2 = 0;
	controlbox::device::FileWriterCommandHandler * fw3 = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::comsys::Command * command = 0;
	controlbox::device::PollEventGenerator * peg1 = 0;
	controlbox::device::PollEventGenerator * peg2 = 0;
	controlbox::device::PollEventGenerator * peg3 = 0;

	logger.info("Getting a reference to the DeviceFactory... ");
	df = controlbox::device::DeviceFactory::getInstance();
	db = DeviceDB::getInstance();

	logger.info("Initializing some Devices... ");
	fw1  = df->getDeviceFileWriter( "./filewriter1.log", "FileWriter_1");
	peg1 = df->getDevicePoller(1000, "Poller1000");
	logger.info("%s", db->printDB().c_str());
	fw2  = df->getDeviceFileWriter( "./filewriter2.log", "FileWriter_2");
	fw3  = df->getDeviceFileWriter( "./filewriter3.log", "FileWriter_3");
	peg2 = df->getDevicePoller(2000, "Poller2000");
	peg3 = df->getDevicePoller(3000, "Poller3000");
	logger.info("DONE!");

	logger.info("%s", db->printDB( Device::CH_FILEWRITER ).c_str());

	logger.info("%s", db->printDB().c_str());


	command = controlbox::comsys::Command::getCommand(controlbox::device::PollEventGenerator::SEND_POLL_DATA, Device::EG_POLLER, "Poller", "PollData");
	cd = new controlbox::comsys::CommandDispatcher(fw1, false);
	cd->setDefaultCommand(command);
	peg1->setDispatcher(cd);
	peg1->enable();
	sleep(30);

	logger.info("Shutting down the PollEventGenerator... ");
	peg1->disable();
	peg2->disable();
	peg3->disable();
	delete peg1;
	delete peg2;
	logger.info("%s", db->printDB(Device::EG_POLLER).c_str());
	delete peg3;			// Overrided device: THIS one must be unregistered!!!
	logger.info("DONE!");


	logger.info("%s", db->printDB().c_str());

	logger.info("Shutting down the FileWriterCommandHandler... ");
	delete fw3;
	delete fw2;
	delete fw1;
	logger.info("DONE!");

	logger.info("%s", db->printDB().c_str());

	return 0;

}


/// Command TEST case
int test_command(log4cpp::Category & logger) {

	controlbox::comsys::Command * command = 0;

	logger.info("Initializing a DaricomCommand... ");
	command = controlbox::comsys::Command::getCommand(1, Device::DEVICE_TE, "SampiTE", "SampiTE");
	command->setParam( "StringParam", "ParamValue" );
	command->setParam( "IntParam", 2 );
	command->setParam( "FloatParam",  (float)1.23 );
	command->setParam( "MultiParam1", 1, false);
	command->setParam( "MultiParam1", "due", false);
	command->setParam( "MultiParam2", 10, false);
	command->setParam( "MultiParam3", 100, false); // Test multiparam overriding by empting the list
	command->setParam( "MultiParam1", 3, false);
	command->setParam( "MultiParam2", 20, false);
	command->setParam( "MultiParam3", 200, false); // Test multiparam overriding by empting the list
	command->setParam( "MultiParam1", "quattro", false);
	command->setParam( "MultiParam1", 5, false);
	command->setParam( "MultiParam2", 30, false);
	command->setParam( "MultiParam3", 100); // Test multiparam overriding by empting the list
	logger.info("DONE!");

	logger.info("Testing MultiParam support on DaricomCommand... ");
	logger.info("Multiparam1 has: %u values", command->paramCount("MultiParam1") );
	logger.info("Multiparam2 has: %u values", command->paramCount("MultiParam2") );
	logger.info("Multiparam3 has: %u values", command->paramCount("MultiParam3") );
	try {
		logger.info(" MultiParam1[1] = %s", (command->param("MultiParam1", 1)).c_str() );
		logger.info(" MultiParam1[2] = %s", (command->param("MultiParam1", 2)).c_str() );
		logger.info(" MultiParam1[3] = %s", (command->param("MultiParam1", 3)).c_str() );
		logger.info(" MultiParam1[5] = %s", (command->param("MultiParam1", 5)).c_str() );
		logger.info(" MultiParam1[4] = %s", (command->param("MultiParam1", 4)).c_str() );
		logger.info(" MultiParam2[1] = %s", (command->param("MultiParam2", 1)).c_str() );
		logger.info(" MultiParam2[2] = %s", (command->param("MultiParam2", 2)).c_str() );
		logger.info(" MultiParam2[3] = %s", (command->param("MultiParam2", 3)).c_str() );
		logger.info(" MultiParam2[1] = %s", (command->param("MultiParam2", 1)).c_str() );
		logger.info(" MultiParam1[4] = %s", (command->param("MultiParam1", 4)).c_str() );
		logger.info(" MultiParam2[3] = %s", (command->param("MultiParam2", 3)).c_str() );
	} catch (exceptions::UnknowedParamException) {
		logger.error("The required param does NOT exist");
	}
	logger.info("DONE!");

	return 0;

}

/// WSProxyCommandHandler TEST case
int test_wsproxy(log4cpp::Category & logger) {

	controlbox::device::DeviceFactory * df;
	controlbox::device::WSProxyCommandHandler * proxy = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::PollEventGenerator * peg = 0;
	controlbox::device::DeviceTime * time = 0;
	controlbox::comsys::Command * command = 0;
	std::string theTime;

	logger.debug("01 - Initializing the DeviceFactory... ");
	df = controlbox::device::DeviceFactory::getInstance();
	logger.debug("DONE!");

	logger.debug("02 - Initializing a WSProxy for testing... ");
	proxy = df->getWSProxy();
	logger.debug("DONE!");

	logger.debug("03 - Getting DeviceTime... ");
	time = df->getDeviceTime();
	logger.debug("DONE!");

	theTime = time->time();
	logger.debug("Current time is: %s", theTime.c_str());

// ::sleep(15);
	logger.debug("03 - Building a new Command... ");
	command = controlbox::comsys::Command::getCommand(controlbox::device::DeviceInCabin::SEND_GENERIC_DATA, Device::DEVICE_IC, "DeviceInCabin", "UserData");

// 	command->setParam( "dist_event", "09;Daricom Test" );
	command->setParam( "dist_evtType", 0x09 );
	command->setParam( "dist_evtData", "Daricom Test - TE messages upload" );
	command->setParam( "timestamp", time->time() );
	logger.debug("DONE!");

	logger.debug("04 - Sending the TEST TITLE command... ");
	proxy->notify(command);
	logger.debug("DONE!");

/*
	// FIXME Attention: if this command will be deleted before WSProxy has
	// finisce using it we get a SEGFAULT!!!
	logger.debug("waiting 30s before continuing...");
	::sleep(5);
	delete command;
*/

	logger.debug("05 - Preparing a poll generator for sending periodic data... ");
	command = controlbox::comsys::Command::getCommand(controlbox::device::PollEventGenerator::SEND_POLL_DATA, Device::EG_POLLER, "SendPollData", "PollData");
	cd = new controlbox::comsys::CommandDispatcher(proxy, false);
	cd->setDefaultCommand(command);
	//peg = df->getDevicePoller((sleeptime*1000)/10, "SendDataPoller");
	peg = df->getDevicePoller(60000, "SendDataPoller");
	peg->setDispatcher(cd);
	logger.debug("DONE!");

	logger.debug("06 - Starting poller and testing for a while [%ds]... ", sleeptime);
	peg->enable();
	sleep(sleeptime);
	logger.debug("DONE!");

	logger.debug("07 - Shutting down WSProxy... ");
	delete proxy;
	logger.debug("DONE!");

	return 0;

/*
	logger.info("01 - Initializing a DeviceGPS for WS Testing... ");
	devGPS = DeviceGPS::getNewInstance();
	logger.info("DONE!");

	logger.info("03 - Initializing a DaricomCommand for WS Testing... ");
	command = new DaricomCommand(WSProxyCommandHandler::SEND_DATA, WSPROXY, 0, "WSProxyCommand");
	command->setParam( "infoType", WSProxyCommandHandler::EVENT_MANUAL ); // Send a "MANUAL INPUT"
	command->setParam( "time", devGPS->time() );
	command->setParam( "message", "This is just a TEST from Daricom Srl" );
	logger.info("DONE!");

	//commandLink = ch->prepareCommand(WSProxyCommandHandler::NET_LINK_STATUS_UPDATE);
	//commandLink->setParam("status", DeviceNET::LINK_UP);

	logger.info("04 - Initializing a CommandDispatcher for WS Testing... ");
	cd = new controlbox::comsys::CommandDispatcher(ch);
	devGPS->setDispatcher(cd, true);
	sleep(1);			// Waiting for devGPS to update current Time
	command->setParam( "time", devGPS->time() );
	cd->setDefaultCommand(command);
	cd->resume(true);
	logger.info("DONE!");

	logger.info("05 - Initializing a PollEventGenerator for WS Testing... ");
	peg = new controlbox::device::PollEventGenerator(1000, cd);
	logger.info("DONE!");

	logger.info("\tThe system is up and running!");

	sleep(3);			// LINK_DOWN time period
	logger.info("Tearing up the network link... ");
	cd->dispatch(commandLink);	// Changing to LINK_UP
	sleep(3);			// LINK_UP time period => 2queued + 3live messages shuld be uploaded

	logger.info("Shutting down the PollEventGenerator... ");
	peg->disable();
	delete peg;
	logger.info("DONE!");

	logger.info("Weating for queued SOAP messages to be uploaded... ");
	sleep(10);

	logger.info("Shutting down the DeviceGPS... ");
	devGPS->disable();
	delete devGPS;
	logger.info("DONE!");

	logger.info("Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");
*/

}

/// NMEA parser testing
#if 0
int test_nmeaparser(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceGPS * devGPS = 0;


	logger.info("01 - Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("02 - Initializing an CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("03 - Initializing a DeviceGPS for Testing... ");
	if ( (devGPS = controlbox::device::DeviceGPS::getInstance(cd,false)) != 0 ) {
		logger.info("DONE!");
	} else {
		logger.error("DeviceGPS building problems!");
		return EXIT_FAILURE;
	}

	logger.info("04 - Starting parsing... ");
	devGPS->enable();

	sleep(300);
	devGPS->disable();

	logger.info("05 - Shutting down the DeviceGPS... ");
	delete devGPS;
	logger.info("DONE!");

	logger.info("06 - Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("07 - Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}
#endif

/// GPRS testing
int test_devicegprs(log4cpp::Category & logger, int sleeptime) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceGPRS * devGPRS = 0;
	controlbox::device::ATcontrol * at = 0;
	controlbox::QueryRegistry * qr = 0;
	unsigned short level;


	logger.info("---- Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("---- Initializing an CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("---- Initializing a DeviceGPRS for Testing... ");
	try {
		devGPRS = controlbox::device::DeviceGPRS::getInstance("tinlink");
		if (devGPRS) {
			devGPRS->setDispatcher(cd);
			devGPRS->enable();
		} else {
			logger.fatal("Failed building a DeviceGPRS: ABORTING!");
			goto gprs_failed;

		}
	} catch (exceptions::Exception e) {
		cout << "Exception caught" << endl;
	}

// 	logger.info("---- Initializiig a CommandAT");
// 	at = controlbox::device::ATcontrol::getInstance();
// 	qr = QueryRegistry::getInstance();
// 	at->setDispatcher(cd);
// 	at->enable();
// 	logger.info("Dump QueryRegistry... %s", qr->printRegistry().c_str());

	sleep(5);
	logger.info("Connecting GPRS...");
	if ( devGPRS->connect("tinlink") == OK ) {
		logger.info("---- Connection UP AND RUNNRING");
	} else {
		logger.error("---- Connection PROBLEMS");
		goto gprs_shutdown;
	}

	sleep(2);
	logger.info("Testing signal level...");
	devGPRS->signalLevel(level);
	logger.info("Signal level [%u]", level);

// 	sleep(2);
// 	logger.info("Testing SMS...");
// 	devGPRS->sendSMS("+393473153808", "cBox Test");


	logger.info("---- The system is ready for network testing! Repeatly connection [%d times, one time each %d seconds]...", cycles, sleeptime);
	while ( cycles-- ) {
		sleep(sleeptime);
		if ( devGPRS->connect("tinlink") == OK ) {
			logger.info("---- Connection UP AND RUNNRING");
		} else {
			logger.error("---- Connection PROBLEMS");
		}
	}

/*
	logger.info("06 - Testing rapid re-linking... ");
	// 1
	devGPRS->disconnect();
	sleep(10);
	if ( devGPRS->connect("tinlink") == OK ) {
		logger.info("04 - Connection UP AND RUNNRING");
	} else {
		logger.error("04 - Connection PROBLEMS");
		goto shutdown;
	}
	sleep(3);
	// 2
	devGPRS->disconnect();
	sleep(1);
	if ( devGPRS->connect("tinlink") == OK ) {
		logger.info("04 - Connection UP AND RUNNRING");
	} else {
		logger.error("04 - Connection PROBLEMS");
		goto shutdown;
	}
	sleep(5);
	// 3
	devGPRS->disconnect();
	sleep(3);
	if ( devGPRS->connect("tinlink") == OK ) {
		logger.info("04 - Connection UP AND RUNNRING");
	} else {
		logger.error("04 - Connection PROBLEMS");
		goto shutdown;
	}
	sleep(10);
*/


gprs_shutdown:

//	devGPRS->disable();

// 	logger.info("---- Shutting down the CommandAT... ");
// 	delete at;
// 	logger.info("DONE!");

	logger.info("---- Shutting down the DeviceGPRS... ");
	devGPRS->disconnect();
	sleep(10);
	delete devGPRS;
	logger.info("DONE!");

gprs_failed:
	logger.info("---- Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("---- Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}

/// Analog Sensors testing
int test_deviceas(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceAnalogSensors * devAS = 0;

	controlbox::device::DeviceAnalogSensors::t_asIdList asIdList;
	unsigned cSensors, i, j;
	controlbox::device::DeviceAnalogSensors::t_asIdList::iterator aSensorId;
	float aValue;
	std::ostringstream readedValues;

	logger.info("01 - Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("02 - Initializing an CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("03 - Initializing a DeviceAS for Testing... ");
	devAS = controlbox::device::DeviceAnalogSensors::getInstance();
	devAS->setDispatcher(cd, true);
	logger.info("DONE!");

	logger.info("04 - Starting Analog Sensors test... ");


	logger.debug("Retriving Analog Sensor IDs... ");
	cSensors = devAS->listId(asIdList);
	aSensorId = asIdList.begin(); i = 0;
	logger.debug("Loaded [%d] sensors IDs:", cSensors);
	while ( aSensorId != asIdList.end() ) {
		logger.debug("%3d => %20s", i, (*aSensorId).c_str());
		aSensorId++; i++;
	}

	sleep(2);

	j = cycles;
	logger.info("Repeatly reading sensors values [%d times, one time each %d seconds]...", cycles, sleeptime);
	while ( j ) {
		readedValues.str("loop ");
		readedValues << setw(3) << cycles-j << ": ";
		aSensorId = asIdList.begin(); i = 0;
		while ( aSensorId != asIdList.end() ) {
			devAS->read((*aSensorId).c_str(), aValue);
			readedValues << "[" << setw(10) << fixed << setprecision(5) << aValue << "] ";
			GCC_SPLIT_BLOCK
			aSensorId++; i++;
		}
		logger.info("%s", readedValues.str().c_str());
		sleep(sleeptime); j--;
	}


	logger.info("05 - Shutting down the DeviceAS... ");
	delete devAS;
	logger.info("DONE!");

	logger.info("06 - Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("07 - Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}


/// GPIO testing
int test_devicegpio(log4cpp::Category & logger) {
	device::DeviceFactory * df = device::DeviceFactory::getInstance();
	device::DeviceGPIO * devGPIO = 0;

	logger.info("03 - Initializing a DeviceDS for Testing... ");
	devGPIO = df->getDeviceGPIO();
	logger.info("DONE!");

	logger.info("04 - Powering On GPRS... ");
	devGPIO->gprsPower(device::DeviceGPIO::GPRS1, device::DeviceGPIO::GPIO_ON);

	logger.info("05 - Testing TTY's mutex... ");

	logger.info("Locking TTY1... ");
	devGPIO->ttyLock(device::DeviceGPIO::TTY1_PORT0);
	logger.info("Locking TTY2... ");
	devGPIO->ttyLock(device::DeviceGPIO::TTY2_PORT3);

	logger.info("06 - Waiting %d [s]... ", sleeptime);
	::sleep(sleeptime);

	logger.info("UnLocking TTY1... ");
	devGPIO->ttyUnLock(device::DeviceGPIO::TTY1_PORT0);
	logger.info("UnLocking TTY2... ");
	devGPIO->ttyUnLock(device::DeviceGPIO::TTY2_PORT3);

	logger.info("06 - Powering Down GPRS... ");
	devGPIO->gprsPower(device::DeviceGPIO::GPRS1, device::DeviceGPIO::GPIO_OFF);


	logger.info("07 - Shutting down the DeviceDS... ");
	delete devGPIO;
	logger.info("DONE!");

	return 0;

}

/// Digital Sensors testing
int test_deviceds(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceDigitalSensors * devDS = 0;

// 	controlbox::device::DeviceDigitalSensors::t_dsIdList dsIdList;
	unsigned cSensors, i, j;
// 	controlbox::device::DeviceAnalogSensors::t_asIdList::iterator aSensorId;
	std::ostringstream readedValues;

	logger.info("01 - Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("02 - Initializing an CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("03 - Initializing a DeviceDS for Testing... ");
	devDS = controlbox::device::DeviceDigitalSensors::getInstance();
	devDS->setDispatcher(cd);
	devDS->enable();
	logger.info("DONE!");

	logger.info("04 - Starting Digital Sensors test... ");


	sleep(sleeptime);



	logger.info("05 - Shutting down the DeviceDS... ");
	delete devDS;
	logger.info("DONE!");

	logger.info("06 - Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("07 - Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}

/// Odometer testing
#if 0
int test_deviceodo(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceDigitalSensors * devDS = 0;
	controlbox::device::DeviceOdometer * devOdo = 0;
	double newDist, oldDist;

	logger.info("01 - Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("02 - Initializing an CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

// 	logger.info("03 - Initializing a DeviceDS for Testing... ");
// 	devDS = controlbox::device::DeviceDigitalSensors::getInstance();
// 	devDS->setDispatcher(cd);
// 	devDS->enable();
// 	logger.info("DONE!");

	logger.info("04 - Initializing an Odometer for Testing... ");
	devOdo = controlbox::device::DeviceOdometer::getInstance();
	devOdo->setDispatcher(cd);
	devOdo->enable();
	logger.info("DONE!");

	logger.info("05 - Starting Odometer test... ");

	oldDist = 0;
	while (cycles) {
		newDist = devOdo->distance(controlbox::device::DeviceOdometer::M);
		cout << endl << "Distance: " << newDist
		     << " m,   Speed: " << devOdo->speed(controlbox::device::DeviceOdometer::KMH)
		     << " km/h,   Delta: " << newDist - oldDist << " m"
		     << " (" << (newDist - oldDist) * 3.6 / sleeptime << " km/h)";
// 		logger.info("Distance: %08lu m, Speed: %5lu km/h",
// 			(unsigned long)devOdo->distance(controlbox::device::DeviceOdometer::M),
// 			(unsigned long)devOdo->speed(controlbox::device::DeviceOdometer::KMH));
		oldDist = newDist;
		sleep(sleeptime);
		cycles--;
	}

// 	logger.info("05 - Shutting down the DeviceDS... ");
// 	delete devDS;
// 	logger.info("DONE!");

	logger.info("05 - Shutting down the DeviceOdo... ");
	delete devOdo;
	logger.info("DONE!");

	logger.info("06 - Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("07 - Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}

int test_deviceardu(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceArdu * devArdu = 0;
	controlbox::device::DeviceFactory * df = 0;
	controlbox::device::DeviceGPS * devGPS = 0;
	controlbox::device::DeviceOdometer * devODO = 0;
	std::ostringstream buff("");

	logger.info("01 - Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("02 - Initializing an CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("03 - Initializing an Arduino for Testing... ");
	devArdu = controlbox::device::DeviceArdu::getInstance();
	devArdu->setDispatcher(cd);
	devArdu->enable();
	logger.info("DONE!");

	df = controlbox::device::DeviceFactory::getInstance();
	logger.info("04 - Getting a GPS instace for Testing... ");
	devGPS = df->getDeviceGPS();
	logger.info("DONE!");

	logger.info("05 - Getting an ODO instace for Testing... ");
	devODO = df->getDeviceODO();
	logger.info("DONE!");

	logger.info("06 - Testing GPS and ODO instances...");
	buff.str("");
	buff << "Distance: " << devODO->distance()
	     << ", Speed: " << devODO->odoSpeed()
	     << ", Fix: " << devGPS->fixStatus()
	     << ", Lon: " << devGPS->longitude()
	     << ", Lat: " << devGPS->latitude()
	     << ", Speed: " << devGPS->gpsSpeed()
	     << ", Course: " << devGPS->course();
	logger.info("=> %s", buff.str().c_str());
	logger.info("DONE!");

	logger.info("07 - Starting Arduino test... ");

	logger.info("Getting  ");

	while (cycles) {
		buff.str("");
		buff << "Distance: " << devArdu->distance()
		     << ", Speed: " << devArdu->odoSpeed(controlbox::device::DeviceOdometer::KMH)
		     << ", Fix: " << devArdu->fixStatus()
		     << ", Lon: " << devArdu->longitude()
		     << ", Lat: " << devArdu->latitude()
		     << ", Speed: " << devArdu->gpsSpeed()
		     << ", Course: " << devArdu->course();
		logger.info("=> %s", buff.str().c_str());

		sleep(sleeptime);
		cycles--;
	}

	logger.info("08 - Shutting down the DeviceArdu... ");
	delete devArdu;
	logger.info("DONE!");

	logger.info("09 - Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("10 - Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}
#endif

int test_deviceatgps(log4cpp::Category & logger) {
	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceATGPS * devATGPS = 0;
	controlbox::device::DeviceFactory * df = 0;
	controlbox::device::DeviceGPS * devGPS = 0;
	controlbox::device::DeviceOdometer * devODO = 0;
	std::ostringstream buff("");

	logger.info("01 - Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("02 - Initializing a CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("03 - Initializing an ATGPS for Testing... ");
	devATGPS = controlbox::device::DeviceATGPS::getInstance();
	devATGPS->setDispatcher(cd);
	devATGPS->enable();
	logger.info("DONE!");

	df = controlbox::device::DeviceFactory::getInstance();
	logger.info("04 - Getting a GPS instace for Testing... ");
	devGPS = df->getDeviceGPS();
	logger.info("DONE!");

	logger.info("05 - Getting an ODO instace for Testing... ");
	devODO = df->getDeviceODO();
	logger.info("DONE!");

	logger.info("06 - Testing GPS and ODO instances...");
	buff.str("");
	buff << "Distance: " << devODO->distance()
	     << ", Speed: " << devODO->odoSpeed()
	     << ", Fix: " << devGPS->fixStatus()
	     << ", Lon: " << devGPS->longitude()
	     << ", Lat: " << devGPS->latitude()
	     << ", Speed: " << devGPS->gpsSpeed()
	     << ", Course: " << devGPS->course();
	logger.info("=> %s", buff.str().c_str());
	logger.info("DONE!");

	logger.info("07 - Starting ATGPS test... ");

	while (cycles) {
		buff.str("");
		buff << "Distance: " << devATGPS->distance()
		     << ", Speed: " << devATGPS->odoSpeed(controlbox::device::DeviceOdometer::KMH)
		     << ", Fix: " << devATGPS->fixStatus()
		     << ", Lon: " << devATGPS->longitude()
		     << ", Lat: " << devATGPS->latitude()
		     << ", Speed: " << devATGPS->gpsSpeed()
		     << ", Course: " << devATGPS->course();
		logger.info("=> %s", buff.str().c_str());

		sleep(sleeptime);
		cycles--;
	}

	logger.info("08 - Shutting down the DeviceATGPS... ");
	delete devATGPS;
	logger.info("DONE!");

	logger.info("09 - Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("10 - Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}

int test_devicete(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceTE * devTE = 0;

	logger.info("01 - Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("02 - Initializing an CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("03 - Initializing a DeviceTE for Testing... ");
	devTE = controlbox::device::DeviceTE::getInstance();
	devTE->setDispatcher(cd);
	devTE->enable();
	logger.info("DONE!");

	logger.info("04 - Starting TE device test [for %d seconds]", sleeptime);

	sleep(sleeptime);

	logger.info("05 - Shutting down the DeviceTE... ");
	delete devTE;
	logger.info("DONE!");

	logger.info("06 - Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("07 - Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}


//#include <ifstream>
/// ATcontrol interface testing
int test_atinterface(log4cpp::Category & logger) {
	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::ATcontrol * at = 0;
	controlbox::QueryRegistry	* qr = 0;

	logger.info("01 - Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("02 - Initializing an CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("03 - Initializing a CommandAT interface for Testing... ");
	at = controlbox::device::ATcontrol::getInstance();
	qr = QueryRegistry::getInstance();
	logger.info("Dump QueryRegistry... %s", qr->printRegistry().c_str());

	at->setDispatcher(cd);
	at->enable();
	logger.info("DONE!");

	logger.info("04 - Starting ControlAT Interface test [%ds]... ", sleeptime);

	sleep(sleeptime);

	logger.info("05 - Shutting down the CommandAT... ");
	delete at;
	logger.info("DONE!");

	logger.info("06 - Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("07 - Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}
