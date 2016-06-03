
#define BUZZER 0
#define LED 1
#define BUTTON 3

void setup() {
  // put your setup code here, to run once:
  pinMode(BUZZER, OUTPUT);  // HIGH is buzzer on 
  pinMode(LED, OUTPUT);
  
  pinMode(BUTTON, INPUT);  // Button closer to power cord on bread board
  digitalWrite(BUTTON, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Will check to see if the BUTTON is depressed (LOW)
  // If so, sound the buzzer and turn the light on
  if (digitalRead(BUTTON) == LOW) {
    digitalWrite(BUZZER, HIGH);
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);
    digitalWrite(LED, LOW);
  }
}
