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


#include "Command.ih"

namespace controlbox {
namespace comsys {



/*
Command::Command (t_cmdType const & cmdType, std::string const & logName) :
	log(log4cpp::Category::getInstance(std::string("controlbox.comlibs.command."+logName))),
	d_cmdType(cmdType),
	d_devType(Device::UNDEF),
	d_devId(0) {

	LOG4CPP_DEBUG(log, "Command::Command (d_cmdType=%u, logName=%s)", d_cmdType, logName.c_str());

}
*/

Command::Command (t_cmdType const & cmdType, Device::t_deviceType devType,
                  Device::t_deviceId devId, std::string const & logName) :
        d_cmdType(cmdType),
        d_devType(devType),
        d_devId(devId),
        d_prio(10),
        log(log4cpp::Category::getInstance(std::string("controlbox.comlibs.command."+logName))) {

	LOG4CPP_WARN(log, "NEW@%p (type=%u, device=%u, deviceId=%s, logName=%s)",
			this,
			d_cmdType, d_devType, d_devId.c_str(), logName.c_str());

}

Command::~Command () {
	t_mapValue::iterator it;

	LOG4CPP_WARN(log, "DEL@%p (type=%u, device=%u, deviceId=%s)",
			this,
			d_cmdType, d_devType, d_devId.c_str());

	// Walking the multimap to delete dynamic allocated params (string)
	it = d_params.begin();
	while ( it != d_params.end() ) {
		if ( (it->second).type ==  PT_STRING) {
			delete (it->second).value.s;
		}
		it++;
	}
	d_params.clear();

}

Command * Command::getCommand(t_cmdType const & cmdType, Device::t_deviceType devType,
                              Device::t_deviceId devId, std::string const & logName) {

    // TODO: return a Smart Pointer!!!

    return new Command(cmdType, devType, devId, logName);

}


inline
exitCode Command::setDevice(Device::t_deviceType const & devType) {
    d_devType = devType;
    return OK;
}

inline
exitCode Command::setDeviceId(Device::t_deviceId const & devId) {
    d_devId = devId;
    return OK;
}


inline
exitCode Command::eraseParam(std::string const & lable) {
    t_mapValue::iterator first;
    t_mapValue::iterator it;

    // Walking the multimap to delete dynamic allocated params (string)
    it = first = d_params.find(lable);
    if ( it != d_params.end() ) {
        do {
            if ( (it->second).type ==  PT_STRING) {
                delete (it->second).value.s;
            }
            it++;
        } while ( it->first != first->first );
    }
    d_params.erase(lable);

    return OK;
}

void Command::setPrio(unsigned short prio) {
	LOG4CPP_DEBUG(log, "Setting command priority [%hu]", prio);
	d_prio = prio;
}

unsigned short Command::getPrio(void) const {
	return d_prio;
}

inline
exitCode Command::setTheParam(std::string const & lable, t_cmdParam const & p, bool override) {

    LOG4CPP_DEBUG(log, "Command::setTheParam(lable=%s, type=%d, override=%s)", lable.c_str(), p.type, override ? "YES" : "NO" );

    // TODO: delete strings params!!!
    if ( override ) {
        eraseParam(lable);
    }

    d_params.insert ( pair<std::string, t_cmdParam>(lable,p) );

    return OK;
}

exitCode Command::setParam(std::string const & lable, int value, bool override ) {
    t_cmdParam p;

    LOG4CPP_DEBUG(log, "Command::setParam(lable=%s, value=%d, override=%s)", lable.c_str(), value, override ? "YES" : "NO" );

    p.type = PT_INT;
    p.value.i = value;

    return setTheParam(lable, p, override);
}


exitCode Command::setParam(std::string const & lable, double value, bool override ) {
    t_cmdParam p;

    LOG4CPP_DEBUG(log, "Command::setParam(lable=%s, value=%f, override=%s)", lable.c_str(), value, override ? "YES" : "NO" );

    p.type = PT_FLOAT;
    p.value.d = value;

    return setTheParam(lable, p, override);
}

exitCode Command::setParam(std::string const & lable, std::string const & value, bool override) {
    t_cmdParam p;

    LOG4CPP_DEBUG(log, "Command::setParam(lable=%s, value=%s, override=%s)", lable.c_str(), value.c_str(), override ? "YES" : "NO" );

    p.type = PT_STRING;
    p.value.s = new std::string(value);

    return setTheParam(lable, p, override);
}


Command::t_cmdType Command::type() const {
    return d_cmdType;
}

Device::t_deviceType Command::device() const {
    return d_devType;
}


Device::t_deviceId Command::deviceId() const {
    return d_devId;
}

unsigned int Command::paramCount() const {
    return d_params.size();
}

unsigned int Command::paramCount(std::string const & lable) const {
    return d_params.count(lable);
}

inline
Command::t_cmdParam Command::find(std::string const & lable, unsigned int pos)
throw (exceptions::UnknowedParamException) {

    // Trying to optimize a bit multiple access for multi-value params retrivial

    static std::string lastSearchLable;	// the last searched pos used to optimize multi-value param retrivial
    static unsigned int lastPos = 0;		// the last searched pos used to optimeze forward search
    static unsigned int lableCount = 0;		// the elements count for lastSearchLable
    unsigned int delta = 0;			// used to optimize forward search
    static t_mapValue::iterator it;		// the iterator used to optimize forward search of multi-value params

    LOG4CPP_DEBUG(log, "Command::param(lable=%s, pos=%u)", lable.c_str(), pos);

    /*
    	LOG4CPP_DEBUG(log, "\tlastSearchLable: %s", lastSearchLable.c_str());
    	LOG4CPP_DEBUG(log, "\tlastPos:         %u", lastPos);
    	LOG4CPP_DEBUG(log, "\tlableCount:  %u", lableCount);
    	LOG4CPP_DEBUG(log, "delta:             %u", delta);
    */

    // Optimizing multi-value param forward search
    if ( (lable != lastSearchLable) || (pos < lastPos) ) {
        lastSearchLable = lable;
        it = d_params.find(lable);
        lableCount = d_params.count(lable);
        if ( ! lableCount ) {
            throw exceptions::UnknowedParamException("The required param doesn't exist!");
        }
        lastPos = pos;
        delta = pos-1;
    }

    // Check if the required pos is valid
    if ( pos > lableCount ) {
        LOG4CPP_WARN(log, "The required param '%s' has not as much elements! Looking for pos=%u, but size(%s)=%u",
                     lable.c_str(), pos, lable.c_str(), lableCount);
        throw exceptions::UnknowedParamException("The required param has not as much elements!");
    }

    // Optimizing forward search
    if ( pos > lastPos ) {
        delta = pos - lastPos;
        lastPos = pos;
    }

    /*
    	LOG4CPP_DEBUG(log, "");
    	LOG4CPP_DEBUG(log, "\tlastSearchLable: %s", lastSearchLable.c_str());
    	LOG4CPP_DEBUG(log, "\tlastPos:         %u", lastPos);
    	LOG4CPP_DEBUG(log, "\tlableCount:  %u", lableCount);
    	LOG4CPP_DEBUG(log, "delta:             %u", delta);
    */
    // Moving the iterator to the required param:
    // now we are quite sure we get the pos without going
    // out of range
    while ( delta-- ) {
        it++;
    }

    return it->second;

}


long Command::getIParam(std::string const & lable, unsigned int pos)
throw (exceptions::UnknowedParamException) {
    t_cmdParam p;

    p = find(lable, pos);

    return p.value.i;

}


double Command::getFParam(std::string const & lable, unsigned int pos)
throw (exceptions::UnknowedParamException) {
    t_cmdParam p;

    p = find(lable, pos);

    return p.value.d;

}

std::string Command::param(std::string const & lable, unsigned int pos)
throw (exceptions::UnknowedParamException) {
    t_cmdParam p;
    std::ostringstream s_param("");

    LOG4CPP_DEBUG(log, "Command::param(std::string const & lable, unsigned int pos)");

    p = find(lable, pos);

    LOG4CPP_DEBUG(log, "Type: %d", p.type);

    // Halding string conversion for non string params
    switch (p.type) {
    case PT_INT:
        s_param << p.value.i;
        return s_param.str();
    case PT_FLOAT:
        s_param << p.value.d;
        return s_param.str();
    default:
    	break;
    }


    return (*p.value.s);

}

bool Command::hasParam(std::string const & lable) {
    return d_params.find(lable) != d_params.end();
}


exitCode Command::xmlDump(std::string & xmlCommand) {
    t_mapValue::iterator it;		// = (command->paramList())->begin();
    unsigned int countMultiparam = 0;	// if >0 means also we are in MULTIPARAM MODE
    std::string lastLable;
    std::ostringstream xml("");

    LOG4CPP_DEBUG(log, "Command::xmlDump()");


    //d_fwcategory->info("\n");
    xml << "\n<Command type='" << d_cmdType << "'>\n";
    xml << "\t<Device id='" << d_devId << "'>" << d_devType << "</Device>\n";
    xml << "\t<Params>\n";


    //d_fwcategory.info("");

    it = d_params.begin();
    while ( it != d_params.end() ) {

        // Checking for multiparam values
        if ( !countMultiparam && (d_params.count(it->first) > 1) ) {
            xml << "\t\t<MultiValueParam Name='" << it->first << "'>\n";
            countMultiparam = 1;
            lastLable = it->first;
        }

        if ( countMultiparam ) {
            xml << "\t\t\t<Value id='" << countMultiparam++ << "'>";
        } else {
            xml << "\t\t<Param Name='" << it->first << "'>";
        }
        switch ((it->second).type) {
        case PT_INT:
            xml << (it->second).value.i;
            break;
        case PT_FLOAT:
            xml << (it->second).value.d;
            break;
        case PT_STRING:
            xml << *((it->second).value.s);
            break;
        }
        if ( countMultiparam ) {
            xml <<  "</Value>\n";
        } else {
            xml << "</Param>\n";
        }
        it++;

        // Cheching if the current multiparam is finisched
        if ( countMultiparam ) {

            if (it == d_params.end()) {
                xml << "\t\t</MultiValueParam>\n";
                continue;
            }

            if (lastLable != it->first) {
                countMultiparam = 0; // Exiting multiparam mode
            }

        }

    }

    xml << "\t</Params>\n";
    xml << "</Command>\n";

    xmlCommand = xml.str();

    return OK;

} //namespace comsys
} //namespace controlbox


}

