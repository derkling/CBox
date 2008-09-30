//*********************************************************************
//*************  Copyright (C) 2006        DARICOM  ********************
//*********************************************************************
//**
//** The copyright to the computer programs here in is the property of
//** DARICOM The programs may be used and/or copied only with the
//** written permission from DARICOM or in accordance with the terms and
//** conditions stipulated in the agreement/contract under which the
//** programs have been supplied.
//**
//*********************************************************************
//******************** Module information *****************************
//**
//** Project:       ProjectName (ProjectCode/Version)
//** Description:   ModuleDescription
//**
//** Filename:      Filename
//** Owner:         Developer
//** Creation date:  21/06/2006
//**
//*********************************************************************
//******************** Revision history *******************************
//** Revision date       Comments                           Responsible
//** -------- ---------- ---------------------------------- -----------
//**
//**
//*********************************************************************


#include "Somefi.ih"

namespace controlbox {
namespace device {

Somefi::Somefi(std::string const & logName) :
	DeviceTE(DeviceTE::SOMEFI, logName) {
	LOG4CPP_DEBUG(log, "Somefi(std::string const &)");
}

Somefi::~Somefi() {
	LOG4CPP_DEBUG(log, "Destroying the Somefi device");
}



inline exitCode
Somefi::checksum(const char * buf, unsigned len, char chksum[3], bool tx) {
	unsigned sum = 0;
	unsigned index = 0;

	for ( ; index<len; index++) {
		sum += buf[index];
	}

	// NOTE:  (value % 256) == (value & 0xFF)
	sprintf(chksum, "%02x",  (sum & 0xFF));

	// On trasmission we need to SWAP the two bytes
	if (tx) {
		chksum[0] ^= chksum[1];
		chksum[1] ^= chksum[0];
		chksum[0] ^= chksum[1];
	}

	LOG4CPP_DEBUG(log, "len [%d], sum [%d, 0x%X], checksum [%s]", len, sum, sum, chksum);
	return OK;
}

bool
Somefi::verifyChecksum(const char * buf, unsigned len) {
	char rxChksum[] = "\0\0\0";
	char myChksum[] = "\0\0\0";
	bool match = false;

#ifdef DARICOMDEBUG
	if ( d_forceDownload ) return true;
#endif

	memcpy(rxChksum, buf+len-2, 2);
	checksum(buf, len-2, myChksum, false);

	LOG4CPP_DEBUG(log, "Checksum rx [%s], computed [%s]", rxChksum, myChksum);

	match = strncasecmp(rxChksum, myChksum, 2);
	if ( match ) {
		LOG4CPP_WARN(log, "Checksum mismatch on TE data download");
	}

	return !match;

}


exitCode
Somefi::downloadEvents(t_eventList & eventList) {
/*
	unsigned len, index;
	unsigned short retry;
	char buf[DEVICE_TE_SAMPI500_RX_BUFFER];
	Somefi::t_teResps resp;
	unsigned device;
	unsigned downloadedRecords = 0;
	t_event * event;
	bool readOk = false;
	EVENT_CODE(evtLable);
	exitCode result = OK;

	d_tty->detachedMode(false);

	// Sending download command #54
	LOG4CPP_DEBUG(log, "Send command 54");
	write(C54_STX);

	do {
		// Reading next message
		len = read(buf, DEVICE_TE_SAMPI500_RX_BUFFER);
		retry = d_retry+1;
		while ( !len && --retry) {
			LOG4CPP_DEBUG(log, "Retrying download...");
			len = read(buf, DEVICE_TE_SAMPI500_RX_BUFFER);
		};
		if (!len) {
			LOG4CPP_WARN(log, "Device not responding: download ABORTED");
			// If the TE does not respond we wait until the next download time
			result = TE_NO_NEW_EVENTS;
			goto download_end;
		}

		// Verifying message checksum
		if ( !verifyChecksum(buf, len-1) ) {
			LOG4CPP_WARN(log, "Checksum error on downloading message [%d]", downloadedRecords+1);
			//TODO Aborting download to restart from this record
			result = TE_RESTART_DOWNLOAD;
			goto download_end;
		}

		// NOTE: if this is not an ACK we should really come back and FIX THE CODE
		resp = checkResponce(buf);
		if ( resp > NAK_NO_NEW_EVENT ) {
			LOG4CPP_FATAL(log, "Communication error: %s", verboseNak(resp).c_str());
			result = TE_NAK_RECEIVED;
			goto download_end;
		}

		if ( resp == NAK_NO_NEW_EVENT ) {
			LOG4CPP_DEBUG(log, "No more events from SAMPI500");
			if (downloadedRecords) {
				LOG4CPP_DEBUG(log, "Notify events to be uploaded");
				result = OK;
				goto download_end;
			} else {
				result = TE_NO_NEW_EVENTS;
				goto download_end;
			}
		}

		// Queue event (removing all un-necessary fields)
		event = new t_event();
		event->type = getEventType(buf);
		buf[len-3] = 0; //Removing leading checksum and line terminator....

	//NOTE: we don't remove the leading address, record number and event type because is used by the
	//	RAW event formatter
	// 	event->event = std::string(buf+4);
		event->event = std::string(buf);
		LOG4CPP_DEBUG(log, "EVT [%c], [%s]",
				evtLable[event->type],
				(event->event).c_str());

		eventList.push_back(event);
		downloadedRecords++;

		// Sending ACK
		write(C54_ACK);

	} while (true);

	LOG4CPP_INFO(log, "%d new events downloaded", downloadedRecords);

download_end:

	d_tty->detachedMode(true);
	return result;
*/
	return OK;
}


exitCode
Somefi::formatEvent(t_event const & p_event, comsys::Command & cmd) {
	std::ostringstream sbuf("");
	const char * evt;

	evt = p_event.event.c_str();

	sbuf << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (unsigned)0x14;
	cmd.setParam( "dist_evtType", sbuf.str());
	cmd.setParam("dist_evtData", evt+1);
	LOG4CPP_DEBUG(log, "RAW SampiTE_550, DIST-CODE [%s], DATA [%s]",
				sbuf.str().c_str(),
				evt+4);

	return OK;

}

}// namespace device
}// namespace controlbox
