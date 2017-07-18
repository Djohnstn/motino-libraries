/*
 (c)2013 Forward Computing and Control Pty. Ltd. 
 www.forward.com.au
 This code may be freely used for both private and commercial use.
 Provide this copyright is maintained.
 */

#include "SipHash_2_4.h"
#include "HexConversionUtils.h"

const uint8_t key[] PROGMEM = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

void setup() {
  //while(!Serial); // for Due
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // allow a little time to connect the serialMonitor before running the rest of the setup.
  for (int i = 10; i>0; i--) {
    delay(1000);
    Serial.print(F(" "));
    Serial.print(i);
  }
  Serial.println();
  Serial.println(F(" This SipHash test produces the results shown in https://131002.net/siphash/siphash24.c"));
  printKey((uint8_t*)key);
  Serial.println( 
  F(" SipHash-2-4 output with\n   k = 00 01 02 ...\n   and\n   in = (empty string)\n   in = 00 (1 byte)\n   in = 00 01 (2 bytes)\n   in = 00 01 02 (3 bytes)\n   ...\n   in = 00 01 02 ... 3e (63 bytes)"));

  char tmp[17];
 
  for (int j=0; j<64;j++) {
    sipHash.initFromPROGMEM(key);
    for (int i=0; i<j;i++) {
      sipHash.updateHash((byte)i);
    }
    sipHash.finish(); // result in BigEndian format
    reverse64(sipHash.result); // chage to LittleEndian to match  https://131002.net/siphash/siphash24.c
    hexToAscii(sipHash.result,8,tmp,17);
    Serial.println(tmp);
  }
  
    Serial.println( 
  F(" Test with negative byte (>127) SipHash-2-4 output for in = 0x81 with  k = 00 01 02 ...\n result should be (in LittleEndian) 0E7E74971988233A"));
    sipHash.initFromPROGMEM(key);
     sipHash.updateHash((byte)0x81);
    sipHash.finish(); // result in BigEndian format
    reverse64(sipHash.result); // chage to LittleEndian to match  https://131002.net/siphash/siphash24.c
    hexToAscii(sipHash.result,8,tmp,17);
    Serial.println(tmp);
}

void printKey(unsigned char PROGMEM *key) {
  Serial.print(F(" Key:"));
  unsigned char tmpKey[16];
  memcpy_P(tmpKey,key,16);
  char tmp[33];
  hexToAscii((const unsigned char*)tmpKey,16,tmp,33);
  Serial.println(tmp);
}

void loop() { }






