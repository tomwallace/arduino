#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#include "Beeper.h"
#include "Button.h"
#include "EventQueue.h"
#include "PressureSensor.h"
#include "Probe.h"
#include "WaterPump.h"
#include "WortPump.h"

#include "IMenu.h"
#include "CurrentDataMenu.h"
#include "SetBoilDisplayUnitsMenu.h"
#include "SetBoilStopOneMenu.h"
#include "SetBoilStopTwoMenu.h"
#include "ToggleBoilStopMenu.h"

/* AUTOSPARGE CONTROLLER
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
// readStatus
// Define Pins
#define RESET_PIN -1  // set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN -1  // set to any GPIO pin to read end-of-conversion by pin
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

// Define Colors
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

EventQueue AlarmEventQueue("AlarmEventQueue");
EventQueue BuzzerEventQueue("BuzzerEventQueue");

Beeper Alarm("Alarm", ALARM_PIN, &AlarmEventQueue);
Beeper Buzzer("Buzzer", BUZZER_PIN, &BuzzerEventQueue);

Button LeftButton("Left Button", LEFT_BUTTON_PIN, INPUT_PULLUP, LEFT_BUTTON_LIGHT_PIN, &BuzzerEventQueue);
Button RightButton("Right Button", RIGHT_BUTTON_PIN, INPUT_PULLUP, RIGHT_BUTTON_LIGHT_PIN, &BuzzerEventQueue);

Probe MashProbe("Mash Probe", MASH_PROBE_PIN, INPUT);
Probe MashProbeHigh("Mash Probe High", MASH_PROBE_HIGH_PIN, INPUT);
Probe BoilProbe("Boil Probe", BOIL_PROBE_PIN, INPUT);

PressureSensor BoilPressureSensor(&mpr);

WaterPump WaterPump(WATER_PUMP_PIN, 10000, &AlarmEventQueue, &MashProbe, &MashProbeHigh);
WortPump WortPump(WORT_PUMP_PIN, 2000, &AlarmEventQueue, &BoilProbe);

// Global variables
bool initializeComplete = false;
int mode = V1_MODE;  // Default to existing behavior
int startTime = 0;  
int endInitTime = 0;
float boilStopOne = 4;  // Provides default for pause boil to turn off sparge
float boilStopTwo = 7.5;  // Provide default for complete boil stop level
bool atBoilStopOne = true;  // Indicates if we are at the first stop in the boil
bool boilShowGallons = true;  // Provide default for display in gallons

// Menu control variables
CurrentDataMenu CurrentDataMenu(&BoilPressureSensor, &lcd);
ToggleBoilStopMenu ToggleBoilStopMenu(&lcd);
SetBoilStopOneMenu SetBoilStopOneMenu(&lcd);
SetBoilStopTwoMenu SetBoilStopTwoMenu(&lcd);
SetBoilDisplayUnitsMenu SetBoilDisplayUnitsMenu(&lcd);
IMenu * menuItems[] = {&CurrentDataMenu, &ToggleBoilStopMenu, &SetBoilStopOneMenu, &SetBoilStopTwoMenu, &SetBoilDisplayUnitsMenu};

int menuPage = 0;
int sizeOfMenuItems = 5;
int maxMenuPages = 3;
int cursorPosition = 0;
int selectedMenu = 0;
byte upArrow[8] = {0x04,0x0E,0x1F,0x04,0x04,0x04,0x04,0x00};
byte downArrow[8] = {0x04,0x04,0x04,0x04,0x1F,0x0E,0x04,0x00};
byte menuCursor[8] = {0x10,0x08,0x04,0x02,0x04,0x08,0x10,0x00};

// Setup code only runs once
void setup() {
  pinMode(TRINKET_BOARD_LED_PIN, OUTPUT);
  // Set up serial port for output at 9600 bps
  Serial.begin(9600);

  mpr.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(BLUE);
  
  displayLugWrenchWelcomeMessage();
  
  startTime = millis();
  endInitTime = 9000 + startTime;
  
  // Menu items
  lcd.createChar(0, menuCursor); // Create the custom arrow characters in void setup for global use
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);  
}

// Main code that runs as a state machine
void loop() {
  // Get current clock
  unsigned long currentMillis = millis();
  
  if (!initializeComplete) {
    Initialize(currentMillis);
    
    // Provide V2 override for pressure sensor probe in WortPump - this overload is what allows the pressure sensor to be used
    if (mode == V2_MODE) {
      WortPump.SetProbe(&BoilPressureSensor);
    }
    return;
  }

  // Use legacy Version 1.0
  if (mode == V1_MODE) {
    lcd.setBacklight(VIOLET);
    lcd.setCursor(0, 0);
    lcd.print("Using V1.0");

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

  // Use new Version 2.0 code
  } else if (mode == V2_MODE) {
    lcd.setBacklight(GREEN);

    menu();

    LeftButton.Update(currentMillis);
    RightButton.Update(currentMillis);
  
    // Set pump active based on their buttons
    WaterPump.SetIsActive(LeftButton.GetMatchingFunctionOn());
    WortPump.SetIsActive(RightButton.GetMatchingFunctionOn());
  
    MashProbe.Update(currentMillis);
    MashProbeHigh.Update(currentMillis);

    BoilPressureSensor.Update(currentMillis);
  
    WaterPump.Update(currentMillis);
    WortPump.Update(currentMillis);

    Alarm.Update(currentMillis);
    Buzzer.Update(currentMillis);

  // Work in TEST mode
  } else if (mode == TEST_MODE) {
    TestInteractions();
  }
}

// Initialize with button options to determine running mode
void Initialize(long currentMillis) {
  lcd.setBacklight(GREEN);
  lcd.setCursor(0,0);
  lcd.print("Select for 2.0");
  lcd.setCursor(0,1);
  lcd.print("Left for TEST");

  // Check if "Select" button is pressed
  int button = evaluateButton();
  if (button == 5) {
    initializeComplete = true;
    mode = V2_MODE;
    lcd.clear();
    return;
  }
  // Check to see if the "Left" button is pressed
  if (button == 4) {
    initializeComplete = true;
    mode = TEST_MODE;
    lcd.clear();
    return;
  }

  // Our countdown is over - default to Auto Sparge 1.0
  if (currentMillis > endInitTime) {
    initializeComplete = true;
    mode = V1_MODE;
    lcd.clear();
    return;
  }
  int currTimeDisplay = (endInitTime - currentMillis)/1000;
  
  lcd.setCursor(15,1);
  lcd.print(currTimeDisplay);
}

// Version 1.0 TEST Mode
void TestInteractions() {
  lcd.setBacklight(RED);
  lcd.setCursor(0, 0);
  lcd.print("TEST Mode v1.0");
    
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

// Display LugWrench Welcome Message on start up
void displayLugWrenchWelcomeMessage() {
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
}

// Base function for interacting with the menus
void menu() {
  int button = evaluateButton();

  if (selectedMenu == 0)
     mainMenu(button);
  else
     menuItems[selectedMenu - 1]->Interact(button);
}

// Interaction with the main menu listing
void mainMenu(int button) {
  // Draw
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]->GetName());
  //lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]->GetName());
  //lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages) {
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages) {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }

  drawCursor();

  // Interact
  switch (button) {
    case 0: // When button returns as 0 there is no action taken
      return;
    case 1:  // This case will execute if the "forward" button is pressed
      lcd.clear();
      selectedMenu = cursorPosition + 1; // The case that is selected here is dependent on which menu page you are on and where the cursor is.
      return;

      break;
    case 2:
      lcd.clear();
      if (menuPage == 0) {
        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, sizeOfMenuItems - 1);
      }
 
      if (menuPage % 2 == 0 and cursorPosition % 2 == 0) {
        menuPage = menuPage - 1;
        menuPage = constrain(menuPage, 0, maxMenuPages);
      }

      if (menuPage % 2 != 0 and cursorPosition % 2 != 0) {
        menuPage = menuPage - 1;
        menuPage = constrain(menuPage, 0, maxMenuPages);
      }

      cursorPosition = cursorPosition - 1;
      cursorPosition = constrain(cursorPosition, 0, sizeOfMenuItems - 1);
      break;
    case 3:
      lcd.clear();
      if (menuPage % 2 == 0 and cursorPosition % 2 != 0) {
        menuPage = menuPage + 1;
        menuPage = constrain(menuPage, 0, maxMenuPages);
      }

      if (menuPage % 2 != 0 and cursorPosition % 2 == 0) {
        menuPage = menuPage + 1;
        menuPage = constrain(menuPage, 0, maxMenuPages);
      }

      cursorPosition = cursorPosition + 1;
      cursorPosition = constrain(cursorPosition, 0, sizeOfMenuItems - 1);
      break;
  }
}

// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor() {
  for (int x = 0; x < 2; x++) {  // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }

  // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
  // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}

// This function monitors for button presses to know which button was pressed.
int evaluateButton() {
  uint8_t buttons = lcd.readButtons();
  int result = 0;
  if (buttons & BUTTON_RIGHT) {
    result = 1; // right
  } else if (buttons & BUTTON_UP) {
    result = 2; // up
  } else if (buttons & BUTTON_DOWN) {
    result = 3; // down
  } else if (buttons & BUTTON_LEFT) {
    result = 4; // left
  } else if (buttons & BUTTON_SELECT) { // currently unused work in progress
    result = 5; // left
  }
  return result;
}
