/**
 * Library that processes serial data from a buffer into database
 */

#ifndef SERCONFIG_H
#define SERCONFIG_H_H

#include <Arduino.h>
#include <Streaming.h> // support serial << stuff; http://arduiniana.org/libraries/streaming/
#include <Base40.h> // database keys
#include <db1k.h> // eeprom database
#include <serbuf.h> // serial buffer processing

// serial protocol
//  1234567890123456  - length
//  !abc"text         - 4 chars of text
//  !abc#4294967295   - long int
//  !abc$abcdef01     - hex long int
//  !abc!?            - print value - not implemented
//  !abc!x            - delete value
// base40
//  0....+....1....+....2....+....3....+....
//  -./0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZ



class serconfig {

  public:
	void settingsCommand(serbuf& sb, db1k& db, boolean attached);
  };


#endif
