#define WORT_PUMP_LIGHT 3
#define WATER_PUMP_LIGHT 4

#define ALARM 5
#define ALARM_SOUND HIGH
#define ALARM_SILENT LOW
#define ACTIVATE_INTERVAL 3000

#define ON_LENGTH 2000

#define RIGHT_BUTTON_LIGHT 6
#define LEFT_BUTTON_LIGHT 8
#define RIGHT_BUTTON 14
#define LEFT_BUTTON 12

#define BOIL_SENSOR 9
#define MASH_SENSOR_ONE 10
#define MASH_SENSOR_TWO 11

long StartTimeMillis = 0;
int StartWortPumpLight = 0;
int StopWortPumpLight = 0;
int StartWaterPumpLight = 0;
int StopWaterPumpLight = 0;
int StartAlarm = 0;
int StopAlarm = 0;
int StartRightButtonLight = 0;
int StopRightButtonLight = 0;
int StartLeftButtonLight = 0;
int StopLeftButtonLight = 0;


void setup() {
  // put your setup code here, to run once:
  // Buttons are pull-up resistors
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  
  pinMode(ALARM, OUTPUT);
  
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
}

void loop() {
  // put your main code here, to run repeatedly:
  // Get current clock
  unsigned long currentMillis = millis();

  // If in initial start up evaluation, i.e. less than the last time the last light is turned on
  if (currentMillis <= StopLeftButtonLight) {
    // Wort Pump Light
    if ((currentMillis >= StartWortPumpLight) && (currentMillis <= StopWortPumpLight)) {
      digitalWrite(WORT_PUMP_LIGHT, HIGH);
    } else {
      digitalWrite(WORT_PUMP_LIGHT, LOW);
    }
  
    // Water Pump Light
    if ((currentMillis >= StartWaterPumpLight) && (currentMillis <= StopWaterPumpLight)) {
      digitalWrite(WATER_PUMP_LIGHT, HIGH);
    } else {
      digitalWrite(WATER_PUMP_LIGHT, LOW);
    }
  
    // Alarm
    if ((currentMillis >= StartAlarm) && (currentMillis <= StopAlarm)) {
      digitalWrite(ALARM, ALARM_SOUND);
    } else {
      digitalWrite(ALARM, ALARM_SILENT);
    }
  
    // Right Button Light
    if ((currentMillis >= StartRightButtonLight) && (currentMillis <= StopRightButtonLight)) {
      digitalWrite(RIGHT_BUTTON_LIGHT, HIGH);
    } else {
      digitalWrite(RIGHT_BUTTON_LIGHT, LOW);
    }
  
    // Left Button Light
    if ((currentMillis >= StartLeftButtonLight) && (currentMillis <= StopLeftButtonLight)) {
      digitalWrite(LEFT_BUTTON_LIGHT, HIGH);
    } else {
      digitalWrite(LEFT_BUTTON_LIGHT, LOW);
    }
  }
  
  // Now do the button evaluation
  else {
    // TODO: This currently controls the light on the Trinket - the actual light stays on all the time
    // Right Button
    if (digitalRead(RIGHT_BUTTON) == LOW) {
      digitalWrite(RIGHT_BUTTON_LIGHT, HIGH);
    } else {
      digitalWrite(RIGHT_BUTTON_LIGHT, LOW);
    }

    // Left Button
    if (digitalRead(LEFT_BUTTON) == LOW) {
      digitalWrite(LEFT_BUTTON_LIGHT, HIGH);
    } else {
      digitalWrite(LEFT_BUTTON_LIGHT, LOW);
    }

    // Boil Sensor Button - activate if HIGH - Sound Alarm
    if (digitalRead(BOIL_SENSOR) == HIGH) {
      digitalWrite(ALARM, ALARM_SOUND);
    } else {
      digitalWrite(ALARM, ALARM_SILENT);
    }

    // Mash Sensor One Button - activate if HIGH - Light Water Pump Light
    if (digitalRead(MASH_SENSOR_ONE) == HIGH) {
      digitalWrite(WATER_PUMP_LIGHT, HIGH);
    } else {
      digitalWrite(WATER_PUMP_LIGHT, LOW);
    }

    // Mash Sensor Two Button - activate if HIGH - Light Wart Pump Light
    if (digitalRead(MASH_SENSOR_TWO) == HIGH) {
      digitalWrite(WORT_PUMP_LIGHT, HIGH);
    } else {
      digitalWrite(WORT_PUMP_LIGHT, LOW);
    }
  }
}
