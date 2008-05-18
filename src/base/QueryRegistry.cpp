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

#include "QueryRegistry.ih"

namespace controlbox {

QueryRegistry * QueryRegistry::d_instance = 0;


QueryRegistry::QueryRegistry(std::string const & logName) :
        log(log4cpp::Category::getInstance("controlbox."+logName)) {
}

QueryRegistry * QueryRegistry::getInstance() {

    if ( !d_instance ) {
        d_instance = new QueryRegistry();
    }

    LOG4CPP_DEBUG(d_instance->log, "QueryRegistry::getInstance()");

    return d_instance;
}

QueryRegistry::~QueryRegistry() {

    LOG4CPP_DEBUG(log, "QueryRegistry::~QueryRegistry()");

    // flushing the registry <map>
    d_queryRegistry.clear();

}

exitCode QueryRegistry::registerQuery(Querible * querible, t_pQdesc queryDescriptor) {
    t_queryRegistry::iterator queryIt;
    t_queribleRegistry::iterator queribleIt;
    std::string const & query = (queryDescriptor->name);

    LOG4CPP_DEBUG(log, "QueryRegistry::registerQuery(query=%s, querible=%p)", query.c_str(), querible);

    if ( !querible ) {
        LOG4CPP_ERROR(log, "Unable to register a query [%s] without an associated querible", query.c_str());
        return QR_MISSING_QUERIBLE;
    }

    // Avoid query name duplication
    queryIt = d_queryRegistry.find(query);
    if ( (queryIt != d_queryRegistry.end()) ) {
        LOG4CPP_WARN(log, "Query already registered [%s]", query.c_str());
        return QR_QUERY_DUPLICATE;
    }

    d_queryRegistry.insert(t_queryRegistryEntry(query, querible));
    d_queribleRegistry.insert(t_queribleRegistryEntry(querible, queryDescriptor));

    LOG4CPP_INFO(log, "Registered new query [%s] exported by Querible [%s]", query.c_str(), querible->name().c_str());

    return OK;

}


exitCode QueryRegistry::unregisterQuery(t_queryName const & query, bool release) {
    t_queryRegistry::iterator queryIt;
    t_queribleRegistry::iterator queribleIt;
    t_queribleRegistry::iterator firstDescr;
    t_pQdesc	pDesc;

    LOG4CPP_DEBUG(log, "QueryRegistry::unregisterQuery(query=%s, release=%s)", query.c_str(), release ? "YES" : "NO" );

    queryIt = d_queryRegistry.find(query);

    // If the query was NOT registered
    if ( (queryIt == d_queryRegistry.end())) {
        LOG4CPP_WARN(log, "Trying to remove a query [%s] NOT registered", query.c_str());
        return QR_QUERY_NOT_EXIST;
    }

    // looking for a query descriptor
    queribleIt = firstDescr = d_queribleRegistry.find(queryIt->second);
    while ( queribleIt != d_queribleRegistry.end() ) {
        if ( queribleIt->first != firstDescr->first) {
            queribleIt = d_queribleRegistry.end();
            break;
        }
        pDesc = queribleIt->second;
        if ( pDesc->name == query ) {
            break;
        }
        queribleIt++;
    }
    // Eventually releasing the query desciption
    if (queribleIt != d_queribleRegistry.end()) {
        if (release) {
            delete (queribleIt->second);
        }
        d_queribleRegistry.erase(queribleIt);
    }

    d_queryRegistry.erase(queryIt);
    LOG4CPP_INFO(log, "Unregistered query [%s] exported by Querible [%s]", query.c_str(), (queryIt->second)->name().c_str());

    return OK;
}

Querible * QueryRegistry::getQuerible(t_queryName const & query) {
    t_queryRegistry::iterator queryIt;

    queryIt = d_queryRegistry.find(query);
    if ( queryIt != d_queryRegistry.end() ) {
        return queryIt->second;
    }

    return 0;

}

Querible::t_queryDescription const * QueryRegistry::getQueryDescriptor(t_queryName const & query, Querible * querible) {
    t_queribleRegistry::iterator firstDescr;
    t_queribleRegistry::iterator queribleIt;

    // looking for a query descriptor
    queribleIt = firstDescr = d_queribleRegistry.find(querible);
    while ( queribleIt != d_queribleRegistry.end() ) {
        if ( queribleIt->first != firstDescr->first) {
            queribleIt = d_queribleRegistry.end();
            break;
        }
        if ( queribleIt->second->name == query ) {
            break;
        }
        queribleIt++;
    }

    if (queribleIt != d_queribleRegistry.end()) {
        return (queribleIt->second);
    }

    return 0;

}

Querible::t_queryDescription const * QueryRegistry::getQueryDescriptor(t_queryName const & query) {
    Querible * querible;

    querible = getQuerible(query);
    if ( !querible ) {
        return 0;
    }

    return getQueryDescriptor(query, querible);

}



std::string QueryRegistry::printRegistry(t_queryName const & query, std::string const & radix) {
    t_queryRegistry::iterator queryIt;
    t_queribleRegistry::iterator it, first;
    unsigned count = 0;
    std::ostringstream dump("");

    if ( query.size() ) {
        queryIt = d_queryRegistry.find(query);
        if ( queryIt == d_queryRegistry.end() ) {
            LOG4CPP_WARN(log, "Required query is not registered");
            return "";
        }
        it = first = d_queribleRegistry.find(queryIt->second);
        if (it != d_queribleRegistry.end()) {
            dump << "\r\nQuery exported by [" << (it->first)->name() << "]:";
            while ( (it != d_queribleRegistry.end()) &&
                    (it->first == first->first) ) {
                count++;
                dump << "\r\n  ";
                dump << radix << setw(10) << left << it->second->name << " ";
                if (it->second->flags & QST_RO) {
                    dump << "r";
                } else {
                    dump << "-";
                }
                if (it->second->flags & QST_WO) {
                    dump << "w";
                } else {
                    dump << "-";
                }
                dump << " " << setw(40) << left << it->second->description << " ";
                if ( it->second->flags & QST_WO ) {
                    dump << "\r\n" << setw(20) << " " << it->second->supportedValues << " ";
                }
                it++;
            }
        }
    } else {
        it = d_queribleRegistry.begin();
        first = d_queribleRegistry.end();
        while ( it != d_queribleRegistry.end() ) {
            count++;
            if (it->first != first->first) {
                dump << "\r\nQuery exported by [" << (it->first)->name() << "]:";
                first = it;
            }
            dump << "\r\n  ";
            dump << radix << setw(10) << left << it->second->name << " ";
            if (it->second->flags & QST_RO) {
                dump << "r";
            } else {
                dump << "-";
            }
            if (it->second->flags & QST_WO) {
                dump << "w";
            } else {
                dump << "-";
            }
            dump << " " << setw(40) << left << it->second->description << " ";
            if ( it->second->flags & QST_WO ) {
                dump << "\r\n" << setw(20) << " " << it->second->supportedValues << " ";
            }
            it++;
        }
    }

    dump << "\r\nTotal: " << count << " registered query\r\n";

    return dump.str();
}


}

