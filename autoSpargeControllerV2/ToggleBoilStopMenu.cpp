/*
  ToggleBoilStopMenu.cpp - Menu item that toggles the boil kettle stop
  Created by Tom Wallace.
*/

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "ToggleBoilStopMenu.h"

ToggleBoilStopMenu::ToggleBoilStopMenu(Adafruit_RGBLCDShield * lcd) {
   _lcd = lcd;
}

String ToggleBoilStopMenu::GetName() {
  return "Toggle Stop";
}

void ToggleBoilStopMenu::Interact(int button) {
  extern bool atBoilStopOne; // Set in main program for using the first gallon
  extern int selectedMenu;   // Set in main program for currently selected menu
  
  // Draw
  _lcd->setCursor(0, 0);
  _lcd->print("Toggle Boil Stop");
  _lcd->setCursor(0, 1);
  String displayToggle = atBoilStopOne ? "Boil Stop 1" : "Boil Stop 2";
  _lcd->print(displayToggle);

  // Interact
  switch (button) {
    case 2:  // Toggle true
        _lcd->clear();
        atBoilStopOne = true;
        return;
    case 3:  // Toggle false
        _lcd->clear();
        atBoilStopOne = false;
        return;
    case 4:  // This case will execute if the "back" button is pressed
        _lcd->clear();
        selectedMenu = 0;
        return;
   }
}