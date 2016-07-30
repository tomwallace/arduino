/*
 * AUTOSPARGE CONTROLLER
 * version 0.2
 * by Tom Wallace and John Baker
 * This sketch controls the components hooked up to the Auto Sparge assembly, which controls the rate 
 * of sparge water going into the mash tun and the rate of wort leaving the mash tun.  It is designed to
 * work with a standard brewery build from The Electric Brewery.  The controller interacts with two pumps, 
 * and two probes.
 */

// TODO: Consider making an ALARM class
#define ALARM 5
#define ALARM_SOUND HIGH
#define ALARM_SILENT LOW
#define MAINT_MODE_BEEP_INTERVAL 4950
#define MAINT_MODE_BEEP_LENGTH 50

/*
 * The Probe class creates a probe input.  Its inputType specifies if it is INPUT or INPUT_PULLUP
 */
class Probe {
  int PROBE_CLEAR = LOW;
  int PROBE_TOUCH_LIQUID = HIGH;
  
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

// TODO: Consider refactoring common pump stuff into a ParentClass
/*
 * The WortPump class interacts with the various aspects of the controllers use of the wart pump.
 */
class WortPump {
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
  public: WortPump(int outputPin, long onInterval) {
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

  // Adds a second that the pump is on each minute
  void AddSecondOn() {
    OnInterval = OnInterval + 1000;
    OffInterval = OffInterval - 1000;
  }

  // Subtracts a second that the pump is on each minute
  void SubtractsSecondOn() {
    OnInterval = OnInterval - 1000;
    OffInterval = OffInterval + 1000;
  }

  void Update(long currentMillis, Probe boilProbe) {
    // If probe is contacting liquid, pump is always off
    // Pump is also always off if in maintenance mode
    if (boilProbe.IsTouching() || ! IsActive) {
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
 * The WaterPump class interacts with the various aspects of the controllers use of the water pump.
 */
class WaterPump {
  int PUMP_ON = HIGH;
  int PUMP_OFF = LOW;
  
  int OutputPin;   // The pin number that control pump output
  bool IsActive;   // Toggle to let overrides stop pump
  long Delay;      // Milliseconds of delay after change of state from probe

  // Members to detail state
  int CurrentState;
  unsigned long previousMillis;  // Stores the last time the state changed
  
  // Constructor
  public: WaterPump(int outputPin, long delay) {
    OutputPin = outputPin;
    pinMode(OutputPin, OUTPUT);
    
    Delay = delay;
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

  void Update(long currentMillis, Probe mashProbe, Probe mashProbeHigh) {
    // If probe is contacting liquid, pump is always off
    // Pump is also always off if in maintenance mode
    if (mashProbe.IsTouching() || ! IsActive) {
      previousMillis = currentMillis;
      
      CurrentState = PUMP_OFF;
      digitalWrite(OutputPin, CurrentState);
    } else {
      if (currentMillis - previousMillis >= Delay) { 
        previousMillis = currentMillis;
  
        CurrentState = PUMP_ON;
        digitalWrite(OutputPin, CurrentState);
      }
    }

    // Sound alarm if high level probe contacting liquid
    if (mashProbeHigh.IsTouching()) {
      digitalWrite(ALARM, ALARM_SOUND);
    } else {
      digitalWrite(ALARM, ALARM_SILENT);
    }
  }
};

class Button {
  int BUTTON_BEEP_LENGTH = 10;  // Length of beep when button pressed
  int HAS_BEEN_CLICKED_DELAY = 500; // Length of delay before button is eligible for clicking again (prevents burst by holding button down)

  int ButtonPin;  // The pin number attached to the button
  int LightPin;  // The pin the button light is attached to
  long ClickedMillis;  // Clock time button last pressed
  bool HasBeenClicked;  // Used to control click sound
  bool EligibleToBeClicked; // Used with delay to prevent "burst" clicking

  bool MatchingFunctionOn;  // Used to pair the button with a pump function

  // Constructor - inputType should be INPUT or INPUT_PULLUP
  public: Button(int buttonPin, int inputType, int lightPin) {
    ButtonPin = buttonPin;
    LightPin = lightPin;
    pinMode(ButtonPin, inputType);
    ClickedMillis = 0;
    HasBeenClicked = false;
    EligibleToBeClicked = true;
    MatchingFunctionOn = false;
  }

  int GetButtonPin() {
    return ButtonPin;
  }

  bool IsClickedWithDelay() {
    return (HasBeenClicked && (! EligibleToBeClicked));
  }

  bool GetMatchingFunctionOn() {
    return MatchingFunctionOn;
  }

  void Update(long currentMillis, int buzzerPin) {
    // Determine if button has been clicked
    if ((! HasBeenClicked) && (digitalRead(ButtonPin) == LOW)) {
      ClickedMillis = currentMillis;
      HasBeenClicked = true;
      EligibleToBeClicked = false;
      // Toggle light and matching function
      MatchingFunctionOn = MatchingFunctionOn ? false : true;
    }
    // Sound click on button
    if (currentMillis - ClickedMillis <= BUTTON_BEEP_LENGTH) { 
      digitalWrite(buzzerPin, HIGH);
    } else if (currentMillis - ClickedMillis <= BUTTON_BEEP_LENGTH + 5) { 
      digitalWrite(buzzerPin, LOW);
    }
    if (digitalRead(ButtonPin) == HIGH) {
      HasBeenClicked = false;
    }
    if ((digitalRead(ButtonPin) == HIGH) && (currentMillis - ClickedMillis >= HAS_BEEN_CLICKED_DELAY)) {
      EligibleToBeClicked = true;
    }
    // Handle light on/off
    if (MatchingFunctionOn) {
      digitalWrite(LightPin, HIGH);
    } else {
      digitalWrite(LightPin, LOW);
    }
  }
};

/*
 * The MaintenanceMode class isolates functionality related to entering and leaving maintenance mode.
 */
class MaintenanceMode {
  int TIME_BUTTON_HELD_ENTER = 3000;  // Number of milliseconds button must be held to enter Maintenance Mode
  
  bool InMode;
  int InputButtonPin;   // The pin number that receives input to put into Maintenance Mode
  long ButtonMillis;

  // Constructor
  public: MaintenanceMode(int inputButtonPin) {
    InputButtonPin = inputButtonPin;
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

  void Update(long currentMillis, WortPump wortPump) {
    if (digitalRead(InputButtonPin) == LOW) {
      // If we have not recorded the button push time, do so now
      if (ButtonMillis == 0) {
        ButtonMillis = currentMillis;
      }
      if (currentMillis - ButtonMillis > TIME_BUTTON_HELD_ENTER) {
        // TODO: Fix bug when switch happens if on
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
Button LeftButton(12, INPUT_PULLUP, 8);
Button RightButton(14, INPUT_PULLUP, 6);
// TODO: Extend the buttons to handle their lights

Probe MashProbe(10, INPUT);
Probe MashProbeHigh(11, INPUT);
Probe BoilProbe(9, INPUT);

WaterPump WaterPump(4, 10000);
WortPump WortPump(3, 2000);  // TODO: Pull this value from writeable memory



//MaintenanceMode MaintMode(ButtonUp.GetButtonPin());



long MaintModeBeepMillis = 0;
int BuzzerState = ALARM_SILENT;

// put your setup code here, to run once:
void setup() {
  pinMode(ALARM, OUTPUT);
}

// put your main code here, to run repeatedly:
void loop() {
  // Get current clock
  unsigned long currentMillis = millis();

  // TODO: Return to maintenance mode after finished with base functionality
    /*
  // Determine if in maintenance mode, which trumps all other functions
  if (MaintMode.GetInMode()) {
    // Buzzer sounds in background to indicate still in Maintenance Mode
    if ((BuzzerState == ALARM_SILENT) && (currentMillis - MaintModeBeepMillis >= MAINT_MODE_BEEP_INTERVAL)) { 
      MaintModeBeepMillis = currentMillis;
      BuzzerState = ALARM_SOUND;
      digitalWrite(ALARM, BuzzerState);
    } else if ((BuzzerState == ALARM_SOUND) && (currentMillis - MaintModeBeepMillis >= MAINT_MODE_BEEP_LENGTH)) { 
      MaintModeBeepMillis = currentMillis;
      BuzzerState = ALARM_SILENT;
      digitalWrite(ALARM, BuzzerState);
    }
    
    // Determine if needed
    if (ButtonUp.IsClickedWithDelay()) {
      WortPump.AddSecondOn();
    }

  } else {
    // Probe override, which always has the pump off
    if (MashProbe.IsTouching()) {
      digitalWrite(ALARM, ALARM_SOUND);
      WortPump.SetIsActive(false);
    } else {
      digitalWrite(ALARM, ALARM_SILENT);
      WortPump.SetIsActive(true);
    }
  
    WortPump.Update(currentMillis);
  }
    
  
  MaintMode.Update(currentMillis, WortPump);
  ButtonUp.Update(currentMillis, ALARM);
*/

  // Update objects
  LeftButton.Update(currentMillis, ALARM);
  RightButton.Update(currentMillis, ALARM);

  // Set pump active based on their buttons
  WaterPump.SetIsActive(LeftButton.GetMatchingFunctionOn());
  WortPump.SetIsActive(RightButton.GetMatchingFunctionOn());

  WaterPump.Update(currentMillis, MashProbe, MashProbeHigh);
  WortPump.Update(currentMillis, BoilProbe);
}

