/**
 * Configuration Marlin pour FarmBot
 * Basé sur DiscoEasy200 de Dagoma
 * 
 * Modifications principales :
 * - Dimensions : X=1600mm, Y=650mm, Z=600mm
 * - Origine (POM) : Z en position haute, X et Y en coin gauche
 * - Adaptation pour système de culture automatisé
 */

//===========================================================================
//========================= CONFIGURATION DE BASE ===========================
//===========================================================================

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Informations du firmware
#define STRING_CONFIG_H_AUTHOR "(FarmBot, DiscoEasy200 Modified)"
#define CUSTOM_MACHINE_NAME "FarmBot Dagoma"

//===========================================================================
//========================= PARAMÈTRES MACHINE ==============================
//===========================================================================

// Type de carte : Dagoma DiscoEasy200 utilise une carte compatible RAMPS
#define MOTHERBOARD BOARD_RAMPS_14_EFB

// Baudrate pour communication série
#define BAUDRATE 115200

//===========================================================================
//=========================== AXES ET DIRECTIONS ============================
//===========================================================================

// Inversion des directions des moteurs
// À ajuster selon le câblage de votre machine
#define INVERT_X_DIR false    // false = vers la droite positif
#define INVERT_Y_DIR false    // false = vers l'arrière positif
#define INVERT_Z_DIR true     // true = vers le haut positif (IMPORTANT pour Z haute)

// Inversion des endstops
#define X_MIN_ENDSTOP_INVERTING false
#define Y_MIN_ENDSTOP_INVERTING false
#define Z_MAX_ENDSTOP_INVERTING false  // Z MAX car origine en haut

//===========================================================================
//======================== DIMENSIONS DE TRAVAIL ============================
//===========================================================================

/**
 * IMPORTANT : Configuration FarmBot
 * - Origine en coin GAUCHE (X_MIN, Y_MIN)
 * - Z en position HAUTE (Z_MAX)
 * - Correspond à la grille de l'interface web
 */

// Position des endstops
#define USE_XMIN_PLUG
#define USE_YMIN_PLUG
#define USE_ZMAX_PLUG    // Z MAX car origine en position haute

// Direction du homing
#define X_HOME_DIR -1    // Homing vers MIN (gauche)
#define Y_HOME_DIR -1    // Homing vers MIN (gauche/avant)
#define Z_HOME_DIR 1     // Homing vers MAX (haut)

// Dimensions de la zone de travail
#define X_BED_SIZE 1600  // 1600 mm en X
#define Y_BED_SIZE 650   // 650 mm en Y

// Limites des axes
#define X_MIN_POS 0
#define Y_MIN_POS 0
#define Z_MIN_POS 0

#define X_MAX_POS 1600   // 1600 mm
#define Y_MAX_POS 650    // 650 mm
#define Z_MAX_POS 600    // 600 mm (origine en haut)

//===========================================================================
//======================= CONFIGURATION DES MOTEURS =========================
//===========================================================================

// Steps par mm (à calibrer selon votre système d'entraînement)
// Valeurs typiques pour courroies GT2 (2mm pitch) et moteurs 1.8°
// Steps/mm = (steps_par_tour * microstepping) / (pitch * dents_poulie)
// Exemple : (200 * 16) / (2 * 20) = 80 steps/mm

#define DEFAULT_AXIS_STEPS_PER_UNIT   { 80, 80, 400, 95 }  // X, Y, Z, E

// Vitesses maximales (mm/s)
#define DEFAULT_MAX_FEEDRATE          { 300, 300, 5, 25 }  // X, Y, Z, E

// Accélérations maximales (mm/s²)
#define DEFAULT_MAX_ACCELERATION      { 3000, 3000, 100, 10000 }  // X, Y, Z, E

// Accélérations par défaut pour les mouvements
#define DEFAULT_ACCELERATION          500    // X, Y, Z
#define DEFAULT_RETRACT_ACCELERATION  1000   // Extrudeur
#define DEFAULT_TRAVEL_ACCELERATION   1000   // Déplacements sans extrusion

// Jerk (mm/s) - Changement instantané de vitesse
#define DEFAULT_XJERK 10.0
#define DEFAULT_YJERK 10.0
#define DEFAULT_ZJERK  0.4
#define DEFAULT_EJERK  5.0

//===========================================================================
//========================== HOMING ET LEVELING =============================
//===========================================================================

// Vitesses de homing
#define HOMING_FEEDRATE_XY (50*60)   // 50 mm/s
#define HOMING_FEEDRATE_Z  (4*60)    // 4 mm/s

// Bump lors du homing (approche lente après détection)
#define X_HOME_BUMP_MM 5
#define Y_HOME_BUMP_MM 5
#define Z_HOME_BUMP_MM 2

#define HOMING_BUMP_DIVISOR { 2, 2, 4 }  // Diviseur de vitesse pour l'approche lente

// Levée de sécurité après homing
#define Z_HOMING_HEIGHT 10  // Lever Z de 10mm avant homing X/Y

// Ordre de homing personnalisé
#define HOMING_ORDER { AXIS_Z, AXIS_X, AXIS_Y }  // Z d'abord (sécurité)

//===========================================================================
//======================== SÉCURITÉS ET PROTECTIONS =========================
//===========================================================================

// Protection thermique (peut être désactivée si pas de hotend)
#define THERMAL_PROTECTION_HOTENDS
#define THERMAL_PROTECTION_BED

// Températures min/max de sécurité
#define HEATER_0_MINTEMP 5
#define HEATER_0_MAXTEMP 275
#define BED_MINTEMP 5
#define BED_MAXTEMP 150

// Watchdog - Redémarre si le firmware freeze
#define USE_WATCHDOG

// Protection contre les mouvements hors limites
#define MIN_SOFTWARE_ENDSTOPS
#define MAX_SOFTWARE_ENDSTOPS

//===========================================================================
//====================== CONFIGURATION SPÉCIALE FARMBOT =====================
//===========================================================================

/**
 * Configuration du système de coordonnées FarmBot
 * 
 * Correspondance avec l'interface web :
 * - (0,0,0) = Coin gauche avant, Z en position haute
 * - X augmente vers la droite
 * - Y augmente vers l'arrière
 * - Z diminue vers le bas (0 = haut, -600 = bas)
 * 
 * Note : Le Z est inversé par rapport à une imprimante 3D classique
 */

// Activation du mode "Machine Coordinates"
#define RELATIVE_POSITIONING  // Utiliser G91 pour mouvements relatifs

// Désactiver l'extrudeur si non utilisé
#define EXTRUDERS 0  // Pas d'extrudeur pour FarmBot

// Désactiver le chauffage si non utilisé
#define TEMP_SENSOR_0 0
#define TEMP_SENSOR_BED 0

//===========================================================================
//======================== PINS POUR ACCESSOIRES ============================
//===========================================================================

/**
 * Configuration des pins pour les accessoires FarmBot
 * Basé sur RAMPS 1.4
 */

// Pompe à vide (utilise la sortie FAN)
#define FAN_PIN 9  // Pin PWM pour contrôle de la pompe à vide

// Pompe d'arrosage (utilise la sortie du lit chauffant)
#define HEATER_BED_PIN 8  // Utiliser avec relais

// Capteur d'humidité (entrée analogique)
#define TEMP_BED_PIN 14  // A0 - Capteur d'humidité du sol

// Capteurs de position (endstops supplémentaires)
// Utiliser les pins disponibles sur AUX
#define SENSOR_X_PIN 2   // Endstop X min
#define SENSOR_Y_PIN 15  // Endstop Y min (A1)
#define SENSOR_Z_PIN 19  // Endstop Z max (A5)

//===========================================================================
//========================= COMMANDES G-CODE CUSTOM =========================
//===========================================================================

/**
 * Commandes G-Code personnalisées pour FarmBot
 * 
 * M3 P1 - Activer pompe à vide (aspiration graine)
 * M5    - Désactiver pompe à vide
 * M106  - Activer pompe d'arrosage
 * M107  - Désactiver pompe d'arrosage
 * M117  - Lire capteur d'humidité
 */

// Activer les commandes M3/M5 pour contrôle de vitesse
#define SPINDLE_FEATURE
#define SPINDLE_LASER_ACTIVE_HIGH false
#define SPINDLE_LASER_PWM
#define SPINDLE_LASER_PWM_INVERT false

//===========================================================================
//======================= ÉCRAN LCD (OPTIONNEL) =============================
//===========================================================================

// Si vous utilisez un écran LCD RepRapDiscount
// #define REPRAP_DISCOUNT_SMART_CONTROLLER

// Si vous utilisez un écran graphique complet
// #define REPRAP_DISCOUNT_FULL_GRAPHIC_SMART_CONTROLLER

//===========================================================================
//========================= SD CARD (OPTIONNEL) =============================
//===========================================================================

#define SDSUPPORT  // Support carte SD
#define SD_CHECK_AND_RETRY  // Vérification et retry en cas d'erreur

//===========================================================================
//======================= CALIBRATION ET TUNING =============================
//===========================================================================

/**
 * PROCÉDURE DE CALIBRATION :
 * 
 * 1. Vérifier les directions des moteurs :
 *    - Envoyer G1 X10 : doit aller vers la DROITE
 *    - Envoyer G1 Y10 : doit aller vers l'ARRIÈRE
 *    - Envoyer G1 Z10 : doit DESCENDRE (car origine en haut)
 * 
 * 2. Calibrer les steps/mm :
 *    - Mesurer un déplacement de 100mm sur chaque axe
 *    - Ajuster DEFAULT_AXIS_STEPS_PER_UNIT si nécessaire
 *    - Formule : new_steps = old_steps * (distance_demandée / distance_réelle)
 * 
 * 3. Tester le homing (G28) :
 *    - X et Y doivent aller au coin GAUCHE AVANT
 *    - Z doit aller en position HAUTE
 * 
 * 4. Vérifier les limites :
 *    - G1 X1600 Y650 Z-600 doit être atteignable
 *    - Ajuster X_MAX_POS, Y_MAX_POS si nécessaire
 */

// Mode debug pour calibration
// #define DEBUG_LEVELING_FEATURE  // Décommenter pour debug détaillé

//===========================================================================
//======================== SÉCURITÉ FINALE ==================================
//===========================================================================

// Vérification des paramètres critiques
#if Z_HOME_DIR != 1
  #error "Z_HOME_DIR doit être 1 (homing vers le haut)"
#endif

#if X_HOME_DIR != -1 || Y_HOME_DIR != -1
  #error "X et Y doivent homer vers MIN (coin gauche)"
#endif

#if X_MAX_POS != 1600 || Y_MAX_POS != 650 || Z_MAX_POS != 600
  #error "Vérifier les dimensions X=1600, Y=650, Z=600"
#endif

#endif // CONFIGURATION_H

//===========================================================================
//======================= NOTES D'INSTALLATION ==============================
//===========================================================================

/**
 * INSTALLATION DU FIRMWARE :
 * 
 * 1. Télécharger Marlin 2.x depuis https://marlinfw.org/
 * 2. Remplacer Configuration.h par ce fichier
 * 3. Compiler avec Arduino IDE ou PlatformIO
 * 4. Flasher la carte via USB
 * 
 * CONNEXION À L'INTERFACE WEB :
 * 
 * 1. Connecter la carte en USB
 * 2. Utiliser le Web Serial API dans l'interface
 * 3. Baudrate : 115200
 * 
 * COMMANDES DE TEST :
 * 
 * G28         - Homing de tous les axes
 * G1 X100     - Déplacer X de 100mm
 * M3 P1       - Test pompe à vide
 * M106 S255   - Test pompe arrosage
 * M114        - Afficher position actuelle
 * M119        - État des endstops
 * 
 * CORRESPONDANCE COORDONNÉES :
 * 
 * Interface Web (i,j) → G-Code (X,Y)
 * - Case (0,0) = X0 Y0 (coin gauche avant)
 * - Case (8,3) = X1600 Y650 (coin droit arrière)
 * - Z haute = Z0 (origine)
 * - Z basse = Z-600 (profondeur max)
 */
