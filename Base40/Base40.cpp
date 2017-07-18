// Base40 code
// J D Johnston // all commercial rights retained, except where code was from somewhere else...
// December 2015

#include <Arduino.h>
#include "Base40.h"

Base40::Base40(char a, char b, char c) {
	_base40 = base40c3(a, b, c);
}

// decode base 40
// (space)./0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZ or -./0123456789@abcdefghijklmnopqrstuvwxyz
char Base40::decode40(byte b) {
  return (b > 12 ? b - 13 + '@' : b + '-');
}

// unsigned int base40 to char[4]
void Base40::decode40tochar(unsigned int in, char results[]) {
  char first = in / 1600U;
  char mid = (in / 40U) % 40U;
  char last = in % 40U;
  results[0] = Base40::decode40(first);
  results[1] = Base40::decode40(mid);
  results[2] = Base40::decode40(last);
  results[3] = 0;
}

void Base40::decode() {
	Base40::decode40tochar(_base40, _results);
}


