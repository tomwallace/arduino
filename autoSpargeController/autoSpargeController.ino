/*
 * AUTOSPARGE CONTROLLER
 * version 0.5
 * by Tom Wallace and John Baker
 * This sketch controls the components hooked up to the Auto Sparge assembly, which controls the rate 
 * of sparge water going into the mash tun and the rate of wort leaving the mash tun.  It is designed to
 * work with a standard brewery build from The Electric Brewery.  The controller interacts with two pumps, 
 * and two probes.
 * 
 * Test mode can be entered by reboot the trinket (or cycling power) and holding down the Left Button while
 * it finishes booting.  All lights will flash three times if in test mode.  To exit test mode,
 * reboot the trinket.
 */
// Define Pins
#define TRINKET_BOARD_LED_PIN 13
#define ALARM 8 //8
#define BUZZER 9 //11
#define LEFT_BUTTON_PIN 4 //4
#define LEFT_BUTTON_LIGHT_PIN 6 //6
#define RIGHT_BUTTON_PIN 3 //3
#define RIGHT_BUTTON_LIGHT_PIN 5 //5
#define MASH_PROBE_PIN 16 //16
#define MASH_PROBE_HIGH_PIN 17 //17
#define BOIL_PROBE_PIN 12 //15
#define WATER_PUMP_PIN 10 //13
#define WORT_PUMP_PIN 11 //12

#define ALARM_SOUND HIGH
#define ALARM_SILENT LOW

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
    OffInterval = 60000 - onInterval;
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

class Button : Loggable {
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

  bool IsCurrentlyDepressed() {
    return (digitalRead(ButtonPin) == LOW);
  }

  bool GetMatchingFunctionOn() {
    return MatchingFunctionOn;
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

// Create objects
Button LeftButton("Left Button", LEFT_BUTTON_PIN, INPUT_PULLUP, LEFT_BUTTON_LIGHT_PIN);
Button RightButton("Right Button", RIGHT_BUTTON_PIN, INPUT_PULLUP, RIGHT_BUTTON_LIGHT_PIN);

Probe MashProbe("Mash Probe", MASH_PROBE_PIN, INPUT);
Probe MashProbeHigh("Mash Probe High", MASH_PROBE_HIGH_PIN, INPUT);
Probe BoilProbe("Boil Probe", BOIL_PROBE_PIN, INPUT);

WaterPump WaterPump(WATER_PUMP_PIN, 10000);
WortPump WortPump(WORT_PUMP_PIN, 2000);

// Initalize general use variables
bool FirstTimeThroughCode = true;
bool InTestMode = false;

// put your setup code here, to run once:
void setup() {
  pinMode(TRINKET_BOARD_LED_PIN, OUTPUT);
  pinMode(ALARM, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Set up serial port for output at 9600 bps
  Serial.begin(9600);
}

// put your main code here, to run repeatedly:
void loop() {
  // Get current clock
  unsigned long currentMillis = millis();

  // Will only be run in first millisecond of code - used for Test mode evaluation
  if (FirstTimeThroughCode) {
    FirstTimeThroughCode = false;
    // Puts us in Test mode - only way to get out is to cycle power or reset and LeftButton is NOT pushed
    if (LeftButton.IsCurrentlyDepressed()) {
      Serial.println(String(currentMillis) + " - Start-up: Entering test mode");
      InTestMode = true;

      // Flash buttons
      int counter = 0;
      while(counter < 3) {
        digitalWrite(LEFT_BUTTON_LIGHT_PIN, HIGH);
        digitalWrite(RIGHT_BUTTON_LIGHT_PIN, HIGH);
        digitalWrite(TRINKET_BOARD_LED_PIN, HIGH);
        delay(250);
        digitalWrite(LEFT_BUTTON_LIGHT_PIN, LOW);
        digitalWrite(RIGHT_BUTTON_LIGHT_PIN, LOW);
        digitalWrite(TRINKET_BOARD_LED_PIN, LOW);
        delay(250);
        counter++;
      }
    }
  }

  // Test mode routine - only way to get out is to cycle power or reset and LeftButton is NOT pushed
  if (InTestMode) {
    if (LeftButton.IsCurrentlyDepressed()) {
      digitalWrite(LEFT_BUTTON_LIGHT_PIN, HIGH);
      digitalWrite(WATER_PUMP_PIN, HIGH);
      digitalWrite(BUZZER, HIGH);
    } else {
      digitalWrite(LEFT_BUTTON_LIGHT_PIN, LOW);
      digitalWrite(WATER_PUMP_PIN, LOW);
      digitalWrite(BUZZER, LOW);
    }

    if (RightButton.IsCurrentlyDepressed()) {
      digitalWrite(RIGHT_BUTTON_LIGHT_PIN, HIGH);
      digitalWrite(WORT_PUMP_PIN, HIGH);
      digitalWrite(ALARM, HIGH);
    } else {
      digitalWrite(RIGHT_BUTTON_LIGHT_PIN, LOW);
      digitalWrite(WORT_PUMP_PIN, LOW);
      digitalWrite(ALARM, LOW);
    }

    // If Mash Probe then do three "dashes"
    if (digitalRead(MASH_PROBE_PIN) == HIGH) {
      int counter = 0;
      while(counter < 3) {
        digitalWrite(BUZZER, HIGH);
        delay(350);
        digitalWrite(BUZZER, LOW);
        delay(350);
        counter++;
      }
    } else {
      digitalWrite(BUZZER, LOW);
    }

    // If Boil Probe then do three "dots"
    if (digitalRead(BOIL_PROBE_PIN) == HIGH) {
      int counter = 0;
      while(counter < 3) {
        digitalWrite(BUZZER, HIGH);
        delay(75);
        digitalWrite(BUZZER, LOW);
        delay(450);
        counter++;
      }
    } else {
      digitalWrite(BUZZER, LOW);
    }
  }

  // Update objects
  LeftButton.Update(currentMillis, BUZZER);
  RightButton.Update(currentMillis, BUZZER);

  // Set pump active based on their buttons
  WaterPump.SetIsActive(LeftButton.GetMatchingFunctionOn());
  WortPump.SetIsActive(RightButton.GetMatchingFunctionOn());

  MashProbe.Update(currentMillis);
  MashProbeHigh.Update(currentMillis);
  BoilProbe.Update(currentMillis);

  WaterPump.Update(currentMillis, MashProbe, MashProbeHigh);
  WortPump.Update(currentMillis, BoilProbe);
}

