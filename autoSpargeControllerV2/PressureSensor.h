/*
  PressureSensor.h - Library for creating a pressure sensor input.
  Created by Tom Wallace.
*/
#ifndef PressureSensor_h
#define PressureSensor_h

#include "Arduino.h"
#include "Adafruit_MPRLS.h"
#include <utility/Adafruit_MCP23017.h>
#include "IProbe.h"

class PressureSensor : public IProbe {
  public:
    PressureSensor(Adafruit_MPRLS * mpr);
    virtual bool IsTouching();
    virtual void Update(long currentMillis);
    virtual String Display();
    
  private:
    Adafruit_MPRLS * _mpr;
    int _numReadings;
    float* _readings;
    int _initSensorZeroCount;
    float _sensorZero;
    int _readingPointer;

    void AddReading(float reading);
    float AverageReadings();
    float GetGallons();
};

#endif
