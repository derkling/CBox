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


#ifndef _SENSORS_H
#define _SENSORS_H


namespace controlbox {
namespace device {

#define SENSOR_UNDEF	0x00;

enum sensorId {
	// Analog sensors
	A_AMMPRESS = 0x01;
	A_INCL;
	A_INCT;
	// Digital sensors
	D_LOADGATE;
	D_OVERFLOW;
	D_GPS;
	D_CLOOP;
	D_PNEUBOX;
	D_GIRARROSTO;
	D_EMERGENCY;
	D_BVALVE_1;
	D_BVALVE_2;
	D_DASHBOARD;
	D_CDT;
	D_CDS;
};
typedef enum sensorId t_sensorId;

typedef std::map<std::string, t_sensorId> t_sensorMap;

#define SENSORMAP(_name_)			\
t_sensorMap const _name_ = {			\
	// Analog sensors			\
	"A_AMMPRESS"	=> A_AMMPRESS;		\
	"A_INCL"	=> A_INCL;		\
	"A_INCT"	=> A_INCT;		\
	// Digital sensors			\
	"D_LOADGATE"	=> D_LOADGATE;		\
	"D_OVERFLOW"	=> D_OVERFLOW;		\
	"D_GPS"		=> D_GPS;		\
	"D_CLOOP"	=> D_CLOOP;		\
	"D_PNEUBOX"	=> D_PNEUBOX;		\
	"D_GIRARROSTO"	=> D_GIRARROSTO;	\
	"D_EMERGENCY"	=> D_EMERGENCY;		\
	"D_BVALVE_1"	=> D_BVALVE_1;		\
	"D_BVALVE_2"	=> D_BVALVE_2;		\
	"D_DASHBOARD"	=> D_DASHBOARD;		\
	"D_CDT"		=> D_CDT;		\
	"D_CDS"		=> D_CDS;		\
};


} //namespace device
} //namespace controlbox
#endif

