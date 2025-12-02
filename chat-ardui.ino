#include <AccelStepper.h>

/* --------------------- CONFIGURATION AXES --------------------- */
#define DIR_X 5
#define STEP_X 2
#define DIR_Y 6
#define STEP_Y 3
#define DIR_Z 7
#define STEP_Z 4

#define MICROSTEPS 16  // dépend du driver et moteur

/* Paramètres AccelStepper */
#define MAX_SPEED 2000.0 // pas/sec
#define ACCELERATION 1000.0 // pas/sec²

/* --------------------- CAPTEURS / POMPES --------------------- */
#define PIN_WATER 8
#define PIN_MOISTURE A0

/* --------------------- OBJETS ACCELSTEPPER --------------------- */
AccelStepper stepperX(AccelStepper::DRIVER, STEP_X, DIR_X);
AccelStepper stepperY(AccelStepper::DRIVER, STEP_Y, DIR_Y);
AccelStepper stepperZ(AccelStepper::DRIVER, STEP_Z, DIR_Z);

/* --------------------- VARIABLES --------------------- */
bool waterOn = false;
long targetX = 0, targetY = 0, targetZ = 0;  // positions cibles en pas
float stepsPerMM = 100.0; // ajuster selon vis/tr/min

/* --------------------- SETUP --------------------- */
void setup() {
  Serial.begin(115200);

  pinMode(PIN_WATER, OUTPUT);
  digitalWrite(PIN_WATER, LOW);

  // AccelStepper config
  stepperX.setMaxSpeed(MAX_SPEED);
  stepperY.setMaxSpeed(MAX_SPEED);
  stepperZ.setMaxSpeed(MAX_SPEED);

  stepperX.setAcceleration(ACCELERATION);
  stepperY.setAcceleration(ACCELERATION);
  stepperZ.setAcceleration(ACCELERATION);

  Serial.println("OK");
}

/* --------------------- LOOP --------------------- */
void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) processLine(line);
  }

  // Boucle de mise à jour des moteurs
  stepperX.run();
  stepperY.run();
  stepperZ.run();
}

/* --------------------- PARSE G-CODE --------------------- */
void processLine(String line) {
  line.toUpperCase();

  if (line.startsWith("G0") || line.startsWith("G1")) {
    float x = NAN, y = NAN, z = NAN;
    int f = 1000; // feedrate

    parseGcodeParams(line, x, y, z, f);

    // Conversion mm → pas
    if (!isnan(x)) targetX = x * stepsPerMM;
    if (!isnan(y)) targetY = y * stepsPerMM;
    if (!isnan(z)) targetZ = z * stepsPerMM;

    stepperX.moveTo(targetX);
    stepperY.moveTo(targetY);
    stepperZ.moveTo(targetZ);

    Serial.println("OK");
  }
  else if (line.startsWith("M3")) {
    Serial.println("OK"); // pompe graines (front-end)
  }
  else if (line.startsWith("M5")) {
    Serial.println("OK"); // pompe graines off
  }
  else if (line.startsWith("READ_MOISTURE")) {
    int i=-1, j=-1;
    sscanf(line.c_str(), "READ_MOISTURE %d %d", &i, &j);
    int val = analogRead(PIN_MOISTURE);
    int percent = map(val, 0, 1023, 0, 100);
    Serial.print("MOISTURE "); Serial.print(i); Serial.print(" "); Serial.print(j); Serial.print(" "); Serial.println(percent);
  }
  else if (line.startsWith("WATER_ON")) {
    digitalWrite(PIN_WATER, HIGH);
    waterOn = true;
    Serial.println("OK");
  }
  else if (line.startsWith("WATER_OFF")) {
    digitalWrite(PIN_WATER, LOW);
    waterOn = false;
    Serial.println("OK");
  }
  else {
    Serial.println("ACK");
  }
}

/* --------------------- UTIL GCODE --------------------- */
void parseGcodeParams(String line, float &x, float &y, float &z, int &f) {
  int idx;
  if ((idx = line.indexOf('X')) != -1) x = line.substring(idx+1).toFloat();
  if ((idx = line.indexOf('Y')) != -1) y = line.substring(idx+1).toFloat();
  if ((idx = line.indexOf('Z')) != -1) z = line.substring(idx+1).toFloat();
  if ((idx = line.indexOf('F')) != -1) f = line.substring(idx+1).toInt();
}
