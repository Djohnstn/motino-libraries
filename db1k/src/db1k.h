/**
 * Library that stores, reads and configures Arduino  settings
 * 960 bytes of EEPROM are required to store the configuration.
 * large amounts from NetEEPROM
 */

#ifndef DB1K_H
#define DB1K_H

#include <Arduino.h>
#include <EEPROM.h>

/* SIGNATURE number that starts the database memory */
#define DB1K_EEPROM_SIGNATURE 0xdb1b

class db1k {

  public:

  /* constructor */
  db1k();

  /* Sets the location in EEPROM that configuration data will be
   * stored. If not called, then it defaults to 64. This method should
   * generally be called before any other methods.
   */
  //void setEepromOffset(int offset);

  /* Configures the network adapter from settings stored in EEPROM.
   * If no settings can be found in EEPROM then a database will be initialised
   */
  void begin();

    /* does key exist */
  bool exists(word base40key);

  /* store value for key */
  bool writeKeyValue(word base40key, long value);

  /* demove key and value */
  bool deleteKeyValue(word base40key);

  /* Reads value from EEPROM.
   * Parameters:
   *   name of value in base40 format
   * 	value of default if key not found
   */
  long readValue(word base40key, long defaultvalue);

	// read byte from database
  byte getByte(char key1, char key2, char key3);
	// red long from database
  unsigned long getLong(char key1, char key2, char key3);

  /* Checks whether the SIGNATURE number exists in the expected EEPROM location.
   * If the SIGNATURE number exists then it is likely that settings have
   * been stored in EEPROM.
   */
  bool checkSIGNATURE();

  // dump contents of eeprom
  //void Dump();
  // dump contents of eeprom database
  void dumpdb();

  
#define db1k_siglength 2
#define db1k_keylength 2
#define db1k_datalength 4
#define db1k_recordlength 6

  private:
  int _eepromOffset;
  //int _offset;
  void readEEPROM(byte data[], int offset, int length);
  void writeEEPROM(byte data[], int offset, int length);
  // find a value
  int findKey(word base40key);
};

//extern db1k DB1K;

#endif
