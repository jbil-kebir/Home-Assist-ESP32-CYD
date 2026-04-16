#ifndef __GLOBAL_H__
#define __GLOBAL_H__

//----------------------------------------------------------------------------
// Si cette ligne est commentée, le CC1101 et le capteur de température doivent 
// être présents.
// Sinon, tout est à distance.
//----------------------------------------------------------------------------
#define __LOCAL_MODE__
//----------------------------------------------------------------------------
// Ligne à décommenter si le module est un écran CYD
// Si cette ligne est décommentée, il faut aussi que #define __LOCAL_MODE__ le soit
//----------------------------------------------------------------------------
#define __CYD__
//----------------------------------------------------------------------------
// Si ce commutateur n'est pas commenté, il n'y a pas d'émission RF mais 
// tout le processus de commande est exécuté.
// Ce commutateur ne devrait pas être actif si __LOCAL_MODE__ ne l'est pas.
//----------------------------------------------------------------------------
//#define DISABLE_EFFECTEUR

//----------------------------------------------------------------------------
// Switch pour le DS18B20 local. Voir plus loin la Pin
// Ne l'activer qu'en mode __LOCAL_MODE__ 
//----------------------------------------------------------------------------
#define __LOCAL_DS18B20__

//----------------------------------------------------------------------------
// En cas de perte de la configuration, décommenter la ligne suivante
// et remplissez les champs ssid et passwd par vos propres données.
//
// Une fois le Wifi connecté, connectez-vous par Web, reparamétrez les champs
// et revenez ici effacer vos données
//----------------------------------------------------------------------------
//#define FORCE_CONNEXION_WIFI

#ifdef FORCE_CONNEXION_WIFI
#define FORCE_WIFI_SSID  "xxxxxxxx"
#define FORCE_WIFI_PASSWD "xxxxxxx"
#define FORCE_MQTT_SERVER "119.177.244.151"
#define FORCE_MQTT_PORT 1883
#define FORCE_MQTT_USER "ubu_ntu"
#define FORCE_MQTT_PWD "ffffffffff*"
#endif

// Pins écran
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define TFT_BL 21

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17


#define LED_ON LOW
#define LED_OFF HIGH

#ifdef __LOCAL_MODE__

#define CC1101_GDO0 22
#define CC1101_CS 27
#define CC1101_MOSI 23
#define CC1101_MISO 19
#define CC1101_SCK 18


// Thermomètre DS18B20
#ifdef __LOCAL_DS18B20__
#define DEFAULT_DS18B20_PIN 5
#endif

#define FREQUENCE 434.038

#endif // __LOCAL_MODE__




// Espace de nom pour l'enregistrement NVS
#define NVS_NAME_SPACE "ad200"

// Constantes pour la gestion des communications RF
// Identifiants capteurs
#define ID_NONE                 0
#define ID_CAPTEUR_TH_CAVE      1
#define ID_CAPTEUR_TH_SDB       2
#define ID_CAPTEUR_TH_CH_1ER    3
#define ID_CAPTEUR_TH_RDC       4
// Types de mesures
#define MESURE_TEMPERATURE  0
#define MESURE_HUMIDITE  1
#define MESURE_TENSIOIN  2


// Version du logiciel
#define VERSION "2.8"


//----------------------------------------------------------------------------
// Système de debug par catégories (bitmask cumulatif)
// Usage dans main.cpp : gDebugFlags = DBG_MQTT | DBG_CAPTEURS;
//----------------------------------------------------------------------------
#include <Arduino.h>

constexpr uint32_t DBG_NONE        = 0;
constexpr uint32_t DBG_MQTT        = 1 << 0;
constexpr uint32_t DBG_CAPTEURS    = 1 << 1;
constexpr uint32_t DBG_ACTIONNEURS = 1 << 2;
constexpr uint32_t DBG_CHAUDIERE   = 1 << 3;
constexpr uint32_t DBG_CONFIG      = 1 << 4;
constexpr uint32_t DBG_RESEAU      = 1 << 5;
constexpr uint32_t DBG_ECRAN       = 1 << 6;
constexpr uint32_t DBG_ALL         = 0xFFFFFFFF;

extern uint32_t gDebugFlags;

// Macro principale (style printf)
#define DBG(cat, ...)   do { if (gDebugFlags & (cat)) Serial.printf(__VA_ARGS__); } while(0)
// Macro pour Serial.println(str) — ajoute \n automatiquement
#define DBGLN(cat, msg) do { if (gDebugFlags & (cat)) { Serial.println(msg); } } while(0)

#endif // __GLOBAL_H__
