/*
  WaterPump.cpp - Library for interacting with the various aspects of the controllers use of the water pump.
  Created by Tom Wallace.
*/

#include "Arduino.h"
#include "EventQueue.h"
#include "Loggable.h"
#include "Probe.h"
#include "WaterPump.h"

WaterPump::WaterPump(int outputPin, long delay, EventQueue * alarmEventQueue, Probe * mashProbe, Probe * mashProbeHigh) {
    OutputPin = outputPin; // The pin number that control pump output
    pinMode(OutputPin, OUTPUT);

    _alarmEventQueue = alarmEventQueue;
    _mashProbe = mashProbe;
    _mashProbeHigh = mashProbeHigh;
    
    Delay = delay; // Milliseconds of delay after change of state from probe
    IsActive = true; // Toggle to let overrides stop pump

    CurrentState = PUMP_OFF;  // Pump starts off
    CurrentAlarmState = LOW;
    previousMillis = 0; // Stores the last time the state changed
	
	PUMP_ON = HIGH;
	PUMP_OFF = LOW;
};
  
void WaterPump::SetIsActive(bool isActive) {
    IsActive = isActive;
}

bool WaterPump::GetIsActive() {
    return IsActive;
}

void WaterPump::Update(long currentMillis) {
    int OriginalState = CurrentState;
    
    // If probe is contacting liquid, pump is always off
    if (_mashProbe->IsTouching() || ! IsActive) {
      previousMillis = currentMillis;
      
      CurrentState = PUMP_OFF;
      digitalWrite(OutputPin, CurrentState);
    } else {
      if (currentMillis - previousMillis >= Delay) { 
        previousMillis = currentMillis;
  
        CurrentState = PUMP_ON;
        digitalWrite(OutputPin, CurrentState);
      }
    }

    // Sound alarm if high level probe contacting liquid
    if (_mashProbeHigh->IsTouching() && IsActive) {
      _alarmEventQueue->AddEvent("MashProbeHigh");
    } else {
      _alarmEventQueue->RemoveEvent("MashProbeHigh");
    }

    // If state changed, then log
    if (CurrentState != OriginalState) {
      String state = CurrentState == PUMP_ON ? "ON" : "OFF";
      Log(currentMillis, "Water Pump", "State has changed to " + state); 
    }
}