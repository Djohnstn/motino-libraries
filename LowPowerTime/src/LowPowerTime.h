/**
 * Library that stores readings and times and sent status
 * http://www.codeproject.com/Articles/257589/An-Idiots-Guide-to-Cplusplus-Templates-Part
 */

#ifndef LOWPOWERTIME_H
#define LOWPOWERTIME_H

#include <Arduino.h>
//#include <limits>
#include <Streaming.h>
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <LowPower.h>  // https://github.com/LowPowerLab/LowPower

class LowPowerTime {
  public:
	const char* czero = "0";
	const char* cnada = "";
	//const char* chhmm = ":";
	//const char* cdot = ".";
	const char* cat = "@";

  public:
	boolean radiosleeping;  // was radio put to sleep?
	boolean flashsleeping;  // was flash put to sleep?
	int ok = 0;				// is time ok? set properly
	unsigned long poweroffMillis = 0; // how many millis were we powered off?
	long transmitperiod = 3600000;	// long sanity guard, default 1 hour, milliseconds

	unsigned long timeBaseEpoch; // unix seconds
	unsigned long timeBaseMilliseconds; // this 47 day millisecond base time
	unsigned long timeBaseLastMilliseconds; // this 47 day millisecond base time
	int lastSecond_Milli;
	unsigned long MillisPerSec = 1000UL; // local millis per second
	
	RFM69* _radio;
	
	// replace myMillis() call with local call that counts power off millis as well
	volatile unsigned long myMillis() {
	  unsigned long thisMillis = millis();
	  thisMillis += poweroffMillis;
	  return thisMillis;
	}
	
	unsigned long Seconds() 
	{
		unsigned long milliNow = myMillis();
		//Serial << F(" nowM:") << milliNow;
		if (milliNow < timeBaseLastMilliseconds) {
			// fix numbers for roll-over 4.2G milliseconds = 49.710 days
			unsigned long millifix = milliNow - timeBaseLastMilliseconds;
			millifix = millifix - (millifix % 1000UL); // truncate back to previous full second
			unsigned long secondfix = millifix / 1000UL;	// how many seconds to address
			timeBaseEpoch += secondfix;				// new second base
			timeBaseMilliseconds += millifix;		// new milli base
		}
		unsigned long milliOffset = milliNow - timeBaseMilliseconds;
		//Serial << F(" baseM:") << timeBaseMilliseconds;
		//Serial << F(" offM:") << milliOffset;
		timeBaseLastMilliseconds = milliNow; // keep previous reading
		unsigned long secondsOffset = milliOffset / MillisPerSec;
		lastSecond_Milli = milliOffset % MillisPerSec; //save the milli offset 
		// unix seconds offset - 1099 is to compensate for fast clock 910 for slow clock - will have to work on this...
		//Serial << F(" offS:") << secondsOffset;
		//Serial << F(" baseS:") << timeBaseEpoch;
		return timeBaseEpoch + secondsOffset;
	}

	// what was that last millisecond?
	unsigned long SecondsMillis() 
	{
		return lastSecond_Milli * 1000UL / MillisPerSec; //scale out the milli offset 
	}

	void printFormatted() {
		unsigned long s = Seconds();
		unsigned long ms = SecondsMillis();
		unsigned long todaytime = s % 86400UL;
		int hh = todaytime / 3600UL;
		int mm = (todaytime % 3600UL) / 60;
		int ss = todaytime % 60;
		Serial	<< cat
				<< (hh < 10 ? czero : cnada) << hh << ":" 
				<< (mm < 10 ? czero : cnada) << mm << ":" 
				<< (ss < 10 ? czero : cnada) << ss << "." 
				<< (ms < 100? czero : cnada) << (ms < 10? czero : cnada) << ms;  
		Serial << F(" ") << s << F("s; ");
	}
	void printSecondsMillis() {
		unsigned long s = Seconds();
		unsigned long ms = SecondsMillis();
		Serial	//<< cat
				<< (s) << "." 
				<< (ms < 100? czero : cnada) << (ms < 10? czero : cnada) << ms;  
	}

		// is time base Ok?
	boolean OK() 
	{ 
		//if (0 == timeBaseEpoch) ok = false;		// if time was never set, we are not sure
		// if we are not sure of time, say so.
		if (ok < 2) return false;
		// so, OK is true, so ...
		if (0 == timeBaseEpoch) return false;		// if time was never set, we really are not sure
		long diffS = (long)(Seconds() - timeBaseEpoch);	// seconds difference between now and last time set
		return (diffS < 7200L);	// as long as we have seen new time within 2 hours, time must be Ok, otherwise not  sure of time sync any more
	}

	// if sent 10.951 seconds, can back up current 10 second on millisecond base by 951 milliseconds
	unsigned long Seconds(unsigned long currentTime, int milliOffset) 
	{ 
		unsigned long now = Seconds();		// what time is it now?
		long drift = (currentTime - now);	// how far has clock drifted?
		// calculate new seconds per millisecond rate
		unsigned long diffS = currentTime - timeBaseEpoch;	// seconds difference
		if (currentTime) {
			if (abs(drift) < 3) {
				if (ok < 100) ok++;	// count OK level
			} // if less than 3 seconds difference, time is OK // unless time = 0
			else {
				ok = 0;
			}
		}		
		if (diffS > 15) {	// minimum seconds to consider new milliseconds per second	// what about time going backwards?
			//unsigned long diffSM = diffS * 1000UL; // set base time for this second
			unsigned long diffM = (myMillis() - milliOffset) - timeBaseMilliseconds; // milliseconds difference
			unsigned long rateSpM = diffM / diffS;	// milliseconds per second
			if ((rateSpM < 1600UL) && (rateSpM > 400UL)) {		// reasonable range and then some
				if (rateSpM > MillisPerSec) MillisPerSec++;		// bias the results up
				if (rateSpM < MillisPerSec) MillisPerSec--;		// bias the results down
				//MillisPerSec = ((MillisPerSec * 3 + rateSpM) + 2UL) >> 2;	// calculate new EMWA averaged milliseconds per second , +2 for rounding
				MillisPerSec = (MillisPerSec * 3 + rateSpM) >> 2;	// calculate new EMWA averaged milliseconds per second , with biased MillisPerSec
				//Serial << F(" ms/s: ") << MillisPerSec;
			}
		}
		// set current second base
		timeBaseEpoch = currentTime;
		timeBaseMilliseconds = myMillis() - milliOffset; // set base time for this second
		timeBaseLastMilliseconds = timeBaseMilliseconds; // keep previous reading
		return Seconds();
	}

	unsigned long Seconds(unsigned long currentTime) 
	{ // set current second base
		return Seconds(currentTime, 500);	// if we don't know the milliseconds, the average will be 500 milliseconds! Maybe.
	}
	
	void sleepRadioAndFlash() {
	  _radio -> sleep();
	  radiosleeping = true;  // radio is asleep
	//  if (!flashsleeping) flash.sleep();
	//  flashsleeping = true; // flash is asleep
	}

	void superLowPowerSleep1(unsigned long offMillis) {
      sleepRadioAndFlash();
	}

	void superLowPowerSleep2(period_t sleep_period, unsigned long offMillis) {
		  sleepRadioAndFlash();
		  LowPower.powerDown(sleep_period, ADC_OFF, BOD_OFF);  
		  poweroffMillis += offMillis; // sleep and keep up with how many millis we lost
		  //LowPower.idle(SLEEP_1S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
		  //              SPI_OFF, USART0_OFF, TWI_OFF);
	}

	void lowPowerDelay(long thisdelay) {
		int sanity = 128;
		if (thisdelay > transmitperiod) thisdelay = transmitperiod;  // guard against something insane
		//IFECHO Serial << F("Lp:") << thisdelay << endl;
		//if (thisdelay >= 15) {
		//	//IFECHO delay(10);
		//	superLowPowerSleep1(thisdelay);  // shut down radio and flash and flush serial
		//}
		//else {
		//}
		while ((thisdelay > 1) && (sanity-- >= 0)) { // 65 * 8 seconds is 8:40 delay; 255 is over 34 minutes
		  period_t lowpowerSleepTime = SLEEP_30MS;
		  int sleepMStoSubtract = 0;
		  // we have some time to sleep, use it
		  if      (thisdelay >= 8192) {lowpowerSleepTime = SLEEP_8S;    sleepMStoSubtract = 8192; }
		  else if (thisdelay >= 4096) {lowpowerSleepTime = SLEEP_4S;    sleepMStoSubtract = 4096; }
		  else if (thisdelay >= 2048) {lowpowerSleepTime = SLEEP_2S;    sleepMStoSubtract = 2048; }
		  else if (thisdelay >= 1024) {lowpowerSleepTime = SLEEP_1S;    sleepMStoSubtract = 1024; }
		  else if (thisdelay >= 512)  {lowpowerSleepTime = SLEEP_500MS; sleepMStoSubtract = 512;  }
		  else if (thisdelay >= 256)  {lowpowerSleepTime = SLEEP_250MS; sleepMStoSubtract = 256;  }
		  else if (thisdelay >= 128)  {lowpowerSleepTime = SLEEP_120MS; sleepMStoSubtract = 128;  }
		  else if (thisdelay >= 64)   {lowpowerSleepTime = SLEEP_60MS;  sleepMStoSubtract = 64;   }
		  else if (thisdelay >= 32)   {lowpowerSleepTime = SLEEP_30MS;  sleepMStoSubtract = 32;   }
		  else if (thisdelay >= 16)   {lowpowerSleepTime = SLEEP_15MS;  sleepMStoSubtract = 16;   }
		  else                        {                                 sleepMStoSubtract = 0;    }
		  if (0 != sleepMStoSubtract) {
			superLowPowerSleep2(lowpowerSleepTime, sleepMStoSubtract);  // cpu down
			thisdelay -= sleepMStoSubtract; // +1 for the blink above, get rid of when stop blinking
		  }
		  else if (thisdelay > 0) {
			superLowPowerSleep1(thisdelay);
			delay(thisdelay);
			thisdelay = 0;
		  }
		}
		//IFECHO Serial << "\n";
	}
};

#endif
