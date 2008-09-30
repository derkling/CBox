//*********************************************************************
//*************  Copyright (C) 2006        DARICOM  ********************
//*********************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** DARICOM The programs may be used and/or copied only with the
//** written permission from DARICOM or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//*********************************************************************
//******************** Module information *****************************
//**
//** Project:       ProjectName (ProjectCode/Version)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Developer
//** Creation date:  21/06/2006
//**
//*********************************************************************
//******************** Revision history *******************************
//** Revision date       Comments                           Responsible
//** -------- ---------- ---------------------------------- -----------
//**
//**
//*********************************************************************


#ifndef _SAMPI550TE_H
#define _SAMPI550TE_H

#include <controlbox/base/Utility.h>
#include <controlbox/devices/te/DeviceTE.h>

#define SAMPI550_MAX_CMD_LEN	18
#define SAMPI550_MAX_RSP_LEN	255

namespace controlbox {
namespace device{

/// A Sampi550 is a DeviceTE for SAMPI550's "Testate Elettroniche".
/// This class provide access to SAMPI550's TE devices connected to
/// a TTY (serial) port. The class provide initialization
/// code and methods to tear up a TE connection and provide support
/// for TE model's specific features: package format handling,
/// checksum computation, etc. <br>
/// Supported models are:
/// <br>
/// <h5>Configuration params availables for this class:</h5>
/// <ul>
///	<li>
///		<b>paramname</b> - <i>Default: SAMPITE550_DEFAULT_PARAMNAME</i><br>
///		Parameter description
///	</li>
/// </ul>
/// @see DeviceTE
class Sampi550 : public DeviceTE  {


  protected:

	/// A TE command
	struct teCmd {
		char cmd [SAMPI550_MAX_CMD_LEN];	///> The HEX rapresentation of the TE command
		unsigned len;				///> The length of the hex command
	};
	typedef struct teCmd t_teCmd;

	/// The array of availables commands.
	/// @see t_teCmd
	static t_teCmd d_teCmd[];

	/// The type of availables commands.
	/// @note this is used as index for the d_teCmd command array
	/// @see d_teCmd
	enum cmdType {
		CMD_28 = 0,	///> Command 28: download next event marked as "downloadable"
		CMD_29,		///> Command 29: mark an event as downloaded
	};
	typedef enum cmdType t_cmdType;

	struct teFrame28_rx {
		u8	ack;
		u8	tkn;
		u8	addr;
		u8	cmd;
		u8	found;
		u8	upd;
		u16	num;
		u8	data[205];
		u16	chkh;
		u8	chkl;
		u8	end;
	};
	typedef struct teFrame28_rx t_teFrame28_rx;

	/// Types of events this TE could generate
	enum eventType {
		EMPTY = 0x00,
		POWERON,
		POWEROFF,
		POFFPULSES,
		ERROR,
		LOADING,
		DELIVERY,
		LEAKAGE = 0x08,
		DELETEDLOAD,
		HWTEST,
		CRASH = 0x0C,
		RESET,
		BATCH,
	};
	typedef enum eventType t_eventType;

	/// Event type name
	/// @note this should match values on t_eventType
	static const char *eventTypeLable[];

  public:

	/// Create a new Sampi TE device initially disabled.
	/// In order to enable it you need to attach a Dispatcher.
	/// @param logName the log category, this name is prepended by the
	///		class namespace "daricom.comlibs."
	Sampi550(std::string const & logName = "Sampi550");

	/// Class destructor.
	~Sampi550();

  protected:


	/// Download events from the TE.
	/// This method should be implemented by sub-classes to download
	/// all new events from the TE device and it's expected to store them
	/// into the provided  list.
	exitCode downloadEvents(t_eventList & eventList);

	/// Format an event.
	/// This method should parse the event string and fill some of the command
	/// parameters; this method is expected to return at least those parameters:
	/// <ul>
	///	<li>timestamp</li>
	///	<li>dist_event</li>
	///	<li>dist_msgType</li>
	/// </ul>
	exitCode formatEvent(t_event const & event, comsys::Command & cmd);

	/// Return the command type associated to a generic TE event.
	/// This method should be implemented by sub-classes simply returning
	/// the command type corresponding to the specific device.
	inline DeviceTE::t_cmdType cmdTypeGenericCode() {
		return DeviceTE::SEND_GENERIC_SAMPI550;
	};

	/// Compute a buffer's checksum.
	/// The checksum is obtained by adding the ASCII value of all characters
	/// of the message, mod 256. The alphabetical hexadecimal characters
	/// (non numerical) of the checksum (A, B, C, D, E, F) should be
	/// represented as capital letters. The two bytes are swapped placing
	/// first the lower byte followed by the higher one.
	/// @param buf the buffer to witch the checksum should be computed
	/// @param len the size of the buffer
	/// @param chksum the computed checksum returned.
	/// @return OK on checksum computation success.
	exitCode checksum(const char * buf, unsigned len, char chksum[3]);

	/// Verify the checksum of the passed message.
	/// @param msg the message to verify
	/// @param len the lenght of the passed message
	/// @return true if the received message checksum is valid
	bool verifyChecksum(const char * buf, unsigned len);

	exitCode queryNewEvent(char * event, int & len);

	exitCode markMessageDownloaded(u16 loc, char * resp, int & len);

};

}// namespace device
}// namespace controlbox
#endif

