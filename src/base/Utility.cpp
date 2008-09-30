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

//-----[ Base64 Encoding/Decoding routines ]------------------------------------

exitCode Utils::b64enc(const char *p_in, size_t p_inlen, char *p_out, size_t p_outlen) {

	// Check for OUTPUT size based on input len...
	if ( p_outlen < BASE64_LENGTH(p_inlen) ) {
		// FAIL: input too long
		return CONVERSION_ERROR;
	}
	// Encode input buffer
	base64_encode (p_in, p_inlen, p_out, p_outlen);

	return OK;

}

exitCode Utils::b64enc(const char *p_in, size_t p_inlen, char **p_out, size_t &p_outlen) {
	size_t l_outlen;

	l_outlen = base64_encode_alloc(p_in, p_inlen, p_out);
	if ( p_out==NULL && l_outlen==0 && p_inlen!= 0 ) {
		// FAIL: input too long
		return CONVERSION_ERROR;
	}
	if ( p_out==NULL ) {
		// FAIL: memory allocation error
		return OUT_OF_MEMORY;
	}

	return OK;
}


exitCode Utils::b64dec(const char *p_in, char *p_out, size_t & p_outlen) {

#warning Dummy b64dec implementation: Base64 decoding NOT yet supported
	p_outlen = 0;
	p_out[0] = 0;

	return OK;
}


}
