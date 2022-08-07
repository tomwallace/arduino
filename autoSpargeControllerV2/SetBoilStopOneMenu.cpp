/*
  SetBoilStopOneMenu.cpp - Menu item that sets the boil kettle stop 1
  Created by Tom Wallace.
*/

#include "Adafruit_RGBLCDShield.h"
#include "Arduino.h"
#include "SetBoilStopOneMenu.h"

SetBoilStopOneMenu::SetBoilStopOneMenu(Adafruit_RGBLCDShield * lcd) {
   _lcd = lcd;
}

String SetBoilStopOneMenu::GetName() {
  return "Boil Stop 1";
}

void SetBoilStopOneMenu::Interact(int button) {
  extern float boilStopOne;  // Set in main program for the first gallon stop for the boil
  extern int selectedMenu;   // Set in main program for currently selected menu
  
  // Draw
  _lcd->setCursor(0, 0);
  _lcd->print("Set Boil Stop 1 Value");
  _lcd->setCursor(0, 1);
  _lcd->print(boilStopOne);

  // Interact
  switch (button) {
    case 2:  // Increase value
        _lcd->clear();
        boilStopOne += 0.5;
        return;
    case 3:  // Decrease value
        _lcd->clear();
        boilStopOne -= 0.5;
        return;
    case 4:  // This case will execute if the "back" button is pressed
        _lcd->clear();
        selectedMenu = 0;
        return;
   }
}