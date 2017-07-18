// ShortPacket support routines in here eventually
// J D Johnston // all commercial rights retained, except where code was from somewhere else...
// April 2015, September 2016, October 2016

#include <Arduino.h>
#include <Streaming.h>

#include "ShortPacket2.h"

// ==================   SIPHASH   ======================================

#include "SipHash_2_4.h"

#define IFECHO if(_verbose)
const byte fox2 = 0xff;

unsigned long ShortPacket2::SipTOTP(uint8_t phmacKey[], char* structure, int structurelength) {
	// debug - dump first 3 bytes of key -- looks good
	//IFECHO {
	//	Serial << F("\n<");
	//	for (byte ix=0; ix<16; ix+=4) {
	//		Serial << _HEX(phmacKey[0+ix]) << F(".") << _HEX(phmacKey[1+ix]) << F(".") << _HEX(phmacKey[2+ix]) << F(".") << _HEX(phmacKey[3+ix]) << F(":");
	//	}
	//}
    sipHash.initFromRAM(phmacKey);
    for (int i = 0; i < structurelength; i++) {
	//	IFECHO {Serial << F("-") << _HEX((byte)structure[i]);}
		sipHash.updateHash((byte)structure[i]); // last part of 8 byte time value
    }
	//IFECHO Serial << F("> ");

    sipHash.finish(); // result in BigEndian format
    reverse64(sipHash.result); // change to LittleEndian to match  https://131002.net/siphash/siphash24.c
    int s_offset = sipHash.result[7] & 0x7; // 0 - 7 offset 
    unsigned long s_truncatedHash = 0;
    for (int j = 0; j < 4; ++j) { // get bytes into hash
      s_truncatedHash <<= 8;
      s_truncatedHash |= sipHash.result[s_offset--];
      if (s_offset < 0) {s_offset = 7;}
    }
    //s_truncatedHash &= 0x7FFFFFFF;  // truncate the hash to signed +
    s_truncatedHash %= 0x300200; //not 3276509;     // truncate the hash part 2: 1,000,000 is normal 3,276,509 is a prime larger than 1,000,000
	//IFECHO Serial << F("; hash:") << _HEX(s_truncatedHash) << F(";") << endl;
    return s_truncatedHash;
}


// ================== END SIPHASH ======================================

// enable serial output
void ShortPacket2::SpSetVerbose(bool verbosity) {
	_verbose = verbosity;
}

// call this first to set up radio payload
bool ShortPacket2::Begin(unsigned long time) {
	_time = time;
	sPayload.type = MESSAGE_HEADR_ID; // header
	traffic_IX = 0;    // nothing to send
	sPayload.len = traffic_IX;	// mark the packet
	sPayload.nonceSum = (unsigned long)nonce << 22; // keep session number
	unsigned long packettime = ((_time >> 5) & 0x3fffffUL);
	//IFECHO Serial << F(" nonce:") << _HEX(nonce) << F(" packettime: ") << _HEX(packettime);
	sPayload.nonceSum += packettime;   // add packet time key to hashed field // 3f,ff,ff = 22 bits + 5 bits (/32) = 27 bits
													// this "epoch" is 27 bits or 4.25 years long
	//IFECHO Serial << F(" nonceSum: ") << _HEX(sPayload.nonceSum) << endl;
	char t2 = _time;
	Add_c(1, t2); // add time value
	return true;
}

// LONG values
bool ShortPacket2::Add_L(char payload_name, long payload_value) {
  bool bAccepted = false;
  if (traffic_IX < MESSAGE_MAX_DATA - 5) {
    sPayload.body[traffic_IX++] = ((char) payload_name << 2) + ((char) 0x2);
    sPayload.body[traffic_IX++] = (payload_value >> 24) & fox2;   // top part1?
    sPayload.body[traffic_IX++] = (payload_value >> 16) & fox2;   // top part2?
    sPayload.body[traffic_IX++] = (payload_value >> 8) & fox2;    // top part3?
    sPayload.body[traffic_IX++] = payload_value & fox2;        // bottom part?
    sPayload.len = traffic_IX;
    bAccepted = true;
  }
  return bAccepted;
}

// INT values
bool ShortPacket2::Add_i(char payload_name, int payload_value) {
  bool bAccepted = false;
  if (traffic_IX < MESSAGE_MAX_DATA - 3) {
    sPayload.body[traffic_IX++] = ((char) payload_name << 2) + ((char) 0x1);
    sPayload.body[traffic_IX++] = payload_value / 256;    // top part?
    sPayload.body[traffic_IX++] = payload_value & fox2;   // bottom part?
    sPayload.len = traffic_IX;
    bAccepted = true;
  }
  else {
  }
  return bAccepted;
}

// BYTE values
bool ShortPacket2::Add_c(char payload_name, char payload_value) {
  bool bAccepted = false;
  if (traffic_IX < MESSAGE_MAX_DATA - 2) {
    sPayload.body[traffic_IX++] = ((char) payload_name << 2) + ((char) 0x0);
    sPayload.body[traffic_IX++] = payload_value;   // only part
    sPayload.len = traffic_IX;
    bAccepted = true;
  }
  else {
  }
  return bAccepted;
}

unsigned long ShortPacket2::Time() {	// return the current time
		return _time;
} 

bool ShortPacket2::AddTime(unsigned long time) {				// add whatever part of time is changed
	if (time == _time) {
		return true;					// already on this time, don't add any data
	} 
	else {
		int tmtop = time >> 16;
		int _tmtop = _time >> 16;
		if (tmtop == _tmtop) {
			char tmshift = time >> 8; 	//keep bits 17-23
			char _tmshift = _time >> 8;
			if (tmshift == _tmshift) {				// send only last byte of time
				char t2 = time;
				_time = time;
				return Add_c(1, t2);	// add the last byte of time
			}
			else {									// send only last two bytes of time
				int t3 = time;
				_time = time;
				return Add_i(1, t3);	// add the last two bytes of time
			}
		}
		else {
			_time = time;
			return Add_L(1, time);	// add the full time
		}
	}
}

bool ShortPacket2::Add(unsigned long time, char payload_name, unsigned long payload_value) {
	if (AddTime(time)) return Add_L(payload_name, payload_value);
	return false;
}
bool ShortPacket2::Add(unsigned long time, char payload_name, long payload_value) {
	if (AddTime(time)) return Add_L(payload_name, payload_value);
	return false;
}
bool ShortPacket2::Add(unsigned long time, char payload_name, int payload_value) {
	if (AddTime(time)) return Add_i(payload_name, payload_value);
	return false;
}
bool ShortPacket2::Add(unsigned long time, char payload_name, char payload_value) {
	if (AddTime(time)) return Add_c(payload_name, payload_value);
	return false;
}

  // finalize the payload packet
bool ShortPacket2::Close(uint8_t hmacKey[]) {
  bool bAccepted = false;
  if (traffic_IX <= MESSAGE_MAX_LENGTH - 6) { // enough space for the payload trailer?
    unsigned long checksum = 0;
    sPayload.len = traffic_IX;
	checksum = SipTOTP (hmacKey, (char *) &sPayload, traffic_IX + MESSAGE_HDR_LENGTH);
    bAccepted = true;
	sPayload.nonceSum = ((unsigned long)(nonce) << 22) + checksum;	// can't update nonce until after send! checksum calculated on olde nonce!
	nonce++; // increment message number for next message
  }
  return bAccepted;
}

// display the packet for inspection
void ShortPacket2::SerialDisplay() {
	if (!_verbose) return;	// bail if not verbose
	// header
    Serial 	<< F("{: '") << (sPayload.type) << F("' ") 
			<< (int)(sPayload.nonceSum >> 22) << F(".") 
			<< _HEX(sPayload.nonceSum & 0x3fffff) << F(" (") << sPayload.len << F(")$");
	// data 
    for(byte i = 0; i < min(sPayload.len, min(traffic_IX, MESSAGE_MAX_DATA)); i++)
        Serial << (sPayload.body[i]<16 ? "0" : "") << _HEX((unsigned char)sPayload.body[i]) << ( i < traffic_IX-1 ? ":" : "$");
	// trailer
	Serial << F("}\n");	// end packet print
}

// full transmit size of packet
byte ShortPacket2::sendSize() {
	return sPayload.len + MESSAGE_HDR_LENGTH;
}

// data transmit size in packet
byte ShortPacket2::dataSize() {
	return sPayload.len;
}

// pointer to packet
byte* ShortPacket2::Data() {
	return  (byte *) &sPayload;
}

// copy payload from receive buffer to data buffer
byte ShortPacket2::setPayload(const void* Data, int dataLen) {
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
byte ShortPacket2::miniPacketName(byte index) {
	return sPayload.body[index] >> 2;
}

// get length of current payload object
byte ShortPacket2::miniPacketDataLength(byte index) {
	byte packedLength = sPayload.body[index] & 0x3;
	byte thislength;
	switch (packedLength) {		// mask off low 2 bits of entry power
		case 0:      // 1 byte
			thislength = 1;
			break;
		case 1:		// 2 byte
			thislength = 2;
			break;
		case 2:		// 4 byte
			thislength = 4;
			break;
		default:
			thislength = fox2;		// gotta do something?
	}
	return thislength;
}

byte ShortPacket2::firstPacket() {
	if(sPayload.type != '=') return fox2; // bad packet = no data!
	return 0; // address of first packet
}

byte ShortPacket2::nextPacket(byte mp) {
	byte ix = mp;
	byte thislength = miniPacketDataLength(ix);
    if( ix < traffic_IX-1) {
	}
	if (thislength == fox2 || thislength == 0) {
		ix = fox2;
	}
	else {
		ix += thislength + 1;						// point to next 
	}
	if (ix >= sPayload.len) ix = fox2;				// past eof
	if (ix >= MESSAGE_MAX_DATA) ix = fox2;			// past eof
	return ix;
}

// find address of each mini-packet
byte ShortPacket2::minipacketIX(byte tag) {
	byte temptag;
	byte thislength;
	byte ix = 0;
	temptag = miniPacketName(ix);
    while( temptag != tag && ix < traffic_IX-1) {
		thislength = miniPacketDataLength(ix);
		if (thislength == fox2) {
			ix = traffic_IX; // bad packet, quit searching
		}
		else {
			ix += thislength + 1;						// point to next 
			temptag = miniPacketName(ix);			// value of next tag
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
byte ShortPacket2::miniPacketValue_c(byte packetIX) {
	byte thisValue;
	byte thislength = miniPacketDataLength(packetIX);
	if (thislength == 1) {
		thisValue = sPayload.body[packetIX+1];
		//IFECHO Serial << " val_c: " << thisValue <<", ";
		return thisValue;
	}
	else {
		return fox2;
	}
}

// length 2
int ShortPacket2::miniPacketValue_i(byte packetIX) {
	int thisValue;
	byte thislength = miniPacketDataLength(packetIX);
	if (thislength == 2) {
		thisValue = sPayload.body[packetIX+1] * 256;
		thisValue += sPayload.body[packetIX+2];
		return thisValue;
	}
	else {
		return 0xffff;
	}
}

// length 4
long ShortPacket2::miniPacketValue_L(byte packetIX) {
	unsigned long thisValue;
	byte thislength = miniPacketDataLength(packetIX);
	if (thislength == 4) {
		thisValue = (unsigned long)sPayload.body[packetIX+1] << 24;
		thisValue += (unsigned long)sPayload.body[packetIX+2] << 16;
		thisValue += (unsigned long)sPayload.body[packetIX+3] << 8;
		thisValue += (unsigned long)sPayload.body[packetIX+4];
		return thisValue;
	}
	else {
		return 0x80000000UL;
	}
}

// is packet valid?

// enum status {
	// OK,
	// BadHeader,
	// TooLong,
	// TooShort,
	// NoTimeCode,
	// NonceOutOfOrder,
	// ChecksumFail
// }

ShortPacket2::status ShortPacket2::validPayload(unsigned long time, uint8_t hmacKey[]) {
	if (!(sPayload.type == MESSAGE_HEADR_ID)) {
		sPayload.len = 0;
		return status::BadHeader;
		//IFECHO Serial << F("*bad header*");  return false;
	}	// bad header
	if (!(sPayload.len<=MESSAGE_MAX_DATA)) {
		sPayload.len = 0;
		return status::TooLong;
		//IFECHO Serial << F("*msg too long*"); return false;
	}		// message too long
	if (!(sPayload.len>=2)) {
		sPayload.len = 0; 
		//IFECHO Serial << F("*msg short*"); return false;
		return status::TooShort;
	}		// message too short
    traffic_IX = sPayload.len;
	// find time
	byte ixtime = minipacketIX(2);	// 2 is a time tag - supposed to be full 4 byte size
	if (ixtime == fox2)	ixtime = minipacketIX(1);	// 1 is a compressed time tag
	if (ixtime == fox2) {					// if no time code, this isn't a good packet
		sPayload.len = 0;					//IFECHO Serial << F("*no time code*");  return false;
		return status::NoTimeCode;
	}
	unsigned long timetemp = time; // pretend packet time is current
	unsigned int packettimeint;
	byte packettimelow;
	byte timesize = miniPacketDataLength(ixtime);
	switch (timesize) {
		case 1: timetemp = (time & 0xffffff00) | miniPacketValue_c(ixtime); break;
		case 2: timetemp = (time & 0xffff0000) | miniPacketValue_i(ixtime); break;
		case 4: miniPacketValue_L(ixtime); break;
		default:
			return status::NoTimeCode;			// time code has bad length
	} 
	packettimelow = timetemp; 	// miniPacketValue_c(ixtime);
	packettimeint = timetemp;	// sixteen bit version of time
	//IFECHO Serial << F(" time: ") << time;
	//IFECHO Serial << F(" packettimelow: ") << _DEC(packettimelow);
	unsigned long packettime = timetemp; //((time) & 0xffffff00UL) + packettimelow;
	//IFECHO Serial << F(" packettime 0: ") << packettime;
	switch (timesize) {
		case 1: 
			{
			byte locltimlow = (lowByte(time) >> 5);
			byte pckttimlow = (packettimelow >> 5);
			// network packet is probably in same 256 second window as local time
			//IFECHO Serial << F(" local time low: ") << locltimlow << F(" pckt time low: ") << pckttimlow;
			// if network node time is 7 and local time is 0, then network time may be in previous 256 second time 
			if ((6 <= pckttimlow) && (1 >= locltimlow)) packettime = ((time - 256UL) & 0xffffff00UL) + packettimelow;  
			//IFECHO Serial << F(" -1: ") << packettime;
			// if network node time is 0 and local time is 7, then network time may be in next 256 second time 
			if ((1 >= pckttimlow) && (6 <= locltimlow)) packettime = ((time + 256UL) & 0xffffff00UL) + packettimelow;  
			//IFECHO Serial << F(" +1: ") << packettime << endl;
			_time = packettime;
			}
			break;
		case 2:
			{
			unsigned int localtimeint = (unsigned int)time >> 5;
			unsigned int packettimeintx = packettimeint >> 5;
			// wraparound edge cases - not sure if these are valid edge values or can just be approximations
			if ((50000 <= packettimeintx) && (10000 >= localtimeint)) packettime = ((time - 65536UL) & 0xffff0000UL) + packettimeint;
			if ((10000 <= packettimeintx) && (50000 >= localtimeint)) packettime = ((time + 65536UL) & 0xffff0000UL) + packettimeint;
			}
			break;
		case 4:			// nothing to do - we got an actual exact time 
			break;
		default:
			break;
	}
	unsigned long savesum = sPayload.nonceSum;	// save old sum
	//IFECHO Serial << F(" savesum: ") << _HEX(savesum);
	// hack the packet, set sum in packet to what it should be
	unsigned long nonceTemp = sPayload.nonceSum >> 22;
	nonce = nonceTemp;

	// time, allowing for drift (+/- 32 seconds) // hint for this is in packet
	sPayload.nonceSum = (unsigned long)nonceTemp << 22; // keep session number
	sPayload.nonceSum += ((packettime >> 5) & 0x3fffffUL);   // add packet time key to hashed field // 3f,ff,ff = 22 bits + 5 bits (/32) = 27 bits
    unsigned long checksum = 0;
	checksum = SipTOTP (hmacKey, (char *) &sPayload, traffic_IX + MESSAGE_HDR_LENGTH);
	//sPayload.nonceSum = (unsigned long)(nonce << 22) + checksum;
	sPayload.nonceSum = (unsigned long)(nonceTemp << 22) + checksum;

	//IFECHO Serial << F(" calculated checksum ")<< _HEX(checksum) << F(" -> ") << _HEX(sPayload.nonceSum) << F(" vs ") << _HEX(savesum);

	if (!(sPayload.nonceSum == savesum)) {
		sPayload.nonceSum = savesum; 
		//IFECHO Serial << F(" *bad checksum*");return false;
		return status::ChecksumFail;
	}	// bad header
	byte paylen = sPayload.len;
	nonce = nonceTemp;

	// didn't find a reason to complain, call it good.
	return OK;
}

