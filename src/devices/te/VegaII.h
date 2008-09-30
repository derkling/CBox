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


#ifndef _VEGAIITE_H
#define _VEGAIITE_H

#include <controlbox/base/Utility.h>
#include <controlbox/devices/te/DeviceTE.h>

#define VEGAII_MAX_CMD_LEN	18
#define VEGAII_MAX_RSP_LEN	255

namespace controlbox {
namespace device{

/// A VegaII is a DeviceTE for VEGAII's "Testate Elettroniche".
/// This class provide access to VEGAII's TE devices connected to
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
class VegaII : public DeviceTE  {

  protected:

	/// A TE command
	struct teCmd {
		char cmd [VEGAII_MAX_CMD_LEN];	///> The HEX rapresentation of the TE command
		unsigned len;			///> The length of the hex command
	};
	typedef struct teCmd t_teCmd;

	/// The array of availables commands.
	/// @see t_teCmd
	static t_teCmd d_teCmd[];

	static const char *eventTypeLable[];

	/// Nak code lables
	static const char *nakCodeLable[];

	/// The type of availables commands.
	/// @note this is used as index for the d_teCmd command array
	/// @see d_teCmd
	enum cmdType {
		CMD_31 = 0,	///> Command 31: master ACK
		CMD_32,		///> Command 32: master NAK
		CMD_34,		///> Command 34: download new event
	};
	typedef enum cmdType t_cmdType;


  public:

	/// Create a new Sampi TE device initially disabled.
	/// In order to enable it you need to attach a Dispatcher.
	/// @param logName the log category, this name is prepended by the
	///		class namespace "daricom.comlibs."
	VegaII(std::string const & logName = "VegaII");

	/// Class destructor.
	~VegaII();

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
		return DeviceTE::SEND_GENERIC_VEGAII;
	};

	/// Compute a buffer's checksum.
	/// The checksum could be differently computed if we send
	/// a message to the TE or if we receive it from the TE.
	/// @param buf the buffer to witch the checksum should be computed
	/// @param len the size of the buffer
	/// @param chksum the computed checksum returned
	/// @param tx if true compute checksum for a message being sent
	///	to the TE, by default it compute checksum of a received message.
	/// @return OK on checksum computation success.
	inline exitCode checksum(const char * buf, unsigned len, char chksum[3]);

	/// Verify the checksum of the passed message.
	/// @param msg the message to verify
	/// @param len the lenght of the passed message
	/// @return true if the received message checksum is valid
	bool verifyChecksum(const char * msg, unsigned len);

	exitCode queryNewEvent(char * event, int & len);

	exitCode sendACK();

	exitCode checkNAK(char * event);

	exitCode logEvent(char * p_event);

};

}// namespace device
}// namespace controlbox
#endif

