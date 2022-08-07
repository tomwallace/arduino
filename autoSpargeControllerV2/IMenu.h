/*
  IMenu.h - Header library file for interface for menu items
  Created by Tom Wallace.
*/
#ifndef IMenu_h
#define IMenu_h

#include "Arduino.h"

class IMenu {
  public: 
    virtual ~IMenu() {};
    virtual String GetName() = 0;
    virtual void Interact(int button) = 0;
};

#endif
