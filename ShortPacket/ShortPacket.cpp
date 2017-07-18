// ShortPacket support routines in here eventually
// J D Johnston // all commercial rights retained, except where code was from somewhere else...
// April 2015

#include <Arduino.h>
#include <CRC16.h>    // CRC16 with 16 number array
#include <Streaming.h>

#include "ShortPacket.h"

const unsigned int MESSAGE_HEADR_ID = 0x5b23; // #[ - message type id number / socket
const unsigned char MESSAGE_TRAIL_ID_HI = 0x5d; // ]# - message type id number / socket
const unsigned char MESSAGE_TRAIL_ID_LO = 0x23; // ]# - message type id number / socket
unsigned int sessionidnum;
unsigned int messageidnum;
bool _verbose = true;
unsigned char traffic_IX = 0;  // #[L^ - 3rd position is zeroth in array, max of 57

const char* TagNames[] {
  "A0",
  "RSSI",
  "RadioTemp",
  "CPUVcc",
  "CPUTemp",
  "Sessionp1",
  "checksum",
  "epoch",
  "millis"
};


PAYLOAD sPayload;


// enable serial output
void SpSetVerbose(bool verbosity) {
	_verbose = verbosity;
}

unsigned int getSessionid() {
	return sessionidnum;
}

// message id
unsigned int getMessageid() {
	return messageidnum;
}


// call this first to set up radio payload
void prep_payload_header() {
  traffic_IX = 0;    // nothing to send
  sPayload.type = MESSAGE_HEADR_ID; // header
  sPayload.sessionid = sessionidnum; // keep session number
  //sPayload.messageid = messageidnum++; // increment message number
  sPayload.messageid = messageidnum; // message number - only increment message number of actually planning on sending a packet
  sPayload.len = traffic_IX;
}

// LONG values
byte prep_payload_L(char payload_name, char payload_type, long payload_value) {
  byte bAccepted = false;
  if (traffic_IX < MESSAGE_MAX_DATA - 6) {
    sPayload.traffic[traffic_IX++] = (char) payload_name;
    sPayload.traffic[traffic_IX++] = (char) payload_type;
    sPayload.traffic[traffic_IX++] = (payload_value >> 24) & 0xff;    // top part?
    sPayload.traffic[traffic_IX++] = (payload_value >> 16) & 0xff;    // top part2
    sPayload.traffic[traffic_IX++] = (payload_value >> 8) & 0xff;    // top part3?
    sPayload.traffic[traffic_IX++] = payload_value & 0xff;   // bottom part?
    sPayload.len = traffic_IX;
    bAccepted = true;
  }
  return bAccepted;
}

// INT values
byte prep_payload_i(char payload_name, char payload_type, int payload_value) {
  byte bAccepted = false;
  if (traffic_IX < MESSAGE_MAX_DATA - 4) {
    sPayload.traffic[traffic_IX++] = (char) payload_name;
    sPayload.traffic[traffic_IX++] = (char) payload_type;
    sPayload.traffic[traffic_IX++] = payload_value / 256;    // top part?
    sPayload.traffic[traffic_IX++] = payload_value & 0xff;   // bottom part?
    sPayload.len = traffic_IX;
    bAccepted = true;
  }
  else {
  }
  return bAccepted;
}

// BYTE values
byte prep_payload_c(char payload_name, char payload_type, char payload_value) {
  byte bAccepted = false;
  if (traffic_IX < MESSAGE_MAX_DATA - 3) {
    sPayload.traffic[traffic_IX++] = (char) payload_name;
    sPayload.traffic[traffic_IX++] = (char) payload_type;
    sPayload.traffic[traffic_IX++] = payload_value & 0xff;   // bottom part?
    sPayload.len = traffic_IX;
    bAccepted = true;
  }
  else {
  }
  return bAccepted;
}


byte prep_payload_final() {
  // finalize the payload packet
  byte bAccepted = false;
  if (traffic_IX <= MESSAGE_MAX_LENGTH - 6) { // enough space for the payload trailer?
    unsigned int crcstart = 0xffff; // crc start with all ones
    unsigned int checksum = 0;
    unsigned char traffic_IX_save = traffic_IX  + 2;  // save spot for checksum to come later
    byte bOK = prep_payload_i(ent_checksum, et_uint, checksum); // create the checksum slot
    sPayload.traffic[traffic_IX++] = MESSAGE_TRAIL_ID_HI;  // first byte of trailer header
    sPayload.traffic[traffic_IX++] = MESSAGE_TRAIL_ID_LO;  // last byte of trailer header
    sPayload.len = traffic_IX;
    checksum = get_crc_16(crcstart, (byte *) &sPayload, traffic_IX + MESSAGE_HDR_LENGTH);  // 7 bytes of header + the payload
    sPayload.traffic[traffic_IX_save] = checksum / 256;    // top of checksum
    sPayload.traffic[traffic_IX_save+1] = checksum & 0xff;    // bottom of checksum
    //sPayload.len = traffix_IX + 2;
    bAccepted = true;
	messageidnum++; // increment message number for next message
  }
  return bAccepted;
}

// display the packet for inspection
void serial_display_packet() {
	if (!_verbose) return;	// bail if not verbose
	// header
    Serial << F("Packet: ") << (char)(sPayload.type>>8) << (char)(sPayload.type & 0xff) << F(" ") 
        << sPayload.sessionid << F(".") << sPayload.messageid << F("~") << _HEX(sPayload.len) << F(":$");
	// data 
    for(byte i = 0; i < min(sPayload.len, min(traffic_IX-2, MESSAGE_MAX_DATA)); i++)
        Serial << (sPayload.traffic[i]<16 ? "0" : "") << _HEX((unsigned char)sPayload.traffic[i]) << ( i < traffic_IX-3 ? ":" : "$");
	// trailer
    Serial << (char)sPayload.traffic[sPayload.len-2] << (char)sPayload.traffic[sPayload.len-1] << endl;
	
}

// full transmit size of packet
byte getSendsize() {
	return sPayload.len + MESSAGE_HDR_LENGTH;
}

//
// pointer to packet
byte* getPayload() {
	return  (byte *) &sPayload;
}

// copy payload from receive buffer to data buffer
byte setPayload(const void* Data, int dataLen) {
	if (dataLen <= 64) {
	    memcpy(&sPayload, Data, dataLen);
		traffic_IX = dataLen - MESSAGE_HDR_LENGTH;
		return true;
	}
	else {
		sPayload.len = 0;
		traffic_IX = 0;
		return false;
	}
}

// get attribute of minipacket 
byte miniPacketAttribute(byte index) {
	return sPayload.traffic[index + 1];
}
// get length of current payload object
byte miniPacketDataLength(byte attribute) {
	byte thislength;
	switch (attribute & 0xf8) {		// mask off low 3 bits of entry power
		case et_ubyte:      // 1 byte
		case et_sbyte:      // 1 byte
			thislength = 1;
			break;
		case et_uint:		// 2 byte
		case et_sint:		// 2 byte
		case et_session:		// 2 byte
		case et_sessionp1:		// 2 byte
		case et_sessionp2:		// 2 byte
		case et_checksum:		// 2 byte
			thislength = 2;
			break;
		case et_ulong:		// 4 byte
		case et_slong:		// 4 byte
			thislength = 4;
			break;
		default:
			thislength = 0xff;		// gotta do something?
	}
	return thislength;
}

byte firstPacket() {
	return 0; // address of first packet
}
byte nextPacket(byte mp) {
	//Serial << "miniPacketIX(tag=" << _HEX(tag) << ")";
	byte temptag;
	byte tempattr;
	byte thislength = 0;
	// data 
	byte ix = mp;
	temptag = sPayload.traffic[ix];
    if( ix < traffic_IX-2) {
		//Serial << " ix = " << ix;
		//Serial << " temptag = " << _HEX(temptag);
		tempattr = miniPacketAttribute(ix);
		//Serial << " tempattr = " << _HEX(tempattr);
		thislength = miniPacketDataLength(tempattr);
		//Serial << " thislength = " << thislength << endl;
	}
	if (thislength == 0xff || thislength == 0) {
		//ix = traffic_IX; // bad packet, quit searching
		ix = 0xff;
	}
	else {
		ix += thislength + 2;						// point to next 
	}
	return ix;
}
byte miniPacketName(byte mp) {
	return sPayload.traffic[mp];
}
byte miniPacketType(byte mp) {
	byte tempattr = miniPacketAttribute(mp);
	return tempattr;
}
byte miniPacketLength(byte mp) {
	byte tempattr = miniPacketAttribute(mp);
	byte thislength = miniPacketDataLength(tempattr);
	return thislength;
}


// find address of each mini-packet
byte minipacketIX(byte tag) {
	//Serial << "miniPacketIX(tag=" << _HEX(tag) << ")";
	byte temptag;
	byte tempattr;
	byte thislength;
	// data 
	byte ix = 0;
	temptag = sPayload.traffic[ix];
    while( temptag != tag && ix < traffic_IX-2) {
		//Serial << " ix = " << ix;
		//Serial << " temptag = " << _HEX(temptag);
		tempattr = miniPacketAttribute(ix);
		//Serial << " tempattr = " << _HEX(tempattr);
		thislength = miniPacketDataLength(tempattr);
		//Serial << " thislength = " << thislength << endl;
		if (thislength == 0xff) {
			ix = traffic_IX; // bad packet, quit searching
		}
		else {
			ix += thislength + 2;						// point to next 
			temptag = sPayload.traffic[ix];			// value of next tag
		}
	}
	if (temptag == tag) {
		return ix;
	}
	else {
		return -1;		// error = -1 = notfound
	}
}

// get value of mini-packet
byte miniPacketValue_c(byte packetIX) {
	byte tempattr;
	byte thislength;
	tempattr = miniPacketAttribute(packetIX);
	thislength = miniPacketDataLength(tempattr);
	if (thislength == 1) {
		return sPayload.traffic[packetIX+2];
	}
	else {
		return 0xff;
	}
}

int miniPacketValue_i(byte packetIX) {
	byte tempattr;
	byte thislength;
	int thisValue;
	tempattr = miniPacketAttribute(packetIX);
	if (_verbose) Serial << "miniPacketAttribute(packetIX("<< packetIX <<")=" << _HEX(tempattr) <<");";
	thislength = miniPacketDataLength(tempattr);
	if (_verbose) Serial << " thislength=" << thislength <<";";
	if (thislength == 2) {
		thisValue = sPayload.traffic[packetIX+2] * 256;
		thisValue += sPayload.traffic[packetIX+3];
		return thisValue;
	}
	else {
		return 0xffff;
	}
}

long miniPacketValue_L(byte packetIX) {
	byte tempattr;
	byte thislength;
	long thisValue;
	tempattr = miniPacketAttribute(packetIX);
	if (_verbose) Serial << "miniPacketAttribute(packetIX("<< packetIX <<")=" << _HEX(tempattr) <<");";
	thislength = miniPacketDataLength(tempattr);
	if (_verbose) Serial << " thislength=" << thislength <<";";
	if (thislength == 4) {
		thisValue = sPayload.traffic[packetIX+2] << 24;
		thisValue += (unsigned char)sPayload.traffic[packetIX+3] << 16;
		thisValue += (unsigned char)sPayload.traffic[packetIX+4] << 8;
		thisValue += (unsigned char)sPayload.traffic[packetIX+5];
		return thisValue;
	}
	else {
		return 0x80000000UL;
	}
}

// is packet valid?

byte validPayload() {
	//Serial.print("validPayload("); Serial.print(false); Serial.print(");"); // debugging
	if (!(sPayload.type == MESSAGE_HEADR_ID)) {Serial << F("*bad header*");return false;}	// bad header
	if (!(sPayload.sessionid == sessionidnum)) {Serial << F("*bad sessionid*");return false;}	// bad header
	if (!(sPayload.len<=MESSAGE_MAX_DATA)) {Serial << F("*msg too long*");return false;}		// message too long
	if (!(sPayload.len>=6)) {Serial << F("*msg short*");return false;}		// message too short
	byte paylen = sPayload.len;
	unsigned char messageTrailID_HI = sPayload.traffic[paylen-2];
	unsigned char messageTrailID_LO = sPayload.traffic[paylen-1];
	if (!((messageTrailID_HI == MESSAGE_TRAIL_ID_HI) && (messageTrailID_LO == MESSAGE_TRAIL_ID_LO))) {
		Serial << F("*bad trailer*") << messageTrailID_HI << messageTrailID_LO;return false;
	} // invalid message trailer
	// check checksum
	byte ixchecksum = minipacketIX(ent_checksum);
	//Serial << "ChecksumIX " << ixchecksum << endl;
	if (ixchecksum == 0xff) {Serial << F("*no checksum*"); return false;}
	int packetchecksum = miniPacketValue_i(ixchecksum);
	// hack the packet, set checksum in packet back to 0x0000
	sPayload.traffic[ixchecksum+2] = 0;
	sPayload.traffic[ixchecksum+3] = 0;
	// recalculate the checksum
    unsigned int crcstart = 0xffff; // crc start with all ones
	int calcChecksum = get_crc_16(crcstart, (byte *) &sPayload, sPayload.len + MESSAGE_HDR_LENGTH);  // 7 bytes of header + the payload
	if (calcChecksum != packetchecksum) {
		Serial << "\n";
		serial_display_packet();
		Serial << F("*checksum mismatch*") << calcChecksum << F("!=") << packetchecksum; return false;
	}
	// didn't find a reason to complain, call it good.
	return true;
}






