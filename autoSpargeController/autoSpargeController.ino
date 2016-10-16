/*
 * AUTOSPARGE CONTROLLER
 * version 0.4
 * by Tom Wallace and John Baker
 * This sketch controls the components hooked up to the Auto Sparge assembly, which controls the rate 
 * of sparge water going into the mash tun and the rate of wort leaving the mash tun.  It is designed to
 * work with a standard brewery build from The Electric Brewery.  The controller interacts with two pumps, 
 * and two probes.
 */
// TODO: Consider making an ALARM class
#define ALARM 8
#define ALARM_SOUND HIGH
#define ALARM_SILENT LOW
#define MAINT_MODE_BEEP_INTERVAL 4950
#define MAINT_MODE_BEEP_LENGTH 50

/*
 * The Logging base class manages sending logging messages out to the serial port
 */
class Loggable {
  public: Logger() {};
  
  void Log(long currentMillis, String callingObjName, String msg) {
    Serial.println(String(currentMillis) + " - " + callingObjName + ": " + msg);
  }
};

/*
 * The Probe class creates a probe input.  Its inputType specifies if it is INPUT or INPUT_PULLUP
 */
class Probe : Loggable {
  int PROBE_CLEAR = LOW;
  int PROBE_TOUCH_LIQUID = HIGH;
  String ProbeName;
  int CurrentState = PROBE_CLEAR;
  
  int InputPin;   // The pin number that receives probe input

  // Constructor - inputType should be INPUT or INPUT_PULLUP
  public: Probe(String probeName, int inputPin, int inputType) {
    InputPin = inputPin;
    ProbeName = probeName;
    pinMode(InputPin, inputType);
  }

  bool IsTouching() {
    return (CurrentState == PROBE_TOUCH_LIQUID);
  }

  void Update(long currentMillis) {
    int OriginalState = CurrentState;
    CurrentState = digitalRead(InputPin);
    
    // If state changed, then log
    if (CurrentState != OriginalState) {
      String state = CurrentState == PROBE_TOUCH_LIQUID ? "TOUCH LIQUID" : "CLEAR";
      Log(currentMillis, ProbeName, "State has changed to " + state); 
    }
  }
};

/*
 * The WortPump class interacts with the various aspects of the controllers use of the wart pump.
 */
class WortPump : Loggable {
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
    OffInterval = 60000 - onInterval;  // TODO: Change to be a minute.  Set for 10 seconds now for testing.
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
    int OriginalState = CurrentState;
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
    // If state changed, then log
    if (CurrentState != OriginalState) {
      String state = CurrentState == PUMP_ON ? "ON" : "OFF";
      Log(currentMillis, "Wort Pump", "State has changed to " + state); 
    }
  }
};

/*
 * The WaterPump class interacts with the various aspects of the controllers use of the water pump.
 */
class WaterPump : Loggable {
  int PUMP_ON = HIGH;
  int PUMP_OFF = LOW;
  
  int OutputPin;   // The pin number that control pump output
  bool IsActive;   // Toggle to let overrides stop pump
  long Delay;      // Milliseconds of delay after change of state from probe

  // Members to detail state
  int CurrentState;
  int CurrentAlarmState;
  unsigned long previousMillis;  // Stores the last time the state changed
  
  // Constructor
  public: WaterPump(int outputPin, long delay) {
    OutputPin = outputPin;
    pinMode(OutputPin, OUTPUT);
    
    Delay = delay;
    IsActive = true;

    CurrentState = PUMP_OFF;  // Pump starts off
    CurrentAlarmState = ALARM_SILENT;
    previousMillis = 0;
  }

  void SetIsActive(bool isActive) {
    IsActive = isActive;
  }

  bool GetIsActive() {
    return IsActive;
  }

  void Update(long currentMillis, Probe mashProbe, Probe mashProbeHigh) {
    int OriginalState = CurrentState;
    int OriginalAlarmState = CurrentAlarmState;
    
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
      CurrentAlarmState = ALARM_SOUND;
    } else {
      digitalWrite(ALARM, ALARM_SILENT);
      CurrentAlarmState = ALARM_SILENT;
    }

    // If state changed, then log
    if (CurrentState != OriginalState) {
      String state = CurrentState == PUMP_ON ? "ON" : "OFF";
      Log(currentMillis, "Water Pump", "State has changed to " + state); 
    }

    // If alarm state changed, then log
    if (CurrentAlarmState != OriginalAlarmState) {
      String state = CurrentAlarmState == ALARM_SOUND ? "SOUNDING" : "SILENT";
      Log(currentMillis, "Alarm", "State has changed to " + state); 
    }
  }
};

class Button : public Loggable {
  int BUTTON_BEEP_LENGTH = 10;  // Length of beep when button pressed
  int HAS_BEEN_CLICKED_DELAY = 500; // Length of delay before button is eligible for clicking again (prevents burst by holding button down)

  String ButtonName;  // Name of the button for logging
  int ButtonPin;  // The pin number attached to the button
  int LightPin;  // The pin the button light is attached to
  long ClickedMillis;  // Clock time button last pressed
  bool HasBeenClicked;  // Used to control click sound
  bool EligibleToBeClicked; // Used with delay to prevent "burst" clicking

  bool MatchingFunctionOn;  // Used to pair the button with a pump function

  // Constructor - inputType should be INPUT or INPUT_PULLUP
  public: Button(String buttonName, int buttonPin, int inputType, int lightPin) {
    ButtonName = buttonName;
    ButtonPin = buttonPin;
    LightPin = lightPin;
    pinMode(ButtonPin, inputType);
    pinMode(LightPin, OUTPUT);
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

  // Pulses the light in 1 second increments, used to indicate in maintenance mode
  void PulseLight(long currentMillis) {
    if (((currentMillis / 1000) % 2) == 1) {
      digitalWrite(LightPin, HIGH);
    } else {
      digitalWrite(LightPin, LOW);
    }
  }

  void Update(long currentMillis, int buzzerPin) {
    // Determine if button has been clicked
    if ((! HasBeenClicked) && (digitalRead(ButtonPin) == LOW)) {
      // Log click
      String sMillis = String(currentMillis);
      String sButtonPin = String(ButtonPin);
      //Serial.println(sMillis + "Button Pin: " + sButtonPin + " is currently pushed.");
      Log(currentMillis, ButtonName, "currently pushed.");
      
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
  int TIME_BUTTON_HELD_ENTER = 2000;  // Number of milliseconds button must be held to enter Maintenance Mode
  
  bool InMode;
  int InputButtonPinOne;   // The pin number that receives input to put into Maintenance Mode (button one)
  int InputButtonPinTwo;   // The pin number that receives input to put into Maintenance Mode (button two)
  long ButtonMillis;

  // Constructor
  public: MaintenanceMode(int inputButtonPinOne, int inputButtonPinTwo) {
    InputButtonPinOne = inputButtonPinOne;
    InputButtonPinTwo = inputButtonPinTwo;
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

  void Update(long currentMillis) {
    if ((digitalRead(InputButtonPinOne) == LOW) && (digitalRead(InputButtonPinTwo) == LOW)) {
      // If we have not recorded the push time of the buttons, do so now
      if (ButtonMillis == 0) {
        ButtonMillis = currentMillis;
      }
      if (currentMillis - ButtonMillis > TIME_BUTTON_HELD_ENTER) {
        // TODO: Fix bug when switch happens if on
        ToggleMode();
        ButtonMillis = 0;     
      }
    } else {
      ButtonMillis = 0;
    }
  }
};

// Create objects
Button LeftButton("Left Button", 4, INPUT_PULLUP, 6);
Button RightButton("Right Button", 3, INPUT_PULLUP, 5);

Probe MashProbe("Mash Probe", 16, INPUT);
Probe MashProbeHigh("Mash Probe High", 17, INPUT);
Probe BoilProbe("Boil Probe", 15, INPUT);

WaterPump WaterPump(13, 10000);
WortPump WortPump(12, 2000);  // TODO: Pull this value from writeable memory

MaintenanceMode MaintMode(LeftButton.GetButtonPin(), RightButton.GetButtonPin());



long MaintModeBeepMillis = 0;
int BuzzerState = ALARM_SILENT;

// put your setup code here, to run once:
void setup() {
  pinMode(ALARM, OUTPUT);

  // Set up serial port for output at 9600 bps
  Serial.begin(9600);
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

  // Determine if in maintenance mode, which trumps all other functions
  if (MaintMode.GetInMode()) {
    // If in Maintenance Mode, the pumps are always off
    WaterPump.SetIsActive(false);
    WortPump.SetIsActive(false);
    // TODO: Make lights turn off as default before flashing

    // Make lights flash to indicate in mode
    LeftButton.PulseLight(currentMillis);
    RightButton.PulseLight(currentMillis);
  }

  // Set pump active based on their buttons
  WaterPump.SetIsActive(LeftButton.GetMatchingFunctionOn());
  WortPump.SetIsActive(RightButton.GetMatchingFunctionOn());

  MashProbe.Update(currentMillis);
  MashProbeHigh.Update(currentMillis);
  BoilProbe.Update(currentMillis);

  WaterPump.Update(currentMillis, MashProbe, MashProbeHigh);
  WortPump.Update(currentMillis, BoilProbe);

  MaintMode.Update(currentMillis);
}

