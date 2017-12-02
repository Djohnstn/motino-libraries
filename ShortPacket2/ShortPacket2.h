/*
ShortPacket.h - Arduino library for supporting the Binary packed Data Packet transfers
*/

#ifndef SHORTPACKET2_H
#define SHORTPACKET2_H

class ShortPacket2 {
	
	public:
	
	#define MESSAGE_HDR_LENGTH 6     // how many bytes in the header
	#define MESSAGE_MAX_LENGTH 55    // maximum message length
	#define MESSAGE_MAX_DATA 49      // maximum message length - packet header

	struct PAYLOAD {
	  char type;  // '='
	  unsigned char len;  		// length of payload area in body space
	  unsigned long nonceSum;   // message# (10 bits), HMAC (22 bits)
								// nonceSum starts as current time/30, once HMAC calculated, replace with nonce and hmac
	  unsigned char body[MESSAGE_MAX_DATA];  // 64 byte message max length, after headers = 58,
								// fields are 1 byte with top 6 bits as tag, least two bits 0=1byte, 1=2bytes, 2=4bytes
	} sPayload;

	// packet received status
	enum status {
		OK,
		BadHeader,
		TooLong,
		TooShort,
		NoTimeCode,
		NonceOutOfOrder,
		ChecksumFail
	};
	unsigned long _time;
	unsigned long nonce;
	char traffic_IX = 0;  // 0th position is zeroth in array, max of 57
	char _scale;	// current scale of data values  (1E^(n))eg: -3 = *1E-3  
	bool _verbose = false;
	bool _full;
	const char MESSAGE_HEADR_ID = '='; // #[ - message type id number / socket

	// enable serial output
	void SpSetVerbose(bool verbosity);

	unsigned long SipTOTP(uint8_t phmacKey[], char* structure, int structurelength);

	
	// call this first to set up radio payload
	bool Begin(unsigned long time);
	
	// Add a new value	--- long, int, or byte
	bool Add(unsigned long time, char payload_name, unsigned long payload_value, char scale);
	bool Add(unsigned long time, char payload_name, long payload_value, char scale);
	bool Add(unsigned long time, char payload_name, int  payload_value, char scale);
	bool Add(unsigned long time, char payload_name, char payload_value, char scale);
	

	// finalize the packet
	bool Close(uint8_t phmacKey[]);

	// is the packet full if we add this much data?
	bool Full(signed char newdata);
	
	// print the packet
	void SerialDisplay();

	// how long is the packet?
	byte sendSize();

	// how long is the packet?
	byte dataSize();

	// pointer to packet
	byte* Data();

	// set payload
	byte setPayload(const void* Data, int dataLen);


	// is packet valid?
	ShortPacket2::status validPayload(unsigned long time, uint8_t hmacKey[]);

	// packet time
	unsigned long Time();	// return the current time
	char Scale();		// current scale

	// minipacket extractors

	byte miniPacketAttribute(byte index);
	byte miniPacketDataLength(byte attribute);
	
	byte firstPacket();
	byte nextPacket(byte mp);
	byte miniPacketName(byte mp);
	byte miniPacketLength(byte mp);

	byte minipacketIX(byte tag);

	byte miniPacketValue_c(byte mp);
	int  miniPacketValue_i(byte mp);
	long miniPacketValue_L(byte mp);

	private:
	bool AddTime(unsigned long time);				// add whatever part of time is changed
	bool Add_L(char payload_name, long payload_value);				// internal handles
	bool Add_i(char payload_name, int payload_value);
	bool Add_c(char payload_name, char payload_value);
	bool Add_E(char payload_scale);					// scale / scale 1E^(n)
	union timebyte {
		unsigned long integer;
		unsigned char byt[sizeof(long)];
	} trunctime;


};
#endif
