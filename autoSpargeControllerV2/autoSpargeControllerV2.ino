#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include "Adafruit_MPRLS.h"

// TODO: Alter the includes to be live
//#include "Beeper.h"
//#include "Button.h"
//#include "EventQueue.h"
//#include "Probe.h"
//#include "WaterPump.h"
//#include "WortPump.h"

/* TODO: Evaluate instructions to see if still accurate.
 * AUTOSPARGE CONTROLLER
 * version 2.0
 * by Tom Wallace and John Baker
 * This sketch is the upgraded version of the AutoSparge Controller, which controls the components hooked up 
 * to the Auto Sparge assembly, which controls the rate of sparge water going into the mash tun and the rate of 
 * wort leaving the mash tun.  It is designed to work with a standard brewery build from The Electric Brewery.  
 * The controller interacts with two pumps, and two probes.
 * 
 * The upgrade for version 2.0 is the use of a RGB display component, including selectable menus and the ability to
 * enter preset boil kettle fill levels.  Additionally, version 2.0 makes use of a pressure sensor probe for the 
 * boil kettle to predict the actual volume, rather than having the probe level set the max fill amount.
 * 
 * Test mode can be entered by reboot the trinket (or cycling power) and holding down the Left Button while
 * it finishes booting.  All lights will flash three times if in test mode.  To exit test mode,
 * reboot the trinket.
 */

// Define Pins
#define RESET_PIN -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN -1  // set to any GPIO pin to read end-of-conversion by pin

// Define Colors
// TODO: Remove those not needed
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

// Define Modes
#define TEST_MODE 0
#define V1_MODE 1
#define V2_MODE 2 

// Create objects
Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN);
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Global variables
bool initializeComplete = false;
int mode = -1;
int startTime = V1_MODE;  // Default to existing behavior
int endInitTime = 0;

// Setup code only runs once
// TODO: Break into private methods if helps with organization
void setup() {
  mpr.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(GREEN);
  
  // Lug Wrench welcome message  
  String welcome = "Lug Wrench Brewing Company Auto Sparge V2.0";
  for (int i = 0; i < (welcome.length() + 16); i++) {
    int start = 0;
    if (i > 16) {
      start = i - 16;
    }
    
    String subWelcome = welcome.substring(start, i + 1);
    int cursorLocation = 16 - subWelcome.length();
    if (i > 16) {
      cursorLocation = 0;
    }
    
    lcd.setCursor(cursorLocation, 0);
    lcd.print(subWelcome);
    delay(150);
    lcd.clear();
  }
  
  lcd.clear();
  delay(1000);

  startTime = millis();
  endInitTime = 9000 + startTime;
  /*  
  for (readIndex1 = 0; readIndex1 < numReadings; readIndex1++){
      total1 = total1 - readings1[readIndex1];
      readings1[readIndex1] = mpr.readPressure();
      total1 = total1 + readings1[readIndex1];
  }
  SensorZero = total1 / numReadings;
  */
}

// Main code that runs as a state machine
void loop() {
  int currTime = millis();
  
  if (!initializeComplete) {
    // Selection of operating mode - defaults to Autosparge 1.0
    lcd.setBacklight(GREEN);
    lcd.setCursor(0,0);
    lcd.print("Select for 2.0");
    lcd.setCursor(0,1);
    lcd.print("Left for TEST");

    // Check if "Select" button is pressed
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      if (buttons & BUTTON_SELECT) {
        initializeComplete = true;
        mode = V2_MODE;
        lcd.clear();
        return;
      }
      if (buttons & BUTTON_LEFT) {
        initializeComplete = true;
        mode = TEST_MODE;
        lcd.clear();
        return;
      }
    }

    // Our countdown is over
    if (currTime > endInitTime) {
      initializeComplete = true;
      mode = V1_MODE;
      lcd.clear();
      return;
    }
    int currTimeDisplay = (endInitTime - currTime)/1000;
    
    lcd.setCursor(15,1);
    lcd.print(currTimeDisplay);
    return;    
  }

  // Use legacy Version 1.0
  if (mode == V1_MODE) {
    lcd.setBacklight(VIOLET);
    lcd.setCursor(0, 0);
    lcd.print("Using V1.0");
  } else if (mode == V2_MODE) {
    lcd.setBacklight(BLUE);
    lcd.setCursor(0, 0);
    lcd.print("Using V2.0");
  } else if (mode == TEST_MODE) {
    lcd.setBacklight(RED);
    lcd.setCursor(0, 0);
    lcd.print("TEST Mode");
  }

}

/* ORIGINAL CODE
// Define Pins
#define TRINKET_BOARD_LED_PIN 13
#define ALARM_PIN 8 //8
#define BUZZER_PIN 9 //11
#define LEFT_BUTTON_PIN 4 //4
#define LEFT_BUTTON_LIGHT_PIN 6 //6
#define RIGHT_BUTTON_PIN 3 //3
#define RIGHT_BUTTON_LIGHT_PIN 5 //5
#define MASH_PROBE_PIN 16 //16
#define MASH_PROBE_HIGH_PIN 17 //17
#define BOIL_PROBE_PIN 12 //15
#define WATER_PUMP_PIN 10 //13
#define WORT_PUMP_PIN 11 //12

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

WaterPump WaterPump(WATER_PUMP_PIN, 10000, &AlarmEventQueue, &MashProbe, &MashProbeHigh);
WortPump WortPump(WORT_PUMP_PIN, 2000, &AlarmEventQueue, &BoilProbe);

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
  } else {
    // NORMAL - Update objects
    LeftButton.Update(currentMillis);
    RightButton.Update(currentMillis);
  
    // Set pump active based on their buttons
    WaterPump.SetIsActive(LeftButton.GetMatchingFunctionOn());
    WortPump.SetIsActive(RightButton.GetMatchingFunctionOn());
  
    MashProbe.Update(currentMillis);
    MashProbeHigh.Update(currentMillis);
    BoilProbe.Update(currentMillis);
  
    WaterPump.Update(currentMillis);
    WortPump.Update(currentMillis);

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

    // If Mash Probe High then Trinket LED
    if (digitalRead(MASH_PROBE_HIGH_PIN) == HIGH) {
      digitalWrite(TRINKET_BOARD_LED_PIN, HIGH);
    } else {
      digitalWrite(TRINKET_BOARD_LED_PIN, LOW);
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
    }
}
*/
