/*
ShortPacket.h - Arduino library for supporting the Binary packed Data Packet transfers
*/

#ifndef SHORTPACKET_H
#define SHORTPACKET_H

enum ENTRY_NAME {
  ent_A0,
  ent_RSSI,
  ent_RadioTemp,
  ent_CPUVcc,
  ent_CPUTemp,
  ent_Sessionp1,
  ent_checksum,
  ent_epoch,
  ent_millis
};

enum ENTRY_POWER {    // value * 10^(ENTRY_POWER)
  ex_int = 0,
  ex_deci = 1,
  ex_centi = 2,
  ex_milli = 3
};

enum ENTRY_TYPES {
  et_ubyte = 0x10,      // 1 byte
  et_sbyte = 0x18,      // 1 byte
  et_uint  = 0x20,      // 2 byte
  et_sint  = 0x28,      // 2 byte
  et_ulong  = 0x30,      // 4 byte
  et_slong  = 0x38,      // 4 byte
  et_session   = 0xe0,  // 2 byte sessionid - phase 0 / in use
  et_sessionp1 = 0xe4,  // 2 byte sessionid - phase 1 / propose - from gateway
  et_sessionp2 = 0xe8,  // 2 byte sessionid - phase 2 / commit - from gateway
  et_checksum = 0xf0    // 2 byte checksum
};

extern const char* TagNames[];


#define MESSAGE_HDR_LENGTH 7                // how many bytes in the header
#define MESSAGE_MAX_LENGTH 54                 // maximum message length
#define MESSAGE_MAX_DATA 48                 // maximum message length - ckecksumID, checksumlength, checksum, and trailer (total 6 bytes)

struct PAYLOAD {
  unsigned int type;  // #[
  unsigned int sessionid;    // session number, prevent replay attacks, this number changed whenever gateway feels like it, eg: 15 minutes or after control function sent
  unsigned int messageid;    // message number, prevent replay attacks, this number incremented with every radio output message
  unsigned char len;  // length of payload
  unsigned char traffic[MESSAGE_MAX_LENGTH];  // 64 byte message max length, after headers = 57, also - 3 bytes for checksum, - 2 bytes for trailer ]# signature, then fill packet with random number
};


// enable serial output
void SpSetVerbose(bool verbosity);

// session id
unsigned int getSessionid();

// message id
unsigned int getMessageid();


// call this first to set up radio payload
void prep_payload_header();

// LONG values
byte prep_payload_L(char payload_name, char payload_type, long payload_value);

// INT values
byte prep_payload_i(char payload_name, char payload_type, int payload_value);

// BYTE values
byte prep_payload_c(char payload_name, char payload_type, char payload_value);

// finalize the packet
byte prep_payload_final();

// print the packet
void serial_display_packet();

// how long is the packet?
byte getSendsize();

// pointer to packet
byte* getPayload();

// set payload
byte setPayload(const void* Data, int dataLen);


// is packet valid?
byte validPayload();

// minipacket extractors

byte firstPacket();
byte nextPacket(byte mp);
byte miniPacketName(byte mp);
byte miniPacketType(byte mp);
byte miniPacketLength(byte mp);

byte miniPacketValue_c(byte mp);
int miniPacketValue_i(byte mp);
long miniPacketValue_L(byte mp);


#endif
