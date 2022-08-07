/*
  Probe.cpp - Library for creating a probe input.  Its inputType specifies if it is INPUT or INPUT_PULLUP
  Created by Tom Wallace.
*/

#include "Arduino.h"
#include "Probe.h"
#include "Loggable.h"

Probe::Probe(String probeName, int inputPin, int inputType) {
  InputPin = inputPin;
  ProbeName = probeName;
  pinMode(InputPin, inputType);
  
  PROBE_CLEAR = LOW;
  PROBE_TOUCH_LIQUID = HIGH;
  CurrentState = PROBE_CLEAR;
}

bool Probe::IsTouching() {
  return (CurrentState == PROBE_TOUCH_LIQUID);
}

void Probe::Update(long currentMillis) {
  int OriginalState = CurrentState;
  CurrentState = digitalRead(InputPin);
    
  // If state changed, then log
  if (CurrentState != OriginalState) {
    String state = CurrentState == PROBE_TOUCH_LIQUID ? "TOUCH LIQUID" : "CLEAR";
    Log(currentMillis, ProbeName, "State has changed to " + state); 
  }
}

String Probe::Display() {
  return "";
}
