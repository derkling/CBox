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


#include "Object.ih"

namespace controlbox {

Object::Object(std::string const & logName) :
#if CONTROLBOX_DEBUG > 1
	ct( log4cpp::Category::getInstance("controlbox.calltrace")),
#endif
        log( log4cpp::Category::getInstance("controlbox."+logName) ) {

}

exitCode Object::preloadParams() {
    LOG4CPP_DEBUG(log, "TODO: Params configuration NOT implemented by this class");
}

}
