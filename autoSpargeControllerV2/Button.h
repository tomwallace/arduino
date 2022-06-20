/*
  Button.h - Library for creating a button, which can trigger a number of things. Its inputType specifies if it is INPUT or INPUT_PULLUP
  Created by Tom Wallace.
*/
#ifndef Button_h
#define Button_h

#include "Arduino.h"
#include "EventQueue.h"
#include "Loggable.h"

class Button : public Loggable {
  private:
	int BUTTON_BEEP_LENGTH;
	int HAS_BEEN_CLICKED_DELAY;
	EventQueue * _buzzerEventQueue;
	String ButtonName;
	int ButtonPin;
	int LightPin;
	long TurnOffClickSoundMillis;
	bool EligibleToBeClicked;
	long EligibleToBeClickedMillis;
	bool MatchingFunctionOn;

  public: 
	Button(String buttonName, int buttonPin, int inputType, int lightPin, EventQueue * buzzerEventQueue);
	bool IsCurrentlyDepressed();
	bool GetMatchingFunctionOn();
	void Update(long currentMillis);
};

#endif
