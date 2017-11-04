/*
  Beeper.cpp - Library for represents a sound output, such as the alarm or buzzer.  The sound is controlled by an EventQueue.
  Created by Tom Wallace.
*/

#include "Arduino.h"
#include "Beeper.h"
#include "EventQueue.h"
#include "Loggable.h"

Beeper::Beeper(String beeperName, int outputPin, EventQueue * eventQueue) {
	_beeperName = beeperName;
	_outputPin = outputPin;
	_eventQueue = eventQueue;

	SOUND = HIGH;
	SILENT = LOW;

	pinMode(_outputPin, OUTPUT);
	digitalWrite(_outputPin, SILENT);
	_currentState = SILENT;
}

void Beeper::Update(long currentMillis) {
    
	int OriginalState = _currentState;
    
    if (_eventQueue->IsPopulated()) {
		digitalWrite(_outputPin, SOUND);
		_currentState = SOUND;
    } else {
		digitalWrite(_outputPin, SILENT);
		_currentState = SILENT;
    }
      
    // If state changed, then log
    if (_currentState != OriginalState) {
      String state = _currentState == SOUND ? "SOUNDING" : "SILENT";
      Log(currentMillis, _beeperName, "State has changed to " + state); 
    }
}