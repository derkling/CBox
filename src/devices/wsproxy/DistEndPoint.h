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


#ifndef _DISTENDPOINT_H
#define _DISTENDPOINT_H

#include "EndPoint.h"

#include <controlbox/devices/gprs/DeviceGPRS.h>


/// The first id for EndPoint configuration params lables
#define DIST_EP_FIRST_ID	0
/// The maximun number of configurables EndPoints
#define DIST_EP_MAXNUM		3

// #define DIST_CHECK_ONLY_ONE_RESPONCE
// #define DIST_DATAFORMAT_42
#define DIST_DATAFORMAT_43

#ifdef DIST_DATAFORMAT_42
#  include <controlbox/devices/wsproxy/soapConcentratoreSoapProxy.h>
#endif
#ifdef DIST_DATAFORMAT_43
#  include <controlbox/devices/wsproxy/soapConcentratore43Soap12Proxy.h>
#endif


namespace controlbox {
namespace device {

/// Class defining the DIST WebService EndPoint.
/// An EndPoint encapsulate a WebService Porxie, providing
/// the methods needed to upload a message to the associated WebService.<br>
/// @note this class could be used to upload messages to the DIST WebService.
/// <br>
/// <h5>Configuration params used by this class:</h5>
/// <ul>
///	<li>
///		<b>fileEndPoint_category</b> - <i>Default: DEFAULT_FILEENDPOINT_CATEGORY</i><br>
///		The catogery<br>
///		Size: [size]
///	</li>
/// </ul>
/// @see EndPoint
class DistEndPoint : public EndPoint {

protected:
	typedef list<std::string> t_serverIpAddrList;

protected:
    /// The gSOAP WebService Proxy
#ifdef DIST_DATAFORMAT_42
	ConcentratoreSoap d_csoap;
#endif
#ifdef DIST_DATAFORMAT_43
	Concentratore43Soap12 d_csoap;
#endif

	/// The GPRS device to use.
	DeviceGPRS * d_devGPRS;

	/// The netlink to use
	std::string d_netlink;

	t_serverIpAddrList srvIpAddrList;

	/// The WebService endpoint
	std::string d_endpoint;

public:
	/// @param logName the base logname to witch will be appended
	///		the endPoint identifier 'DistEndPoint'
	DistEndPoint(std::string const & paramBase, std::string const & logName);

	~DistEndPoint();

	exitCode upload(unsigned int & epEnabledQueues, std::string const & msg, EndPoint::t_epRespList &respList);

protected:

	/// Check server responce for errors or piggibacked commands
	/// @param xml the XML server responce
	/// @return OK if no errors on server upload
	exitCode checkResponce(unsigned short srvNumber, char * xml, EndPoint::t_epRespList &respList);

	/// Log WebService Error Responces
	void logSOAPFault(struct soap * csoap);

	bool srvEnabled(unsigned int & epEnabledQueues, unsigned short srvNumber);
	unsigned int getQueueMask(unsigned int & epEnabledQueues, unsigned short srvNumber);
	void resetQueueBit(unsigned int & epEnabledQueues, unsigned short srvNumber);

};

}// namespace device
}// namespace controlbox
#endif
