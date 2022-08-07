/*
  CurrentDataMenu.cpp - Menu item that displays current boil kettle pressure data
  Created by Tom Wallace.
*/

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "CurrentDataMenu.h"
#include "IProbe.h"

CurrentDataMenu::CurrentDataMenu(IProbe * probe, Adafruit_RGBLCDShield * lcd) {
   _probe = probe;
   _lcd = lcd;
}

String CurrentDataMenu::GetName() {
  return "Current Data";
}

void CurrentDataMenu::Interact(int button) {
  extern float boilStopOne;  // Set in main program for the first gallon stop for the boil
  extern float boilStopTwo;  // Set in main program for the second gallon stop for the boil
  extern bool atBoilStopOne; // Set in main program for using the first gallon
  extern int selectedMenu;   // Set in main program for currently selected menu
  extern bool boilShowGallons;  // Set in main program as an option to show gallons or pressure
  
  // Draw
  if (boilShowGallons) {
    _lcd->setCursor(0, 0);
    _lcd->print("Curr/Targ Gal");
    _lcd->setCursor(0, 1);
    String suffix = atBoilStopOne ? "[" + String(boilStopOne,1) + "]/" + String(boilStopTwo,1) : String(boilStopOne,1) + "/[" + String(boilStopTwo,1) + "]";
    String displayValue = _probe->Display() + "/" + suffix;
    _lcd->print(displayValue);
  } else {
    _lcd->setCursor(0, 0);
    _lcd->print("Current Pressure");
    _lcd->setCursor(0, 1);
    _lcd->print(_probe->Display());
  }

  // Interact
  switch (button) {
    case 4:  // This case will execute if the "back" button is pressed
        _lcd->clear();
        selectedMenu = 0;
        return;
   }
}