#ifndef __GLOBAL_H__
#define __GLOBAL_H__

//------------------------------------------------------------------------------
// Journal web (http://<IP>/logs) : Serial est redirigé vers gSerialJournal, qui écrit
// sur le port série ET dans le journal (voir MyJournal.h).
// Arduino.h doit être inclus avant le #define (il déclare le vrai Serial).
// Les fichiers qui n'incluent pas global.h écrivent seulement sur le port série.
//------------------------------------------------------------------------------
#include <Arduino.h>
#include "MyJournal.h"
#define Serial gSerialJournal

//================ ESP32C3 ================
//          MISO - GPIO5   ||   5V
//          MOSI - GPIO6   ||   GND
//          SS   - GPIO7   ||   3.3V
//          SDA  - GPIO8   ||   GPIO4 -  SCK
//          SCL  - GPIO9   ||   GPIO3
//                 GPIO10  ||   GPIO2
//          RX   - GPIO20  ||   GPIO1
//          TX   - GPIO21  ||   GPIO0

//================ ESP32S3 ================
//               - GPIO1   ||   5V
//               - GPIO2   ||   GND
//               - GPIO3   ||   3.3V
//               - GPIO4   ||   GPIO9  -  MOSI
//          SDA  - GPIO5   ||   GPIO8  -  MISO
//          SCL  - GPIO6   ||   GPIO7  -  SCK
//          TX   - GPIO43  ||   GPIO44 -  RX


// Envoi des valeurs de test. Voir main.cpp : 
//   static float mes[nbMes][2] = {
//                          {10.0,90.0},
//                          {20.0,80.0},
//                          {40.0,60.0},
//                          {-12.0,2.0}
//                        };
//#define TEST_EMISSION_RCS


//#define DEBUG_NO_DEEP_SLEEP
//#define _RCSWITCH_MODE_ // Envoi par RCSwitch possible si décommentée
//#define __DESACTIVE_ENVOI_MQTT__ // Désactive la remontée de mesure par MQTT, laissant uniquement la RF

//----------------------------------------------------------------------------
// Liaison et capteurs : fixés par le profil du nœud dans platformio.ini (env:ThChRDC_Xiao_C3, env:ThRemise_Xiao_S3...,
// qui définit PROFIL_NOEUD). Le bloc suivant ne sert qu'aux profils génériques (xiao_c3, xiao_s3 et leurs _debug) :
// essais, nouveau nœud.
//----------------------------------------------------------------------------
#ifndef PROFIL_NOEUD
#define _WIFI_MODE_ // Envoi par WIFI possible si décommentée
//#define _LORA_P2P_MODE_ // Envoi par LORA
//----------------------------------------------------------------------------
// Décommenter en fonction des capteurs / détecteurs installés
//----------------------------------------------------------------------------
#define CAPTEUR_DHT20 // Fonctionne en I2C
//#define CAPTEUR_DS18B20
//#define FLOTTEUR_VERTICAL // ThRemise seulement (ThCave / ThNomade : pas de flotteur)
//#define CAPTEUR_RGB_TCS34725 // Fonctionne en I2C — ThSdb (capteur chaudière) seulement
//#define LED_CAPTEUR_RGB // Signale l'état du capteur (ON/OFF) — ThSdb (capteur chaudière) seulement
#define CAPTEUR_BATTERIE // Mesure de la tension de la batterie (via un pont diviseur de tension)
//#define CAPTEUR_MICRO_MAX4466 // Détection des bips ON/OFF par micro (écoute continue : pas de deep sleep)
//#define CAPTEUR_JOUR_NUIT // Détecteur jour/nuit : photorésistance + 10 kΩ, mesure JOUR_NUIT_PIN, alimentation JOUR_NUIT_ALIM_PIN (voir MyJourNuit.h)
//#define CAPTEUR_PRESENCE_RESEAU // Présence sur le réseau électrique : nœud derrière une prise RF (PrGuirlande...), trame PRESENT, répond à PING (voir MyPresenceReseauElectrique.h)

//----------------------------------------------------------------------------
// Décommenter en fonction des actionneurs installés
//----------------------------------------------------------------------------
//#define ACTIONNEUR_IR
#endif // PROFIL_NOEUD

//----------------------------------------------------------------------------
// En cas de perte de la configuration, décommenter la ligne suivante.
// Les identifiants Wi-Fi / MQTT sont dans secret.h (exclu de Git) ;
// modèle : secret.exemple.h (valeurs factices, utilisé si secret.h est absent).
//
// Une fois le Wifi connecté, connectez-vous par Web et reparamétrez les champs.
//----------------------------------------------------------------------------
//#define FORCE_CONNEXION_WIFI

// Toujours inclus : FORCE_WIFI_* et FORCE_MQTT_* servent aussi de Wi-Fi et de broker par défaut d'une puce neuve (voir MyWifi.h, MyMqtt.h)
#if __has_include("secret.h")
  #include "secret.h"
#else
  #include "secret.exemple.h"
#endif

// -----------------------------------------------------------------------------
// Détection automatique de la carte ESP32-C3 ou ESP32-S3
// -----------------------------------------------------------------------------
#if defined(__ESP32_C3__)
  #define IS_ESP32_C3       1
  #define IS_ESP32_S3       0
  // BOARD_XIAO_C3 (profils xiao_c3 et xiao_c3_debug de platformio.ini) : carte « Capteur C3 Xiao » (pcb/Capteur C3 Xiao/).
  // Sinon : ESP32-C3 Super Mini.
  #if defined(BOARD_XIAO_C3)
  #define BOARD_NAME        "XIAO ESP32-C3"
  #else
  #define BOARD_NAME        "ESP32-C3 Super Mini"
  #endif

  // Pins spécifiques ESP32-C3 (tirés de ton mapping)
  //#define LORA_MISO_PIN     5
  //#define LORA_SCK_PIN      4   // ou 7 selon tes commentaires
  //#define LORA_MOSI_PIN     6
  //#define LORA_CS_PIN       7
  //#define LORA_DIO2_PIN     38  // non utilisé sur C3 ?
  //#define LORA_DIO1_PIN     3   // ou autre libre
  //#define LORA_RESET_PIN    2
  //#define LORA_BUSY_PIN     1   // ou RADIOLIB_NC

  #if defined(BOARD_XIAO_C3)
    // XIAO D4 / D5 (pas GPIO8 / GPIO9 : broches de démarrage, GPIO9 = bouton BOOT)
    #define DEFAULT_SDA_PIN 6   // XIAO D4 // DHT20 entre autre
    #define DEFAULT_SCL_PIN 7   // XIAO D5 // DHT20 entre autre
  #else
    #define DEFAULT_SDA_PIN 8   // Pin 4 // DHT20 entre autre
    #define DEFAULT_SCL_PIN 9   // Pin 5 // DHT20 entre autre
  #endif

  // Broches CC1101 (RCSwitch) : toujours définies, même si _RCSWITCH_MODE_ est désactivé,
  // pour que setup() puisse les repasser en INPUT_PULLDOWN quand l'actionneur RF n'est pas utilisé.
  #define CC1101_GDO0       3
  #define CC1101_CS         7
  #define CC1101_MOSI       6
  #define CC1101_MISO       5
  #define CC1101_SCK        4
  #define CC1101_POWER_GND_GPIO 20

  #if defined(BOARD_XIAO_C3)
  // Carte « Capteur C3 Xiao » : D0 (GPIO2, broche de démarrage, doit être haute au démarrage)
  // n'est pas câblée ; D8 / D9 (GPIO8 / GPIO9) non plus.
  // Toujours définie (même si CAPTEUR_DS18B20 est désactivé) pour pouvoir neutraliser
  // cette broche en setup() : pas de DS18B20 sur cette carte, D3 est libre.
  #define DS18B20_PIN 5 // XIAO D3 (non câblée)

  // Etat batterie : pont 220 k / 100 k (défini même si CAPTEUR_BATTERIE est désactivé : sert de
  // valeur par défaut dans CBatterieAA::loadFromNVS(), toujours compilé). Entrée analogique
  // obligatoire : GPIO0 à 4 (ADC1) seulement sur le C3.
  #define BATTERIE_PIN    4 // XIAO D2 (ADC1_CH4)

  // Toujours définie (même si FLOTTEUR_VERTICAL est désactivé) pour pouvoir neutraliser
  // cette broche en setup() quand le flotteur n'est pas câblé.
  #define DEFAULT_TOR_PIN 3 // XIAO D1

  // Détecteur jour/nuit (sortie analogique). Entrée ADC1 obligatoire : sur le XIAO seules
  // D0 (GPIO2), D1 (GPIO3) et D2 (GPIO4) sont des ADC1. D2 = batterie ; D0 = broche de démarrage
  // (doit être haute à la mise sous tension : une cellule dans l'obscurité ou en pleine lumière
  // la tirerait vers le bas). Reste D1, partagée avec le flotteur et le micro (voir les #error).
  // Toujours définie (même si CAPTEUR_JOUR_NUIT est désactivé) pour pouvoir neutraliser
  // cette broche en setup().
  #define JOUR_NUIT_PIN 3 // XIAO D1 (ADC1_CH3), connecteur du flotteur
  // Alimentation du pont de la photorésistance (coupée hors mesure et en deep sleep).
  // D3 (GPIO5) : libre, pas de DS18B20 sur cette carte (non câblée sur le PCB : fil à ajouter).
  // Pas D0 (GPIO2) : le pont la tirerait vers le bas au démarrage.
  #define JOUR_NUIT_ALIM_PIN 5 // XIAO D3

  // Toujours définies (même si CAPTEUR_RGB_TCS34725 / LED_CAPTEUR_RGB sont désactivés) pour
  // pouvoir neutraliser ces broches en setup().
  #define CAPTEUR_RGB_TCS34725_INTERRUPT 21 // XIAO D6, non câblée (U0TXD : messages du ROM au démarrage)
  #define LED_CAPTEUR_RGB_PIN 20            // XIAO D7

  #ifdef _RCSWITCH_MODE_
    #error "BOARD_XIAO_C3 : pas de CC1101 sur la carte Capteur C3 Xiao (ses broches servent au flotteur, à l'I2C, à la mesure et à la LED)"
  #endif
  #if defined(CAPTEUR_DS18B20)
    #error "BOARD_XIAO_C3 : pas de DS18B20 sur la carte Capteur C3 Xiao"
  #endif
  #if defined(CAPTEUR_MICRO_MAX4466) && defined(FLOTTEUR_VERTICAL)
    #error "BOARD_XIAO_C3 : GPIO3 utilisée à la fois par le micro MAX4466 et par le flotteur"
  #endif
  #if defined(CAPTEUR_MICRO_MAX4466) && (defined(CAPTEUR_DHT20) || defined(CAPTEUR_RGB_TCS34725))
    #error "BOARD_XIAO_C3 : GPIO7 utilisée à la fois par la LED du micro MAX4466 et par l'I2C (SCL)"
  #endif
  #if defined(CAPTEUR_JOUR_NUIT) && defined(FLOTTEUR_VERTICAL)
    #error "BOARD_XIAO_C3 : GPIO3 utilisée à la fois par le détecteur jour/nuit et par le flotteur"
  #endif
  #if defined(CAPTEUR_JOUR_NUIT) && defined(CAPTEUR_MICRO_MAX4466)
    #error "BOARD_XIAO_C3 : GPIO3 utilisée à la fois par le détecteur jour/nuit et par le micro MAX4466"
  #endif
  #if defined(ACTIONNEUR_IR) && defined(LED_CAPTEUR_RGB)
    #error "BOARD_XIAO_C3 : GPIO20 utilisée à la fois par l'actionneur IR et par la LED du capteur RGB"
  #endif

  #else // Super Mini
  // Toujours définie (même si CAPTEUR_DS18B20 est désactivé) pour pouvoir neutraliser
  // cette broche en setup() quand le capteur n'est pas câblé.
  #define DS18B20_PIN 2 // Pin 111

  #ifdef CAPTEUR_DS18B20
    // Etat batterie (défini même si CAPTEUR_BATTERIE est désactivé : sert de valeur
    // par défaut dans CBatterieAA::loadFromNVS(), toujours compilé)
    #define BATTERIE_PIN    3 // Pin 12
  #else
    // Etat batterie
    // Partage la broche du DS18B20 (2) quand celui-ci est désactivé : voir la neutralisation
    // des broches inutilisées dans setup() (main.cpp), qui tient compte de ce partage.
    #define BATTERIE_PIN    2 // Pin 111
  #endif // CAPTEUR_DS18B20

  // Toujours définie (même si FLOTTEUR_VERTICAL est désactivé) pour pouvoir neutraliser
  // cette broche en setup() quand le flotteur n'est pas câblé.
  #define DEFAULT_TOR_PIN 1 // Pin 10

  // Détecteur jour/nuit (sortie analogique). Entrée ADC1 obligatoire (GPIO0 à 4) : GPIO0 est
  // la seule libre (pas une broche de démarrage, pas de quartz 32 kHz sur la Super Mini).
  // Toujours définie (même si CAPTEUR_JOUR_NUIT est désactivé) pour pouvoir neutraliser
  // cette broche en setup().
  #define JOUR_NUIT_PIN 0 // GPIO0 (ADC1_CH0)
  // Alimentation du pont de la photorésistance (coupée hors mesure et en deep sleep).
  // GPIO10 : libre (les autres servent au CC1101, à l'I2C ou sont des broches de démarrage).
  #define JOUR_NUIT_ALIM_PIN 10

  // Toujours définies (même si CAPTEUR_RGB_TCS34725 / LED_CAPTEUR_RGB sont désactivés) pour
  // pouvoir neutraliser ces broches en setup(). LED_CAPTEUR_RGB_PIN (1) partage la broche de
  // DEFAULT_TOR_PIN sur cette carte : la neutralisation dans setup() tient compte de ce partage.
  #define CAPTEUR_RGB_TCS34725_INTERRUPT 21 // Optionnel
  #define LED_CAPTEUR_RGB_PIN 1
  #endif // BOARD_XIAO_C3

  // Noeud chauffage SdB sur XIAO ESP32C3 (GPIO0 et GPIO1 non sortis sur cette carte).
  // Broches micro et IR : valeurs par défaut, modifiables par la page Web (NVS).
  // Broches des LED : uniquement ici.

  // Toujours définie (même si CAPTEUR_MICRO_MAX4466 est désactivé) pour pouvoir neutraliser
  // cette broche en setup(). Entrée analogique obligatoire : GPIO0 à 4 (ADC1) seulement sur le C3.
  // Partage CC1101_GDO0 (3) : la neutralisation dans setup() ne touche pas une broche utilisée.
  #define CAPTEUR_MICRO_MAX4466_PIN 3  // XIAO D1 (ADC1_CH3)
  #define CAPTEUR_MICRO_MAX4466_LED_PIN 7 // XIAO D5. LED bleue : flash à chaque bip valide détecté

  #ifdef ACTIONNEUR_IR
    // GPIO20 (U0RXD) : aucun rôle au démarrage, UART0 inutilisé (moniteur sur l'USB, ARDUINO_USB_CDC_ON_BOOT).
    // Pas GPIO8 (strapping : l'émetteur l'empêchait d'entrer en mode téléchargement) ni GPIO21
    // (U0TXD : messages du ROM au démarrage, l'émetteur clignoterait).
    // Partage CC1101_POWER_GND_GPIO (20) : la neutralisation dans setup() ne touche pas une broche utilisée.
    #define ACTIONNEUR_IR_PIN 20         // XIAO D7
    // LED témoin IR : allumée pendant l'émission
    #define ACTIONNEUR_IR_LED_PIN 10     // XIAO D10. LED rouge
  #endif

#elif defined(__ESP32_S3__)
  #define IS_ESP32_C3       0
  #define IS_ESP32_S3       1
  #define BOARD_NAME        "ESP32-S3 (XIAO / Super Mini S3)"

  //#ifdef _LORA_P2P_MODE_
    // Pins spécifiques ESP32-S3 (tirés de ton mapping)
    #define LORA_MISO_PIN     8
    #define LORA_SCK_PIN      7
    #define LORA_MOSI_PIN     9
    #define LORA_CS_PIN       41
    #define LORA_DIO2_PIN     38
    #define LORA_DIO1_PIN     39
    #define LORA_RESET_PIN    42
    #define LORA_BUSY_PIN     40

    #define LORA_FREQUENCE    868.1  // Europe : sous-bande g1 868,0-868,6 MHz (1 % du temps d'émission, 14 dBm)
    #define LORA_BANDWIDTH    125.0
    #define LORA_SPREADING_FACTOR 7
    #define LORA_CODING_RATE  5
    #define LORA_SYNC_WORD    0x12
    #define LORA_OUTPUT_POWER 14.0
    #define LORA_PREAMBLE_LENGTH 8
    #define LORA_TCXO_VOLTAGE 1.8  // Wio-SX1262 Seeed : TCXO 1.8V (et non 1.6V)

    #define START_STOP "==========" // A mettre en début et en fin de message
  //#endif // _LORA_P2P_MODE_

    #define DEFAULT_SDA_PIN   5   // Pin 5 // DHT20 entre autre
    #define DEFAULT_SCL_PIN   6   // Pin 6 // DHT20 entre autre

  // Toujours définie (même si CAPTEUR_DS18B20 est désactivé) pour pouvoir neutraliser
  // cette broche en setup() quand le capteur n'est pas câblé.
  #define DS18B20_PIN 2 // Pin 2

  #ifdef CAPTEUR_DS18B20
    // Etat batterie (défini même si CAPTEUR_BATTERIE est désactivé : sert de valeur
    // par défaut dans CBatterieAA::loadFromNVS(), toujours compilé)
    #define BATTERIE_PIN    3 // Pin 3
  #else
    // Etat batterie
    // Partage la broche du DS18B20 (2) quand celui-ci est désactivé : voir la neutralisation
    // des broches inutilisées dans setup() (main.cpp), qui tient compte de ce partage.
    #define BATTERIE_PIN    2 // Pin 2
  #endif // CAPTEUR_DS18B20

  // Toujours définie (même si FLOTTEUR_VERTICAL est désactivé) pour pouvoir neutraliser
  // cette broche en setup() quand le flotteur n'est pas câblé.
  #define DEFAULT_TOR_PIN 1 // Pin 1

  // Détecteur jour/nuit (sortie analogique). Entrée ADC1 obligatoire (GPIO1 à 10 sur le S3 ;
  // ADC2 inutilisable avec le Wi-Fi). D3 (GPIO4) est libre : ni broche de démarrage, ni Wio-SX1262
  // (qui prend D8-D10 = GPIO7-9 et les broches 38-42).
  // Toujours définie (même si CAPTEUR_JOUR_NUIT est désactivé) pour pouvoir neutraliser
  // cette broche en setup().
  #define JOUR_NUIT_PIN 4 // XIAO D3 (ADC1_CH3)
  // Alimentation du pont de la photorésistance (coupée hors mesure et en deep sleep).
  // D2 (GPIO3), voisine de D3 : libre sans DS18B20 (sinon BATTERIE_PIN = 3, voir le #error).
  #define JOUR_NUIT_ALIM_PIN 3 // XIAO D2

  // Toujours définies (même si CAPTEUR_RGB_TCS34725 / LED_CAPTEUR_RGB sont désactivés) pour
  // pouvoir neutraliser ces broches en setup().
  #define CAPTEUR_RGB_TCS34725_INTERRUPT 43 // Optionnel
  #define LED_CAPTEUR_RGB_PIN 44

  #ifdef ACTIONNEUR_IR
    #error "ACTIONNEUR_IR : broches non définies pour l'ESP32-S3"
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
    #error "CAPTEUR_MICRO_MAX4466 : broche non définie pour l'ESP32-S3"
  #endif
  #if defined(CAPTEUR_JOUR_NUIT) && defined(CAPTEUR_DS18B20)
    #error "ESP32-S3 : GPIO3 utilisée à la fois par l'alimentation du détecteur jour/nuit et par la batterie (BATTERIE_PIN = 3 avec CAPTEUR_DS18B20)"
  #endif

#else
  #error "Carte non supportée : ni ESP32-C3 ni ESP32-S3 détectée"
#endif // #if defined(__ESP32_C3__)

// -----------------------------------------------------------------------------
// Contrôle des conflits de broches du micro MAX4466 et de l'actionneur IR (ESP32-C3)
// -----------------------------------------------------------------------------
// Câblage XIAO : micro GPIO3, LED bleue GPIO7, IR GPIO20, LED rouge GPIO10 (sans conflit).
#if defined(__ESP32_C3__)
#ifdef CAPTEUR_MICRO_MAX4466
  #ifdef _RCSWITCH_MODE_
    #error "CAPTEUR_MICRO_MAX4466 : GPIO3 (micro) et GPIO7 (LED bleue) déjà utilisées par le CC1101 (GDO0, CS)"
  #endif
  #if defined(CAPTEUR_BATTERIE) && defined(CAPTEUR_DS18B20)
    #error "CAPTEUR_MICRO_MAX4466 : GPIO3 déjà utilisée par CAPTEUR_BATTERIE (BATTERIE_PIN = 3 avec CAPTEUR_DS18B20)"
  #endif
#endif // CAPTEUR_MICRO_MAX4466
#ifdef ACTIONNEUR_IR
  #ifdef _RCSWITCH_MODE_
    #error "ACTIONNEUR_IR : GPIO20 déjà utilisée par l'alimentation du CC1101 (CC1101_POWER_GND_GPIO)"
  #endif
  #if !defined(BOARD_XIAO_C3) && defined(CAPTEUR_JOUR_NUIT)
    #error "ACTIONNEUR_IR : GPIO10 (LED rouge) déjà utilisée par l'alimentation du détecteur jour/nuit (Super Mini)"
  #endif
#endif // ACTIONNEUR_IR
#endif // __ESP32_C3__

#ifdef _RCSWITCH_MODE_
    #define CC1101_ON   HIGH 
    #define CC1101_OFF  LOW 
#endif // _RCSWITCH_MODE_
#ifdef FLOTTEUR_VERTICAL
    #define DEFAULT_TOR_PULL_UP_DOWN_MODE INPUT_PULLUP
    // Réveil du deep sleep par le flotteur, dans les deux sens (cuve pleine, cuve vide) : ESP32-S3 seulement (EXT1).
    // Valeurs par défaut ; réglables par la page Web et par SET (mfv_rev, mfv_revmin, mfv_ack, mfv_ackn).
    #define FLOTTEUR_REVEIL_DEFAUT          true    // Réveil par le flotteur actif
    #define FLOTTEUR_REVEIL_MIN_S_DEFAUT    300     // Anti-rafale : délai minimal entre deux réveils par le flotteur (s)
    #define FLOTTEUR_REVEIL_MIN_S_MAX       86400
    // Confirmation par le contrôleur principal (« <nom du flotteur> ACK ») de chaque trame du flotteur, en LoRa
    #define FLOTTEUR_ACK_DEFAUT             true    // Confirmation attendue avant le deep sleep
    #define FLOTTEUR_ACK_ESSAIS_DEFAUT      3       // Envois de la trame sans confirmation avant de dormir quand même
    #define FLOTTEUR_ACK_ESSAIS_MAX         10
    #define FLOTTEUR_ACK_ATTENTE_MS         2000    // Attente de la confirmation après chaque envoi (ms)
#endif

// -----------------------------------------------------------------------------
// Seuils d'envoi des mesures (voir MemoireRTC.h)
// -----------------------------------------------------------------------------
// Hors forçage, une mesure n'est envoyée que si elle s'écarte de la dernière valeur ENVOYÉE (gardée en mémoire RTC
// pendant le deep sleep) d'au moins ce seuil. Le forçage (page Web : « Intervalle forçage remontée », en secondes)
// envoie de toute façon la mesure au plus tard après ce délai. Seuils trop fins : le bruit du capteur fait partir la
// mesure à presque chaque réveil (constaté sur ThChRDC le 15/11/2026 avec l'humidité au dixième).
#define SEUIL_ENVOI_TEMP_DHT20      0.2f    // °C
#define SEUIL_ENVOI_HUM_DHT20       5.0f    // % d'humidité relative
#define SEUIL_ENVOI_BATTERIE_V      0.05f   // V

// -----------------------------------------------------------------------------
// Délai d'alerte des capteurs (voir CapteurBase.h)
// -----------------------------------------------------------------------------
// Chaque mesure porte « alerte:<s> » : durée sans nouvelles de ce capteur au-delà de laquelle le contrôleur (CYD)
// le signale. Calculé par le nœud : plus long écart normal entre deux envois (forçage arrondi au réveil suivant)
// + retard toléré (page Web, par capteur) + marge. Le contrôleur ne peut donc jamais alerter sur un nœud qui suit
// son rythme normal.
#define DELAI_ALERTE_MARGE_S            60      // Marge fixe : réveil, connexion, relais LoRa, dérive de l'horloge
#define DELAI_ALERTE_EVEIL_S            10      // Durée d'éveil estimée par cycle de deep sleep (ajoutée à chaque sommeil)
#define DELAI_ALERTE_RETARD_DEFAUT_S    0       // Retard toléré par défaut (s) : alerte dès qu'un envoi manque
#define DELAI_ALERTE_RETARD_MAX_S       604800  // Retard toléré maximal (7 jours)
// Forçage réglable par SET <préfixe>force (CCapteurBase::setParamAlerte(), mêmes bornes que jn_force)
#define FORCAGE_MIN_S                   60
#define FORCAGE_MAX_S                   86400

// Taille maximale d'un paquet LoRa (SX1262 : 255 octets), marqueurs START_STOP (2 × 10) et IP éventuelle compris :
// au-delà, la réponse CONFIG est découpée en « <nom> CONFIG ... » puis « <nom> CONFIGSUITE ... » (CConfig::envoieConfig()
// retire elle-même les marqueurs et l'IP : texte utile ≤ 235 octets sans Wi-Fi). Un paquet plus long est refusé par la
// radio (CMyLoraRxTx::sendPacket() repasse alors en réception)
#define LORA_PAQUET_MAX                 255
// Attente avant de traiter (et donc de répondre à) une commande reçue par LoRa : le relais peut encore émettre
// (un paquet court dure ~100 ms en SF7 / 125 kHz) et n'écoute pas pendant ce temps
#define LORA_DELAI_REPONSE_MS           300
// Fenêtre d'écoute des commandes après la dernière émission, avant le deep sleep (LoRa seul ; Wi-Fi : 1 s, MyConfig.h).
// Doit couvrir : silence attendu par le relais (FILE_CMD_SILENCE_MS = 600 ms) + durée de la commande (~200 ms).
// Relancée par toute émission (réponse à une commande). Avant le 16/11 : 1000 ms
#define LORA_FENETRE_ECOUTE_MS          1500
// Écoute avant émission (CAD, attendCanalLibre() dans MyLoraRxTx.cpp) : canal occupé → attente aléatoire entre MIN et MAX
// puis nouvel essai, ESSAIS fois au plus (puis émission quand même). Une trame dure ~100 à 390 ms en SF7 / 125 kHz
#define LORA_CAD_ESSAIS                 8
#define LORA_CAD_ATTENTE_MIN_MS         50
#define LORA_CAD_ATTENTE_MAX_MS         300

// -----------------------------------------------------------------------------
// Macro pour logs de debug (optionnel)
// -----------------------------------------------------------------------------
#if defined(DEBUG) || defined(_DEBUG)
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

// Espace de nom pour l'enregistrement NVS
#define NVS_NAME_SPACE "ThCh1er"

// Version du logiciel
#define VERSION "4.1"

// Chien de garde de loop() : un tour de boucle plus long redémarre la puce. Plus long que la plus longue attente
// prévue dans loop() (reconnexion Wi-Fi : WIFI_TIMEOUT_RAPIDE_MS + WIFI_TIMEOUT_MS = 18 s)
#define LOOP_WDT_TIMEOUT_S 30

#endif // __GLOBAL_H__
