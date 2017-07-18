/**
 * Library that stores readings and times and sent status
 * http://www.codeproject.com/Articles/257589/An-Idiots-Guide-to-Cplusplus-Templates-Part
 */

#ifndef READING2_H
#define READING2_H

#include <Arduino.h>
//#include <limits>
#include <Streaming.h>


template<class T>
class Reading2 {

#define LOCALHISTORY 4

  public:

  enum ReadingState
	{
		Invalid = 0,
		Valid = 1,
		NeedSend = 2,
		PreSent = 4,
		Sent = 8
	};

  
  unsigned long nowtime;		// time base, all data times are offset from here
  int oldtime[LOCALHISTORY];

  //T nowdata;
  T olddata[LOCALHISTORY];

  char taginfo;
  //bool sentValid;
  //byte oldflags[LOCALHISTORY];

  //ReadingState nowState;
  ReadingState oldState[LOCALHISTORY];
  
  Reading(char tag)
  {
	  taginfo = tag;
	  //nowState = Invalid;
	  for (int ix = 0; ix < LOCALHISTORY; ix++) {
		  oldState[ix] = ReadingState::Invalid;
	  }
  }
  
  char Tag() {
	  return taginfo;
  }
  
  byte Length() {
	  if (sizeof(T) == 1) return 0;
	  if (sizeof(T) == 2) return 1;
	  if (sizeof(T) == 4) return 2;
	  return 3;
  }
  
  bool Add(T newdata, unsigned long newtime)
  {
	  olddata = nowdata;
	  oldtime = nowtime;
	  oldsent = nowsent;
	  oldvalid = nowvalid;
	  oldPreSent = nowPreSent;
	  oldNeedSend = (oldsent)? false : (oldNeedSend | nowNeedSend);	// of old needed to be sent, new old still needs to be sent //jdj6/5/17
	  //Serial << F("d:") << newdata << F("t:") << newtime << endl;
 	  nowdata = newdata;
	  nowtime = newtime;
	  nowsent = false;
	  nowvalid = true;
	  nowPreSent = false;
	  nowNeedSend = (nowdata == olddata)? false: true;	 //jdj6/5/17
	  if ((nowtime - oldtime) > 900UL) {nowNeedSend = true;}	// if time has jumped more than a bit, need to send data
	  return dataChanged();
  }
  
  unsigned long GetTime()
  {
	if (oldvalid  && (!oldsent)) {
		return oldtime;
	} 
	else {
		if (nowvalid  && (!nowsent)) {
			return nowtime;
		} 
	}
	return 0UL;
  }

  T GetData()
  {
	if (oldvalid  && (!oldsent)) {
		oldPreSent = true;
		return olddata;
	} 
	else {
		if (nowvalid  && (!nowsent)) {
			nowPreSent = true;
			return nowdata;
		} 
	}
	//return std::numeric_limits<T>.min();
	return -1;
  }

  void Sent()
  {
	  if (oldPreSent & !oldsent) {sentdata = olddata; sentValid = true; oldsent = true; oldNeedSend = false;}
	  else if (nowPreSent & !nowsent) {sentdata = nowdata; sentValid = true; nowsent = true; nowNeedSend = false;}
  }
  bool dataChanged(T delta)
  {
	  if (!nowvalid && !oldvalid) return false;
	  if (oldNeedSend || nowNeedSend) return true; 
	  if (((abs(nowdata - olddata) > delta) || (sentValid && (abs(nowdata - sentdata) > delta))) && (!nowsent) && (nowvalid)) {
		if (!oldsent) oldNeedSend = true;
		nowNeedSend = true;
		return true;  
	  }
	  return false;
  }
  bool dataChanged()
  {
	  if (!nowvalid && !oldvalid) return false;
	  if (oldNeedSend || nowNeedSend) return true; 
	  if ((nowdata != olddata) && (!nowsent) && (nowvalid)) {
		if (!oldsent) oldNeedSend = true;
		nowNeedSend = true;
		return true;  
	  }
	  return false;
  }

  // send one piece of data to buffer
  bool Send(buffer* buff) 
  {
	  
  }
  
  void Unsent()
  {
	  if (oldPreSent && !oldsent) {oldPreSent= false; oldsent = false;}
	  else if (nowPreSent && !nowsent) {nowPreSent = false; nowsent = true;}
  }

  
};


#endif
