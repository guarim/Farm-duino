#include <Arduino.h>

// --- CONFIGURATION PIN ---
// Capteurs (Fins de course / Plots)
const int PIN_SENSOR_X = 2; // Interruption obligatoire
const int PIN_SENSOR_Y = 3; // Interruption obligatoire
const int PIN_SENSOR_Z = 18; // Mega 2560 (ou autre pin int)

// Moteurs (Drivers type L298N ou Step/Dir simplifiés)
// Ici logique DIR/PWM pour moteurs DC avec encodeur plot
const int MOT_X_DIR = 22;
const int MOT_X_PWM = 23;
const int MOT_Y_DIR = 24;
const int MOT_Y_PWM = 25;
const int MOT_Z_DIR = 26;
const int MOT_Z_PWM = 27;

// Outils
const int PIN_PUMP = 40;
const int PIN_WATER = 41;
const int PIN_MOISTURE = A0;

// --- VARIABLES GLOBALES (VOLATILE pour Interrupts) ---
volatile int posX = 0; 
volatile int posY = 0;
volatile int posZ = 0; // 0 = Haut, 1, 2, 3 = Bas

// Cibles
int targetX = 0;
int targetY = 0;
int targetZ = 0;

// États système
bool isMoving = false;
String inputString = "";
boolean stringComplete = false;

// --- INTERRUPTIONS (Comptage Plots) ---
void isr_countX() {
  // Simple logique : si DIR est HIGH on incrémente, sinon décrémente
  // Debounce matériel recommandé (condensateur), sinon debounce logiciel ici :
  static unsigned long last_intr_time = 0;
  unsigned long intr_time = millis();
  if (intr_time - last_intr_time > 200) { // 200ms min entre plots
    if (digitalRead(MOT_X_DIR) == HIGH) posX++; else posX--;
  }
  last_intr_time = intr_time;
}

void isr_countY() {
  static unsigned long last_intr_time = 0;
  unsigned long intr_time = millis();
  if (intr_time - last_intr_time > 200) { 
    if (digitalRead(MOT_Y_DIR) == HIGH) posY++; else posY--;
  }
  last_intr_time = intr_time;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_SENSOR_X, INPUT_PULLUP);
  pinMode(PIN_SENSOR_Y, INPUT_PULLUP);
  
  // Attacher interruptions (FALLING = détection plot métal/trou)
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_X), isr_countX, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_Y), isr_countY, FALLING);

  pinMode(MOT_X_DIR, OUTPUT); pinMode(MOT_X_PWM, OUTPUT);
  pinMode(MOT_Y_DIR, OUTPUT); pinMode(MOT_Y_PWM, OUTPUT);
  pinMode(MOT_Z_DIR, OUTPUT); pinMode(MOT_Z_PWM, OUTPUT);
  
  pinMode(PIN_PUMP, OUTPUT);
  pinMode(PIN_WATER, OUTPUT);
  
  Serial.println("FarmBot Ready. Waiting G-Code...");
}

// --- LOGIQUE DE MOUVEMENT ---
void updateMotor(int current, int target, int pinDir, int pinPwm) {
  if (current == target) {
    analogWrite(pinPwm, 0); // Stop
  } else {
    // Sens
    digitalWrite(pinDir, (target > current) ? HIGH : LOW);
    // Vitesse (Peut être réduite si |target-current| == 1 pour précision)
    analogWrite(pinPwm, 200); 
  }
}

void loop() {
  // 1. Parsing G-Code
  if (stringComplete) {
    parseGCode(inputString);
    inputString = "";
    stringComplete = false;
  }

  // 2. Asservissement Position (Boucle principale)
  // Note: C'est un asservissement "Bang-Bang" simple pour les plots
  if (posX != targetX || posY != targetY) {
    updateMotor(posX, targetX, MOT_X_DIR, MOT_X_PWM);
    updateMotor(posY, targetY, MOT_Y_DIR, MOT_Y_PWM);
    isMoving = true;
  } else {
    if (isMoving) {
      // On vient de s'arrêter
      Serial.println("OK"); // Acquittement pour l'interface web
      isMoving = false;
    }
  }
  
  // (Logique Z à implémenter de façon similaire ou au temps)
}

// --- PARSER SIMPLIFIÉ ---
void parseGCode(String cmd) {
  cmd.trim();
  char type = cmd.charAt(0); // G ou M
  int val = cmd.substring(1).toInt(); // Numéro commande

  if (type == 'G') {
    if (val == 0 || val == 1) { // Mouvement
      // Chercher X
      int idxX = cmd.indexOf('X');
      int idxY = cmd.indexOf('Y');
      int idxZ = cmd.indexOf('Z');
      
      if (idxX != -1) {
        // Ex: G0 X5 -> on lit jusqu'au prochain espace ou fin
        // Simplification pour l'exemple :
        // Le code réel doit extraire la sous-chaine numérique après X
        String sub = cmd.substring(idxX + 1);
        targetX = sub.toInt(); 
      }
      if (idxY != -1) {
        String sub = cmd.substring(idxY + 1);
        targetY = sub.toInt();
      }
       if (idxZ != -1) {
        // Gestion Z spécifique (souvent timer ou capteur haut/bas)
      }
    }
  }
  else if (type == 'M') {
    if (val == 3) digitalWrite(PIN_PUMP, HIGH); // Pompe ON
    if (val == 5) digitalWrite(PIN_PUMP, LOW);  // Pompe OFF
    
    // Fonction Arrosage automatique
    if (val == 900) { 
        checkAndWater();
    }
  }
}

// Réception Série
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

// --- FONCTIONS MÉTIER ---
void checkAndWater() {
    int sensorVal = analogRead(PIN_MOISTURE);
    // Calibration à faire : 0(sec) - 1023(humide) ou l'inverse
    int percent = map(sensorVal, 1023, 0, 0, 100); 
    
    Serial.print("Humidite: "); Serial.println(percent);
    
    if (percent < 30) {
        Serial.println("Arrosage en cours...");
        digitalWrite(PIN_WATER, HIGH);
        delay(20000); // 20 secondes bloquant (ou utiliser millis())
        digitalWrite(PIN_WATER, LOW);
    }
}
