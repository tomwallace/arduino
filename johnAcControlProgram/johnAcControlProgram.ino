/*
 * Sketch to regulate John Baker's attic AC unit
 * REAL ONE
 * Version 1.0.0
 */

 // CONFIGURATION VARIABLES
 #define MILLIS_NEXT_COOL_CHECK 2000 // Amount of time to wait to determine if cool input has changed
 #define MILLIS_NEXT_TEMP_READ 5000 // Amount of time to wait after solenoid has been activated before next temp check
 #define MILLIS_RUN_UNIT 250  // Amount of time to activate the solenoid
 #define PULSE_ALARM_EVERY_SECOND 2 // Seconds in pulse of alarm
 #define TEMP_MARGIN_IGNORE 50  // + or - temp difference to ignore for error correction
 #define MAX_NUM_TRIES 3  // Number of tries before failure

 // PIN SETTINGS ON BOARD
 #define TEMP_SENSOR 15 // Analog 1.  Normal pin is 1(A)
 #define SOLENOID_OUT 13  // Normal pin is 1 digital
 #define ALARM 4  // HIGH = alarm sound
 #define AC_ON_OFF 3  // With pullup.  LOW = AC on, HIGH = AC off
 #define BUTTON 5  // With pullup.  Normal pin is 0.


bool isAcOn = false;
int tries = 0;
bool isCallCool = false;
long nextTestTime = 0;
long nextTempStep = 0;
long nextTempReading = 0;
long manualOverrideStopTime = 0;

bool coolInputIsLow = true;
bool inTempCorrectionLoop = false;
bool isAlarmOn = false;

int tempOne = 0;
int tempTwo = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(AC_ON_OFF, INPUT_PULLUP);  // Low = AC on, High = AC off

  pinMode(SOLENOID_OUT, OUTPUT);
  pinMode(ALARM, OUTPUT);      // High = Alarm sounds

  digitalWrite(ALARM, LOW);

  // Set up serial port for output at 9600 bps - serial commands commented out because normal trinket does not support
  //Serial.begin(9600);
}

void ResetParameters() {
  nextTestTime = 0;
  inTempCorrectionLoop = false;
  tempOne = 0;
  tempTwo = 0;
  tries = 0;
  nextTempStep = 0;
  nextTempReading = 0;
  isAlarmOn = false;
}

void TemperatureCorrectionLoop(long currentMillis) {
  // We need to start the next cycle
  if (nextTempStep == 0) {
    //Serial.println(String(currentMillis) + " - Starting next cyle (2)");
    nextTempStep = currentMillis + MILLIS_RUN_UNIT;
    nextTempReading = nextTempStep + MILLIS_NEXT_TEMP_READ;
    
    digitalWrite(SOLENOID_OUT, HIGH);
  } else {
    // Have we completed the wait to collect the second temperature
    if (currentMillis >= nextTempReading) {
      // Done with the cycle, so see what happened
      ++tries;
      //Serial.println(String(currentMillis) + " - Tries = " + String(tries));
      
      tempTwo = analogRead(TEMP_SENSOR);
      //Serial.println(String(currentMillis) + " - Ready to read temp. IsCallCool: " + String(isCallCool) + " Temp1: " + String(tempOne) + " Temp2: " + String(tempTwo));

      // Evaluate for success
      if ((isCallCool == true && ((tempTwo + TEMP_MARGIN_IGNORE) < tempOne)) || (isCallCool == false && ((tempTwo - TEMP_MARGIN_IGNORE) > tempOne))) {
        //Serial.println(String(currentMillis) + " - Exit 3 - Temperature adjustment successful");
        isAcOn = isCallCool;
        ResetParameters();
        return;
      }

      if (tries >= MAX_NUM_TRIES) {
        isAlarmOn = true;
      }
        
      // Otherwise, we have to try again
      nextTempStep = 0;
      nextTempReading = 0;
      
    } else {
      if (currentMillis <= nextTempStep) {
        // Solenoid should be on
        digitalWrite(SOLENOID_OUT, HIGH);
      } else {
        // Turn off the solenoid now
        digitalWrite(SOLENOID_OUT, LOW);  
      }
    }
  }
}

void loop() {  
  // Get current clock
  unsigned long currentMillis = millis();

  // Is manual interrupt pushed
  if (digitalRead(BUTTON) == LOW && manualOverrideStopTime == 0) {
    //Serial.println(String(currentMillis) + " - Manual override button pressed");
    manualOverrideStopTime = currentMillis + MILLIS_RUN_UNIT;
  }
  if (manualOverrideStopTime != 0) {
    // We have completed the pulse, so reset
    if (currentMillis >= manualOverrideStopTime) {
      manualOverrideStopTime = 0;
      digitalWrite(SOLENOID_OUT, LOW);
      //Serial.println(String(currentMillis) + " - Manual override cycle done");
      return;
    }

    // Otherwise Pulse solenoid
    digitalWrite(SOLENOID_OUT, HIGH);
    return;
  }

  // Is the alarm sounding
  if (isAlarmOn == true) {
    //Serial.println(String(currentMillis) + " - Alarm sounding");
    // Pulse alarm
      if (((currentMillis / 1000) % PULSE_ALARM_EVERY_SECOND) == 1) {
        digitalWrite(ALARM, HIGH);
      } else {
        digitalWrite(ALARM, LOW);
      }
    return;
  }

  // Are we in the temp correction loop
  if (inTempCorrectionLoop) {
    TemperatureCorrectionLoop(currentMillis);
    
  } else {
    // We need to start the next test cycle
    if (nextTestTime == 0) {
      nextTestTime = currentMillis + MILLIS_NEXT_COOL_CHECK;
      coolInputIsLow = (digitalRead(AC_ON_OFF) == LOW);
      //Serial.println(String(currentMillis) + " - Starting next test cyle (1). coolInputIsLow: " + String(coolInputIsLow) + " isAcOn: " + String(isAcOn));
    }
    // Otherwise we are in an evaluation loop
    else {
      // If our wait time has elapsed, now evaluate, otherwise wait longer
      if (currentMillis >= nextTestTime) {
        bool currentCoolInputIsLow = (digitalRead(AC_ON_OFF) == LOW);
        
        // Test for exit condition which is if the current reading does not match the first
        if (coolInputIsLow != currentCoolInputIsLow) {
          //Serial.println(String(currentMillis) + " - Exit 1 - coolInputIsLow != currentCoolInputIsLow");
          ResetParameters();
          return;
        }
  
        // Test for exit condition which is if the call matches current AC state
        if (coolInputIsLow == isAcOn) {
          //Serial.println(String(currentMillis) + " - Exit 2 - coolInputIsLow == isAcOn");
          ResetParameters();
          return;
        }
  
        // Work with actual temperature changes
        isCallCool = coolInputIsLow;
        tempOne = analogRead(TEMP_SENSOR);
        inTempCorrectionLoop = true;
        nextTempStep = 0;
        nextTempReading = 0;
        //Serial.println(String(currentMillis) + " - Setting to enter Temperature Correction Loop");
      } 
    }
  }
}

