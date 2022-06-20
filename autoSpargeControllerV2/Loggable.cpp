/*
  Loggable.cpp - Library base class manages sending logging messages out to the serial port.
  Created by Tom Wallace.
*/

#include "Arduino.h"
#include "Loggable.h"
#include "EventQueue.h"

Loggable::Loggable() {};
  
void Loggable::Log(long currentMillis, String callingObjName, String msg) {
	Serial.println(String(currentMillis) + " - " + callingObjName + ": " + msg);
}