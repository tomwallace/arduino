/*
 * AUTOSPARGE CONTROLLER
 * version 0.1
 * by Tom Wallace and John Baker
 * This sketch controls the components hooked up to the Auto Sparge assembly, which controls the rate 
 * of sparge water going into the mash tun and the rate of wort leaving the mash tun.  It is designed to
 * work with a standard brewery build from The Electric Brewery.  The controller interacts with two pumps, 
 * and two probes.
 */

/*
 * The Pump class interacts with the various aspects of the controllers use of pumps.
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
    OffInterval = 10000 - onInterval;  // TODO: Change to be a minute.  Set for 10 seconds now for testing.
    IsActive = true;

    CurrentState = PUMP_OFF;  // Pump starts off
    previousMillis = 0;
  }

  void SetIsActive(bool isActive) {
    IsActive = isActive;
  }

  bool GetIsActive() {
    return IsActive;
  }

  void Update(long currentMillis) {
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

/*
 * The Probe class creates a probe input.  Its inputType specifies if it is INPUT or INPUT_PULLUP
 */
class Probe {
  int PROBE_CLEAR = HIGH;
  int PROBE_TOUCH_LIQUID = LOW;
  
  int InputPin;   // The pin number that receives probe input

  // Constructor - inputType should be INPUT or INPUT_PULLUP
  public: Probe(int inputPin, int inputType) {
    InputPin = inputPin;
    pinMode(InputPin, inputType);
  }

  bool IsTouching() {
    return (digitalRead(InputPin) == PROBE_TOUCH_LIQUID);
  }
};

/*
 * The MaintenanceMode class isolates functionality related to entering and leaving maintenance mode.
 */
class MaintenanceMode {
  int TIME_BUTTON_HELD_ENTER = 3000;  // Number of milliseconds button must be held to enter Maintenance Mode
  
  bool InMode;
  int InputPin;   // The pin number that receives input to put into Maintenance Mode
  long ButtonMillis;

  // Constructor - inputType should be INPUT or INPUT_PULLUP
  public: MaintenanceMode(int inputPin, int inputType) {
    InputPin = inputPin;
    pinMode(InputPin, inputType);
    InMode = false;
    ButtonMillis = 0;
  }

  bool GetInMode() {
    return InMode;
  }

  // Toggles the mode
  void ToggleMode() {
    if (InMode) {
      InMode = false;
    } else {
      InMode = true;
    }
  }

  void Update(long currentMillis, Pump wortPump) {
    if (digitalRead(InputPin) == LOW) {
      // If we have not recorded the button push time, do so now
      if (ButtonMillis == 0) {
        ButtonMillis = currentMillis;
      }
      if (currentMillis - ButtonMillis > TIME_BUTTON_HELD_ENTER) {
        ToggleMode();
        ButtonMillis = 0;
        // Pump active level is opposite if we are in Maintenance Mode
        wortPump.SetIsActive((! InMode));     
      }
    } else {
      ButtonMillis = 0;
    }
  }
};

// Create objects
Pump WortPump(1, 2000);
Probe MashProbe(3, INPUT);  // TODO: Change to PIN 2 when can get it working properly
MaintenanceMode MaintMode(4, INPUT_PULLUP);

#define BUZZER 0
#define BUZZER_SOUND HIGH
#define BUZZER_SILENT LOW
#define MAINT_MODE_BEEP_INTERVAL 4950
#define MAINT_MODE_BEEP_LENGTH 50

//bool InMaintenanceMode = false;
long MaintModeBeepMillis = 0;
int BuzzerState = BUZZER_SILENT;

// put your setup code here, to run once:
void setup() {
  pinMode(BUZZER, OUTPUT);
}

// put your main code here, to run repeatedly:
void loop() {
  // Get current clock
  unsigned long currentMillis = millis();
    
  // Determine if in maintenance mode, which trumps all other functions
  if (MaintMode.GetInMode()) {
    // Buzzer sounds in background to indicate still in Maintenance Mode
    if ((BuzzerState == BUZZER_SILENT) && (currentMillis - MaintModeBeepMillis >= MAINT_MODE_BEEP_INTERVAL)) { 
      MaintModeBeepMillis = currentMillis;
      BuzzerState = BUZZER_SOUND;
      digitalWrite(BUZZER, BuzzerState);
    } else if ((BuzzerState == BUZZER_SOUND) && (currentMillis - MaintModeBeepMillis >= MAINT_MODE_BEEP_LENGTH)) { 
      MaintModeBeepMillis = currentMillis;
      BuzzerState = BUZZER_SILENT;
      digitalWrite(BUZZER, BuzzerState);
    }
    
  } else {
    // Probe override, which always has the pump off
    if (MashProbe.IsTouching()) {
      digitalWrite(BUZZER, BUZZER_SOUND);
      WortPump.SetIsActive(false);
    } else {
      digitalWrite(BUZZER, BUZZER_SILENT);
      WortPump.SetIsActive(true);
    }
  
    WortPump.Update(currentMillis);
  }

  MaintMode.Update(currentMillis, WortPump);
}

