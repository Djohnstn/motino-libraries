/*
Reboot.h - Arduino library for rebooting / resetting the arduino
*/

// ref: http://www.instructables.com/id/two-ways-to-reset-arduino-in-software/

#ifndef Reboot_H
#define Reboot_H
#include "Arduino.h"
class Reboot
{
	public: 
		void(* Reboot) (void) = 0;//declare reset function at address 0

		// example: Reboot(); //call reset 		

		
		// pdemetrios  pdemetrios Reply 5 years ago
		//sorry but the code has modified the correct is
		//void RebootWDT() {
		//	WDTCSR=(1<<WDE) | (1<<WDCE) ;
		//	WDTCSR= (1<<WDE)
		//	for(;;)
		//}
};
#endif
