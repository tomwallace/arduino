/*
  SetBoilStopOneMenu.h - Menu item that sets the boil kettle stop 1
  Created by Tom Wallace.
*/
#ifndef SetBoilStopOneMenu_h
#define SetBoilStopOneMenu_h

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "IMenu.h"

class SetBoilStopOneMenu : public IMenu {
  public: 
	SetBoilStopOneMenu(Adafruit_RGBLCDShield * lcd);
	virtual String GetName();
    virtual void Interact(int button);
  private:
	Adafruit_RGBLCDShield * _lcd;
};

#endif