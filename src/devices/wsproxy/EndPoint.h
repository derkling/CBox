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


#ifndef _ENDPOINT_H
#define _ENDPOINT_H

#include <controlbox/base/Utility.h>
#include <controlbox/base/Configurator.h>

namespace controlbox {
namespace device {

/// Class defining an EndPoint.
/// An EndPoint encapsulate a WebService Porxie, providing
/// the methods needed to upload a message to the associated WebService.<br>
/// @note This class is <i>abstract</i> and should be derived in order to
///	actually implement an EndPoint.
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>epmask</b> - <i>Default: EP_MASK</i><br>
///		The mask of EndPoint enabled<br>
///		Format: any hexadecimal between 0x00 and 0xFF
///	</li>
/// </ul>
/// @see WSProxyCommandHandler
class EndPoint  {

public:

	/// The available EndPoint implementation.
	/// This is the unique ID associated to each EndPoint and is used
	/// within the confiugration file.
	/// @note the last entry should always be named WS_EP_ALL and
	///	is the bitwise OR of all others IDs
	enum idEndPoint {
		WS_EP_FILE = 0x1,
		WS_EP_DIST = 0x2,
		/// This is the epmaks and must be the last entry: it defines
		/// the EP that could be enabled (forcing off all those with
		/// corresponding bit set to 0)
		WS_EP_ALL  = 0x3
	};
	typedef enum idEndPoint t_idEndPoint;

	/// Code of commands from EndPoint
	enum epCmdCode {
		EPCMD_CIM = 0x01, //01 CIM
		EPCMD_MSG,        //02 messaggio per l’autista
		EPCMD_FRQ,        //03 frequenza di trasmissione dei dati di funzionamento dell’autobotte (messaggi di tipo 01)
		EPCMD_LDP,        //04 piano di carico
		EPCMD_ODO,        //05 costante odometrica
		EPCMD_DIS,        //06 impostazione odometro
		EPCMD_WSA,        //07 indirizzo web service
		EPCMD_OOT,        //08 attivazione/disattivazione telemetria
	};
	typedef enum epCmdCode t_epCmdCode;

	/// A command from server
	struct epCmd {
		int code;		///> the command code, should match one of t_epCmdCode values
		std::string value;	///> data associated to the command
	};
	typedef struct epCmd t_epCmd;

	/// A list of server commands
	typedef list<t_epCmd*> t_epCmdList;

	/// Responce from server
	struct epResp {
		t_idEndPoint epType;	///> The type of EndPoint generating this responce
		unsigned short epCode;	///> The code of the EndPoint generating this message
		bool result;		///> true if no errors on send data, false otherwise
		std::string errorCode;	///> if result is false: this is the error code
		t_epCmdList cmds;	///> a list of EndPoint commands piggibacked with the responce
	};
	typedef struct epResp t_epResp;

	/// The list of EndPoint responces
	typedef list<t_epResp*> t_epRespList;

protected:

    /// The Configurator to use for getting configuration params
    Configurator & d_configurator;

    /// Logger
    /// Use this logger reference, related to the 'log' category, to log your messages
    log4cpp::Category & log;

    /// The EndPoint Name
    std::string d_name;

    /// The base for configuration params lables
    std::string d_paramBase;

    /// The bitmask for the current EndPoint.
    /// This bitfield must have only ONE bit set: the bit corresponding
    /// to the actual EndPoint.
    unsigned int d_epId;

   /// The bitmask for the current EndPoint's queue
   /// Each endPoint could have one or more queue: this bitmask define the set of
   /// queue enabled for this EndPoint
   unsigned int d_epQueueMask;

   /// The queue mask LSB position for each EndPoint
   unsigned short d_qmShiftCount;

   /// The bitmask of all enabled EndPoint queues
   static unsigned int d_epEnabledQueueMask;


public:

    /// @param epId
    /// @param paramBase the base for this EndPoint configuration params lables
    /// @param logName the base logname: each subclass will append its own
    ///			endPoint identifier
    EndPoint(unsigned int epId, std::string const & paramBase, std::string const & logName);

    virtual ~EndPoint();

    /// Build and EndPoint of the required type
    static EndPoint * getEndPoint(unsigned short const type,
    				std::string const & paramBase,
    				std::string const & logName);

    static unsigned int getEndPointQueuesMask(void) {
	return d_epEnabledQueueMask;
    }

    /// Return the name of the current EndPoint
    inline std::string name() {
        return d_name;
    };

//     inline unsigned int mask() {
//         return d_epQueueMask;
//     };
//
//     inline void set(unsigned int & mask) {
//         mask |= d_epQueueMask;
//     };
//
//     inline void reset(unsigned int & mask) {
//         mask ^= d_epQueueMask;
//     };

    /// Process a data command
    /// @param msgCount the number of this message
    /// @param epEnabledQueues the bitmask of EndPoint's Queues to witch 'msg' is
    ///		still to be send
    /// @return OK on upload success.
    /// @note if the EndPoint successfully process 'msg' it must clear the bit
    ///	inot epRequired corresponding to itself
    exitCode process(unsigned int msgCount, std::string const & msg, unsigned int & epEnabledQueues, EndPoint::t_epRespList &respList);

protected:

    /// This method could be implemented by subclasses and define
    /// the actual web service upload code along with any eventual reponce
    /// piggibacked from the endpoint.
    /// @param msg the message to upload
    /// @param resp the eventually returned EndPoint responce
    virtual exitCode upload(unsigned int & epEnabledQueues, std::string const & msg, EndPoint::t_epRespList &respList) =0;

};



}// namespace device
}// namespace controlbox
#endif
