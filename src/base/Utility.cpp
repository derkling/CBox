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





#include "Utility.ih"

#define BUFF_SIZE 4096

namespace controlbox {


std::string Utils::strFormat(const char* stringFormat, ...) {
    va_list va;
    char buff[BUFF_SIZE];
    std::string str;

    va_start(va, stringFormat);
    vsnprintf(buff, BUFF_SIZE, stringFormat, va);
    //str = log4cpp::StringUtil::vform(stringFormat, va);
    va_end(va);

    return string(buff);
}

}
