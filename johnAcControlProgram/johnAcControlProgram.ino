/*
 * Sketch to regulate John Baker's attic AC unit
 * REAL ONE
 * Version 1.0.0
 */

 #define MILLIS_NEXT_COOL_CHECK 5000 // TODO: change to 2000 for real sketch
 #define MILLIS_NEXT_TEMP_READ 5000 // TODO: change to 5000 for real sketch
 #define MILLIS_RUN_UNIT 5000  // TODO: change to 250 for real sketch

 #define MAX_NUM_TRIES 3

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

bool coolInputIsLow = true;
bool inTempCorrectionLoop = false;
bool isAlarmOn = false;

int tempOne = 0;
int tempTwo = 0;

// TODO: Remove Serial commands because make it unable to compile for John's trinket
void setup() {
  // put your setup code here, to run once:
  pinMode(TEMP_SENSOR, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(AC_ON_OFF, INPUT_PULLUP);  // Low = AC on, High = AC off

  pinMode(SOLENOID_OUT, OUTPUT);
  pinMode(ALARM, OUTPUT);      // High = Alarm sounds

  digitalWrite(ALARM, LOW);

  // Set up serial port for output at 9600 bps
  Serial.begin(9600);
}

void ResetParameters() {
  nextTestTime = 0;
  inTempCorrectionLoop = false;
  tempOne = 0;
  tempTwo = 0;
  tries = 0;
  nextTempStep = 0;
  nextTempReading = 0;
}

void TemperatureCorrectionLoop(long currentMillis) {
  // We need to start the next cycle
  if (nextTempStep == 0) {
    Serial.println(String(currentMillis) + " - Starting next cyle (2)");
    nextTempStep = currentMillis + MILLIS_RUN_UNIT;
    nextTempReading = nextTempStep + MILLIS_NEXT_TEMP_READ;
    
    digitalWrite(SOLENOID_OUT, HIGH);
  } else {
    // Have we completed the wait to collect the second temperature
    if (currentMillis >= nextTempReading) {
      // Done with the cycle, so see what happened
      digitalWrite(SOLENOID_OUT, LOW);
      
      ++tries;
      Serial.println(String(currentMillis) + " - Tries = " + String(tries));
      
      tempTwo = analogRead(TEMP_SENSOR);
      Serial.println(String(currentMillis) + " - Ready to read temp. IsCallCool: " + String(isCallCool) + " Temp1: " + String(tempOne) + " Temp2: " + String(tempTwo));

      // Evaluate for success
      if ((isCallCool == true && tempTwo < tempOne) || (isCallCool == false && tempTwo > tempOne)) {
        Serial.println(String(currentMillis) + " - Temperature adjustment successful");
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
      
    } else if (currentMillis >= nextTempStep && currentMillis < nextTempReading) {
      // Pulse solenoid
      if (((currentMillis / 1000) % 2) == 1) {
        digitalWrite(SOLENOID_OUT, HIGH);
      } else {
        digitalWrite(SOLENOID_OUT, LOW);
      }  
    }
  }
}

// TODO: Implement manual push button interrupt
void loop() {  
  // Get current clock
  unsigned long currentMillis = millis();

  // TODO: Check why this is sounding
  digitalWrite(ALARM, LOW);

  // Is the alarm sounding
  if (isAlarmOn == true) {
    Serial.println(String(currentMillis) + " - Alarm sounding");
    // TODO: Make it pulse
    //digitalWrite(ALARM, HIGH);
    return;
  }

  // Are we in the temp correction loop
  if (inTempCorrectionLoop) {
    TemperatureCorrectionLoop(currentMillis);
    
  } else {
    // We need to start the next test cycle
    if (nextTestTime == 0) {
      Serial.println(String(currentMillis) + " - Starting next test cyle (1)");
      nextTestTime = currentMillis + MILLIS_NEXT_COOL_CHECK;
      coolInputIsLow = (digitalRead(AC_ON_OFF) == LOW);
    }
    // Otherwise we are in an evaluation loop
    else {
      // If our wait time has elapsed, now evaluate, otherwise wait longer
      if (currentMillis >= nextTestTime) {
        bool currentCoolInputIsLow = (digitalRead(AC_ON_OFF) == LOW);
        
        // Test for exit condition which is if the current reading does not match the first
        if (coolInputIsLow != currentCoolInputIsLow) {
          Serial.println(String(currentMillis) + " - Exit 1 - coolInputIsLow != currentCoolInputIsLow");
          ResetParameters();
          return;
        }
  
        // Test for exit condition which is if the call matches current AC state
        if (coolInputIsLow == isAcOn) {
          Serial.println(String(currentMillis) + " - Exit 2 - coolInputIsLow == isAcOn");
          ResetParameters();
          return;
        }
  
        // Work with actual temperature changes
        isCallCool = coolInputIsLow ? false : true;
        tempOne = analogRead(TEMP_SENSOR);
        inTempCorrectionLoop = true;
        nextTempStep = 0;
        nextTempReading = 0;
        Serial.println(String(currentMillis) + " - Setting to enter Temperature Correction Loop");
      } 
    }
  }
}

