/*
BlinkLED.h - Arduino library for supporting the Base40 text
*/

#ifndef FTDIAttached_H
#define FTDIAttached_H
#include "Arduino.h"

// FTDI ports for Motino, probably many devices. use this to see if we have serial connected.
#define D0 0
#define D1 1

class FTDIAttached
{
	public: 
		FTDIAttached();
		bool Attached();
		
	private:
		bool _Attached; // is the FTDI attached? assume not.

};
#endif

