/*
  WortPump.cpp - The WortPump class interacts with the various aspects of the controllers use of the wort pump.
  Created by Tom Wallace.
*/

#include "Arduino.h"
#include "EventQueue.h"
#include "Loggable.h"
#include "Probe.h"
#include "WortPump.h"

WortPump::WortPump(int outputPin, long onInterval, EventQueue * alarmEventQueue, Probe * boilProbe) {
	OutputPin = outputPin; // The pin number that control pump output
    pinMode(OutputPin, OUTPUT);

    _alarmEventQueue = alarmEventQueue;
    _boilProbe = boilProbe;
    
    OnInterval = onInterval; // Milliseconds in a minute the pump is on
    OffInterval = 60000 - onInterval; // Milliseconds in a minute the pump is off
    IsActive = true; // Toggle to let overrides stop pump
    IsAlarmForToggle = true; // Used to alternate the alarm when probe is touching

    CurrentState = PUMP_OFF;  // Pump starts off
    previousMillis = 0;
	
	PUMP_ON = HIGH;
	PUMP_OFF = LOW;
}

void WortPump::SetIsActive(bool isActive) {
    IsActive = isActive;
}

bool WortPump::GetIsActive() {
    return IsActive;
}

void WortPump::Update(long currentMillis) {
    int OriginalState = CurrentState;
    
    // If probe is contacting liquid, pump is always off
    if (_boilProbe->IsTouching() || ! IsActive) {
      CurrentState = PUMP_OFF;
      digitalWrite(OutputPin, CurrentState);
    } else {
      if ((CurrentState == PUMP_OFF) && (currentMillis - previousMillis >= OffInterval)) { 
        previousMillis = currentMillis;
  
        CurrentState = PUMP_ON;
        digitalWrite(OutputPin, CurrentState);
      } else if ((CurrentState == PUMP_ON) && (currentMillis - previousMillis >= OnInterval)) { 
        previousMillis = currentMillis;
  
        CurrentState = PUMP_OFF;
        digitalWrite(OutputPin, CurrentState);
      }
    }

    // Handle pulsing alarm every 0.5 seconds if probe is touching
    if (_boilProbe->IsTouching() && IsActive) {
      // TODO: Consider changing to pulsing it works - inaccurate - need to bounce milli times
      bool canToggle = (currentMillis % 500) == 0;
      if (canToggle) {
        IsAlarmForToggle = IsAlarmForToggle ? false : true;
        Log(currentMillis, "AlarmState", "State has changed to " + IsAlarmForToggle); 
      }
      if (IsAlarmForToggle) {
        _alarmEventQueue->AddEvent("BoilProbe");
      } else {
        _alarmEventQueue->RemoveEvent("BoilProbe");
      }
    } else {
      _alarmEventQueue->RemoveEvent("BoilProbe");
    }
    
    // If state changed, then log
    if (CurrentState != OriginalState) {
      String state = CurrentState == PUMP_ON ? "ON" : "OFF";
      Log(currentMillis, "Wort Pump", "State has changed to " + state); 
    }
}