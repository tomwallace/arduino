/*
  WaterPump.h - Library for interacting with the various aspects of the controllers use of the water pump.
  Created by Tom Wallace.
*/
#ifndef WaterPump_h
#define WaterPump_h

#include "Arduino.h"
#include "EventQueue.h"
#include "Loggable.h"
#include "Probe.h"

class WaterPump : public Loggable {
  private:
	int PUMP_ON;
	int PUMP_OFF;
	EventQueue * _alarmEventQueue;
	Probe * _mashProbe;
	Probe * _mashProbeHigh;
	int OutputPin;
	bool IsActive;
	long Delay;
	int CurrentState;
	int CurrentAlarmState;
	unsigned long previousMillis;
  
  public: 
	WaterPump(int outputPin, long delay, EventQueue * alarmEventQueue, Probe * mashProbe, Probe * mashProbeHigh);
	void SetIsActive(bool isActive);
	bool GetIsActive();
	void Update(long currentMillis);
};

#endif