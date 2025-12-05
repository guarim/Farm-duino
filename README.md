# Farm-duino
C'est un projet fascinant et complexe qui combine l'interface web (IHM), le contrôle machine (G-Code/Arduino) et le traitement d'image (Vision par ordinateur).

Voici une solution complète en un seul fichier HTML. Cette application est une "Preuve de Concept" (POC) fonctionnelle qui inclut :

Configuration : Paramétrage de la zone et grille.

Designer : Une interface visuelle pour "peindre" les plantes sur la grille.

Communication Arduino : Utilisation de l'API Web Serial (standard moderne pour connecter USB -> Navigateur).

Logique Métier : Génération du G-Code pour le semis, l'arrosage et le désherbage.

Vision : Accès à la webcam et algorithme de détection de vert (mauvaises herbes).

Note importante : Pour que la connexion USB (Web Serial) et la Webcam fonctionnent, ce fichier doit être ouvert soit en localhost, soit via HTTPS (une contrainte de sécurité des navigateurs modernes).

Résumé des fonctionnalités intégrées :
Interface Graphique (HTML/Canvas) :

À gauche : Paramétrage de la surface (Longueur/Largeur) et de la grille (Nombre de cases). Calcul automatique de la surface par case.

Liste des 6 plantes éditables.

Canvas central : Un damier interactif. Cliquez sur les cases pour assigner la plante sélectionnée.

Communication Machine (Web Serial API) :

Le bouton "Connexion Arduino" ouvre la fenêtre native du navigateur pour choisir le port COM.

Si connecté, envoie du vrai G-Code. Sinon, simule les envois dans la console.

Algorithmes de Cycle (Logique Farmbot) :

Semis : Calcule les trajectoires. Va chercher la graine en (0, 100 + n*100, -400), active la pompe à vide (M106), va à la case, plante, relâche (M107).

Tourelle : Gère la rotation de l'outil via un axe supplémentaire (Axe E dans le code, G0 E80 pour 80°).

Arrosage : Simule une lecture de capteur d'humidité. Si < 30%, active la pompe à eau (M3) pendant un délai (accéléré pour la démo).

Désherbage : Logique de vision.

Vision par Ordinateur (Canvas API) :

Capture le flux de la webcam.

Analyse les pixels : détecte le vert dominant.

Implémente la logique demandée : Si le vert couvre > 35% de l'image, on considère que c'est la plante cultivée (pas d'action). Sinon, si du vert est détecté, c'est une mauvaise herbe -> Action mécanique.

Instructions de test :
Sauvegardez le code dans un fichier .html.

Ouvrez-le avec Chrome ou Edge (Firefox ne supporte pas Web Serial par défaut).

Pour tester la caméra et le port série, utilisez un serveur local (ex: extension "Live Server" de VS Code) ou http://localhost, car ces API sont bloquées sur les fichiers locaux simples (file://) pour des raisons de sécurité.

J'ai créé une configuration Marlin complète pour votre FarmBot basé sur la carte DiscoEasy200 de Dagoma. Voici les points clés :

🎯 Configuration Principale
Dimensions adaptées :

X = 1600 mm (longueur)
Y = 650 mm (largeur)
Z = 600 mm (hauteur)

Origine (POM) configurée :

Z en position HAUTE (Z_HOME_DIR = 1, utilise Z_MAX_PLUG)
X et Y au coin GAUCHE (X_HOME_DIR = -1, Y_HOME_DIR = -1)
Correspond exactement à la grille de l'interface web !

🔧 Spécificités FarmBot
Système de coordonnées :

(0, 0, 0) = Coin gauche avant, Z en haut
X augmente vers la droite
Y augmente vers l'arrière
Z diminue vers le bas (0 = haut, -600 = bas maximum)

Pins pour accessoires :

Pompe à vide : Pin 9 (FAN) - Contrôlée par M3/M5
Pompe arrosage : Pin 8 (BED) - Contrôlée par M106/M107
Capteur humidité : Pin A0 - Lecture analogique

📋 Procédure d'installation

Télécharger Marlin 2.x depuis marlinfw.org
Remplacer Configuration.h par ce fichier
Compiler avec Arduino IDE ou PlatformIO
Flasher la carte DiscoEasy200

✅ Tests de calibration
gcodeG28           // Homing (Z monte, X et Y vont à gauche)
G1 X100 Y100  // Test déplacement
M3 P1         // Test pompe à vide
M106 S255     // Test pompe arrosage
M119          // Vérifier endstops
Le firmware est prêt à être compilé et flashé ! 🌱🤖


