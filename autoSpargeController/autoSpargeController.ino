#include "Beeper.h"
#include "Button.h"
#include "EventQueue.h"
#include "Probe.h"
#include "WaterPump.h"
#include "WortPump.h"

/*
 * AUTOSPARGE CONTROLLER
 * version 1.0
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

