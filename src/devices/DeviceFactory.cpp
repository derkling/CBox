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


#include "DeviceFactory.ih"


namespace controlbox {
namespace device {

DeviceFactory * DeviceFactory::d_instance = 0;


DeviceFactory::DeviceFactory(std::string const & logName) :
        d_devDB(* DeviceDB::getInstance()),
        log(log4cpp::Category::getInstance("controlbox."+logName)) {
}

DeviceFactory * DeviceFactory::getInstance(std::string const & logName) {

    if ( !d_instance ) {
        d_instance = new DeviceFactory(logName);
    }

    LOG4CPP_DEBUG(d_instance->log, "DeviceFactory::getInstance()");

    return d_instance;
}

DeviceFactory::~DeviceFactory() {

    LOG4CPP_DEBUG(log, "DeviceFactory::~DeviceFactory()");


}

DeviceSignals * DeviceFactory::getDeviceSignals(std::string const & logName) {
    Device * dev;
    DeviceSignals * devSig;


    dev = d_devDB.getDevice(Device::DEVICE_SINGALS);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device SIGNALS found on DeviceDB");
        devSig = dynamic_cast<DeviceSignals *>(dev);
        if ( ! devSig ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceSignals");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device SIGNALS not found on DeviceDB: building a new one");
        devSig = DeviceSignals::getInstance(logName);
    }

    return devSig;

}

DeviceI2CBus * DeviceFactory::getDeviceI2CBus(std::string const & logName) {
    Device * dev;
    DeviceI2CBus * devI2C;


    dev = d_devDB.getDevice(Device::DEVICE_I2CBUS);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device I2CBus found on DeviceDB");
        devI2C = dynamic_cast<DeviceI2CBus *>(dev);
        if ( ! devI2C ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceI2CBus");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device I2CBus not found on DeviceDB: building a new one");
        devI2C = DeviceI2CBus::getInstance(logName);
    }

    return devI2C;

}

DeviceTime * DeviceFactory::getDeviceTime(std::string const & logName) {
    Device * dev;
    DeviceTime * devTime;

//-----[ Dependencies required ]------------------------------------------------
    // Firt we possibly build all time sources
//     getDeviceGPS();
//     getDeviceGPRS("DeviceGPRS"); NOTE if they are more than one, what GPRS we use to keep time?!?!
//------------------------------------------------------------------------------

    dev = d_devDB.getDevice(Device::DEVICE_TIME, logName);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "DeviceTime found on DeviceDB");
        devTime = dynamic_cast<DeviceTime *>(dev);
        if ( ! devTime ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceTime");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "DeviceTime not found on DeviceDB: building a new one");
        devTime = DeviceTime::getInstance(logName);
    }

    return devTime;

}

FileWriterCommandHandler * DeviceFactory::getDeviceFileWriter(const std::string & fileName,
        std::string const & logName, bool append) {
    Device * dev;
    FileWriterCommandHandler * fw;


    dev = d_devDB.getDevice(Device::CH_FILEWRITER, fileName);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Filewriter found on DeviceDB");
        fw = dynamic_cast<FileWriterCommandHandler *>(dev);
        if ( ! fw ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to FileWriterCommandHandler");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Filewriter not found on DeviceDB: building a new one");
        fw = new FileWriterCommandHandler(fileName, logName, append);
    }

    return fw;

}

PollEventGenerator * DeviceFactory::getDevicePoller(timeout_t pollTime, std::string const & logName) {
    Device * dev;
    PollEventGenerator * peg;


    dev = d_devDB.getDevice(Device::EG_POLLER, pollTime);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "PollEventGenerator found on DeviceDB");
        peg = dynamic_cast<PollEventGenerator *>(dev);
        if ( ! peg ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to PollEventGenerator");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "PollEventGenerator not found on DeviceDB: building a new one");
        peg = new PollEventGenerator(pollTime, logName);
    }

    return peg;

}

DeviceGPRS * DeviceFactory::getDeviceGPRS(std::string const & logName,
		unsigned short module) {
    Device * dev;
    DeviceGPRS * devGPRS;


    dev = d_devDB.getDevice(Device::DEVICE_GPRS, module);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device GPRS found on DeviceDB");
        devGPRS = dynamic_cast<DeviceGPRS *>(dev);
        if ( ! devGPRS ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceGPRS");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device GPRS not found on DeviceDB: building a new one");
        devGPRS = DeviceGPRS::getInstance(module, logName);
    }

    return devGPRS;

}

DeviceGPS * DeviceFactory::getDeviceGPS(std::string const & logName, DeviceGPS::t_protocols proto) {
    Device * dev = 0;
    DeviceGPS * devGPS = 0;


    dev = d_devDB.getDevice(Device::DEVICE_GPS, proto);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device GPS found on DeviceDB");
        devGPS = dynamic_cast<DeviceGPS *>(dev);
        if ( ! devGPS ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceGPS");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device GPS not found on DeviceDB: building a new one");
        switch (proto) {
        case DeviceGPS::DEVICEGPS_PROTO_ATGPS:
        	devGPS = DeviceATGPS::getInstance(logName);
        	break;
        case DeviceGPS::DEVICEGPS_PROTO_ARDU:
        	LOG4CPP_ERROR(log, "ARDU Protocol not MORE supported");
		//devGPS = DeviceArdu::getInstance(logName);
		break;
        case DeviceGPS::DEVICEGPS_PROTO_NMEA:
        	LOG4CPP_ERROR(log, "NMEA Protocol not MORE supported");
        	//devGPS = DeviceGPS::getInstance(logName, proto);
        	break;
        }
    }

    return devGPS;

}

DeviceOdometer * DeviceFactory::getDeviceODO(std::string const & logName) {
    Device * dev;
    DeviceOdometer * devODO;

    dev = d_devDB.getDevice(Device::DEVICE_ODO);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device ODO found on DeviceDB");
        devODO = dynamic_cast<DeviceOdometer *>(dev);
        if ( ! devODO ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceODO");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device ODO not found on DeviceDB: building a new one");
        //devODO = DeviceArdu::getInstance(logName);
        devODO = DeviceATGPS::getInstance(logName);
    }

    return devODO;

}

DeviceATGPS * DeviceFactory::getDeviceATGPS(std::string const & logName) {
    Device * dev;
    DeviceATGPS * devATGPS;

    dev = d_devDB.getDevice(Device::DEVICE_ATGPS);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device ATGPS found on DeviceDB");
        devATGPS = dynamic_cast<DeviceATGPS *>(dev);
        if ( ! devATGPS ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceATGPS");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device ATGPS not found on DeviceDB: building a new one");
        devATGPS = DeviceATGPS::getInstance(logName);
    }

    return devATGPS;

}

DeviceAnalogSensors * DeviceFactory::getDeviceAS(std::string const & logName) {
    Device * dev;
    DeviceAnalogSensors * devAS;


    dev = d_devDB.getDevice(Device::DEVICE_AS, 0);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device AS found on DeviceDB");
        devAS = dynamic_cast<DeviceAnalogSensors *>(dev);
        if ( ! devAS ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceAnalogSensors");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device AS not found on DeviceDB: building a new one");
        devAS = DeviceAnalogSensors::getInstance(logName);
    }

    return devAS;

}

DeviceGPIO * DeviceFactory::getDeviceGPIO(std::string const & logName) {
    Device * dev;
    DeviceGPIO * devGPIO;


    dev = d_devDB.getDevice(Device::DEVICE_GPIO, 0);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device GPIO found on DeviceDB");
        devGPIO = dynamic_cast<DeviceGPIO *>(dev);
        if ( ! devGPIO ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceGPIO");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device GPIO not found on DeviceDB: building a new one");
        devGPIO = DeviceGPIO::getInstance(logName);
    }

    return devGPIO;

}

DeviceDigitalSensors * DeviceFactory::getDeviceDS(std::string const & logName) {
    Device * dev;
    DeviceDigitalSensors * devDS;


    dev = d_devDB.getDevice(Device::DEVICE_DS, 0);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device DS found on DeviceDB");
        devDS = dynamic_cast<DeviceDigitalSensors *>(dev);
        if ( ! devDS ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceDigitalSensors");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device DS not found on DeviceDB: building a new one");
        devDS = DeviceDigitalSensors::getInstance(logName);
    }

    return devDS;

}

DeviceTE * DeviceFactory::getDeviceTE(std::string const & logName) {
    Device * dev;
    DeviceTE * devTE;


    dev = d_devDB.getDevice(Device::DEVICE_TE, 0);
    if ( dev ) {
        LOG4CPP_DEBUG(log, "Device TE found on DeviceDB");
        devTE = dynamic_cast<DeviceTE *>(dev);
        if ( ! devTE ) {
            LOG4CPP_ERROR(log, "Failure on dynamic_cast to DeviceTE");
            return 0;
        }
    } else {
        LOG4CPP_DEBUG(log, "Device TE not found on DeviceDB: building a new one");
        devTE = DeviceTE::getInstance();
    }

    return devTE;

}


WSProxyCommandHandler * DeviceFactory::getWSProxy(std::string const & logName) {
	static WSProxyCommandHandler * wsProxy = 0;

	if (wsProxy) {
		return wsProxy;
	}

	wsProxy = WSProxyCommandHandler::getInstance("WSProxy");

	return wsProxy;

}


}// namespace device
}// namespace controlbox
