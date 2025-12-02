/* FarmBot minimal Arduino sketch
   - Serial G-code receiver (simple)
   - Capteurs C0/C1/C2 -> incrément/décrément counters
   - Moisture loop every 2 hours -> measure and water if <30%
   Pins mapping: adjust to your wiring
*/

#include <Arduino.h>

#define PIN_C0 2   // digital input for sensor X
#define PIN_C1 3   // Y
#define PIN_C2 4   // Z

#define PIN_VACUUM 8    // vacuum pump control (M3/M5)
#define PIN_WATER_PUMP 9 // water pump control
#define PIN_MOISTURE A0   // moisture analog input (for demo; one sensor per case assumed)

volatile long counterC0 = 0;
volatile long counterC1 = 0;
volatile long counterC2 = 0;

String recvLine = "";
float curX = 0, curY = 0, curZ = 0;

// Moisture loop timing
unsigned long lastMoistureMillis = 0;
const unsigned long moistureInterval = 2UL * 60UL * 60UL * 1000UL; // 2 hours in ms
// for debug shorten to 2*60*1000UL for 2 minutes while testing

// Example grid (should be filled by user/uploaded via serial or compiled in)
// For demo we define a small list of planted centers (x_mm,y_mm).
struct CasePos { float x; float y; int plantIndex; };
CasePos plantedCases[] = {
  {  66.6,  75.0, 0 }, // example coordinates (mm)
  { 200.0, 100.0, 1 }
};
const int plantedCount = sizeof(plantedCases)/sizeof(plantedCases[0]);

// simple G-code parse helpers
void executeGcodeLine(const String &line);

// interrupts for sensors (example: count rising edges)
void isrC0(){ counterC0++; }
void isrC1(){ counterC1++; }
void isrC2(){ counterC2++; }

void setup() {
  Serial.begin(115200);
  pinMode(PIN_C0, INPUT_PULLUP);
  pinMode(PIN_C1, INPUT_PULLUP);
  pinMode(PIN_C2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_C0), isrC0, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_C1), isrC1, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_C2), isrC2, RISING);

  pinMode(PIN_VACUUM, OUTPUT); digitalWrite(PIN_VACUUM, LOW);
  pinMode(PIN_WATER_PUMP, OUTPUT); digitalWrite(PIN_WATER_PUMP, LOW);
  pinMode(PIN_MOISTURE, INPUT);

  lastMoistureMillis = millis();
  Serial.println("Arduino FarmBot minimal ready.");
}

void loop() {
  // 1) Serial line reception (G-code)
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (recvLine.length() > 0) {
        Serial.print("RX: "); Serial.println(recvLine);
        executeGcodeLine(recvLine);
        recvLine = "";
      }
    } else {
      recvLine += c;
    }
  }

  // 2) Moisture loop
  unsigned long now = millis();
  if (now - lastMoistureMillis >= moistureInterval) {
    lastMoistureMillis = now;
    Serial.println("Moisture loop start...");
    for (int idx = 0; idx < plantedCount; idx++) {
      // Move to center of case
      moveTo(plantedCases[idx].x, plantedCases[idx].y, 0);
      delay(300);
      // descend sensor (simulate)
      moveTo(curX, curY, -20); // lower sensor
      delay(200);
      int raw = analogRead(PIN_MOISTURE);
      float percent = map(raw, 0, 1023, 0, 100);
      Serial.print("Moisture at case "); Serial.print(idx); Serial.print(" = "); Serial.print(percent); Serial.println("%");
      if (percent < 30.0) {
        Serial.println("Humidité < 30% -> arrosage 20s");
        // activate pump (may require relay driver)
        digitalWrite(PIN_WATER_PUMP, HIGH);
        delay(20000);
        digitalWrite(PIN_WATER_PUMP, LOW);
      }
      moveTo(curX, curY, 0); // up
    }
    Serial.println("Moisture loop end.");
  }

  // 3) Could add other background tasks here
}

// Simplified moveTo: replace with stepper control
void moveTo(float x, float y, float z) {
  Serial.print("Moving to X"); Serial.print(x); Serial.print(" Y"); Serial.print(y); Serial.print(" Z"); Serial.println(z);
  // In a real system you will command steppers and wait for movement + sensor readings
  curX = x; curY = y; curZ = z;
  // The spec says: sequence starts only when sensor counters correspond to expected position.
  // Here we simulate: wait until counters equal some expected values (not implemented automatically).
  // Example: after a move to center, you could wait for C0/C1/C2 increments to match targets.
}

// Simple G-code handler: handles G0/G1 X.. Y.. Z.., M3 (vacuum on), M5 (vacuum off), G4 P(ms) dwell
void executeGcodeLine(const String &line) {
  String s = line;
  s.trim();
  if (s.length() == 0) return;
  if (s[0] == ';') return; // comment
  // tokenise
  char cmd = s.charAt(0);
  if (cmd == 'G' || cmd == 'g') {
    int gnum = s.substring(1).toInt();
    if (gnum == 0 || gnum == 1) {
      // find X,Y,Z
      float X=curX, Y=curY, Z=curZ;
      // parse tokens
      int idx = 1;
      while (idx < s.length()) {
        char c = s.charAt(idx);
        if (c == 'X' || c=='x') {
          int j = idx+1; while (j < s.length() && (isDigit(s[j])||s[j]=='.'||s[j]=='-'||s[j]=='+')) j++;
          X = s.substring(idx+1, j).toFloat();
          idx = j;
        } else if (c == 'Y' || c=='y') {
          int j = idx+1; while (j < s.length() && (isDigit(s[j])||s[j]=='.'||s[j]=='-'||s[j]=='+')) j++;
          Y = s.substring(idx+1, j).toFloat();
          idx = j;
        } else if (c == 'Z' || c=='z') {
          int j = idx+1; while (j < s.length() && (isDigit(s[j])||s[j]=='.'||s[j]=='-'||s[j]=='+')) j++;
          Z = s.substring(idx+1, j).toFloat();
          idx = j;
        } else idx++;
      }
      moveTo(X,Y,Z);
    } else if (gnum == 4) {
      // dwell: look for P value (ms)
      int ppos = s.indexOf('P');
      if (ppos >= 0) {
        int ms = s.substring(ppos+1).toInt();
        Serial.print("Dwell for "); Serial.print(ms); Serial.println(" ms");
        delay(ms);
      }
    }
  } else if (cmd == 'M' || cmd == 'm') {
    int mnum = s.substring(1).toInt();
    if (mnum == 3) {
      Serial.println("M3: Vacuum ON");
      digitalWrite(PIN_VACUUM, HIGH);
    } else if (mnum == 5) {
      Serial.println("M5: Vacuum OFF");
      digitalWrite(PIN_VACUUM, LOW);
    }
  } else {
    // unknown
    Serial.print("Unknown command: "); Serial.println(s);
  }
}
