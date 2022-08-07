/*
  ToggleBoilStopMenu.h - Menu item that toggles the boil kettle stop
  Created by Tom Wallace.
*/
#ifndef ToggleBoilStopMenu_h
#define ToggleBoilStopMenu_h

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "IMenu.h"

class ToggleBoilStopMenu : public IMenu {
  public: 
	ToggleBoilStopMenu(Adafruit_RGBLCDShield * lcd);
	virtual String GetName();
    virtual void Interact(int button);
  private:
	Adafruit_RGBLCDShield * _lcd;
};

#endif