/*
  SetBoilStopTwoMenu.cpp - Menu item that sets the boil kettle stop 2
  Created by Tom Wallace.
*/

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "SetBoilStopTwoMenu.h"

SetBoilStopTwoMenu::SetBoilStopTwoMenu(Adafruit_RGBLCDShield * lcd) {
   _lcd = lcd;
}

String SetBoilStopTwoMenu::GetName() {
  return "Boil Stop 2";
}

void SetBoilStopTwoMenu::Interact(int button) {
  extern float boilStopTwo;  // Set in main program for the second gallon stop for the boil
  extern int selectedMenu;   // Set in main program for currently selected menu
  
  // Draw
  _lcd->setCursor(0, 0);
  _lcd->print("Set Boil Stop 2 Value");
  _lcd->setCursor(0, 1);
  _lcd->print(boilStopTwo);

  // Interact
  switch (button) {
    case 2:  // Increase value
        _lcd->clear();
        boilStopTwo += 0.5;
        return;
    case 3:  // Decrease value
        _lcd->clear();
        boilStopTwo -= 0.5;
        return;
    case 4:  // This case will execute if the "back" button is pressed
        _lcd->clear();
        selectedMenu = 0;
        return;
   }
}