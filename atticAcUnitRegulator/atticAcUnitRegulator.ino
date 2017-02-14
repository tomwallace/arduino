/*
 * Sketch to regulate John Baker's attic AC unit 
 * 
 */

// John this is your divider variable
#define JOHN_DIVISOR 10


// Other constants
#define BUTTON 0
#define TEMP_SENSOR 1
#define LIGHT 1

#define INTERVAL_FLASH_NORMAL 500
#define INTERVAL_FLASH_DOUBLE 100
#define INTERVAL_INDICATE_ANSWER_COMING 2000
#define INTERVAL_INDICATE_DONE 1000
#define INTERVAL_MIN_NEXT_CLICK_REGISTER 2000

// Calculate the absolute value of a number, as we don't have access to the Math library
long AbsoluteValue(long num) {
  if (num >= 0)
    return num;

  return num * -1;
}

// Creates the timing for the results pulses
long SetResultsTimeStartNextPulse(long currentMillis, int resultsNumPulses) {
  // Pause before double flash
  if (resultsNumPulses == 2) {
    return currentMillis + INTERVAL_INDICATE_ANSWER_COMING;
  }

  // Set up the double flash to signal end
  if (resultsNumPulses < 2) {
    return currentMillis + INTERVAL_FLASH_DOUBLE;
  } 

  // Return a normal answer pulse
  return currentMillis + INTERVAL_FLASH_NORMAL; 
}

bool InResultsCycle = false;
long FirstTempMeasurement = 0;
int ResultsNumPulses = 0;
long TimeStartNextPulse = 0;
long TimeNextButtonClickAllowed = 0;
int LEDonTime = INTERVAL_FLASH_NORMAL;

void setup() {
  // put your setup code here, to run once:
  pinMode(TEMP_SENSOR, INPUT);
  pinMode(LIGHT, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  // Set up serial port for output at 9600 bps
//  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Get current clock
  unsigned long currentMillis = millis();
  
  // In cycle to display the results
  if (InResultsCycle == true) {
    // If no more pulses, then out of results cycle
    if (ResultsNumPulses == 0) {
      InResultsCycle = false;
      TimeStartNextPulse = 0;
   //   Serial.println("Set InResultsCycle = false");
    } else {
      //Set flash on time
      if (ResultsNumPulses <= 2) {
        LEDonTime = INTERVAL_FLASH_DOUBLE;
      } else {
        LEDonTime = INTERVAL_FLASH_NORMAL;
      }
      // Process pulses
      if (currentMillis >= TimeStartNextPulse && currentMillis <= (TimeStartNextPulse + LEDonTime)) {
        digitalWrite(LIGHT, HIGH);
      } else {
        digitalWrite(LIGHT, LOW);
      }

      if (currentMillis > (TimeStartNextPulse + LEDonTime)) {
        --ResultsNumPulses;
     //   Serial.println("Set ResultsNumPulses = " + String(ResultsNumPulses));

        TimeStartNextPulse = SetResultsTimeStartNextPulse(currentMillis, ResultsNumPulses); 
      //  Serial.println("Set TimeStartNextPulse = " + String(TimeStartNextPulse));
      }
    }

  // If we are not 
  } else { 
    if ((digitalRead(BUTTON) == LOW) && (currentMillis > TimeNextButtonClickAllowed)) {
        // Prevent button double click
        TimeNextButtonClickAllowed = currentMillis + INTERVAL_MIN_NEXT_CLICK_REGISTER;
        
        // If no FirstTempMeasurement, then this is the first button press
        if (FirstTempMeasurement == 0) {
          FirstTempMeasurement = analogRead(TEMP_SENSOR);
       //   Serial.println("Setting FirstTempMeasurement = " + String(FirstTempMeasurement));
        
        // Second button push
        } else {
          InResultsCycle = true;
  
          long SecondTempMeasurement = analogRead(TEMP_SENSOR);
         // Serial.println("Setting SecondTempMeasurement = " + String(SecondTempMeasurement));
          
          long Difference = AbsoluteValue(FirstTempMeasurement - SecondTempMeasurement);
          // Cast as long to truncate - don't have access to Math library
          long TruncatedRatio = (long)(Difference/JOHN_DIVISOR);
       //   Serial.println("TruncatedRation = " + String(TruncatedRatio));
          
          // Account for final double pulse
          ResultsNumPulses = TruncatedRatio + 2;
          // Set time to start first result pulse
          TimeStartNextPulse = currentMillis + INTERVAL_INDICATE_ANSWER_COMING;

          // Reset first temp measurement for next time
          FirstTempMeasurement = 0;
      }
    }
  }
}

