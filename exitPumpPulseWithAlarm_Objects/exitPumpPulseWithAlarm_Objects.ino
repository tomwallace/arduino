/*
 * USE OBJECTS AND STATE MACHINE MODEL
 * This sketch will pulse the wort pump at the defined interval
 * so long as the probe does not encounter liquid.  If the probe 
 * encounters liquid, then the pulse stops and an alarm sounds. When
 * the probe is not touching liquid, then restart the pulse.
 */

class Pump {
  int PUMP_ON = HIGH;
  int PUMP_OFF = LOW;
  
  int OutputPin;   // The pin number that control pump output
  long OnInterval; // Milliseconds in a minute the pump is on
  long OffInterval; // Milliseconds in a minute the pump is off
  bool IsActive;   // Toggle to let overrides stop pump

  // Members to detail state
  int CurrentState;
  unsigned long previousMillis;  // Stores the last time the state changed
  
  // Constructor
  public: Pump(int outputPin, long onInterval) {
    OutputPin = outputPin;
    pinMode(OutputPin, OUTPUT);
    
    OnInterval = onInterval;
    OffInterval = 60000 - onInterval;
    IsActive = true;

    CurrentState = PUMP_OFF;  // Pump starts off
    previousMillis = 0;
  }

  public: void SetIsActive(bool isActive) {
    IsActive = isActive;
  }

  public: bool GetIsActive() {
    return IsActive;
  }

  void Update() {
    // Get current clock
    unsigned long currentMillis = millis();
    if (! IsActive) {
      CurrentState = PUMP_OFF;
      digitalWrite(OutputPin, CurrentState);
    } else {
      if ((CurrentState == PUMP_OFF) && (currentMillis - previousMillis >= OffInterval)) { 
        previousMillis = currentMillis;
  
        CurrentState = PUMP_ON;
        digitalWrite(OutputPin, CurrentState);
      } else if ((CurrentState == PUMP_ON) && (currentMillis - previousMillis >= OnInterval)) { 
        previousMillis = currentMillis;
  
        CurrentState = PUMP_OFF;
        digitalWrite(OutputPin, CurrentState);
      }
    }
  }
};

Pump WortPump(1, 2000);

#define BUZZER 0
#define BUZZER_SOUND HIGH
#define BUZZER_SILENT LOW

#define PROBE 2  // TODO: Change to PIN 2 when can get it working properly
#define PROBE_TOUCH_LIQUID HIGH
#define PROBE_CLEAR LOW

// put your setup code here, to run once:
void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(PROBE, INPUT);
}

// put your main code here, to run repeatedly:
void loop() {
  // Probe override, which always has the pump off
  if (digitalRead(PROBE) == PROBE_TOUCH_LIQUID) {
    digitalWrite(BUZZER, BUZZER_SOUND);
    WortPump.SetIsActive(false);
  } else {
    digitalWrite(BUZZER, BUZZER_SILENT);
    WortPump.SetIsActive(true);
  }

  WortPump.Update();
}
