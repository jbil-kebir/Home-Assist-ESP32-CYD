// MyMax4466.h
#pragma once

#ifndef __CMAX4466_H__
#define __CMAX4466_H__
#include "global.h"
#include <functional>
#include <Preferences.h>
#include <Arduino.h>
#include <WebServer.h>
#include <arduinoFFT.h>   // bibliothèque : kosme/arduinoFFT
#include "MyDateTime.h"

#ifdef CAPTEUR_MICRO_MAX4466

//----------------------------------------------------------------------------
// Détection de bips par micro MAX4466 (repris de Programmes unitaires/CalibrageMax4466)
//
// Le ton du bip (~3950 Hz) est recherché par FFT sur des blocs de MAX4466_NB_ECHANTILLONS
// échantillons acquis à MAX4466_FREQ_ECHANTILLONNAGE (256 à 16 kHz = bloc de 16 ms).
// Bip court (~130 ms) → ON (1). Bip long (~390 ms) → OFF (0).
//
// Publication : "<nomEquipement> OnOff <date> <heure> 0|1" sur home/<subtopic>state
// (format CTor, reconnu par CRemoteTor de Home-Assist).
// Ne JAMAIS publier l'état sur home/<subtopic>command : Home-Assist l'interprète comme
// un ordre de bascule RF (→ bip → boucle infinie).
//
// L'écoute est continue : le capteur actif empêche le deep sleep (voir CConfig::loop()).
//----------------------------------------------------------------------------
#define MAX4466_NB_ECHANTILLONS       256      // Puissance de 2 obligatoire (FFT)
#define MAX4466_FREQ_ECHANTILLONNAGE  16000.0f // Hz — tenable par analogRead() sur C3

// Valeurs par défaut issues du calibrage
#define MAX4466_DEFAULT_FREQ_CIBLE    3950     // Hz (2ème harmonique dominant)
#define MAX4466_DEFAULT_FREQ_TOL      200      // ±Hz
#define MAX4466_DEFAULT_SEUIL         500      // Amplitude FFT min — bruit ambiant ~80-200, bips réels >700
#define MAX4466_DEFAULT_COURT_MIN     80       // ms
#define MAX4466_DEFAULT_COURT_MAX     280      // ms → ON
#define MAX4466_DEFAULT_LONG_MIN      300      // ms
#define MAX4466_DEFAULT_LONG_MAX      750      // ms → OFF
#define MAX4466_DEFAULT_SILENCE_MIN   4000     // ms entre deux bips de même type (la télécommande RF répète ~3x le code)
#define MAX4466_DUREE_FLASH_LED       150      // ms d'allumage de la LED témoin sur un bip valide

// Structure utilisée par handleMqttCommand()
struct MQTT_COMMAND_5 {
  String sExpediteur="";
  String sCommand="";
  String sArg="";
};


class CMax4466 {
private:
  bool _initialized = false;
  unsigned char mucPin = CAPTEUR_MICRO_MAX4466_PIN;
  // en secondes : Intervalle de forcage de la remontée de l'état, même s'il n'a pas changé
  unsigned long mulIntervalleForcageRemonteeMesure = 600L, mulDefaultIntervalleForcageRemonteeMesure = 600L;
  // Paramètres de détection (NVS + page Web)
  unsigned int muiFreqCible = MAX4466_DEFAULT_FREQ_CIBLE;
  unsigned int muiFreqTol = MAX4466_DEFAULT_FREQ_TOL;
  unsigned int muiSeuil = MAX4466_DEFAULT_SEUIL;
  unsigned int muiCourtMin = MAX4466_DEFAULT_COURT_MIN;
  unsigned int muiCourtMax = MAX4466_DEFAULT_COURT_MAX;
  unsigned int muiLongMin = MAX4466_DEFAULT_LONG_MIN;
  unsigned int muiLongMax = MAX4466_DEFAULT_LONG_MAX;
  unsigned int muiSilenceMin = MAX4466_DEFAULT_SILENCE_MIN;
  bool mbTrace = false; // Affiche l'amplitude toutes les secondes (aide au réglage du seuil)
  Preferences prefs;

  // Acquisition / FFT (float : le C3 n'a pas de FPU, le calcul en double serait plus lent)
  float vReal[MAX4466_NB_ECHANTILLONS];
  float vImag[MAX4466_NB_ECHANTILLONS];
  ArduinoFFT<float> FFT;

  // Machine d'état de détection des bips
  bool mbBipActif = false;
  unsigned long mulBipStart = 0L;
  unsigned long mulLastOnMs = 0L;   // anti-répétition RF indépendant par type
  unsigned long mulLastOffMs = 0L;
  unsigned long mulLastForcage = 0L;

  // LED témoin (optionnelle) : allumée brièvement à chaque bip valide
  int miLedPin = -1;                    // -1 = pas de LED
  unsigned long mulLedAllumeeDepuis = 0L;
  bool mbLedAllumee = false;
  void flashLed();
  void gereLed();

  int miLastVal = -1; // -1 = état inconnu (aucun bip reçu), 0 = OFF, 1 = ON
  int miNewVal = -1;
  const char* default_domotique_topic_prefix = "home/";
  const char* nvs_namespace = NVS_NAME_SPACE;  //
  String mPrefixNVS = "mic_";  // Préfixe par défaut
  CMyDateTime *mDateTime=nullptr;

  void acquireSamples();
  float getTargetAmplitude();
  void saveEtatToNVS();

public:
    CMax4466(CMyDateTime& dateTime,
        std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) :
        FFT(vReal, vImag, MAX4466_NB_ECHANTILLONS, MAX4466_FREQ_ECHANTILLONNAGE),
        mDateTime(&dateTime),
        onMqttPublish(cbonMqttPublish)
        {}
  String domotique_prefix;
  String nomEquipement = "Noeud";
  String mqttSubTopic = "noeud/";
  bool active = false;
  bool mbMesureRemontee = false;
  String mqttSubTopicCommand;
  String mqttSubTopicState;

  // MQTT callback
  std::function<int(const char*, const char*)> onMqttPublish;
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void setActive(bool state);
  void setPrefixNVS(const char* pr) {mPrefixNVS = pr;}
  void setLedPin(int pin); // LED témoin de détection, initialisée à l'état bas
  String getHTML();

  bool setup(const String pref);
  int loop();

  void print() const;
  int getLastMesure() const;

  int handleMqttCommand(const String& payload);
  bool publieSurMqtt(bool force=false);
  int publie(bool force=false);
};
#endif // CAPTEUR_MICRO_MAX4466
#endif // __CMAX4466_H__
