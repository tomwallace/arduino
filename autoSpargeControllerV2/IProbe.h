/*
  IProbe.h - Header library file for interface for probe inputs
  Created by Tom Wallace.
*/
#ifndef IProbe_h
#define IProbe_h

#include "Arduino.h"

class IProbe {
  public: 
    virtual ~IProbe() {};
    virtual bool IsTouching() = 0;
    virtual void Update(long currentMillis) = 0;
    virtual String Display() = 0;
};

#endif
