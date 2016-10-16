/*
 * AUTOSPARGE TEST BED
 * version 2.0
 * by Tom Wallace and John Baker
 * This sketch tests the various controls, sensors, and outputs on the Autosparge test bed.
 */
 
// Config: PIN Numbers
#define WORT_PUMP_LIGHT 12
#define WATER_PUMP_LIGHT 13

#define ALARM 8
#define ALARM_SOUND HIGH
#define ALARM_SILENT LOW

#define BEEPER 11
#define BEEPER_SOUND HIGH
#define BEEPER_SILENT LOW

#define RIGHT_BUTTON_LIGHT 5
#define LEFT_BUTTON_LIGHT 6
#define RIGHT_BUTTON 3
#define LEFT_BUTTON 4

#define BOIL_SENSOR 15
#define MASH_SENSOR_ONE 16
#define MASH_SENSOR_TWO 17

// Config: Durations
#define ACTIVATE_INTERVAL 3000
#define ON_LENGTH 2000

long StartTimeMillis = 0;
long StartWortPumpLight = 0;
long StopWortPumpLight = 0;
long StartWaterPumpLight = 0;
long StopWaterPumpLight = 0;
long StartAlarm = 0;
long StopAlarm = 0;
long StartBeeper = 0;
long StopBeeper = 0;
long StartRightButtonLight = 0;
long StopRightButtonLight = 0;
long StartLeftButtonLight = 0;
long StopLeftButtonLight = 0;

bool LogWORT_PUMP_LIGHT = false;
bool LogWATER_PUMP_LIGHT = false;
bool LogALARM_SOUND = false;
bool LogRIGHT_BUTTON_LIGHT = false;
bool LogLEFT_BUTTON_LIGHT = false;
bool LogBEEPER_SOUND = false;

void setup() {
  // put your setup code here, to run once:
  // Buttons are pull-up resistors
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  
  pinMode(ALARM, OUTPUT);
  pinMode(BEEPER, OUTPUT);
  
  pinMode(WORT_PUMP_LIGHT, OUTPUT);
  pinMode(WATER_PUMP_LIGHT, OUTPUT);
  pinMode(LEFT_BUTTON_LIGHT, OUTPUT);
  pinMode(RIGHT_BUTTON_LIGHT, OUTPUT);

  StartTimeMillis = millis();
  StartWortPumpLight = StartTimeMillis + ACTIVATE_INTERVAL;
  StopWortPumpLight = StartWortPumpLight + ON_LENGTH;
  StartWaterPumpLight = StopWortPumpLight + ACTIVATE_INTERVAL;
  StopWaterPumpLight = StartWaterPumpLight + ON_LENGTH;
  StartAlarm = StopWaterPumpLight + ACTIVATE_INTERVAL;
  StopAlarm = StartAlarm + ON_LENGTH;
  StartRightButtonLight = StopAlarm + ACTIVATE_INTERVAL;
  StopRightButtonLight = StartRightButtonLight + ON_LENGTH;
  StartLeftButtonLight = StopRightButtonLight + ACTIVATE_INTERVAL;
  StopLeftButtonLight = StartLeftButtonLight + ON_LENGTH;
  StartBeeper = StopLeftButtonLight + ACTIVATE_INTERVAL;
  StopBeeper = StartBeeper + ON_LENGTH;

  // Set up serial port for output at 9600 bps
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Get current clock
  unsigned long currentMillis = millis();

  // If in initial start up evaluation, i.e. less than the last time the last light is turned on
  if (currentMillis <= StopBeeper) {
    // Wort Pump Light
    if ((currentMillis >= StartWortPumpLight) && (currentMillis <= StopWortPumpLight)) {
      if (LogWORT_PUMP_LIGHT == false) {
        Serial.println("Initial test of WORT_PUMP_LIGHT on");
        LogWORT_PUMP_LIGHT = true;
      }
      
      digitalWrite(WORT_PUMP_LIGHT, HIGH);
    } else {
      digitalWrite(WORT_PUMP_LIGHT, LOW);
    }
  
    // Water Pump Light
    if ((currentMillis >= StartWaterPumpLight) && (currentMillis <= StopWaterPumpLight)) {
      if (LogWATER_PUMP_LIGHT == false) {
        Serial.println("Initial test of WATER_PUMP_LIGHT on");
        LogWATER_PUMP_LIGHT = true;
      }

      digitalWrite(WATER_PUMP_LIGHT, HIGH);
    } else {
      digitalWrite(WATER_PUMP_LIGHT, LOW);
    }

    // Alarm
    if ((currentMillis >= StartAlarm) && (currentMillis <= StopAlarm)) {
      if (LogALARM_SOUND == false) {
        Serial.println("Initial test of ALARM_SOUND on");
        LogALARM_SOUND = true;
      }
      
      digitalWrite(ALARM, ALARM_SOUND);
    } else {
      digitalWrite(ALARM, ALARM_SILENT);
    }

    // Right Button Light
    if ((currentMillis >= StartRightButtonLight) && (currentMillis <= StopRightButtonLight)) {
      if (LogRIGHT_BUTTON_LIGHT == false) {
        Serial.println("Initial test of RIGHT_BUTTON_LIGHT on");
        LogRIGHT_BUTTON_LIGHT = true;
      }
      
      digitalWrite(RIGHT_BUTTON_LIGHT, HIGH);
    } else {
      digitalWrite(RIGHT_BUTTON_LIGHT, LOW);
    }
  
    // Left Button Light
    if ((currentMillis >= StartLeftButtonLight) && (currentMillis <= StopLeftButtonLight)) {
      if (LogLEFT_BUTTON_LIGHT == false) {
        Serial.println("Initial test of LEFT_BUTTON_LIGHT on");
        LogLEFT_BUTTON_LIGHT = true;
      }
      
      digitalWrite(LEFT_BUTTON_LIGHT, HIGH);
    } else {
      digitalWrite(LEFT_BUTTON_LIGHT, LOW);
    }
 
    // Beeper
    if ((currentMillis >= StartBeeper) && (currentMillis < StopBeeper)) {
      if (LogBEEPER_SOUND == false) {
        Serial.println("Initial test of BEEPER_SOUND on");
        LogBEEPER_SOUND = true;
      }
      
      digitalWrite(BEEPER, BEEPER_SOUND);
    } else {
      digitalWrite(BEEPER, BEEPER_SILENT);
    }

  }
  
  // Now do the button evaluation
  else {
    // Right Button
    if (digitalRead(RIGHT_BUTTON) == LOW) {
      Serial.println("Pressed RIGHT_BUTTON");
      digitalWrite(RIGHT_BUTTON_LIGHT, HIGH);
    } else {
      digitalWrite(RIGHT_BUTTON_LIGHT, LOW);
    }

    // Left Button
    if (digitalRead(LEFT_BUTTON) == LOW) {
      Serial.println("Pressed LEFT_BUTTON");
      digitalWrite(LEFT_BUTTON_LIGHT, HIGH);
    } else {
      digitalWrite(LEFT_BUTTON_LIGHT, LOW);
    }

    // Boil Sensor Button - activate if HIGH - Sound Alarm
    if (digitalRead(BOIL_SENSOR) == HIGH) {
      Serial.println("Pressed BOIL_SENSOR button");
      digitalWrite(ALARM, ALARM_SOUND);
    } else {
      digitalWrite(ALARM, ALARM_SILENT);
    }

    // Mash Sensor One Button - activate if HIGH - Light Water Pump Light
    if (digitalRead(MASH_SENSOR_ONE) == HIGH) {
      Serial.println("Pressed MASH_SENSOR_ONE button");
      digitalWrite(WATER_PUMP_LIGHT, HIGH);
    } else {
      digitalWrite(WATER_PUMP_LIGHT, LOW);
    }

    // Mash Sensor Two Button - activate if HIGH - Light Wart Pump Light
    if (digitalRead(MASH_SENSOR_TWO) == HIGH) {
      Serial.println("Pressed MASH_SENSOR_TWO button");
      digitalWrite(WORT_PUMP_LIGHT, HIGH);
    } else {
      digitalWrite(WORT_PUMP_LIGHT, LOW);
    }
  }
}
