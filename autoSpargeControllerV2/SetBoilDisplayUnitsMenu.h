/*
  SetBoilDisplayUnitsMenu.h - Menu item that sets the display units for the pressure sensor in the boil kettle
  Created by Tom Wallace.
*/
#ifndef SetBoilDisplayUnitsMenu_h
#define SetBoilDisplayUnitsMenu_h

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "IMenu.h"

class SetBoilDisplayUnitsMenu : public IMenu {
  public: 
	SetBoilDisplayUnitsMenu(Adafruit_RGBLCDShield * lcd);
	virtual String GetName();
    virtual void Interact(int button);
  private:
	Adafruit_RGBLCDShield * _lcd;
};

#endif