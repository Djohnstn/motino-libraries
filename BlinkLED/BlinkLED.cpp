// BlinkLED code
// J D Johnston // all commercial rights retained, except where code was from somewhere else...
// June 2016, October 2016

#include <Arduino.h>
#include "BlinkLED.h"

	BlinkLED::BlinkLED(byte pin) {
		_PIN = pin;
		_ON = false;
		pinMode(_PIN, OUTPUT);
		LastBlink = 0;
	}

	void BlinkLED::BlinkOn() {
		digitalWrite(_PIN, HIGH);
		LastBlink = millis();	// reset blink counter when we do blink 
		_ON = true;
	}

	void BlinkLED::BlinkOff() {
		digitalWrite(_PIN, LOW);
		_ON = false;
	}

	void BlinkLED::Blink(int DELAY_MS)
	{
		BlinkOn();
		delay(DELAY_MS);
		BlinkOff();
	}

	bool BlinkLED::BlinkEvery(int DELAY_MS, long everyMs)
	{
		long nowMillis = millis();
		if ((nowMillis - LastBlink) >= everyMs) {
			//LastBlink = ++nowMillis;    // save this time
			Blink(DELAY_MS);
			return true;
		}
		else {
			return false;
		}
	}

	// non-Blocking BlinkLED Function 
	bool BlinkLED::BlinkFree(int DELAY_MS, long everyMs)
	{
		long nowMillis = millis();
		if (_ON && ((nowMillis - LastBlink) >= DELAY_MS)) {
			 BlinkOff(); 
		}
		else {
			if (!_ON && ((nowMillis - LastBlink) >= everyMs)) {
				BlinkOn();
			}
			else {
			}
		}
		return _ON;
	}

