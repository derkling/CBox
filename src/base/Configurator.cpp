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





#include "Configurator.ih"

namespace controlbox {

Configurator * Configurator::d_instance = 0;

Configurator::Configurator(std::string const & confFile):
        log(log4cpp::Category::getInstance(std::string("controlbox.Configurator"))),
        d_confFile(confFile) {

    LOG4CPP_DEBUG(log,"Configurator::Configurator(), using config file: %s", d_confFile.c_str() );

}

Configurator & Configurator::getInstance(std::string const & confFile) {

    if ( !d_instance ) {
        d_instance = new Configurator(confFile);
    }

    // TODO: Test for usage/request of different configuration files

    return *d_instance;

}


Configurator::~Configurator() {

    LOG4CPP_DEBUG(log, "Configurator::~Configurator()");

    syncToFile();

    d_property.clear();

}


std::string Configurator::param(std::string const & param, std::string const & defaultValue, bool keep)
throw (exceptions::IOException) {
    t_mapProperty::iterator it;
    string		line;
    char		c_line[MAX_CONFFILE_LINE];
    short		c_lineLableLength;
    char *		c_lineValue;
    char *		c_param;
    size_t		c_paramLen;
    int 		pos;
    ifstream 	c_file;
    short 		i;
    short		lineLength;
    std::string	fileParam;
    bool		found;

    LOG4CPP_DEBUG(log, "Configurator::param(param = [%s], defaultValue = [%s])", param.c_str(), defaultValue.c_str());

    // FIRST: Looking for (modified/new) param on memory
    it = d_property.find(param);
    if ( it != d_property.end() ) {
        LOG4CPP_DEBUG(log, "Loading attribute [%s] from memory: [%s]", param.c_str(), (it->second).value.c_str());
        return (it->second).value;
    }

    // Otherwise we should look on configuration file
    found = false;
    c_file.open(d_confFile.c_str(), ios_base::in);
    if ( ! c_file ) {
        throw exceptions::IOException();
    }

    c_param = (char *)param.c_str();
    c_paramLen = strlen(c_param);
    while ( ! c_file.eof() ) {

        getline(c_file, line);

        if ( ! line.size() || ! strncmp(line.c_str(), "#", 1) ) {
            // Jumping comment lines
            continue;
        }

        strncpy(c_line, line.c_str(), MAX_CONFFILE_LINE);
        lineLength = strlen(c_line);

        // TODO: check if the first space is a TAB!!!

        // Find first "space" (either a space or a tab) after the lable
        for (i=0; i<=lineLength && !isblank(c_line[i]); i++);
        c_lineLableLength = i;
        // Find the start of the value (jumping all "spaces" after lable)
        for (; i<=lineLength && isblank(c_line[i]); i++);

        if ( i==lineLength ) {
            LOG4CPP_WARN(log, "Missing params on configuration string [%s]", c_line);
            c_lineValue = c_line;
        } else {
            c_lineValue = c_line+i-1;
            (*c_lineValue) = 0;
            c_lineValue++;
        }

        // If the two lables have different length they are not equals
        if ( c_lineLableLength != c_paramLen ) {
            continue;
        }

        //LOG4CPP_DEBUG(log, "Line:  [%s](%d) [%s]", c_line, c_paramLen, c_lineValue);
        //LOG4CPP_DEBUG(log, "Param: [%s](%d)", c_param, c_lineLableLength);
        //LOG4CPP_DEBUG(log, "Compare: %d", strncmp(c_line, c_param, MAX_CONFFILE_LINE));

        if ( ! strncmp(c_line, c_param, c_lineLableLength) ) {
            c_file.close();
            LOG4CPP_DEBUG(log, "Loading attribute [%s] from file: [%s]", param.c_str(), c_lineValue);
            fileParam = string(c_lineValue);
            if ( keep ) {
                addParam(param, fileParam);
            }
            return fileParam;
        }
    }

    c_file.close();
    LOG4CPP_DEBUG(log, "Loading attribute [%s] from default value: [%s]", param.c_str(), defaultValue.c_str());
    if ( keep ) {
        addParam(param, defaultValue);
    }
    return defaultValue;
}

inline
exitCode Configurator::addParam (std::string const & param, std::string const & value, bool sync) {
    t_param entry;

    entry.toSync = sync;
    entry.value = value;

    d_property.insert(pair<std::string, t_param>(param, entry));

    LOG4CPP_INFO(log, "Added param [%s] on memory", param.c_str());

}



exitCode Configurator::setParam(std::string const & param, std::string const & value, bool sync)
throw (exceptions::IOException) {
    t_mapProperty::iterator it;

    LOG4CPP_DEBUG(log, "setParam(param=%s, value=%s, sync=%s)", param.c_str(), value.c_str(), sync ? "YES" : "NO" );

    it = d_property.find(param);
    if ( it == d_property.end() ) {
        // The param is not yet on memory
        LOG4CPP_DEBUG(log, "Setting param [%s] value on memory", param.c_str());
        addParam(param, value, sync);
    } else {
        // The param is already on memory: updating value
        LOG4CPP_DEBUG(log, "Updating param [%s] value on memory ([%s] => [%s])", param.c_str(), (it->second).value.c_str(), value.c_str());
        (it->second).toSync = sync;
        (it->second).value = value;
    }

    LOG4CPP_DEBUG(log, "Updated param [%s] value on memory", param.c_str());

    return OK;

}

exitCode Configurator::syncToFile() {

    LOG4CPP_WARN(log, "Configurator::syncToFile(): TO BE IMPLEMENTED");



    return OK;

}

/*
exitCode Configurator::strFilterCompile(string const & sFilter, t_strFilter & cFilter) {

	// A temporary substring filter to insert into the t_strFilter
	t_subStrFilter * ssFilter;
	// A temporary copy of sFilter nedded by the FIELD tokenizer
	char * fileds;
	// A string filter FIELD
	char * aFiled;
	// A temporary copy of aFiled needed by the PARAM tokenizer
	char * curField;
	// A temporary param for OPT tokenizer
	char * temp;
	// A temporary for secure type conversions
	char * convert;


	char *fields = strdup(sFilter.c_str());
	// FIELD tokenizer
	aFiled = strsep (&fields, FILTER_FIELD_SEPARATOR);
	while ( aFiled != NULL ) {

		ssFilter = new t_subStrFilter;

		curField = strdup (aFiled);

		// NOTE: SOSTITUIRE i cout con delle chiamate LOG4CPP_DEBUG
		//cout << "Cur field: " << curField << endl;

		// PARAM tokenizer
		ssFilter->lable = string(strsep (&curField, FILTER_PARAM_SEPARATOR));

		// PARAM tokenizer
		temp = strsep (&curField, FILTER_PARAM_SEPARATOR);
		// OPT tokenizer
		convert = strsep(&temp, FILTER_OPT_SEPARATOR);
		ssFilter->delta = 0;
		if ( convert != NULL ) {
			sscanf( strsep(&convert, FILTER_OPT_SEPARATOR), "%hd", &(ssFilter->delta));
		}
		// OPT tokenizer
		convert = strsep(&temp, FILTER_OPT_SEPARATOR);
		ssFilter->count = 1;
		if ( convert != NULL ) {
			sscanf( strsep(&convert, FILTER_OPT_SEPARATOR), "%hd", &(ssFilter->count));
		}

		// PARAM tokenizer
		temp  = strsep (&curField, FILTER_PARAM_SEPARATOR);
		// OPT tokenizer
		convert = strsep(&temp, FILTER_OPT_SEPARATOR);
		ssFilter->size = 0;
		if ( convert != NULL ) {
			sscanf( strsep(&convert, FILTER_OPT_SEPARATOR), "%hd", &(ssFilter->size));
		}
		// OPT tokenizer
		convert = strsep(&temp, FILTER_OPT_SEPARATOR);
		ssFilter->factor = 1;
		if ( convert != NULL ) {
			sscanf( strsep(&convert, FILTER_OPT_SEPARATOR), "%f", &(ssFilter->factor));
		}

		// NOTE: SOSTITUIRE i cout con delle chiamate LOG4CPP_DEBUG
		LOG4CPP_DEBUG(log, "- %s,%d[%d],%d[%d]", (ssFilter->lable).c_str(), ssFilter->delta, ssFilter->count, ssFilter->size, ssFilter->factor);

		// Save this subStrFilter
		cFilter.push_back(ssFilter);

		// Looking if ther is another subStrFilter...
		aFiled = strsep (&fields, FILTER_FIELD_SEPARATOR);
	}

	return OK;

}
*/



}
