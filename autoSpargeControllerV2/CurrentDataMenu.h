/*
  CurrentDataMenu.h - Menu item that displays current boil kettle pressure data
  Created by Tom Wallace.
*/
#ifndef CurrentDataMenu_h
#define CurrentDataMenu_h

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "IMenu.h"
#include "IProbe.h"

class CurrentDataMenu : public IMenu {
  public: 
	CurrentDataMenu(IProbe * probe, Adafruit_RGBLCDShield * lcd);
	virtual String GetName();
    virtual void Interact(int button);
  private:
    IProbe * _probe;
	Adafruit_RGBLCDShield * _lcd;
};

#endif