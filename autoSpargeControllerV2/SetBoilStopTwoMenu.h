/*
  SetBoilStopTwoMenu.h - Menu item that sets the boil kettle stop 2
  Created by Tom Wallace.
*/
#ifndef SetBoilStopTwoMenu_h
#define SetBoilStopTwoMenu_h

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "IMenu.h"

class SetBoilStopTwoMenu : public IMenu {
  public: 
	SetBoilStopTwoMenu(Adafruit_RGBLCDShield * lcd);
	virtual String GetName();
    virtual void Interact(int button);
  private:
	Adafruit_RGBLCDShield * _lcd;
};

#endif