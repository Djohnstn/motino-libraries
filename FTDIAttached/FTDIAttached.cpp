// FTDIAttached code
// J D Johnston // all commercial rights retained, except where code was from somewhere else...
// June 2015

#include <Arduino.h>
#include "FTDIAttached.h"

	bool check_FTDI() {
		//delay(100); // do we need to wait a bit for ports to settle?
		int val_d0;
		int val_d1;
		pinMode(D0, INPUT); // or ? INPUT_PULLUP D0 is normally classified as RX
		pinMode(D1, INPUT); // or ? INPUT_PULLUP D1 is normally classified as TX
		val_d0 = digitalRead(D0);
		val_d1 = digitalRead(D1);
		byte status = (val_d0 && val_d1)? 1: 0;
		return (status);
		//return (Attached()); // if both inputs are high, then attached we are, else not attached!
	}



	FTDIAttached::FTDIAttached() {
#ifdef __AVR_ATmega1284P__
		_Attached = true;	// hard code this for now
#else
		_Attached = check_FTDI();
#endif
	}

	bool FTDIAttached::Attached() {
		if (!_Attached) {
		//	if (Serial.available()) {_Attached = 1;}	// if there is serial input, then there must be serial attached
		}
		return _Attached;
	}



