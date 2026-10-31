#ifndef __GLOBAL_H__
#define __GLOBAL_H__

//------------------------------------------------------------------------------
// Journal web (http://<IP>/logs) : Serial est redirigé vers gSerialJournal, qui écrit
// sur le port série ET dans le journal (voir MyJournal.h, même fichier que Capteur et Relais-Lora).
// Arduino.h doit être inclus avant le #define (il déclare le vrai Serial).
// Les fichiers qui n'incluent pas global.h écrivent seulement sur le port série.
//------------------------------------------------------------------------------
#include <Arduino.h>
#include "MyJournal.h"
#ifdef Serial
#undef Serial
#endif
#define Serial gSerialJournal

//----------------------------------------------------------------------------
// Contrôleur principal (__LOCAL_MODE__) : le CC1101 et le capteur de température
// doivent être présents. CYD auxiliaire : tout est à distance.
// Choisi par le profil de compilation de platformio.ini :
//   cyd-principal, principal-sans-cyd -> __LOCAL_MODE__ ; cyd-aux (-DCYD_AUXILIAIRE) -> auxiliaire.
//----------------------------------------------------------------------------
#ifndef CYD_AUXILIAIRE
#define __LOCAL_MODE__
//----------------------------------------------------------------------------
// Switch pour le DS18B20 local. Voir plus loin la Pin
// Ne l'activer qu'en mode __LOCAL_MODE__
// Si la ligne est commentée en mode __LOCAL_MODE__, le thermomètre local
// est un DHT20 (I2C). Voir plus loin les pins SDA/SCL.
//----------------------------------------------------------------------------
#define __LOCAL_DS18B20__
#endif
//----------------------------------------------------------------------------
// Module à écran CYD : tous les profils sauf principal-sans-cyd (-DSANS_CYD).
//----------------------------------------------------------------------------
#ifndef SANS_CYD
#define __CYD__
#endif
#if defined(SANS_CYD) && defined(CYD_AUXILIAIRE)
#error "Un CYD auxiliaire a forcement un ecran : SANS_CYD et CYD_AUXILIAIRE sont incompatibles"
#endif
//----------------------------------------------------------------------------
// Si ce commutateur n'est pas commenté, il n'y a pas d'émission RF mais 
// tout le processus de commande est exécuté.
// Ce commutateur ne devrait pas être actif si __LOCAL_MODE__ ne l'est pas.
//
// Modif : un commutateur active les émissions pour la chaudière, l'autre pour les RCSwitch. 
// Cela permet de n'avoir que la chaudière ou que les RCS désactivés, selon les besoins.
//----------------------------------------------------------------------------
//#define DISABLE_EFFECTEUR_CHAUDIERE
//#define DISABLE_EFFECTEUR_RC_SWITCH

//----------------------------------------------------------------------------
// En mode __LOCAL_MODE__, si le DS18B20 n'est pas utilisé, le DHT20 est utilisé.
//----------------------------------------------------------------------------
#if defined(__LOCAL_MODE__) && !defined(__LOCAL_DS18B20__)
#define __LOCAL_DHT20__
#endif

//----------------------------------------------------------------------------
// En cas de perte de la configuration, décommenter la ligne suivante
// Les identifiants Wi-Fi / MQTT sont dans secret.h (exclu de Git).
//
// Une fois le Wifi connecté, connectez-vous par Web et reparamétrez les champs.
//----------------------------------------------------------------------------
//#define FORCE_CONNEXION_WIFI

#ifdef FORCE_CONNEXION_WIFI
// Identifiants dans secret.h (exclu de Git) ; modèle : secret.exemple.h
#if __has_include("secret.h")
  #include "secret.h"
#else
  #include "secret.exemple.h"
#endif
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


// Thermomètre local : DS18B20 ou DHT20
#ifdef __LOCAL_DS18B20__
#define DEFAULT_DS18B20_PIN 5
#else
#define DEFAULT_DHT20_SDA_PIN 5
#define DEFAULT_DHT20_SCL_PIN 26
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


// Confirmation des trames des flotteurs (contrôleur principal, __LOCAL_MODE__) : « <nom> ACK » sur ce topic
#define TOPIC_CONFIRMATION_FLOTTEUR "home/thermometre/command"

// Version du logiciel
#define VERSION "3.5"


//----------------------------------------------------------------------------
// Système de debug par catégories (bitmask cumulatif)
// Usage dans main.cpp : gDebugFlags = DBG_MQTT | DBG_CAPTEURS;
// Modifiable en direct par les cases de la page /logs (non sauvegardé : valeur de main.cpp au redémarrage).
// Une trace DBG va sur Serial (donc dans le journal web) et est publiée sur MQTT home/log (MyLogger).
//----------------------------------------------------------------------------
#include <Arduino.h>
#include "MyLogger.h"

constexpr uint32_t DBG_NONE        = 0;
constexpr uint32_t DBG_MQTT        = 1 << 0;
constexpr uint32_t DBG_CAPTEURS    = 1 << 1;
constexpr uint32_t DBG_ACTIONNEURS = 1 << 2;
constexpr uint32_t DBG_CHAUDIERE   = 1 << 3;
constexpr uint32_t DBG_CONFIG      = 1 << 4;
constexpr uint32_t DBG_RESEAU      = 1 << 5;
constexpr uint32_t DBG_ECRAN       = 1 << 6;
constexpr uint32_t DBG_NOEUD       = 1 << 7; // Noeud chauffage SdB : séquence ON/OFF, présence
constexpr uint32_t DBG_ALL         = 0xFFFFFFFF;

extern uint32_t gDebugFlags;

// Macro principale (style printf)
#define DBG(cat, ...)   do { if (gDebugFlags & (cat)) { Serial.printf(__VA_ARGS__); gLogger.addf(__VA_ARGS__); } } while(0)
// Macro pour Serial.println(str) — ajoute \n automatiquement
#define DBGLN(cat, msg) do { if (gDebugFlags & (cat)) { Serial.println(msg); gLogger.add(String(msg).c_str()); } } while(0)

#endif // __GLOBAL_H__
