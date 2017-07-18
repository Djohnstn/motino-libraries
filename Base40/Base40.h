/*
Base40.h - Arduino library for supporting the Base40 text
*/

#ifndef BASE40_H
#define BASE40_H
#include "Arduino.h"
class Base40
{
	public: 
		unsigned int _base40;
		char _results[4];	
		Base40(char a, char b, char c);
		
		// with constants, these inline functions get optimized into nothing (very often, nice compiler!)
		// get base 40 of a character value
		inline unsigned int base40(char ic, unsigned int fac) {
		  // -=0,.=1,/=2,0=3,...,9=12,@=13,A=14,...,Z=39,a=14,...,z=39
		  // if number,val=val- '-', lower,val=from 'a', upper,val=from '@'
		  return (ic < '@' ? ic - '-' : (ic & 0x5f) - '@' + 13) * fac;
		}

		// get base 40 value of 3 characters
		inline unsigned int base40c3(char c1, char c2, char c3) {
		  return base40(c1, 1600U) + base40(c2, 40U) + base40(c3, 1U); 
		}

		void decode();
		
	private:
		void decode40tochar(unsigned int in, char results[]);
		char decode40(byte b);

};
#endif
