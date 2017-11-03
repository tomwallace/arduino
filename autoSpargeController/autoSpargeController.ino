#include "EventQueue.h"
/*
 * AUTOSPARGE CONTROLLER
 * version 0.6
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
#define ALARM_PIN 8 //8
#define BUZZER_PIN 11 //*9 //11
#define LEFT_BUTTON_PIN 4 //4
#define LEFT_BUTTON_LIGHT_PIN 6 //6
#define RIGHT_BUTTON_PIN 3 //3
#define RIGHT_BUTTON_LIGHT_PIN 5 //5
#define MASH_PROBE_PIN 16 //16
#define MASH_PROBE_HIGH_PIN 17 //17
#define BOIL_PROBE_PIN 15 //*12 //15
#define WATER_PUMP_PIN 13 //*10 //13
#define WORT_PUMP_PIN 12 //*11 //12

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


// TODO: Add updateable interface with debug added and forces Update method
/*
 * EventQueue is a simple class that tracks events (non-duplicate) and informs if any events are in the queue.
 */
 /*
class EventQueue2 : Loggable {
  private: String _queue;
  private: String _queueName;
  
  public: EventQueue2(String queueName) {
    _queue = "";
    _queueName = queueName;
  };

  void AddEvent(String event) {
    // If event is not already on queue, add it
    if (_queue.indexOf(event) == -1) {
      Log(9999, _queueName, "Adding event " + event);
      _queue = _queue + event;
      Log(9999, _queueName, "Queue now " + _queue);
    }
  }

  void RemoveEvent(String event) {
    //Log(9999, _queueName, "Removing event " + event);
    _queue.replace(event, "");
    //Log(9999, _queueName, "Queue now " + _queue);
  }

  bool IsPopulated() {
    return (_queue.length() > 0);
  }
};
*/
/*
 * The Beeper class represents a sound output, such as the alarm or buzzer.  The sound is controlled 
 * by an EventQueue.
 */
class Beeper : Loggable {
  private: int _outputPin;
  private: EventQueue * _eventQueue;
  private: int _currentState;
  private: String _beeperName;
  
  private: int SOUND = HIGH;
  private: int SILENT = LOW;

  public: Beeper(String beeperName, int outputPin, EventQueue * eventQueue) {
    _beeperName = beeperName;
    _outputPin = outputPin;
    _eventQueue = eventQueue;

    pinMode(_outputPin, OUTPUT);
    digitalWrite(_outputPin, SILENT);
    _currentState = SILENT;
  }

  void Update(long currentMillis) {
    int OriginalState = _currentState;
    
    if (_eventQueue->IsPopulated()) {
      Log(currentMillis, _beeperName, "State is populated");
      digitalWrite(_outputPin, SOUND);
      _currentState = SOUND;
    } else {
      digitalWrite(_outputPin, SILENT);
      _currentState = SILENT;
    }
      
    // If state changed, then log
    if (_currentState != OriginalState) {
      String state = _currentState == SOUND ? "SOUNDING" : "SILENT";
      Log(currentMillis, _beeperName, "State has changed to " + state); 
    }
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

  private: EventQueue * _alarmEventQueue;
  
  int OutputPin;   // The pin number that control pump output
  long OnInterval; // Milliseconds in a minute the pump is on
  long OffInterval; // Milliseconds in a minute the pump is off
  bool IsActive;   // Toggle to let overrides stop pump

  // Members to detail state
  int CurrentState;
  unsigned long previousMillis;  // Stores the last time the state changed
  
  // Constructor
  public: WortPump(int outputPin, long onInterval, EventQueue * alarmEventQueue) {
    OutputPin = outputPin;
    pinMode(OutputPin, OUTPUT);

    _alarmEventQueue = alarmEventQueue;
    
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

    // Handle pulsing alarm every 2 seconds if probe is touching
    if (boilProbe.IsTouching() && IsActive) {
      // TODO: Consider changing to pulsing it works
      //bool canToggle = (currentMillis % 2000) == 0;
      //if (canToggle) {
      //  CurrentAlarmState = CurrentAlarmState == ALARM_SOUND ? ALARM_SILENT : ALARM_SOUND;
      //}
      _alarmEventQueue->AddEvent("BoilProbe");
    } else {
      _alarmEventQueue->RemoveEvent("BoilProbe");
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

  private: EventQueue * _alarmEventQueue;
  
  int OutputPin;   // The pin number that control pump output
  bool IsActive;   // Toggle to let overrides stop pump
  long Delay;      // Milliseconds of delay after change of state from probe

  // Members to detail state
  int CurrentState;
  int CurrentAlarmState;
  unsigned long previousMillis;  // Stores the last time the state changed
  
  // Constructor
  public: WaterPump(int outputPin, long delay, EventQueue * alarmEventQueue) {
    OutputPin = outputPin;
    pinMode(OutputPin, OUTPUT);

    _alarmEventQueue = alarmEventQueue;
    
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
    if (mashProbeHigh.IsTouching() && IsActive) {
      _alarmEventQueue->AddEvent("MashProbeHigh");
    } else {
      _alarmEventQueue->RemoveEvent("MashProbeHigh");
    }

    // If state changed, then log
    if (CurrentState != OriginalState) {
      String state = CurrentState == PUMP_ON ? "ON" : "OFF";
      Log(currentMillis, "Water Pump", "State has changed to " + state); 
    }
  }
};

class Button : Loggable {
  int BUTTON_BEEP_LENGTH = 10;  // Length of beep when button pressed
  int HAS_BEEN_CLICKED_DELAY = 500; // Length of delay before button is eligible for clicking again (prevents burst by holding button down)

  private: EventQueue * _buzzerEventQueue;

  String ButtonName;  // Name of the button for logging
  int ButtonPin;  // The pin number attached to the button
  int LightPin;  // The pin the button light is attached to
  long TurnOffClickSoundMillis;  // When to turn off the click sound
  bool EligibleToBeClicked; // Used with delay to prevent "burst" clicking
  long EligibleToBeClickedMillis; // To determine when to set the EligibleToBeClicked flag

  bool MatchingFunctionOn;  // Used to pair the button with a pump function

  // Constructor - inputType should be INPUT or INPUT_PULLUP
  public: Button(String buttonName, int buttonPin, int inputType, int lightPin, EventQueue * buzzerEventQueue) {
    ButtonName = buttonName;
    ButtonPin = buttonPin;
    LightPin = lightPin;
    _buzzerEventQueue = buzzerEventQueue;
    
    pinMode(ButtonPin, inputType);
    pinMode(LightPin, OUTPUT);
    TurnOffClickSoundMillis = 0;
    EligibleToBeClicked = true;
    EligibleToBeClickedMillis = 0;
    MatchingFunctionOn = false;
  }
  
  bool IsCurrentlyDepressed() {
    return (digitalRead(ButtonPin) == LOW);
  }

  bool GetMatchingFunctionOn() {
    return MatchingFunctionOn;
  }

  void Update(long currentMillis) {    
    // Determine if button has been clicked
    if (IsCurrentlyDepressed() && EligibleToBeClicked) {
      // Log click
      Log(currentMillis, ButtonName, "currently pushed.");
      
      EligibleToBeClicked = false;
      EligibleToBeClickedMillis = currentMillis + HAS_BEEN_CLICKED_DELAY;

      // Sound click on button
      _buzzerEventQueue->AddEvent(ButtonName);
      TurnOffClickSoundMillis = currentMillis + BUTTON_BEEP_LENGTH;
      
      // Toggle light and matching function
      MatchingFunctionOn = MatchingFunctionOn ? false : true;
    }

    // Determine when button is eligible to be clicked again to prevent button bursting
    if (!IsCurrentlyDepressed() && (currentMillis > EligibleToBeClickedMillis)) {
      EligibleToBeClicked = true;
    }

    // Turn off sound click when ready
    if (currentMillis > TurnOffClickSoundMillis) {
      _buzzerEventQueue->RemoveEvent(ButtonName);
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
EventQueue AlarmEventQueue("AlarmEventQueue");
EventQueue BuzzerEventQueue("BuzzerEventQueue");

Beeper Alarm("Alarm", ALARM_PIN, &AlarmEventQueue);
Beeper Buzzer("Buzzer", BUZZER_PIN, &BuzzerEventQueue);

Button LeftButton("Left Button", LEFT_BUTTON_PIN, INPUT_PULLUP, LEFT_BUTTON_LIGHT_PIN, &BuzzerEventQueue);
Button RightButton("Right Button", RIGHT_BUTTON_PIN, INPUT_PULLUP, RIGHT_BUTTON_LIGHT_PIN, &BuzzerEventQueue);

Probe MashProbe("Mash Probe", MASH_PROBE_PIN, INPUT);
Probe MashProbeHigh("Mash Probe High", MASH_PROBE_HIGH_PIN, INPUT);
Probe BoilProbe("Boil Probe", BOIL_PROBE_PIN, INPUT);

WaterPump WaterPump(WATER_PUMP_PIN, 10000, &AlarmEventQueue);
WortPump WortPump(WORT_PUMP_PIN, 2000, &AlarmEventQueue);

// Initalize general use variables
bool FirstTimeThroughCode = true;
bool InTestMode = false;

// put your setup code here, to run once:
void setup() {
  pinMode(TRINKET_BOARD_LED_PIN, OUTPUT);

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
    TestInteractions();
    /*
    if (LeftButton.IsCurrentlyDepressed()) {
      digitalWrite(LEFT_BUTTON_LIGHT_PIN, HIGH);
      digitalWrite(WATER_PUMP_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(LEFT_BUTTON_LIGHT_PIN, LOW);
      digitalWrite(WATER_PUMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }

    if (RightButton.IsCurrentlyDepressed()) {
      digitalWrite(RIGHT_BUTTON_LIGHT_PIN, HIGH);
      digitalWrite(WORT_PUMP_PIN, HIGH);
      digitalWrite(ALARM_PIN, HIGH);
    } else {
      digitalWrite(RIGHT_BUTTON_LIGHT_PIN, LOW);
      digitalWrite(WORT_PUMP_PIN, LOW);
      digitalWrite(ALARM_PIN, LOW);
    }

    // If Mash Probe then do three "dashes"
    if (digitalRead(MASH_PROBE_PIN) == HIGH) {
      int counter = 0;
      while(counter < 3) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(350);
        digitalWrite(BUZZER_PIN, LOW);
        delay(350);
        counter++;
      }
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }

    // If Boil Probe then do three "dots"
    if (digitalRead(BOIL_PROBE_PIN) == HIGH) {
      int counter = 0;
      while(counter < 3) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(75);
        digitalWrite(BUZZER_PIN, LOW);
        delay(450);
        counter++;
      }
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
    */
  } else {
    // NOT in test mode
    // Update objects
    LeftButton.Update(currentMillis);
    RightButton.Update(currentMillis);
  
    // Set pump active based on their buttons
    WaterPump.SetIsActive(LeftButton.GetMatchingFunctionOn());
    WortPump.SetIsActive(RightButton.GetMatchingFunctionOn());
  
    MashProbe.Update(currentMillis);
    MashProbeHigh.Update(currentMillis);
    BoilProbe.Update(currentMillis);
  
    WaterPump.Update(currentMillis, MashProbe, MashProbeHigh);
    WortPump.Update(currentMillis, BoilProbe);

    Alarm.Update(currentMillis);
    Buzzer.Update(currentMillis);
  }
}

void TestInteractions() {
  if (LeftButton.IsCurrentlyDepressed()) {
      digitalWrite(LEFT_BUTTON_LIGHT_PIN, HIGH);
      digitalWrite(WATER_PUMP_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(LEFT_BUTTON_LIGHT_PIN, LOW);
      digitalWrite(WATER_PUMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
    }

    if (RightButton.IsCurrentlyDepressed()) {
      digitalWrite(RIGHT_BUTTON_LIGHT_PIN, HIGH);
      digitalWrite(WORT_PUMP_PIN, HIGH);
      digitalWrite(ALARM_PIN, HIGH);
    } else {
      digitalWrite(RIGHT_BUTTON_LIGHT_PIN, LOW);
      digitalWrite(WORT_PUMP_PIN, LOW);
      digitalWrite(ALARM_PIN, LOW);
    }

    // If Mash Probe then do three "dashes"
    if (digitalRead(MASH_PROBE_PIN) == HIGH) {
      int counter = 0;
      while(counter < 3) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(350);
        digitalWrite(BUZZER_PIN, LOW);
        delay(350);
        counter++;
      }
    }
    //else if (!LeftButton.IsCurrentlyDepressed()) {
    //  digitalWrite(BUZZER_PIN, LOW);
    //}

    // If Boil Probe then do three "dots"
    if (digitalRead(BOIL_PROBE_PIN) == HIGH) {
      int counter = 0;
      while(counter < 3) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(75);
        digitalWrite(BUZZER_PIN, LOW);
        delay(450);
        counter++;
      }
    }
    //else {
    //  digitalWrite(BUZZER_PIN, LOW);
    //}
}

