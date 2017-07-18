/**
 * Library that processes serial data from a buffer into database
 */

#ifndef SERBUF_H
#define SERBUF_H

#include <Arduino.h>

#define serBufSize 20

class serbuf {

  public:
	char serBuff[serBufSize];

	serbuf();		// constructor

	void serReset();
	void ungetCh();
	char getCh();
	int readline();
	int readline(int readch, char *buffer, int len);
	
	private:
	char serPos;
	char serLen;

  
  };


#endif
