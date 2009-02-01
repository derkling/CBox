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

#include "EndPoint.ih"


namespace controlbox {
namespace device {

unsigned int EndPoint::d_epEnabledQueueMask = 0x0;

EndPoint::EndPoint(unsigned int p_epId,
			 t_epType p_epType,
			std::string const & p_paramBase,
			std::string const & p_logName) :
        d_configurator(Configurator::getInstance()),
        d_paramBase(p_paramBase),
        d_epId(p_epId),
        d_epType(p_epType),
        d_failures(EP_MIN_FAILS),
        d_qmShiftCount(0),
        log( log4cpp::Category::getInstance(p_logName) ) {

	std::ostringstream lable("");
	std::string str;
	std::string param;
	unsigned int queueMaskBitSel = 0x1;

	LOG4CPP_DEBUG(log, "Loading configuration");

	// Loading the EndPoint Name
	lable.str("");
	lable << d_paramBase.c_str() << "_name";
	d_name = d_configurator.param(lable.str().c_str(), "Unspecified");

	// Loading the EndPoint QueueMask
	lable.str("");
	lable << d_paramBase.c_str() << "_qmask";
	str = d_configurator.param(lable.str().c_str(), "0x0");
	sscanf(str.c_str(), "%i", &d_epQueueMask);

	// Updating enabled queues bitmask
	EndPoint::d_epEnabledQueueMask |= d_epQueueMask;
	LOG4CPP_DEBUG(log, "Enabled queues mask updateed [0x%02X]",
				EndPoint::d_epEnabledQueueMask);

	// Preparing queue mask shifter
	d_qmShiftCount = 0;
	while ( !(d_epQueueMask & queueMaskBitSel) &&
		d_qmShiftCount < 16 ) {
		d_qmShiftCount++;
		queueMaskBitSel <<= 1;
	};

	LOG4CPP_DEBUG(log, "Queue mask shift count [%d]",
				d_qmShiftCount);

}

EndPoint::~EndPoint() {

    LOG4CPP_DEBUG(log, "Destroying EndPoint [%s]", d_name.c_str());

}

EndPoint *
EndPoint::getEndPoint(unsigned short const type,
			std::string const & paramBase,
			std::string const & logName) {

	switch (type) {
	case WS_EP_FILE:
		return new FileEndPoint(paramBase, logName);
		break;
	case WS_EP_DIST:
		return new DistEndPoint(paramBase, logName);
	}

	return 0;

}

char EndPoint::getQueueLable(unsigned int queue) {
	unsigned int enabled;
	unsigned short i;
	char lable = 'A';

	enabled = EndPoint::getEndPointQueuesMask();

	for (i=0; i<(sizeof(queue)*8); i++) {
		if (queue & (0x1<<i)) {
			break;
		}
		lable++;
	}
	return lable;

}

exitCode EndPoint::process(unsigned int msgCount, std::string const & msg, unsigned int & epEnabledQueues, EndPoint::t_epRespList &respList) {
	exitCode result;

	// Checking if the current endpoint is required for this command
	LOG4CPP_DEBUG(log, "Required Queues: 0x%02X, This EndPoint's Queues: 0x%02X => Enabled: %s",
				epEnabledQueues, d_epQueueMask,
				(epEnabledQueues & d_epQueueMask) ? "YES" : "NO");

	if ( !(epEnabledQueues & d_epQueueMask) ) {
		LOG4CPP_DEBUG(log, "Message processing not required for this EndPoint");
		return OK;
	}

	LOG4CPP_DEBUG(log, "EP-SWITCH: message processing START, queue mask [0x%02X]", epEnabledQueues);

	result = this->upload(epEnabledQueues, msg, respList);
// 	if (result == OK) {
// 		LOG4CPP_INFO(log, "==> %d [0x%02X]", msgCount, epEnabledQueues);
// 	}

	LOG4CPP_DEBUG(log, "EP-SWITCH: message processing END, queue mask [0x%02X]", epEnabledQueues);

	return result;

}

}// namespace device
}// namespace controlbox

