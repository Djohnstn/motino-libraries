/**
 * Library that stores readings and times and sent status
 * http://www.codeproject.com/Articles/257589/An-Idiots-Guide-to-Cplusplus-Templates-Part
 */

#ifndef READING2_H
#define READING2_H

#include <Arduino.h>
//#include <limits>
//#include <Streaming.h>
#include <ShortPacket2.h>  // data packet definitions

#ifdef DEBUG
#undef DEBUG
#endif

template<class T>
class Reading2 {

#define LOCALHISTORY 5

  public:

  enum ReadingState
	{
		Invalid = 1,
		Valid = 2,
		NeedSend = 4,
		PreSent = 8,
		Sent = 16
	};
  
  unsigned long nowtime;		// time base, all data times are offset from here
  int oldtime[LOCALHISTORY];
  T olddata[LOCALHISTORY];
  ReadingState oldState[LOCALHISTORY];
  char taginfo;
  char _scale;		// power of number -3 = millis, -2 = hundredths, -1 = tenths
  char _width;		// variance of data allowed 
  Reading2(char tag, char scale)
  {
	Reading2(tag, scale, 0);
  }
  Reading2(char tag, char scale, char width)
  {
	taginfo = tag;
	_scale = scale;
	_width = width;	//fix 1 - 7/30/2017
	for (byte ix = 0; ix < LOCALHISTORY; ix++) {
	  oldState[ix] = ReadingState::Invalid;
	}
  }
  
  // char Tag() {
	  // return taginfo;
  // }
  
  byte Length() {
	  if (sizeof(T) == 1) return 0;
	  else if (sizeof(T) == 2) return 1;
	  else if (sizeof(T) == 4) return 2;
	  return 3;
  }
  
  bool Add(T newdata, unsigned long newtime)
  {
	ReadingState newState = ReadingState::Valid;
	// actions to do
	bool pushData = false;
	bool reTime = false;
	bool didSomething = false;
	// if startup, just put data in
	//Serial.print("+A");
	if (ReadingState::Invalid == oldState[0])
	{
		//Serial.print("+I");
		//retime = true;
		nowtime = newtime;
		oldtime[0] = 0; // delta from newtime to olddtime
		newState = ReadingState::NeedSend;
		oldState[0] = newState;    // brand new data needs to be sent
		olddata[0] = newdata;
		didSomething = true;
	}
	else
	{       
		// running, push old data back, if new data significantly different or old data or something
		if (abs((int)(newdata - olddata[0])) <= _width)	// question this logic?
		{
			// //Serial.print("<W");
			// duplicate data, just fix time entry
			if (ReadingState::Valid == oldState[0])
			{
				//Serial.print("+t");
				reTime = true;
			}
			else
			{
				//Serial.print("+d");
				pushData = true;
			}
			didSomething = true;
		}
		else
		{
			//Serial.print(">W");
			// data changed or new, push it all down
			// need to think about how to avoid bouncing between two numbers due to analog to digital round off;
/* 			T maxval = olddata[0];
			T minval = maxval;
			char validValues = 0;
			for (int ix = LOCALHISTORY - 2; ix >= 0; ix--) 
			{
				if (ReadingState::Invalid != oldState[ix]) {
					validValues++;
					if (olddata[ix] > maxval) maxval = olddata[ix];
					if (olddata[ix] < minval) minval = olddata[ix];
				}
			}
			T range = maxval - minval;
			// are we bouncing between two values?
			// diff from slope: 4,4,5,5,5 -> bounce: 4,5,4,5,4
			// do we add differences?
			if (newdata - olddata[0] <= 1 & range <= 1 & validValues > 2)
			{
				// fix this newnumber as 1/2 of between old and new and fix temp scale for extra 0.5 ?
				// not right now, just mark new data as valid, no send needed
			}
			else {	// ok, its different and not a bounce, so send it
				newState = ReadingState::NeedSend;
			}
 */
			newState = ReadingState::NeedSend;
			pushData = true;
			didSomething = true;
		}
	}
	if (pushData)
	{
		//Serial.print("+P");
		// check if needsend should be popped up
		if (ReadingState::NeedSend == oldState[LOCALHISTORY - 1]) 
		{
			oldState[LOCALHISTORY - 2] = ReadingState::NeedSend;
		}
		// push data down
		for (int ix = LOCALHISTORY - 2; ix >= 0; ix--)
		{
			oldtime[ix + 1] = oldtime[ix];
			oldState[ix + 1] = oldState[ix];
			olddata[ix + 1] = olddata[ix];
		}
		olddata[0] = newdata;
		oldState[0] = newState;
		if (ReadingState::Valid == oldState[1]) oldState[1] = ReadingState::NeedSend;
		reTime = true;
	}
	if (reTime)
	{
		//Serial.print("+T");
		// oldtime[1] = (int)(newtime - (nowtime + oldtime[1]));
		oldtime[1] = (int)(newtime - (nowtime - oldtime[1]));	// fix 2 - 7/30/2017
		// if in danger of overflowing the time delta, force a send.
		if (oldtime[1] > 30000 && ReadingState::Valid == oldState[0]) oldState[0] = ReadingState::NeedSend;
		oldtime[0] = 0;
		nowtime = newtime;
	}
	return didSomething;

  }
  
  unsigned long GetTime(int8_t ix)
  {		// get time for first value that needs to be sent, assume gettime always followed by getdata!
		// can do multiple datachanged() / gettime / getdata() triples
		// or one / send(buffer) / send (buffer) / sendto radio / sent() or / unsent() 
	unsigned long tm = nowtime;	// is this too simplistic?
	// uint8_t n = -1;
	//Serial.print("?T");
	if (ix >= 0 && ix < LOCALHISTORY && ReadingState::NeedSend == oldState[ix]) {
		tm = nowtime;
		//Serial.print("?s");
		for (int iy = ix; iy >= 0; iy--) 
		{
			//Serial.print(iy);
			tm -= oldtime[iy];	// back down all the time differences.
		}
	}
#ifdef DEBUG
		//Serial.print("{n"); Serial.print(ix, DEC); Serial.print(" T"); Serial.print(tm); Serial.println("}"); 
#endif
		return tm;
  }

  T GetData(int8_t ix)
  {
	//Serial.print("?V");
	if (ix >= 0 && ix < LOCALHISTORY && ReadingState::NeedSend == oldState[ix]) {
		//Serial.print(ix);
		oldState[ix] = ReadingState::PreSent;
		return olddata[ix];
	}
	// }
	//Serial.print("f");
	return -1;
  }

  void SentOK()
  {		// tells object the data has all been sent marked ::preSent
	for (int ix = LOCALHISTORY - 1; ix >= 0; ix--) 
	{
		if (ReadingState::PreSent == oldState[ix]) {
			oldState[ix] = ReadingState::Sent;
		}
	}
  }
  // data changed if beyond threshold;
  bool dataChanged(T delta)
  {
	  return dataChanged();
  }
  int8_t dataChanged()
  {
	//Serial.print("?D");
	for (int8_t ix = LOCALHISTORY - 1; ix >= 0; ix--) 
	{
		if (ReadingState::NeedSend == oldState[ix]) {
			//Serial.print(ix);
			//Serial.print("T");
			return ix;
		}
	}
	//Serial.print("F");
	return -1;
  }

  // send one piece of data to buffer
  bool Send(ShortPacket2* packet) 
  {
	  //Serial.print("@S"); Serial.print (taginfo, DEC);
	  bool worked = false;
	  int8_t x = dataChanged();
	  if (x > 0) 
	  {
		  if (!packet->Full(2)) {
			//Serial.print("~");
			worked = packet -> Add(GetTime(x), taginfo, GetData(x), _scale);
			//Serial.print(worked);
		  }
	  }
	  // else 
	  // {
		// Serial.print("X");
	  // }
	  return worked;
  }
  
  void Unsent()
  {		// didn't send data, return it to unsent, needing send;
	for (int ix = LOCALHISTORY - 1; ix >= 0; ix--) 
	{
		if (ReadingState::PreSent == oldState[ix]) {
			oldState[ix] = ReadingState::NeedSend;
		}
	}
  }

    // print top of buffer
  void Print() 
  {
	char space= ' ';
	Serial.print (taginfo, DEC);
	Serial.print (space);
	Serial.print (nowtime);
	Serial.print (space);
	Serial.print (olddata[0]);
	Serial.print ('E');
	Serial.println (_scale, DEC);
  }

  // dump all values 
  void PrintAll() 
  {
 #ifdef DEBUG
	Serial.print("* Tag:"); Serial.print (taginfo, DEC);
	Serial.print(" E:"); Serial.print (_scale, DEC);
	Serial.print(" Wid:"); Serial.print (_width, DEC);
	Serial.print(" Now:"); Serial.println(nowtime);
	for (int ix = 0; ix < LOCALHISTORY; ix++) 
	{
		Serial.print("["); Serial.print(ix);
		Serial.print("] T:"); Serial.print(oldtime[ix]);
		Serial.print(" S:"); Serial.print(oldState[ix]);
		Serial.print(" D:"); Serial.println(olddata[ix],DEC);

	}
#endif
  }
};


#endif
