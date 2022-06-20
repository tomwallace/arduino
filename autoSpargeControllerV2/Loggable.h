/*
  Loggable.h - Library base class manages sending logging messages out to the serial port.
  Created by Tom Wallace.
*/
#ifndef Loggable_h
#define Loggable_h

#include "Arduino.h"

class Loggable {
  public: 
	Loggable();
	void Log(long currentMillis, String callingObjName, String msg);
};

#endif