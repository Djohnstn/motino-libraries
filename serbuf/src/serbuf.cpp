#include "serbuf.h"


// constructor 
serbuf::serbuf(void) {
  serbuf::serReset();
}


void serbuf::serReset() {
	//Serial.print ("serReset;");
	serPos = 0;
	serLen = 0;
}

void serbuf::ungetCh() {           // back up buffer position
  if (serPos > 0) {
    serPos--;
  }
}

char serbuf::getCh() {				// get character from current position of buffer
  if ((serPos >= serBufSize) || (serPos >= serLen)) {
    return 0;
  }
  else {
    char c = serBuff[serPos++]; // get character first, inc position second
    return(c);
  }
}

// serial readline
int serbuf::readline()
{
	serLen = serbuf::readline(Serial.read(), serBuff, serBufSize);
	return serLen;
}

// serial readline
int serbuf::readline(int readch, char *buffer, int len)
{
  static int pos = 0;
  int rpos;
  if (readch > 0) {
    switch (readch) {
      case '\n': // Ignore new-lines
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0;  // Reset position index ready for next time
        serbuf::serReset(); // mark the parser position
        return rpos;
      default:
		//Serial.print (readch, HEX);
        if (pos < len-1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
    }
  }
  if (pos >= len - 1) {
    //line is long enough, return it all
    rpos = pos;
    pos = 0;
    serReset(); // mark the parser position
    return rpos;
  }
  else {
    // No end of line has been found, so return -1.
    return -1;
  }
}


