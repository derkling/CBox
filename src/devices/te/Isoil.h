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


#ifndef _ISOIL_H
#define _ISOIL_H

#include <controlbox/base/Utility.h>
#include <controlbox/devices/te/DeviceTE.h>

namespace controlbox {
namespace device{

/// A Isoil is a DeviceTE for ISOIL's "Testate Elettroniche".
/// This class provide access to ISOIL's TE devices connected to
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
class Isoil : public DeviceTE  {

  public:

	/// Create a new Sampi TE device initially disabled.
	/// In order to enable it you need to attach a Dispatcher.
	/// @param logName the log category, this name is prepended by the
	///		class namespace "daricom.comlibs."
	Isoil(std::string const & logName = "Isoil");

	/// Class destructor.
	~Isoil();

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
		return DeviceTE::SEND_GENERIC_ISOIL;
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
	inline exitCode checksum(const char * buf, unsigned len, char chksum[3], bool tx = false);

	/// Verify the checksum of the passed message.
	/// @param msg the message to verify
	/// @param len the lenght of the passed message
	/// @return true if the received message checksum is valid
	bool verifyChecksum(const char * msg, unsigned len);

};

}// namespace device
}// namespace controlbox
#endif

