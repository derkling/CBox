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


#ifndef _DEVICEIC_H
#define _DEVICEIC_H


#include <controlbox/base/Utility.h>
#include <controlbox/base/comsys/CommandGenerator.h>
#include <controlbox/base/Device.h>
#include <controlbox/base/Querible.h>
#include <controlbox/base/Configurator.h>
#include <controlbox/devices/DeviceFactory.h>
#include <controlbox/devices/DeviceTime.h>


namespace controlbox {
namespace device {

/// A DeviceInCabin is a CommandGenerator that...
/// blha blha blha<br>
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>[param]</b> - <i>Default: [default value]</i><br>
///		[description]<br>
///		Size: [size]
///	</li>
/// </ul>
/// @see CommandHandler
/// @see ATcontrol
class DeviceInCabin : public comsys::CommandGenerator, public Device, public Querible {

//------------------------------------------------------------------------------
//				Class Members
//------------------------------------------------------------------------------

  public:

	/// Handled Messages
	enum cmdType {
		SEND_GENERIC_DATA = (DEVICE_IC*SERVICES_RANGE)+1,
		SEND_CODED_EVENT,
		IS_CONC_CON,
		LIST_KNOWEN_COMMANDS
	};
	typedef enum cmdType t_cmdType;

	enum _interface {
		IC_ATCONTROL = 0		///< InCabin control over ATcontrol
	};
	typedef enum _interface t_interface;


  protected:

	/// The control interface for the InCabin
	t_interface d_interface;

	/// The handler registry for query dispatching
	handlerRegistry<DeviceInCabin> * d_hR;

	/// The Time Device to use
	DeviceTime * d_time;
	
	DeviceFactory * d_df;

	/// The logger to use locally.
	log4cpp::Category & log;


//------------------------------------------------------------------------------
//				Class Methods
//------------------------------------------------------------------------------
  public:

	virtual ~DeviceInCabin();

	inline std::string name() const {
		return d_name;
	}


  protected:
	
	DeviceInCabin(std::string const & logName, t_interface interface);

	/// Export supported queries.
	inline exitCode exportQuery();

//-----[ Query handlers ]-------------------------------------------------------
	// List Knowen Commands
	exitCode qh_ListKnowenCommands (t_query & query);
	// Send Generic Data
	exitCode qh_SendGenericData (t_query & query);
	// Send Coded Event
	exitCode qh_SendCodedEvent (t_query & query);
	// Is 'Concentratore' Connected?
	exitCode qh_IsConcConn (t_query & query);
	// Link status Update
	exitCode qh_GPRS_LinkStatusUpdate (t_query & query);

};

}// namespace device
}// namespace controlbox
#endif
 
