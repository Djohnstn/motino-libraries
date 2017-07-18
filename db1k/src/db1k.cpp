#include "db1k.h"
#include <Streaming.h> // support serial << stuff; http://arduiniana.org/libraries/streaming/
#include <Base40.h>		// base40

// read a byte range
void db1k::readEEPROM(byte data[], int offset, int length) {
	//Serial << " read: @ " << offset << " for: " << length; 
	for (int ix = 0; ix < length; ix++) {
		data[ix] = EEPROM.read(offset + ix);
	}
}

// update a byte range
void db1k::writeEEPROM(byte data[], int offset, int length) {
	//Serial << " write: @ " << offset << " for: " << length; 
    for (int ix = 0; ix < length; ix++) {
	  // only write if value does not match
	  if (EEPROM.read(offset + ix) != data[ix]) {
		EEPROM.write(offset + ix, data[ix]);
	  }
    }
}

// constructir sets eeprom offset 
db1k::db1k(void) {
  db1k::_eepromOffset = 65;
}

// init the database
void db1k::begin() {
	int signature = DB1K_EEPROM_SIGNATURE;
	byte* ptr = (byte*)&signature;
	writeEEPROM(ptr, db1k::_eepromOffset, db1k_siglength);
}


bool db1k::checkSIGNATURE() {
	int sig = (EEPROM.read(db1k::_eepromOffset)) + (EEPROM.read(db1k::_eepromOffset + 1) << 8);
	bool foundsig = (DB1K_EEPROM_SIGNATURE == sig);
	//Serial << " sig:" << _HEX(sig) << "=" << _HEX(DB1K_EEPROM_SIGNATURE) << "?" << foundsig << ";";
	return foundsig;
}



// find a key
int db1k::findKey(word base40key) {
	int savepos = 0;
	if (checkSIGNATURE()) {
		int recordpos = 0;
		for (recordpos = (db1k::_eepromOffset + db1k_siglength); recordpos < (EEPROM.length() - db1k_recordlength); recordpos += db1k_recordlength) {
			int key;
			byte* ptr = (byte*)&key;
			readEEPROM(ptr, recordpos, db1k_keylength);
			if (key == base40key) {
				//Serial << " pos: " << recordpos;
				return recordpos;			// found it!
			}
			if (((key == -1) || (key == 0) )& (savepos == 0)) {
				savepos = recordpos;		// save first available slot
			}
		}
	}
	//Serial << " POS: " << savepos;
	return savepos;
}

// does value exist?
  bool db1k::exists(word base40key) {
	  return (0 != findKey(base40key));
  }


// set a value
bool db1k::writeKeyValue(word base40key, long value) {
	//Serial << "writeKeyValue " << base40key << " " << value << " " << checkSIGNATURE() << " ";
	if (checkSIGNATURE()) {
		int recordpos = findKey(base40key);
		//Serial << " writeKeyValue: recordpos: " << recordpos;
		if (recordpos) {
			byte* ptr = (byte*)&base40key;
			byte* ptr2 = (byte*)&value;
			writeEEPROM(ptr, recordpos, db1k_keylength);
			writeEEPROM(ptr2, recordpos + db1k_keylength, db1k_datalength);
			return true;
			//}
		}
	}
	return false;
}

// delete a value
bool db1k::deleteKeyValue(word base40key) {
	//Serial << "writeKeyValue " << base40key << " " << value << " " << checkSIGNATURE() << " ";
	if (checkSIGNATURE()) {
		int recordpos = findKey(base40key);
		//Serial << " writeKeyValue: recordpos: " << recordpos;
		if (recordpos) {
			word zerokey = -1;
			long zerovalue = -1;
			byte* ptr = (byte*)&zerokey;
			byte* ptr2 = (byte*)&zerovalue;
			writeEEPROM(ptr, recordpos, db1k_keylength);
			writeEEPROM(ptr2, recordpos + db1k_keylength, db1k_datalength);
			return true;
			//}
		}
	}
	return false;
}


long db1k::readValue(word base40key, long defaultvalue) {
	if (checkSIGNATURE()) {
		int recordpos = findKey(base40key);
		if (recordpos) {
			int key;
			byte* ptr = (byte*)&key;
			readEEPROM(ptr, recordpos, db1k_keylength);
			if (key == base40key) {
				long value;
				byte* ptrv = (byte*)&value;
				readEEPROM(ptrv, recordpos + db1k_keylength, db1k_datalength);
				return value;
			}
		}
		//}
	}
	return defaultvalue;
}

	// read long from database
unsigned long db1k::getLong(char key1, char key2, char key3){
	Base40 k (key1, key2, key3);
	//k.decode();
	//word key = k._results;//Base40.base40c3(key1, key2, key3);
	unsigned long value = readValue(k._base40, 0);
	return value;
}

	// read byte from database
byte db1k::getByte(char key1, char key2, char key3) {
	unsigned long value = db1k::getLong(key1, key2, key3);
	byte valueb = value;
	return valueb;
}


// dump eeprom to serial
//void db1k::Dump () {
//	Serial << endl;
//	for (int address = 0;  address < EEPROM.length(); address += 32) {
//		// print address
//		Serial << (address < 256 ? F("0"): F("")) << (address < 16 ? F("0"): F("")) << _HEX(address) << F(" ");
//		// print data
//		for (int offset = 0; offset < 32; offset++) {
//		  byte value = EEPROM.read(address + offset);
//		  Serial << F(" ") << (value < 16 ? F("0"): F("")); if (255 == value) {Serial << F("..");} else {Serial << _HEX(value);}
//		  if (3 == (offset & 3)) Serial << F(" ");
//		  if (7 == (offset & 7)) Serial << F(":");
//		}
//		Serial << endl; 
//	}
//}

// dump eeprom database to serial
void db1k::dumpdb () {
	Serial << endl;
	if (checkSIGNATURE()) {
		for (int recordpos = (db1k::_eepromOffset + db1k_siglength); recordpos < (EEPROM.length() - db1k_recordlength); recordpos += db1k_recordlength) {
			int key;
			byte* ptr = (byte*)&key;
			readEEPROM(ptr, recordpos, db1k_keylength);
			if ((0 != key ) && (-1 != key)) {
				Base40 expname ('-', '-', '-');
				expname._base40 = key;
				expname.decode();
				Serial << "#"; // << recordpos << F("] ") 
				Serial << expname._results << F("=\"");
				unsigned long value;
				byte* ptrv = (byte*)&value;
				readEEPROM(ptrv, recordpos + db1k_keylength, db1k_datalength);
				char c0 = value >> 24;
				char c1 = value >> 16;
				char c2 = value >> 8;
				char c3 = value;
				if (isPrintable(c0)) Serial.print(c0); 
				if (isPrintable(c1)) Serial.print(c1);
                if (isPrintable(c2)) Serial.print(c2); 
				if (isPrintable(c3)) Serial.print(c3); 
				Serial << F("\", $")<< _HEX(value) << F(", #") << value << endl;
			}
		}
	}
	//Serial << " POS: " << savepos;
}

//struct MyObject {
//  float field1;
//  byte field2;
//  char name[10];
//};

//void eepromDExample() {
//  byte value;
//  int address = 0;
//  value = EEPROM.read(address);
//  address  &= (EEPROM.length() - 1) ;
//  Serial << address << F("\t") << value << endl; 
//  //EEPROM.write(address, value);
//  for (int index = 0 ; index < EEPROM.length() ; index++) {
//
//    //Add one to each cell in the EEPROM
//    //EEPROM[ index ] += 1;
//  }
//  EEPROM.update(address, val);
//
//  float f = 0.00f;
//  EEPROM.get(address, f); // gets float f from address
//
//  MyObject customVar; //Variable to store custom object read from EEPROM.
//  EEPROM.get(eeAddress, customVar);
//
//  MyObject customVar2 = {
//    3.14f,
//    65,
//    "Working!"
//  };
//  EEPROM.put(address, customVar);
//  
//}
