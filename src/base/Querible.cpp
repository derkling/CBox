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


#include "Querible.h"

#include <controlbox/base/QueryRegistry.h>

namespace controlbox {


exitCode Querible::registerQuery(t_queryID id,
				t_queryName name,
				std::string const & description,
				std::string const & values,
				t_querySupportedType flags) {
	
	Querible::t_queryDescription * descr;
	static QueryRegistry * registry = QueryRegistry::getInstance();

	if ( !registry ) {
		return QR_REGISTRY_NOT_FOUND;
	}

	descr = new Querible::t_queryDescription();
	descr->id = id;
	descr->name = name;
	descr->flags = flags;
	descr->description = description;
	descr->supportedValues = values;

	registry->registerQuery(this, descr);
	
}

}

