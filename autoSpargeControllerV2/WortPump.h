/*
  WortPump.h - The WortPump class interacts with the various aspects of the controllers use of the wort pump.
  Created by Tom Wallace.
*/
#ifndef WortPump_h
#define WortPump_h

#include "Arduino.h"
#include "EventQueue.h"
#include "Loggable.h"
#include "IProbe.h"

class WortPump : public Loggable {
  private:
	int PUMP_ON;
	int PUMP_OFF;
	EventQueue * _alarmEventQueue;
	IProbe * _boilProbe;
	int OutputPin;
	long OnInterval;
	long OffInterval;
	bool IsActive;
	bool IsAlarmForToggle;
	int CurrentState;
	unsigned long previousMillis;
  
  // Constructor
  public: 
	WortPump(int outputPin, long onInterval, EventQueue * alarmEventQueue, IProbe * boilProbe);
	void SetIsActive(bool isActive);
	bool GetIsActive();
	void Update(long currentMillis);
  void SetProbe(IProbe * boilProbe);
};

#endif
