/*
 * This sketch will pulse the wort pump at the defined interval
 * so long as the probe does not encounter liquid.  If the probe 
 * encounters liquid, then the pulse stops and an alarm sounds. When
 * the probe is not touching liquid, then restart the pulse.
 */

#define BUZZER 0
#define WORT_PUMP 1
#define PROBE 3  // TODO: Change to PIN 2 when can get it working properly
#define PULSE_ON 2000  // Milliseconds to turn wort pump on out of total inteval
#define PULSE_TOTAL_INTERVAL 60000 // Milliseconds of total interval before resetting clock

int milliSecs = 0;

// put your setup code here, to run once:
void setup() {
  pinMode(BUZZER, OUTPUT);  // HIGH is buzzer on 
  pinMode(WORT_PUMP, OUTPUT); // HIGH is pump on

  pinMode(PROBE, INPUT_PULLUP);
  //digitalWrite(PROBE, HIGH);  // TODO: Remove when get BUTTON 2 working, as John says not needed
}

// put your main code here, to run repeatedly:
void loop() {
  // Check if probe encountered liquid.  If so, sound alarm and reset timer
  if (digitalRead(PROBE) == LOW) {
    digitalWrite(BUZZER, HIGH);
    milliSecs = 0;
  } else {
    digitalWrite(BUZZER, LOW);

    // Run pulse if less than time in interval
    if (milliSecs <= PULSE_ON) {
      digitalWrite(WORT_PUMP, HIGH);
    } else {
      digitalWrite(WORT_PUMP, LOW);
    }
  }

  // Increment our timer
  milliSecs = milliSecs + 1000;
  delay(1000);
   
  // Check for total interval
  if (milliSecs >= PULSE_TOTAL_INTERVAL) {
    milliSecs = 0;
  }
}
