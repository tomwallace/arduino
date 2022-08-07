/*
  SetBoilDisplayUnitsMenu.cpp - Menu item that sets the display units for the pressure sensor in the boil kettle
  Created by Tom Wallace.
*/

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "SetBoilDisplayUnitsMenu.h"

SetBoilDisplayUnitsMenu::SetBoilDisplayUnitsMenu(Adafruit_RGBLCDShield * lcd) {
   _lcd = lcd;
}

String SetBoilDisplayUnitsMenu::GetName() {
  return "Display Units";
}

void SetBoilDisplayUnitsMenu::Interact(int button) {
  extern bool boilShowGallons;  // Set in main program as an option to show gallons or pressure
  extern int selectedMenu;   // Set in main program for currently selected menu
  
  // Draw
  _lcd->setCursor(0, 0);
  _lcd->print("Set Display Units");
  _lcd->setCursor(0, 1);
  String displayUnits = boilShowGallons ? "Gallons" : "Pressure";
  _lcd->print(displayUnits);

  // Interact
  switch (button) {
    case 2:  // Increase value
        _lcd->clear();
        boilShowGallons = true;
        return;
    case 3:  // Decrease value
        _lcd->clear();
        boilShowGallons = false;
        return;
    case 4:  // This case will execute if the "back" button is pressed
        _lcd->clear();
        selectedMenu = 0;
        return;
   }
}