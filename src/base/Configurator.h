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


#ifndef _CONFIGURATOR_H
#define _CONFIGURATOR_H

#include <string>
#include <list>
#include <controlbox/base/Exception.h>
#include <log4cpp/Category.hh>
#include <controlbox/base/Utility.h>

#define DEFAULT_CONFFILE_PATH "/etc/cbox/cbox.conf"

#define MAX_CONFFILE_LINE	256

// Define needed by the filter compiler
#define FILTER_FIELD_SEPARATOR "|"
#define FILTER_PARAM_SEPARATOR ","
#define FILTER_OPT_SEPARATOR "*"


namespace controlbox {


class Configurator {

//-----[ Types ]----------------------------------------------------------------

public:

    struct subStrFilter {
        string lable;
        short int delta;
        short int count;
        short int size;
        float factor;
    };
    typedef subStrFilter t_subStrFilter;

    typedef list<t_subStrFilter *> t_strFilter;

protected:

    struct param {
        bool toSync;		///< Set if the param shuld be updated on disk
        std::string value;
    };
    typedef struct param t_param;

    typedef map<std::string, t_param> t_mapProperty;



//-----[ Members ]--------------------------------------------------------------

public:


protected:

    static Configurator * d_instance;

    /// The logger to use.
    log4cpp::Category & log;

    /// The configuration file path.
    std::string d_confFile;

    /// The memory loaded configuration params.
    /// This map define the set of required params that shuld be kept on
    /// memory
    t_mapProperty d_property;


//-----[ Methods ]--------------------------------------------------------------

public:

    ~Configurator();

    /// Get a reference to a Configurator.
    ///Singleton method to obtain the unique instance
    static Configurator & getInstance(std::string const & confFile = DEFAULT_CONFFILE_PATH);

    /// Retrive a configuration param.
    /// @param param the param you are searching for
    /// @param defaultValue the defaultValue in case in conf file nothing is specificated
    /// @param keep set true if the param should be keept on memory
    /// @return return your tedious parameter
    /// @note ; this method could be used to check for required params and/or
    ///		to preload some params and keep them on memory
    std::string param(std::string const & param, std::string const & defaultValue = "", bool keep = false)
    throw (exceptions::IOException);

    /// Test if a param has a specified value.
    /// @param keep set to true to optimize multiple value test for the same param.
    inline bool testParam(std::string const & p, std::string const & expectedValue, bool keep = false) {
        std::string confParam;
        bool isEqual;

        confParam = d_instance->param(p, "", keep);
        isEqual = (expectedValue == confParam );

        LOG4CPP_DEBUG(log, "Is [%s]==[%s]? %s", expectedValue.c_str(), confParam.c_str(), isEqual ? "YES" : "NO");

        return isEqual;
    }

    /// Set a configuration param.
    /// This method could be used to define a new conf param or to update
    /// its value on disk.
    /// @param param the param whose value you want to update
    /// @param value the value to set
    /// @param synk set if the param should be synched on disk
    exitCode setParam(std::string const & param, std::string const & value = "", bool sync = false)
    throw (exceptions::IOException);


    /*
    	/// Compile a string filter.
    	/// Translate a "string filter" to an optimized form
    	/// suitable for successive usage of the filter on input string.
    	/// @param sFilter the input string rapresenting a "string filter"
    	/// @param cFilter the compiled string filter
    	/// @return OK if the compilation success
    	exitCode strFilterCompile(std::string const & sFilter, t_strFilter & cFilter);
    */

protected:

    Configurator(std::string const & confFile);

    /// Add a param to the memory map.
    /// @param sync set if the param should be synched to disk
    exitCode addParam(std::string const & param, std::string const & value, bool sync = false);

    /// Sync map to file.
    exitCode syncToFile();

};

}// namespace controlbox
#endif
