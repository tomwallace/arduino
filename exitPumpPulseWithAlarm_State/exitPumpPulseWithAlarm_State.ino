/*
 * USE STATE MACHINE MODEL
 * This sketch will pulse the wort pump at the defined interval
 * so long as the probe does not encounter liquid.  If the probe 
 * encounters liquid, then the pulse stops and an alarm sounds. When
 * the probe is not touching liquid, then restart the pulse.
 */

#define BUZZER 0
#define BUZZER_SOUND HIGH
#define BUZZER_SILENT LOW
#define WORT_PUMP 1
#define PUMP_ON HIGH
#define PUMP_OFF LOW
#define PROBE 3  // TODO: Change to PIN 2 when can get it working properly
#define PROBE_TOUCH_LIQUID LOW
#define PROBE_CLEAR HIGH
#define PULSE_ON_INTERVAL 2000  // Milliseconds to turn wort pump on
#define PULSE_OFF_INTERVAL 10000 // Milliseconds to keep the wort pump off

int pumpState = PUMP_OFF;             // At the start, the pump is OFF
long previousMillis = 0;

// put your setup code here, to run once:
void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(WORT_PUMP, OUTPUT);

  //pinMode(PROBE, INPUT_PULLUP);
  pinMode(PROBE, INPUT);
}

// put your main code here, to run repeatedly:
void loop() {
  // Get current clock
  unsigned long currentMillis = millis();

  // State evaluations
  // Probe override, which always has the pump off
  if (digitalRead(PROBE) == PROBE_TOUCH_LIQUID) {
    digitalWrite(BUZZER, BUZZER_SOUND);
    pumpState = PUMP_OFF;
    digitalWrite(WORT_PUMP, pumpState);
  } else {
    digitalWrite(BUZZER, BUZZER_SILENT);
  }
  
  if ((digitalRead(PROBE) == PROBE_CLEAR) && (pumpState == PUMP_OFF) && (currentMillis - previousMillis >= PULSE_OFF_INTERVAL)) { 
    previousMillis = currentMillis;

    pumpState = PUMP_ON;
    digitalWrite(WORT_PUMP, pumpState);
  } else if ((digitalRead(PROBE) == PROBE_CLEAR) && (pumpState == PUMP_ON) && (currentMillis - previousMillis >= PULSE_ON_INTERVAL)) { 
    previousMillis = currentMillis;

    pumpState = PUMP_OFF;
    digitalWrite(WORT_PUMP, pumpState);
  }
}
