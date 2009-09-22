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
#include <cc++/thread.h>

#define __PROVA__



#include "controlbox/base/comsys/EventDispatcher.h"
#include "controlbox/base/comsys/EventGenerator.h"
#include "controlbox/base/comsys/Dispatcher.h"
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
int test_loglibs(log4cpp::Category & logger);
int test_comlibs(log4cpp::Category & logger);
int test_utils(log4cpp::Category & logger);
int test_threads(log4cpp::Category & logger);
int test_devicedb(log4cpp::Category & logger);
int test_command(log4cpp::Category & logger);
int test_wsproxy(log4cpp::Category & logger);
// int test_nmeaparser(log4cpp::Category & logger);
int test_devicegprs(log4cpp::Category & logger);
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

bool useColors = true;

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
			{"loglibstest", no_argument, 0, 'L'},
			{"comlibstest", no_argument, 0, 'l'},
			{"devdbtest", no_argument, 0, 'e'},
			{"commandtest", no_argument, 0, 'd'},
			{"wsproxytest", no_argument, 0, 'w'},
			{"threads", no_argument, 0, 'm'},
			{"gprstest", no_argument, 0, 'n'},
			{"astest", no_argument, 0, 'a'},
			{"gpiotest", no_argument, 0, 'o'},
			{"dstest", no_argument, 0, 'b'},
			{"atgpstest", no_argument, 0, 'g'},
			{"tetest", no_argument, 0, 'i'},
			{"attest", no_argument, 0, 't'},
			{"utilstest", no_argument, 0, 'u'},
			{"nocolors", no_argument, 0, 'y'},
			{0, 0, 0, 0}
		};
	static const char * optstring = "abC:c:dehgilLmnors:tuwy";
	int c;
	bool silent = false;

	// TESTS to do
	bool printHelp = true;
	bool testLoglibs = false;
	bool testComlibs = false;
	bool testUtils = false;
	bool testThreads = false;
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

// log4cpp::Category & pupazza = log4cpp::Category::getInstance("cboxtest");
// test_utils(pupazza);
// return 0;

	// Logger configuration
	std::string confFilename = "/etc/cbox/cbox.conf";

#ifdef CONTROLBOX_DEBUG
	// Forcing unbuffered stdout
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
			case 'L':
				testLoglibs = true;
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
			case 'm':
				testThreads = true;
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
			case 'u':
				testUtils = true;
				printHelp = false;
				break;
			case 'h':
				print_usage(argv[0]);
				return EXIT_SUCCESS;
			case 'y':
				useColors = false;
				break;
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
	cout << "\t\t+   ControlBox   -   by Patrick Bellasi <derkling@gmail.com>   +" << endl;
	cout << "\t\t++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
	cout << endl;

	// Printing versioning info
	cout << "S/W ver. " << PACKAGE_VERSION << " (";
	cout << "Build: " << __DATE__ << " " << __TIME__ << ")" << endl;

	// Initializing Configurator
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

	logger.debug("Dumping memory statistics (/tmp/memstats)");
	system("cat /proc/meminfo > /tmp/memstats");

	logger.debug("Runnig tests...");

	if (testLoglibs) {
		logger.debug("----------- Testing loglibs ---");
		test_loglibs(logger);
	}
	if (testComlibs) {
		logger.debug("----------- Testing comlibs ---");
		test_comlibs(logger);
	}
	if (testUtils) {
		logger.debug("----------- Testing Utilities ---");
		test_utils(logger);
	}
	if (testThreads) {
		logger.debug("----------- Testing Threads ---");
		test_threads(logger);
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
		test_devicegprs(logger);
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

	logger.debug("Upadting memory statistics (/tmp/memstats)");
	system("cat /proc/meminfo >> /tmp/memstats");

	logger.debug("----------- Tests Completed ---\n");
	log4cpp::Category::shutdown();


	return EXIT_SUCCESS;

}

/// Print the Help menu
void print_usage(char * progname) {

	cout << "cBox ver. " << PACKAGE_VERSION << " (";
	cout << "Build: " << __DATE__ << " " << __TIME__ << ")" << endl;
	cout << "\tUsage: " << progname << " [options]" << endl;
	cout << "\tOptions:" << endl;
	cout << "\t-h, --help                 Print this help" << endl;
	cout << "\t-r, --release              Force don't print debug messages" << endl;
	cout << "\t-s, --sleep <seconds>      Seconds to wait between cycles" << endl;
	cout << "\t-C, --configuration        Configuration file path" << endl;
	cout << "\t-c, --cycles <count>       Test cycles number" << endl;
	cout << "\t-y, --nocolors             Disable colors on output" << endl;
	cout << "\t-l, --comlibstest          Do a Test on comlibs" << endl;
	cout << "\t-L, --loglibstest          Do a Test on logging libraries" << endl;
	cout << "\t-u, --utilstest            Do a Test on Utilities" << endl;
	cout << "\t-m, --threadstest          Do a Test on Threads" << endl;
	cout << "\t-e, --devdbtest            Do a Test on DeviceDB" << endl;
	cout << "\t-d, --commandtest          Do a Test on DaricomCommand" << endl;
	cout << "\t-w, --wsproxytest          Do a Test on WSProxyCommandHandler" << endl;
	cout << "\t-g, --atgpstest            Do a Test on DeviceATGPS" << endl;
	cout << "\t-o, --gpiotest             Do a Test on DeviceGPIO" << endl;
	cout << "\t-n, --gprstest             Do a Test on DeviceGPRS" << endl;
	cout << "\t-a, --astest               Do a Test on DeviceAS" << endl;
	cout << "\t-b, --bstest               Do a Test on DeviceDS" << endl;
	cout << "\t-i, --tetest               Do a Test on DeviceTE" << endl;
	cout << "\t-t, --attest               Do a Test on the AT interface" << endl;

	cout << "\nPatrick Bellasi - derkling@gmail.com\n" << endl;

}

//#include <log4cpp/StringUtil.hh>

/// Loglibs TEST case
int test_loglibs(log4cpp::Category & logger) {
	int i;
	ostringstream ostr("");
	controlbox::device::FileWriterCommandHandler * fw = 0;
	const char *pstr;
	const std::string message;
	std::string logName("PollEventGenerator");
	char buffer[1024];
	log4cpp::Category & log(log4cpp::Category::getInstance(std::string("controlbox.comlibs.command.PollEventGener")));

	logger.info("Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("/tmp/filewriter.log");
	logger.info("DONE!");
	logger.info("");

	logger.info("01 - Testing Category::info (static)...");
	logger.info("This is a plain string");
	logger.info("This is a string with some parameter, cycles: %d", cycles);
	logger.info("");

	logger.info("02 - Testing new category...");
	log.info("new category log sentence");
	logger.info("");

	logger.info("03 - Testing Category::info (dynamic)...");
	ostr.str("");
	for (i=0; i<cycles; i++) {
		ostr << "1bcd5fghi0";
		pstr = ostr.str().c_str();

		cout << "COUT ostr: " << (i+1)*10 << " " << ostr.str() << endl;
		cout << "COUT pstr: " << (i+1)*10 << " " << pstr << endl;
		logger.info("c %s", pstr);
	}
	logger.info("");

	logger.info("04 - Testing macros...");
	LOG4CPP_DEBUG(log, "Command::setTheParam(lable=%s, logName=%d, cycles=%s)", logName.c_str(), (cycles > 3) ? "Many" : "Few" );
	logger.info("");

	logger.info("05 - Testing log4cpp using pre-formatted buffer...");
	snprintf(buffer, 1024, "Command::setTheParam(lable=%s, logName=%d, cycles=%s)", logName.c_str(), cycles, (cycles > 3) ? "Many" : "Few" );
	cout << "BUFFER: " << buffer << endl;
	logger.debug(buffer);
	logger.info("");

	logger.info("06 - Testing log4cpp with on-line formatting...");
	logger.debug("Command::setTheParam(lable=%-s, logName=%-d, cycles=%-s)", logName.c_str(), cycles, (cycles > 3) ? "Many" : "Few" );
	logger.info("");

	logger.info("DONE!");

	return 0;

}


/// Comlibs TEST case
int test_comlibs(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::PollEventGenerator * peg = 0;
	controlbox::comsys::Command * command = 0;

	unsigned i,j;
	char lable[] = "Param100\0";

#define MEM_CONSUMING_STRING	""							\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"	\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"	\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"	\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"	\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"	\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"	\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"	\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"	\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"	\
"VeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongStringVeryLongString"


	logger.info("Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("/tmp/filewriter.log");
	logger.info("DONE!");

	logger.info("Initializing a Command... ");
		command = controlbox::comsys::Command::getCommand(1, Device::EG_POLLER, "PollEventGenerator", "PollEventGenerator");
	command->setParam( "StringParam", "ParamValue" );
	command->setParam( "IntParam", 2 );
	command->setParam( "FloatParam",  1.23 );
	logger.info("DONE!");

	logger.info("Initializing a CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	cd->setDefaultCommand(command);

	logger.info("Initializing a PollEventGenerator... ");
	peg = new controlbox::device::PollEventGenerator(2000, cd);
	logger.info("DONE!");

#if 0
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
#endif

	logger.info("Shutting down the PollEventGenerator... ");
	peg->disable();
	delete peg;
	logger.info("DONE!");

	logger.info("Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");



	logger.info("Testing Command MEMORY handling... ");

	logger.info("Initializing a CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	do {
		command = controlbox::comsys::Command::getCommand(1, Device::EG_POLLER, "MC1", "MemoryCheck");

		for (i=0; i<=(cycles/10); i++) {
// 			logger.info("Loading long string (%d bytes)...", strlen(MEM_CONSUMING_STRING));
			command->setParam( "LongStringValue", MEM_CONSUMING_STRING, false);
// 			logger.info("Loading doubles (100)...");
			for ( j=0; j<100; j++) {
				sprintf(lable, "Param%03d", j);
				command->setParam( lable, (double)j, false );
			}
		}
		logger.warn("DISPATCH -#%03d...", cycles);
		cd->dispatchCommand(command);

		usleep(sleeptime);

	} while(--cycles);


	logger.info("Initializing a CommandDispatcher... ");
	cd = new controlbox::comsys::CommandDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("Shutting down the FileWriterCommandHandler... ");
	delete fw;
	logger.info("DONE!");

	return 0;

}

/// Utilities TEST case
int test_utils(log4cpp::Category & logger) {
	typedef struct {
		u8 c[512];
		unsigned l;
	} buffs_t;
	buffs_t in_buf[] = {
		{
		{}, 0 },
		{
		{'A'}, 1 },
		{
		{'A', 'B'}, 2 },
		{
		{'A', 'B', 'C'}, 3 },
		{
		{'A', 'B', 'C', 'D'}, 4 },
		{
		{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
		0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
		0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
		0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
		0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
		0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41,
		0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C,
		0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
		0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62,
		0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D,
		0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
		0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83,
		0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
		0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
		0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4,
		0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
		0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA,
		0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5,
		0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
		0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB,
		0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,
		0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1,
		0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC,
		0xFD, 0xFE, 0xFF,}, 256 },
		{ // Risposta SAMPI 550 comando 28
		{	0x01,						// Tipo evento (01: accensione, 02:spegnimento, 06 Erogazione, 05 Carico)
			0x01, 0x00,					// Progressivo di questo tipo di evento (in questo caso accensione) Little Endian
			0x0a, 0x11, 0x28, 0x0b, 0x06, 0xd7, 0x07,	// (Timestamp 10:11:28 11/06/2007)
			0x01,						// Riservato
			0x0b, 0xf4,					// Checksum riservato
			//Dati Evento - Tipo 01: accensione (165 bytes)
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 1 (double)
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 2
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 3
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 4
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 5
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 6
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 7
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 8
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 9
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 10
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 11
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore n. 12
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore Trafilamento n. 1
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore Trafilamento n. 2
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore Trafilamento n. 3
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Totalizzatore Trafilamento n. 4
			0x00,						// unità di misura usata (0: litri 0: kg)
			0x47, 0x50, 0x4C,				// Sigla prodotto 1 ('G','P','L');
			0x20, 0x20, 0x20,				// Sigla prodotto 2 (' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 3 (' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 4 (' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 5 (' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 6 (' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 7 (' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 8 (' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 9 (' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 10 (' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 11(' ',' ',' ');
			0x20, 0x20, 0x20,				// Sigla prodotto 12 (' ',' ',' ');
			// Padding del buffer /filling up 205 bytes)
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			// Message trailer
			0x34, 0x36,				// Checksum del messaggio (64)
			0x0d,					// Fine frame
			}, 221},
		{
		{0x02, 0x00, 0x0A, 0x1C, 0x01, 0x01, 0x00, 0x5D, 0x06, 0x22, 0x00,
		0x0B, 0x3B, 0x33, 0x19, 0x09, 0xFF, 0x07, 0x01, 0xFF, 0xFF, 0x01,
		0x22, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x0B, 0x3B, 0x1E, 0x19, 0x09, 0xFF, 0x07, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0x03, 0x00, 0x00,
		0x33, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0xFF, 0x11,
		0x41, 0x00, 0x00, 0x00, 0x00, 0x68, 0xFF, 0x11, 0x41, 0x24, 0x04,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x5A, 0x11, 0x41, 0x00,
		0x00, 0x00, 0x00, 0x04, 0x4A, 0x11, 0x41, 0x7C, 0x02, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x04, 0x41, 0x00, 0x00, 0x00,
		0x00, 0x18, 0xFF, 0x04, 0x41, 0x00, 0x60, 0xFF, 0x44, 0x00, 0x00,
		0xFF, 0x3F, 0x62, 0xFF, 0x3C, 0x3B, 0x00, 0x00, 0x16, 0x44, 0x5C,
		0xFF, 0xFF, 0x41, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
		0x00, 0x03, 0x09, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10,
		0x28, 0x45, 0x55, 0x52, 0x29, 0x20, 0x47, 0x50, 0x4C, 0x20, 0x20,
		0x20, 0x20, 0x20, 0x4D, 0x45, 0x54, 0x45, 0x52, 0x20, 0x31, 0x20,
		0x20, 0x41, 0x44, 0x52, 0x43, 0x4F, 0x44, 0x45, 0x30, 0x31, 0x00,
		0x10, 0xFF, 0xFF, 0x57, 0x00, 0xFF, 0x30, 0x57, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x25,
		0x35, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x00, 0x00, 0x10,
		0x60, 0x03, 0x78, 0x0F, 0x24, 0x00, 0x35, 0x39, 0x0D}, 229},
		{
		{"Many thanks to Bob Trower for this wonderfoul Base64 transcoder"}, 63}
	};
	unsigned tests = sizeof(in_buf)/sizeof(buffs_t);
	unsigned t = 0;
	//std::ostringstream hex_buf;
	char hex_buf[4096];
	char out_buf[512];
	char str_buf[4096];
	unsigned pos = 0;
	unsigned int i;
// 	unsigned int j;
	unsigned len;
// 	bool makeSpace = false;


// write(2, in_buf[7].c, in_buf[7].l);
// return 0;

//Utils::b64enc("Ciao Mondo\n\r", 12, out_buf, 512);
//cout << out_buf << endl;
//return 0;

	while (t<tests) {

		//hex_buf.str("");
		hex_buf[0] = 0;
		if ( isprint(in_buf[t].c[0]) ) {
			for (pos=0, i=0; i<in_buf[t].l && pos<(4096-7); i++) {
				pos += sprintf(hex_buf+pos, "%c", in_buf[t].c[i]);
				if ( ! ((i+1)%99) ) {
					pos += sprintf(hex_buf+pos, "\n\t\t\t\t\t\t\t");
				}
			}
		} else {
			for (pos=0, i=0; i<in_buf[t].l && pos<(4096-7); i++) {
				pos += sprintf(hex_buf+pos, "0x%02X ", in_buf[t].c[i]);
				if ( ! ((i+1)%20) ) {
					pos += sprintf(hex_buf+pos, "\n\t\t\t\t\t\t\t");
				}
			}
		}

		logger.info("Testing Base64 transcoder on buffer [size: %u]:\n\t\t\t\t\t\t\t%s", in_buf[t].l, hex_buf);

		Utils::b64enc((char*)in_buf[t].c, in_buf[t].l, out_buf, 512);
		//hex_buf.str("");
		str_buf[0] = 0;
		for (pos=0, i=0; out_buf[i] && pos<(4096-7); i++) {
			//hex_buf << out_buf[i]; j++;
			pos += sprintf(str_buf+pos, "%c", out_buf[i]);
			if ( ! ((i+1)%(99)) ) {
				//hex_buf << "\n\t\t\t\t\t\t\t";
				pos += sprintf(str_buf+pos, "\n\t\t\t\t\t\t\t");
			}
		}
		logger.info("Encoded buffer:\n\t\t\t\t\t\t\t%s", str_buf);

		in_buf[t].c[0] = 0; len=512;
		Utils::b64dec(out_buf, (char*)in_buf[t].c, len);

		//hex_buf.str("");
		hex_buf[0] = 0;
		for (pos=0, i=0; i<len; i++) {
			//hex_buf << "0x" << setw(2) << hex << setfill('0') << (unsigned short)in_buf[t].c[i] << ", ";
			pos += sprintf(hex_buf+pos, "0x%02X, ", in_buf[t].c[i]);
			if ( ! ((i+1)%20) ) {
				//hex_buf << "\n\t\t\t\t\t\t\t";
				pos += sprintf(hex_buf+pos, "\n\t\t\t\t\t\t\t");
			}
		}
		logger.info("Decoded buffer [size: %u]:\n\t\t\t\t\t\t\t%s", len, hex_buf);

		t++;
	};

	return 0;
}

/// Threads TEST case
int test_threads(log4cpp::Category & logger) {

	class SimpleEG : public controlbox::comsys::EventGenerator,
			public controlbox::Worker {
	protected:
		unsigned d_cycles;
		log4cpp::Category & log;
	public:
	SimpleEG(controlbox::comsys::Dispatcher * dispatcher, bool enabled, std::string const & logName) :
		controlbox::comsys::EventGenerator(dispatcher, enabled, logName),
		Worker(Object::log, "cbw_SEG", 0),
		d_cycles(2),
		log(Object::log) {
	}

	// An example of Thread run method
	void run (void) {
		exitCode result;
		unsigned worktime;

		suspendWorker();
		while( !d_doExit ) {

			// Random number between 1 and 10
			// (using high-order bits)
			worktime = 1 + (int) (10.0 * (rand() / (RAND_MAX + 1.0)));
			LOG4CPP_INFO(log, "Doing some work [%u]\n", worktime);

			pollWorker(worktime*1000);

		}

	}

	};

	SimpleEG *seg1 = 0;
	SimpleEG *seg2 = 0;
	controlbox::comsys::EventDispatcher * ed = 0;
	controlbox::device::FileWriterCommandHandler * fw = 0;
	unsigned cycle;

	logger.info("01 - Initializing a FileWriterCommandHandler... ");
	fw = new controlbox::device::FileWriterCommandHandler("./filewriter.log");
	logger.info("DONE!");

	logger.info("02 - Initializing an EventDispatcher... ");
	ed = new controlbox::comsys::EventDispatcher(fw, false);
	logger.info("DONE!");

	logger.info("03 - Building the first thread [SEG-1]...");
	seg1 = new SimpleEG(ed, true, "SEG-1");

	logger.info("04 - Building the second thread [SEG-2]...");
	seg2 = new SimpleEG(ed, true, "SEG-2");

	logger.info("05 - Running threads...");
	seg1->runWorker();
	seg2->runWorker();

	::sleep(2);
	for (cycle=2; cycle; cycle--) {

		logger.info("07 - Sending signal to second thread... ");
		seg2->signalWorker();
		logger.info("08 - Sending signal to first thread... ");
		seg1->signalWorker();
		logger.info("09 - Waiting 5s seconds... ");
		::sleep(10);
	}


	logger.info("10 - Waiting for all threads to complete... ");
	seg1->terminateWorker();
	seg2->terminateWorker();

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
	command = controlbox::comsys::Command::getCommand(1, Device::DEVICE_TE, "Sampi500", "Sampi500");
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
	proxy->notifyCommand(command);
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
int test_devicegprs(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceGPRS * devGPRS = 0;
	unsigned short level;

	// Ensuring there are not running PPPD deamons that lock modems TTY's ports
	system("killall pppd");

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

	sleep(5);
	logger.info("Connecting GPRS...");
	if ( devGPRS->connect("tinlink") == OK ) {
		logger.info("---- Connection UP AND RUNNRING");
	} else {
		logger.error("---- Connection PROBLEMS");
	}

	sleep(2);
	logger.info("Testing signal level...");
	devGPRS->signalLevel(level);
	logger.info("Signal level [%u]", level);


	logger.info("---- The system is ready for network testing! Repeatly connection [%d times, one time each %d seconds]...", cycles, sleeptime);
	while ( cycles-- ) {
		sleep(sleeptime);
		if ( devGPRS->connect("tinlink") == OK ) {
			logger.info("---- Connection UP AND RUNNRING");
		} else {
			logger.error("---- Connection PROBLEMS");
		}
	}


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
	devGPIO->gprsPowerOn(device::DeviceGPIO::GPRS1);

	logger.info("05 - Testing TTY's mutex... ");

	logger.info("Locking TTY1... ");
	devGPIO->ttyLock(device::DeviceGPIO::TTYMUX_PORT1);
// 	logger.info("Locking TTY2... ");
// 	devGPIO->ttyLock(device::DeviceGPIO::TTY2_PORT3);

	logger.info("06 - Waiting %d [s]... ", sleeptime);
	::sleep(sleeptime);

	logger.info("UnLocking TTY1... ");
	devGPIO->ttyUnLock(device::DeviceGPIO::TTYMUX_PORT1);
// 	logger.info("UnLocking TTY2... ");
// 	devGPIO->ttyUnLock(device::DeviceGPIO::TTY2_PORT3);

	logger.info("06 - Powering Down GPRS... ");
	devGPIO->gprsPowerOff(device::DeviceGPIO::GPRS1);


	logger.info("07 - Shutting down the DeviceDS... ");
//	delete devGPIO;
	logger.info("DONE!");

	return 0;

}

/// Digital Sensors testing
int test_deviceds(log4cpp::Category & logger) {

	controlbox::device::FileWriterCommandHandler * fw = 0;
	controlbox::comsys::CommandDispatcher * cd = 0;
	controlbox::device::DeviceDigitalSensors * devDS = 0;
	controlbox::device::DeviceATGPS * devATGPS = 0;
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

	logger.info("04 - Initializing an ATGPS for Testing... ");
	devATGPS = controlbox::device::DeviceATGPS::getInstance();
	devATGPS->setDispatcher(cd);
	devATGPS->enable();
	logger.info("DONE!");

	logger.info("05 - Starting Digital Sensors test... ");


	sleep(sleeptime);


	logger.info("06 - Shutting down the DeviceATGPS... ");
	delete devATGPS;
	logger.info("DONE!");

	logger.info("07 - Shutting down the DeviceDS... ");
	delete devDS;
	logger.info("DONE!");

	logger.info("08 - Shutting down the CommandDispatcher... ");
	delete cd;
	logger.info("DONE!");

	logger.info("09 - Shutting down the FileWriterCommandHandler... ");
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
	buff << "Distance: " << std::setw(10) << std::setfill('0') << devODO->distance()
	     << ", Speed: "  << std::setw(4)  << std::setfill('0') << devODO->odoSpeed()
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
		buff << "Distance: " << std::setw(10) << std::setfill('0') << devATGPS->distance()
		     << ", Speed: "  << fixed << setprecision(2) << std::setw(6) << devATGPS->odoSpeed(controlbox::device::DeviceOdometer::KMH)
		     << ", Fix: " << devATGPS->fixStatus()
		     << ", Lon: " << devATGPS->longitude()
		     << ", Lat: " << devATGPS->latitude()
		     << ", Speed: " << devATGPS->gpsSpeed()
		     << ", Course: " << devATGPS->course();
		logger.info("=> %s", buff.str().c_str());

		// It's done by the ATGps Thread...
// 		logger.info("Checking alarms...");
// 		devATGPS->checkAlarms(false);

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
