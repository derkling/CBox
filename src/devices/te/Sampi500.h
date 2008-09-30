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


#ifndef _SAMPI500TE_H
#define _SAMPI500TE_H

#include <controlbox/base/Utility.h>
#include <controlbox/devices/te/DeviceTE.h>

#define NAK	0x15

#define DEVICETE_DEFAULT_RECORDFORMAT_A "02"

#define SAMPI500_MAX_CMD_LEN	10

namespace controlbox {
namespace device{

/// A Sampi500 is a DeviceTE for SAMPI500's "Testate Elettroniche".
/// This class provide access to SAMPI500's TE devices connected to
/// a TTY (serial) port. The class provide initialization
/// code and methods to tear up a TE connection and provide support
/// for TE model's specific features: package format handling,
/// checksum computation, etc. <br>
/// Supported models are:
/// <br>
/// <h5>Configuration params availables for this class:</h5>
/// <ul>
///	<li>
///		<b>device_te_recordFormat_A</b> - <i>Default: DEVICETE_DEFAULT_RECORDFORMAT_A</i><br>
///		The counters exported by an 'A' record in the correct order as they are
///		present in the downloaded reord.
///		This params must be a string of hexadecimal digits, meaningful values are:
///		<ul>
///			<li>0 - gasolio</li>
///			<li>1 - gasolio "BluDiesel"</li>
///			<li>2 - benzina</li>
///			<li>3 - benzina "BluSuper"</li>
///			<li>4 - GPL</li>
///			<li>5 - bitume liquido</li>
///		</ul>
///		@see
///		<br>
///	</li>
/// </ul>
/// @see DeviceTE
class Sampi500 : public DeviceTE  {

  public:

	/// Create a new Sampi TE device initially disabled.
	/// In order to enable it you need to attach a Dispatcher.
	/// @param logName the log category, this name is prepended by the
	///		class namespace "daricom.comlibs."
	Sampi500(std::string const & logName = "Sampi500");

	/// Class destructor.
	~Sampi500();

  protected:

	/// A TE command
	struct teCmd {
		char cmd [SAMPI500_MAX_CMD_LEN];	///> The HEX rapresentation of the TE command
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
		C54_STX = 0,	///> Command 54: start trasmission
		C54_ACK,	///> Command 54: ACK last received responce
	};
	typedef enum cmdType t_cmdType;

	/// @see verboseNak
	enum teResps {
		ACK = 0,
		NAK_NO_NEW_EVENT,
		NAK_CHECKSUM_ERROR,
		NAK_MISSING_TERMINATOR,
		NAK_START_ERROR,
		NAK_UNKNOWEN
	};
	typedef enum teResps t_teResps;

	exitCode downloadEvents(t_eventList & eventList);

	exitCode formatEvent(t_event const & event, comsys::Command & cmd);

	inline exitCode formatEvent_Raw(const char * evt, comsys::Command & cmd);

	inline exitCode formatEvent_PowerUp(const char * evt, comsys::Command & cmd);

	inline exitCode formatEvent_Shutdown(const char * evt, comsys::Command & cmd);

	inline exitCode formatEvent_Load(const char * evt, comsys::Command & cmd);

	inline exitCode formatEvent_Download(const char * evt, comsys::Command & cmd);

	inline DeviceTE::t_cmdType cmdTypeGenericCode() {
		return DeviceTE::SEND_GENERIC_SAMPI500;
	};

	/// Send a command to the TE.
	/// @param type the command to send
	/// @see d_teCmd
	inline exitCode write(t_cmdType type);

	/// Read TE data
	inline unsigned read(char * resp, int len);

	/// Compute a message's checksum.
	/// The checksum could be differently computed if we are send
	/// a message to the TE or if we received it from the TE.
	/// @param tx if true compute checksum for a message being sent
	///	to the TE, by default it compute checksum of a received message.
	/// @return the computed message.
	inline exitCode checksum(const char * buf, unsigned len, char chksum[3], bool tx = false);

	inline bool verifyChecksum(const char * buf, unsigned len);

	/// Check the responce meaning.
	/// This method check the responce integrity (verifing the checksum)
	/// and return a code describing its kind.
	/// @param msg a message received by the TE without the leading line
	///	terminator
	/// @return a t_teResps describing the kind of message received.
	/// @note msg should not have the leading terminator
	inline Sampi500::t_teResps checkResponce(const char * buf);

	inline std::string verboseNak(t_teResps resp);








	inline exitCode saveEvent(t_eventList & eventList, std::string msg);


	inline DeviceTE::t_eventType getEventType(char * buf);

};

}// namespace device
}// namespace controlbox
#endif

