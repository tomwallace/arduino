/*
  Probe.h - Library for creating a probe input.  Its inputType specifies if it is INPUT or INPUT_PULLUP
  Created by Tom Wallace.
*/
#ifndef Probe_h
#define Probe_h

#include "Arduino.h"
#include "Loggable.h"

class Probe : public Loggable {
  private:
	int PROBE_CLEAR;
	int PROBE_TOUCH_LIQUID;
	String ProbeName;
	int CurrentState;
	int InputPin;   // The pin number that receives probe input

  public: 
	Probe(String probeName, int inputPin, int inputType);
	bool IsTouching();
	void Update(long currentMillis);
};

#endif