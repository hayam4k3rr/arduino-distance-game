#include "Arduino.h"

// --- Configuration ---
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const int BUZZER_PIN = 3;
const int PIN_GREEN = 5;
const int PIN_YELLOW = 6;
const int PIN_RED = 7;

// --- Game Settings ---
const int MIN_DIST = 10;   
const int MAX_DIST = 30;   
const int TOLERANCE = 4;     // +/- 4cm sweet spot
const int WIN_TIME = 5000;   // Hold Green for 5s to Win
const int DEATH_TIME = 5000; // Wander for 5s and you Die
const int DRIFT_SPEED = 400; 

// --- Notes (Hz) ---
#define NOTE_C4  262
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_G4  392

// --- Variables ---
int secretDistance;
int smoothedDistance;    
bool gameWon = false;

// Timers
unsigned long lastSensorRead = 0;
unsigned long timeEnteredSweetSpot = 0;
unsigned long lastDriftTime = 0;
unsigned long lastHappyTime = 0; 
bool inSweetSpot = false; 

// Feedback Timers
unsigned long lastBeep = 0;
unsigned long lastStressBlink = 0;
bool buzzerState = false;
bool ledState = false;

void setup() {
  Serial.begin(9600);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  randomSeed(analogRead(0)); 
  
  playMarioStartup(); 
  
  Serial.println("... SURVIVE ...");
  delay(1000);
  
  resetGame();
}

void loop() {
  if (gameWon) return; 

  if (millis() - lastSensorRead > 50) {
    lastSensorRead = millis();
    smoothedDistance = getFilteredDistance();
    checkGameStatus();
  }

  handleFeedback();
}

// ---------------------------------------------------------
// GAME LOGIC
// ---------------------------------------------------------

void checkGameStatus() {
  int diff = abs(smoothedDistance - secretDistance);

  // --- CASE 1: SAFE ZONE ---
  if (diff <= TOLERANCE) {
    lastHappyTime = millis(); // Reset Death Timer
    
    if (!inSweetSpot) {
      inSweetSpot = true;
      timeEnteredSweetSpot = millis(); 
      lastDriftTime = millis();
      Serial.println(">>> LOCKED ON! (Safe) <<<");
    }
    
    // Drift Mechanic
    if (millis() - lastDriftTime > DRIFT_SPEED) {
      lastDriftTime = millis();
      int driftDirection = random(0, 2) == 0 ? 1 : -1;
      secretDistance += driftDirection;
      if (secretDistance < MIN_DIST) secretDistance = MIN_DIST;
      if (secretDistance > MAX_DIST) secretDistance = MAX_DIST;
    }

    // Win Check
    if (millis() - timeEnteredSweetSpot >= WIN_TIME) {
      triggerVictory();
    }
  } 
  
  // --- CASE 2: DANGER ZONE ---
  else {
    inSweetSpot = false;
    
    unsigned long timeLost = millis() - lastHappyTime;
    
    if (timeLost > 1000 && timeLost % 1000 < 50) {
      Serial.print("WARNING: "); 
      Serial.print((DEATH_TIME - timeLost)/1000);
      Serial.println("s until failure!");
    }

    if (timeLost >= DEATH_TIME) {
      triggerFailure();
    }
  }
}

void handleFeedback() {
  if (inSweetSpot) {
    unsigned long timeHeld = millis() - timeEnteredSweetSpot;
    int stressDelay = map(timeHeld, 0, WIN_TIME, 500, 50);
    
    if (millis() - lastStressBlink >= stressDelay) {
      lastStressBlink = millis();
      ledState = !ledState;
      if (ledState) {
        digitalWrite(PIN_GREEN, HIGH);
        tone(BUZZER_PIN, 800, 20); 
      } else {
        digitalWrite(PIN_GREEN, LOW);
      }
    }
    digitalWrite(PIN_YELLOW, LOW); digitalWrite(PIN_RED, LOW);
  } 
  else {
    int diff = abs(smoothedDistance - secretDistance);
    digitalWrite(PIN_GREEN, LOW); 
    
    if (diff <= 15) {
      digitalWrite(PIN_YELLOW, HIGH); digitalWrite(PIN_RED, LOW);
    } else {
      digitalWrite(PIN_YELLOW, LOW); digitalWrite(PIN_RED, HIGH);
    }

    int beepInterval = (diff > 25) ? 600 : (diff > 15) ? 300 : 100;

    if (millis() - lastBeep >= beepInterval) {
      lastBeep = millis();
      buzzerState = !buzzerState;
      if (buzzerState) tone(BUZZER_PIN, 200, 30); 
      else noTone(BUZZER_PIN);
    }
  }
}

// ---------------------------------------------------------
// AUDIO & UTILS
// ---------------------------------------------------------

void playMarioStartup() {
  Serial.println("--- MARIO START ---");
  int melody[] = { NOTE_E5, NOTE_E5, NOTE_E5, NOTE_C5, NOTE_E5, NOTE_G5, NOTE_G4 };
  int durations[] = { 80, 80, 80, 80, 80, 150, 150 };
  int pauses[] = { 100, 150, 150, 100, 150, 150, 0 };

  for (int i = 0; i < 7; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    if (i % 2 == 0) digitalWrite(PIN_GREEN, HIGH);
    else digitalWrite(PIN_YELLOW, HIGH);
    delay(durations[i]);
    digitalWrite(PIN_GREEN, LOW); digitalWrite(PIN_YELLOW, LOW);
    delay(pauses[i]);
  }
}

void triggerVictory() {
  gameWon = true;
  noTone(BUZZER_PIN);
  Serial.println("VICTORY!");

  int melody[] = {523, 523, 523, 523, 415, 466, 523, 0, 466, 523};
  int duration[] = {100, 100, 100, 300, 300, 300, 300, 100, 100, 600};

  for (int i = 0; i < 10; i++) {
     if (i % 2 == 0) { digitalWrite(PIN_GREEN, HIGH); digitalWrite(PIN_RED, LOW); }
     else { digitalWrite(PIN_GREEN, LOW); digitalWrite(PIN_RED, HIGH); }
     tone(BUZZER_PIN, melody[i], duration[i]);
     delay(duration[i] * 1.3);
  }
  digitalWrite(PIN_GREEN, HIGH); digitalWrite(PIN_YELLOW, HIGH); digitalWrite(PIN_RED, HIGH);
  Serial.println("Press RESET.");
}

void triggerFailure() {
  gameWon = true;
  noTone(BUZZER_PIN);
  Serial.println("GAME OVER");

  // Mario Death Melody (Clean & Sharp)
  // B4 -> F5 -> F5 -> F5 -> E5 -> D5 -> C5
  int melody[] = { 494, 698, 698, 698, 659, 587, 523 };
  int durations[] = { 150, 150, 150, 150, 150, 150, 300 };

  for (int i = 0; i < 7; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    digitalWrite(PIN_RED, HIGH); 
    delay(durations[i]);
    digitalWrite(PIN_RED, LOW);
    delay(50);
  }

  // --- SILENCE & FREEZE ---
  noTone(BUZZER_PIN); // Kill sound instantly
  digitalWrite(PIN_RED, HIGH); // Leave Red Light ON to shame the player
  
  Serial.println("Press RESET to try again.");
  while(true); // Freeze execution here
}

void resetGame() {
  gameWon = false;
  inSweetSpot = false;
  secretDistance = random(MIN_DIST, MAX_DIST);
  
  lastHappyTime = millis(); 
  
  Serial.println("--- NEW TARGET SET ---");
  Serial.print("Target: "); Serial.print(secretDistance); Serial.println("cm");
}

int getFilteredDistance() {
  long total = 0;
  int validReadings = 0;
  for (int i = 0; i < 3; i++) {
    digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long d = pulseIn(ECHO_PIN, HIGH, 15000); 
    if (d > 0) { total += d; validReadings++; }
    delay(4); 
  }
  if (validReadings == 0) return smoothedDistance;
  int avg = (total / validReadings) * 0.034 / 2;
  return (0.7 * smoothedDistance) + (0.3 * avg);
}