/*
 * FarmBot Arduino Control System
 * Gestion des axes X, Y, Z avec capteurs de position
 * Contrôle du semis, arrosage et capteurs
 */

#include <Stepper.h>

// ===== CONFIGURATION MATÉRIEL =====
// Pins des moteurs pas à pas
#define MOTOR_X_STEP 2
#define MOTOR_X_DIR 3
#define MOTOR_Y_STEP 4
#define MOTOR_Y_DIR 5
#define MOTOR_Z_STEP 6
#define MOTOR_Z_DIR 7

// Pins des capteurs de position (tout ou rien)
#define SENSOR_X 8
#define SENSOR_Y 9
#define SENSOR_Z 10

// Pins pour la pompe à vide et l'arrosage
#define PUMP_VACUUM 11
#define PUMP_WATER 12

// Pin capteur d'humidité
#define HUMIDITY_SENSOR A0

// ===== VARIABLES GLOBALES =====
// Position actuelle (incrémentée par les capteurs)
volatile int posX = 0;
volatile int posY = 0;
volatile int posZ = 0;

// Position cible
int targetX = 0;
int targetY = 0;
int targetZ = 0;

// Grille de culture
const int NX = 9;  // Nombre de cases en X
const int NY = 4;  // Nombre de cases en Y
const float LENGTH = 900.0;  // Longueur en mm
const float WIDTH = 400.0;   // Largeur en mm

// Données de plantation
int gridData[9][4];  // Type de plante dans chaque case (0 = vide)

// Timing pour arrosage automatique
unsigned long lastWateringCheck = 0;
const unsigned long WATERING_INTERVAL = 7200000;  // 2 heures en ms

// Configuration des positions outils
const int TOOL_Y_POSITIONS[6] = {100, 200, 300, 400, 500, 600};
const int TOOL_Z_PICKUP = -400;
const int TOOL_Z_SEED = -450;
const int PLANT_DEPTH = -200;
const int Z_SAFE = 0;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  
  // Configuration des pins moteurs
  pinMode(MOTOR_X_STEP, OUTPUT);
  pinMode(MOTOR_X_DIR, OUTPUT);
  pinMode(MOTOR_Y_STEP, OUTPUT);
  pinMode(MOTOR_Y_DIR, OUTPUT);
  pinMode(MOTOR_Z_STEP, OUTPUT);
  pinMode(MOTOR_Z_DIR, OUTPUT);
  
  // Configuration des capteurs de position
  pinMode(SENSOR_X, INPUT_PULLUP);
  pinMode(SENSOR_Y, INPUT_PULLUP);
  pinMode(SENSOR_Z, INPUT_PULLUP);
  
  // Configuration des pompes
  pinMode(PUMP_VACUUM, OUTPUT);
  pinMode(PUMP_WATER, OUTPUT);
  digitalWrite(PUMP_VACUUM, LOW);
  digitalWrite(PUMP_WATER, LOW);
  
  // Interruptions pour les capteurs
  attachInterrupt(digitalPinToInterrupt(SENSOR_X), sensorXTrigger, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR_Y), sensorYTrigger, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR_Z), sensorZTrigger, FALLING);
  
  // Initialisation de la grille
  for (int i = 0; i < NX; i++) {
    for (int j = 0; j < NY; j++) {
      gridData[i][j] = 0;
    }
  }
  
  Serial.println("FarmBot initialisé");
  Serial.println("Commandes disponibles:");
  Serial.println("  GRID:i,j,type - Définir une case");
  Serial.println("  SEED - Lancer le semis");
  Serial.println("  WATER - Contrôle humidité/arrosage");
  Serial.println("  HOME - Retour origine");
  Serial.println("  STATUS - État du système");
}

// ===== INTERRUPTIONS CAPTEURS =====
void sensorXTrigger() {
  if (targetX > posX) posX++;
  else if (targetX < posX) posX--;
}

void sensorYTrigger() {
  if (targetY > posY) posY++;
  else if (targetY < posY) posY--;
}

void sensorZTrigger() {
  if (targetZ > posZ) posZ++;
  else if (targetZ < posZ) posZ--;
}

// ===== FONCTIONS DE DÉPLACEMENT =====
void moveToPosition(int x, int y, int z) {
  targetX = x;
  targetY = y;
  targetZ = z;
  
  Serial.print("Déplacement vers (");
  Serial.print(x); Serial.print(",");
  Serial.print(y); Serial.print(",");
  Serial.print(z); Serial.println(")");
  
  // Boucle jusqu'à atteindre la position
  while (posX != targetX || posY != targetY || posZ != targetZ) {
    // Déplacement X
    if (posX < targetX) {
      digitalWrite(MOTOR_X_DIR, HIGH);
      stepMotor(MOTOR_X_STEP);
    } else if (posX > targetX) {
      digitalWrite(MOTOR_X_DIR, LOW);
      stepMotor(MOTOR_X_STEP);
    }
    
    // Déplacement Y
    if (posY < targetY) {
      digitalWrite(MOTOR_Y_DIR, HIGH);
      stepMotor(MOTOR_Y_STEP);
    } else if (posY > targetY) {
      digitalWrite(MOTOR_Y_DIR, LOW);
      stepMotor(MOTOR_Y_STEP);
    }
    
    // Déplacement Z
    if (posZ < targetZ) {
      digitalWrite(MOTOR_Z_DIR, HIGH);
      stepMotor(MOTOR_Z_STEP);
    } else if (posZ > targetZ) {
      digitalWrite(MOTOR_Z_DIR, LOW);
      stepMotor(MOTOR_Z_STEP);
    }
    
    delay(1);  // Vitesse de déplacement
  }
  
  Serial.println("Position atteinte");
}

void stepMotor(int pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(500);
  digitalWrite(pin, LOW);
  delayMicroseconds(500);
}

// ===== CALCUL DES CENTRES DE CASES =====
void getCellCenter(int i, int j, float &xCenter, float &yCenter) {
  float cellWidth = LENGTH / NX;
  float cellHeight = WIDTH / NY;
  
  xCenter = i * cellWidth + cellWidth / 2.0;
  yCenter = j * cellHeight + cellHeight / 2.0;
}

// ===== SÉQUENCE DE SEMIS =====
void seedCell(int i, int j, int plantType) {
  if (plantType < 1 || plantType > 6) return;
  
  Serial.print("Semis case (");
  Serial.print(i); Serial.print(",");
  Serial.print(j); Serial.print(") - Plante ");
  Serial.println(plantType);
  
  // 1. Montée de sécurité
  moveToPosition(posX, posY, 0);  // Z safe
  
  // 2. Déplacement vers l'outil
  int yTool = TOOL_Y_POSITIONS[plantType - 1];
  moveToPosition(0, yTool / 100, 0);  // Conversion en position grille
  
  // 3. Descendre pour prendre la graine
  moveToPosition(0, yTool / 100, 1);  // Position basse
  
  // 4. Activer la pompe à vide
  digitalWrite(PUMP_VACUUM, HIGH);
  delay(500);  // Aspiration
  
  // 5. Remonter avec la graine
  moveToPosition(0, yTool / 100, 0);
  
  // 6. Déplacement vers la case cible
  moveToPosition(i, j, 0);
  
  // 7. Descendre pour planter
  moveToPosition(i, j, 1);
  
  // 8. Désactiver la pompe (relâcher la graine)
  digitalWrite(PUMP_VACUUM, LOW);
  delay(200);
  
  // 9. Remonter
  moveToPosition(i, j, 0);
  
  Serial.println("Semis terminé");
}

void executeSeeding() {
  Serial.println("=== DÉBUT DU SEMIS ===");
  
  for (int i = 0; i < NX; i++) {
    for (int j = 0; j < NY; j++) {
      if (gridData[i][j] > 0) {
        seedCell(i, j, gridData[i][j]);
        delay(1000);  // Pause entre chaque semis
      }
    }
  }
  
  // Retour à l'origine
  moveToPosition(0, 0, 0);
  Serial.println("=== SEMIS TERMINÉ ===");
}

// ===== SYSTÈME D'ARROSAGE =====
int readHumidity() {
  int rawValue = analogRead(HUMIDITY_SENSOR);
  // Conversion en pourcentage (calibration nécessaire)
  int humidity = map(rawValue, 0, 1023, 0, 100);
  return humidity;
}

void wateringCycle() {
  Serial.println("=== CONTRÔLE D'HUMIDITÉ ===");
  
  for (int i = 0; i < NX; i++) {
    for (int j = 0; j < NY; j++) {
      if (gridData[i][j] == 0) continue;  // Case vide
      
      // Déplacement vers la case
      moveToPosition(i, j, 0);
      moveToPosition(i, j, 1);  // Descendre le capteur
      
      // Lecture de l'humidité
      int humidity = readHumidity();
      Serial.print("Case ("); Serial.print(i);
      Serial.print(","); Serial.print(j);
      Serial.print(") - Humidité: ");
      Serial.print(humidity); Serial.println("%");
      
      // Arrosage si nécessaire
      if (humidity < 30) {
        Serial.println("  -> Arrosage nécessaire");
        digitalWrite(PUMP_WATER, HIGH);
        delay(20000);  // Arrosage pendant 20 secondes
        digitalWrite(PUMP_WATER, LOW);
        Serial.println("  -> Arrosage terminé");
      }
      
      // Remonter
      moveToPosition(i, j, 0);
      delay(500);
    }
  }
  
  Serial.println("=== CYCLE D'ARROSAGE TERMINÉ ===");
}

// ===== BOUCLE PRINCIPALE =====
void loop() {
  // Vérification des commandes série
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command);
  }
  
  // Contrôle automatique de l'arrosage (toutes les 2 heures)
  unsigned long currentTime = millis();
  if (currentTime - lastWateringCheck >= WATERING_INTERVAL) {
    wateringCycle();
    lastWateringCheck = currentTime;
  }
  
  delay(10);
}

// ===== TRAITEMENT DES COMMANDES =====
void processCommand(String cmd) {
  if (cmd.startsWith("GRID:")) {
    // Format: GRID:i,j,type
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    
    int i = cmd.substring(5, firstComma).toInt();
    int j = cmd.substring(firstComma + 1, secondComma).toInt();
    int type = cmd.substring(secondComma + 1).toInt();
    
    if (i >= 0 && i < NX && j >= 0 && j < NY) {
      gridData[i][j] = type;
      Serial.print("Case ("); Serial.print(i);
      Serial.print(","); Serial.print(j);
      Serial.print(") définie: Type ");
      Serial.println(type);
    } else {
      Serial.println("Erreur: indices hors limites");
    }
  }
  else if (cmd == "SEED") {
    executeSeeding();
  }
  else if (cmd == "WATER") {
    wateringCycle();
  }
  else if (cmd == "HOME") {
    Serial.println("Retour à l'origine");
    moveToPosition(0, 0, 0);
  }
  else if (cmd == "STATUS") {
    Serial.println("=== ÉTAT DU SYSTÈME ===");
    Serial.print("Position actuelle: (");
    Serial.print(posX); Serial.print(",");
    Serial.print(posY); Serial.print(",");
    Serial.print(posZ); Serial.println(")");
    
    Serial.print("Grille: "); Serial.print(NX);
    Serial.print("x"); Serial.println(NY);
    
    int plantedCells = 0;
    for (int i = 0; i < NX; i++) {
      for (int j = 0; j < NY; j++) {
        if (gridData[i][j] > 0) plantedCells++;
      }
    }
    Serial.print("Cases plantées: ");
    Serial.println(plantedCells);
  }
  else {
    Serial.println("Commande inconnue");
  }
}
