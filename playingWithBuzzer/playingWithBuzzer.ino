#define BUZZER 0
#define BUZZER_SOUND HIGH
#define BUZZER_SILENT LOW
#define MAINT_MODE_BEEP_INTERVAL 3000
#define MAINT_MODE_BUTTON 4

long MaintModeBeepMillis = 0;
int BuzzerState = BUZZER_SILENT;


// put your setup code here, to run once:
void setup() {
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  // Get current clock
  unsigned long currentMillis = millis();

  if ((BuzzerState == BUZZER_SILENT) && (currentMillis - MaintModeBeepMillis >= 4950)) { 
    MaintModeBeepMillis = currentMillis;

    BuzzerState = BUZZER_SOUND;
    digitalWrite(BUZZER, BuzzerState);
  } else if ((BuzzerState == BUZZER_SOUND) && (currentMillis - MaintModeBeepMillis >= 50)) { 
    MaintModeBeepMillis = currentMillis;

    BuzzerState = BUZZER_SILENT;
    digitalWrite(BUZZER, BuzzerState);
  }
  /*
  if (currentMillis - MaintModeBeepMillis > MAINT_MODE_BEEP_INTERVAL) {
    MaintModeBeepMillis = currentMillis;
    if (BuzzerState == BUZZER_SILENT)
      BuzzerState = BUZZER_SOUND;
    else
      BuzzerState = BUZZER_SILENT;

    digitalWrite(BUZZER, BuzzerState);

         
  }*/

}
