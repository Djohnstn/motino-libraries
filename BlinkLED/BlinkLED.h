/*
BlinkLED.h - Arduino library for supporting the Base40 text
*/

#ifndef BlinkLED_H
#define BlinkLED_H
#include "Arduino.h"
class BlinkLED
{
	public: 
		byte _PIN;
		byte _ON;
		long LastBlink;		// millis of last blink
		BlinkLED(byte pin);
		void BlinkOn();
		void BlinkOff();
		void Blink(int DELAY_MS);
		bool BlinkEvery(int DELAY_MS, long everyMs);	// blink if not blinked in this time
		bool BlinkFree(int DELAY_MS, long everyMs);		// async blink if not blinked in this time
		
	private:

};
#endif
