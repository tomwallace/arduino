/*
  PressureSensor.cpp - Library for creating a pressure sensor input.
  Created by Tom Wallace.
*/

#include "Arduino.h"
#include "PressureSensor.h"
PressureSensor::PressureSensor(Adafruit_MPRLS * mpr) {
  _mpr = mpr;
  _numReadings = 20;
  _sensorZero = 0;
  _readingPointer = 0;
  _readings = new int[_numReadings];
  _initSensorZeroCount = 0;
}

bool PressureSensor::IsTouching() {
  extern float boilStopOne;  // Set in main program for the first gallon stop for the boil
  extern float boilStopTwo;  // Set in main program for the second gallon stop for the boil
  extern bool atBoilStopOne; // Set in main program for using the first gallon stop

  // Make sure if we are UNCONNECTED to say IsTouching = true to prevent pump from firing without the probe
  if (_sensorZero == 0)
    return true;

  if (atBoilStopOne) {
    return GetGallons() >= boilStopOne;
  }
  
  return GetGallons() >= boilStopTwo;
}

void PressureSensor::Update(long currentMillis) {
  // CONNECTED sensor readStatus = 64
  if (_mpr->readStatus() == 64) {
    // Still need to initialize sensor "zero"
    if (_initSensorZeroCount <= 20) {
      _initSensorZeroCount++;
      
      AddReading(_mpr->readPressure());

      // If we are greater than 20 initialization readings, set _sensorZero
      if (_initSensorZeroCount > 20) {
        _sensorZero = AverageReadings();
        Serial.println("Connected - Setting _sensorZero to - " + String(_sensorZero));
      }
    } else {
      // Normal condition - read pressure
      AddReading(_mpr->readPressure());
    }
  } else if (_sensorZero != 0) {
    Serial.println("Unconnected");
    // UNCONNECTED, so reset variables
    _initSensorZeroCount = 0;
    _sensorZero = 0;
  }
}

String PressureSensor::Display() {
  extern bool boilShowGallons;  // Set in main program as an option to show gallons or pressure

  // If _sensorZero == 0 and likely UNCONNECTED, then we are not initialized, so display ---
  if (_sensorZero == 0)
    return "----";

  if (!boilShowGallons)
    return String(AverageReadings() - _sensorZero,2);

  return String(GetGallons(),1);
}

// Private - Adds the new reading to our array of reading numbers, moving the pointer
void PressureSensor::AddReading(int reading) {
  _readings[_readingPointer] = reading;
  _readingPointer += 1;
  if (_readingPointer >= _numReadings)
    _readingPointer = 0;
}

// Private - Returns the average of the array of reading numbers
float PressureSensor::AverageReadings() {
  long total = 0;
  int readIndex1 = 0;
  for (readIndex1 = 0; readIndex1 < _numReadings; readIndex1++){
      total += _readings[readIndex1];
  }
  return total / _numReadings;
}

// Private - Returns pressure in gallons
float PressureSensor::GetGallons() {
  float pressure = AverageReadings() - _sensorZero;

  // TODO - work with different formulas, maybe even based on temp
  // Translate the pressure into gallons applying formula
  return pressure/2;
}
