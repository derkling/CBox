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
//** Description:   Enfora GPRS device back-end
//**
//** Filename:      Enfora-h
//** Owner:         Patrick Bellasi
//** Creation date:  28/07/2007
//**
//******************************************************************************
//******************** Revision history ****************************************
//** Revision Date       Comments                           Responsible
//** -------- ---------- ---------------------------------- --------------------
//**
//**
//******************************************************************************


#ifndef _ENFORAAPI_H
#define _ENFORAAPI_H

#include <controlbox/devices/gprs/DeviceGPRS.h>
#include <controlbox/devices/DeviceGPIO.h>
#include <controlbox/base/Utility.h>

#define ENFORAAPI_API_HEADER_SIZE	4
#define ENFORAAPI_API_HEADER_TYPE_POS	2

#define ENFORAAPI_AT_CMD_MAXLEN		256
#define ENFORAAPI_AT_RESP_MAXLEN	4096

// #define ENFORA_SMS_NUMBER		"+393473153808"
#define ENFORA_SMS_NUMBER		""

namespace controlbox {
namespace device {

/// A Enfora is a DeviceGPRS for Enfora's GPRS devices management.
/// This class provide access to Enfora's GSM/GPRS devices connected to
/// a TTY (serial) port. The class provide initialization
/// code and methods to tear up a GPRS connection and accessing
/// to others supported features as: modem state query, send SMS,
/// etc. <br>
/// Supported models are:
/// <ul>
///	<li>Enfora GPRS module &lt;module number&gt;</li>
/// </ul>
/// <br>
/// <h5>Configuration params availables for this class:</h5>
///		Since we could use more than one GPRS module, in the following params [N] must be
///		a single digit number in the range [0-9].
/// <ul>
///	<li>
///	</li>
/// </ul>
/// <br>
/// <i>Notes on GPRS support</i>
/// <ul>
///	<li>
///	</li>
/// <ul>
/// @see DeviceGPRS
class EnforaAPI : public DeviceGPRS  {

public:

protected:

	typedef enum {
		EMPTY = 0,
		REQU,
		GETU,
		ERROR,
		SENDAT,
		GETAT,
		ACK,
		WAKEUP,
		PASSWD
	} t_apiType;

	typedef enum {
		AT_OK = 0,
		AT_ERROR,
		AT_CONNECT
	} t_atResps;

	/// The GPIO device used to handle modem power on-off
	DeviceGPIO * d_gpio;

	/// The command to start the PPP daemon for non-GPRS connection mode
	std::string d_pppdCommand;

	/// Set true when the API is enabled
	bool d_apiEnabled;

	std::string d_curLinkname;

	/// The modem
	int d_modemSocket;

	struct sockaddr_in d_apiLocalAddr;

	struct sockaddr_in d_apiRemoteAddr;

	std::string	d_apiRemoteIP;

	uint16_t	d_apiRemotePort;

	std::string	d_apiLocalIP;

	uint16_t	d_apiLocalPort;

	std::string	d_smsNotifyNumber;

	/// API Commands headers
	static const char *d_apiHeaders[];

	/// The data part of API responce messages.
	/// @note entries of this array should match values of t_atResps
	/// @see t_atResps
	static const char d_atResults[][12];

	/// An API command
	typedef union {
		char raw[ENFORAAPI_AT_CMD_MAXLEN];					///> the complete message
		char header[ENFORAAPI_API_HEADER_SIZE];					///> the header
		struct {
			char num[2];							///> the API number
			char type[1];							///> Command type
			char resrv[1];							///> Reserved
			char data[ENFORAAPI_AT_CMD_MAXLEN-ENFORAAPI_API_HEADER_SIZE];	///> the AT command
		} field;
	} t_apiCommand;

	/// An API responce
	typedef union {
		char raw[ENFORAAPI_AT_RESP_MAXLEN];					///> the complete responce
		char header[ENFORAAPI_API_HEADER_SIZE];					///> the header
		struct {
			char num[2];							///> the API number
			char type[1];							///> Command type
			char resrv[1];							///> Reserved
			char data[ENFORAAPI_AT_RESP_MAXLEN-ENFORAAPI_API_HEADER_SIZE];	///> the AT command responce
		} field;
	} t_apiResponce;

public:

	/// Create a new Enfora GPRS device initially disabled.
	/// In order to enable it you need to attach a Dispatcher.
	/// The new GPRS device, as default logger category, has
	/// the class namespace "controlbox.comlibs.Enfora"
	/// @param module the module number, used to look for configuration
	///		params. It <b>must be<b> a single digit number in the
	///		range [0-9]
	/// @param logName the log category, this name is prepended by the
	///		class namespace "controlbox.comlibs."
	/// @throw SerialConfigurationException on failing accessing the TTY port
	EnforaAPI(short module, std::string const & logName = "Enfora")
		throw (exceptions::SerialConfigurationException*);

	/// Class destructor.
	~EnforaAPI();

	/// Configure the GPRS.
	exitCode initDeviceGPRS();

	/// Start the parser thread.
	exitCode runParser();

protected:

	/// Power-on the device using GPIO interface
	exitCode powerOn(bool reset = false);

	/// Power-off the device using GPIO interface
	exitCode powerOff();

	/// Load the GPRS configuration.
	exitCode loadConfiguration();

	/// Enable non-GPRS connection for the specified linkname
	exitCode enableAPI();

	exitCode disableAPI();

	exitCode initSocket();

	/// Send an AT command using the API.
	///
	exitCode sendAT(t_apiCommand & msg, t_apiResponce & resp);

	exitCode getResponce(t_apiResponce & resp);

	exitCode addHeader(t_apiCommand & msg, t_apiType t, bool toString = false);

	exitCode updateNetworkConfiguration(void);

	exitCode gprsUP(std::string const & linkname);

	exitCode gprsDOWN();

	exitCode tryConnect(std::string const & linkname);

	exitCode connect(std::string const & linkname);

	exitCode disconnect();

	void pppNotifyState(bool running);

	exitCode sendSMS(std::string number, std::string text);

	exitCode signalLevel(unsigned short & level);

	exitCode gprsStatus(unsigned short & status);

	exitCode gprsRegistered(bool resetOnFailure = true);

	/// Notify friends servers about a network configuration change.
	exitCode notifyFriends();

	/// The thread body: this will hear for unsolicited messages coming from
	/// the modem
	void run (void);


};

}// namespace gprs
}// namespace controlbox
#endif
