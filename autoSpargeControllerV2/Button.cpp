/*
  Button.cpp - Library for creating a button, which can trigger a number of things. Its inputType specifies if it is INPUT or INPUT_PULLUP
  Created by Tom Wallace.
*/

#include "Arduino.h"
#include "Button.h"
#include "EventQueue.h"
#include "Loggable.h"

Button::Button(String buttonName, int buttonPin, int inputType, int lightPin, EventQueue * buzzerEventQueue) {
    ButtonName = buttonName;  // Name of the button for logging
    ButtonPin = buttonPin;  // The pin number attached to the button
    LightPin = lightPin;  // The pin the button light is attached to
    _buzzerEventQueue = buzzerEventQueue;
    pinMode(ButtonPin, inputType);
	
    pinMode(LightPin, OUTPUT);
    TurnOffClickSoundMillis = 0;  // When to turn off the click sound
    EligibleToBeClicked = true;  // Used with delay to prevent "burst" clicking
    EligibleToBeClickedMillis = 0;  // To determine when to set the EligibleToBeClicked flag
    MatchingFunctionOn = false; // Used to pair the button with a pump function
	
	BUTTON_BEEP_LENGTH = 10;  // Length of beep when button pressed
	HAS_BEEN_CLICKED_DELAY = 500; // Length of delay before button is eligible for clicking again (prevents burst by holding button down)
}
  
bool Button::IsCurrentlyDepressed() {
    return (digitalRead(ButtonPin) == LOW);
}

bool Button::GetMatchingFunctionOn() {
    return MatchingFunctionOn;
}

void Button::Update(long currentMillis) {    
    // Determine if button has been clicked
    if (IsCurrentlyDepressed() && EligibleToBeClicked) {
      // Log click
      Log(currentMillis, ButtonName, "currently pushed.");
      
      EligibleToBeClicked = false;
      EligibleToBeClickedMillis = currentMillis + HAS_BEEN_CLICKED_DELAY;

      // Sound click on button
      _buzzerEventQueue->AddEvent(ButtonName);
      TurnOffClickSoundMillis = currentMillis + BUTTON_BEEP_LENGTH;
      
      // Toggle light and matching function
      MatchingFunctionOn = MatchingFunctionOn ? false : true;
    }

    // Determine when button is eligible to be clicked again to prevent button bursting
    if (!IsCurrentlyDepressed() && (currentMillis > EligibleToBeClickedMillis)) {
      EligibleToBeClicked = true;
    }

    // Turn off sound click when ready
    if (currentMillis > TurnOffClickSoundMillis) {
      _buzzerEventQueue->RemoveEvent(ButtonName);
    }
    
    // Handle light on/off
    if (MatchingFunctionOn) {
      digitalWrite(LightPin, HIGH);
    } else {
      digitalWrite(LightPin, LOW);
    }
}