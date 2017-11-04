/*
  Beeper.h - Library for represents a sound output, such as the alarm or buzzer.  The sound is controlled by an EventQueue.
  Created by Tom Wallace.
*/
#ifndef Beeper_h
#define Beeper_h

#include "Arduino.h"
#include "EventQueue.h"
#include "Loggable.h"

class Beeper : public Loggable {
  private: 
	int _outputPin;
	EventQueue * _eventQueue;
	int _currentState;
	String _beeperName;
	int SOUND;
	int SILENT;

  public: 
	Beeper(String beeperName, int outputPin, EventQueue * eventQueue);
	void Update(long currentMillis);
};

#endif