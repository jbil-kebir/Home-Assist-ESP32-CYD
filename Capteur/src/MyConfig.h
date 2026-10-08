#ifndef CCONFIG_H
#define CCONFIG_H

#include "global.h"
#include <Preferences.h>
#include <Arduino.h>
#include <WebServer.h>
#include <map>
#ifdef CAPTEUR_DS18B20
#include "MyDS18B20.h"
#endif
#ifdef CAPTEUR_DHT20
#include "DHT20Sensor.h"
#endif
#ifdef FLOTTEUR_VERTICAL
#include "MyTor.h"
#endif
#ifdef CAPTEUR_RGB_TCS34725
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "DetecteurRGB_TCS34725.h"
#endif
#ifdef CAPTEUR_BATTERIE
#include "MyBatterieAA.h"
#endif
#ifdef CAPTEUR_MICRO_MAX4466
#include "MyMax4466.h"
#endif
#ifdef ACTIONNEUR_IR
#include "MyActionneurIR.h"
#endif
#include "MyWifi.h"
#include "MyLoraRxTx.h"
#include "MyDateTime.h"

class CWifi;
class DHT20Sensor;
class CTor;

#define CONFIG_SUB_TOPIC "configuration/"

class CConfig {
private:
  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;
  String mPrefixNVS = "cfg_";  // Préfixe par défaut

public:
  CConfig(const String& nomEqu, CMyDateTime& dateTime, 
    std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mDateTime(&dateTime), onMqttPublish(cbonMqttPublish) {
    nomEquipement = nomEqu;
  };
  // Thermomètre
  #ifdef CAPTEUR_DS18B20
  MyDS18B20 *ds18b20=nullptr;
  #endif
  #ifdef CAPTEUR_DHT20
  DHT20Sensor *dht20=nullptr;
  #endif
  #ifdef FLOTTEUR_VERTICAL
  CTor *mFlotteurVertical=nullptr;  
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  CDetecteurRGB_TCS34725 *mCapteurRGB=nullptr;
  #endif
  #ifdef CAPTEUR_BATTERIE
  CBatterieAA *mBatterieAA=nullptr;
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
  CMax4466 *mMicro=nullptr;
  #endif
  #ifdef ACTIONNEUR_IR
  CActionneurIR *mActionneurIR=nullptr;
  #endif
  String nomEquipement = "ThCave";
  // === MQTT ===
  //MQTT_DEF  mqqtInfo;
  // === WIFI ===
  CWifi  *mWifi=nullptr;
  CMyDateTime *mDateTime=nullptr;
#ifdef _LORA_P2P_MODE_
  CMyLoraRxTx *mLoraRxTx=nullptr; 
#endif

  // === PRÉFIXE DOMOTIQUE (pour projecteur, guirlande, sdb) ===
  const char* default_domotique_topic_prefix = "home/";
  String domotique_prefix;



  String mqttSubTopic; // configuration
  String  topic_config_command, // home/configuration/
          topic_config_state; 
  // Temps entre deux mesures (en secondes)
  const unsigned int DEFAULT_SLEEP_DURATION_SEC = 120;   // 2 minutes par défaut  
  const unsigned int DEFAULT_SLEEP_DURATIONS_SEC = 120;   // 2 minutes par défaut  
  const unsigned int DEFAULT_SLEEP_DURATIONM_SEC = 350;   // 5 minutes par défaut  
  const unsigned int DEFAULT_SLEEP_DURATIONL_SEC = 600;   // 10 minutes par défaut  
  const unsigned int DEFAULT_WAKE_DURATION_SEC = 20;   // 20 secondes par défaut  
  unsigned int mulSleepDuration = DEFAULT_SLEEP_DURATION_SEC;  // Durée deep-sleep
  // Trois valeurs qui peuvent paramétrées en appelant un mot clé SLEEPS, SLEEPM ou SLEEPL suivi de la durée en secondes. Par exemple SLEEEPS 60 pour 1 minute de sommeil
  // Pour les invoquer, il suffit de faire home/confthmesure/command SLEEPL par exemple 
  unsigned int mulSleepDurationS = DEFAULT_SLEEP_DURATIONS_SEC;  // Durée deep-sleep
  unsigned int mulSleepDurationM = DEFAULT_SLEEP_DURATIONM_SEC;  // Durée deep-sleep
  unsigned int mulSleepDurationL = DEFAULT_SLEEP_DURATIONL_SEC;  // Durée deep-sleep

  unsigned int mulWakeDuration = DEFAULT_WAKE_DURATION_SEC; // Durée avant un deep sleep. Permet de traiter d'éventuelles requêtes MQTT 
  bool mbDeepSleepActive = false;  
  bool mbWakeFromDeepSleep = false; // Variable qui indiquera qu'on s'est réveillé d'un deep sleep
  unsigned long mulDateMiseEnSommeil=0; // en s. Enregistré dans le NVS. Valeur absolue depuis 01/01/1970.
  unsigned long mulDateReveil=0L; // en s. Enregistré dans le NVS. Valeur absolue depuis 01/01/1970.
  unsigned long mulNbSecondesDeSommeil=0L; // Différence entre les deux précédents = durée du dernier sommeil

  // === WATCHDOG ALIVE ===
  // Pour chaque capteur actif resté silencieux pendant mulWatchdogPeriod secondes, on publie (MQTT et LoRa)
  // "<nomEquipement> ALIVE <date> <heure>" (ou "ALIVE UPTIME <s>" si l'heure n'est pas synchronisée) sur son topic d'état.
  // Côté CYD, tout message reçu d'un équipement remet son watchdog à zéro.
  // Commandes (topic de configuration) : WDOG ENABLE, WDOG DISABLE, WDOG <s> (fixe la période et active)
  const unsigned int DEFAULT_WATCHDOG_PERIOD_SEC = 20;
  bool mbWatchdogActive = false; // Désactivé par défaut : inutile pour un capteur qui publie régulièrement. Ignoré en deep sleep.
  unsigned int mulWatchdogPeriod = DEFAULT_WATCHDOG_PERIOD_SEC; // en secondes
  // Sauvegarde du journal web en NVS avant une coupure annoncée (OnOff 0, IRKO) : voir MyJournal.h
  bool mbJournalSauvegarde = true;
  struct EtatWatchdog {
    unsigned long ulDerniereEmission = 0;   // millis() de la dernière publication de l'équipement
    bool bAliveDemarrageEnvoye = false;     // ALIVE de démarrage publié avec succès (MQTT connecté ou LoRa)
    bool bAliveDemarrageTente = false;
    unsigned long ulDerniereTentative = 0;  // millis() de la dernière tentative d'ALIVE de démarrage
  };
  std::map<String, EtatWatchdog> mEtatsWatchdog; // nomEquipement → état de son watchdog

  // === PUCE NON CONFIGURÉE ===
  // Puce neuve (NVS vide) : tant que la configuration n'a pas été enregistrée depuis la page Web,
  // on publie toutes les 60 s sur le topic de configuration "NONCONFIGURE <modèle>-<fin MAC> <date> <heure> <IP>"
  // pour que le CYD affiche l'adresse IP de la puce.
  // NVS "conf" : écrit par saveToNVS() (y compris avant deep sleep, d'où une clé dédiée plutôt que "nom").
  // Absente (puce antérieure à cette clé) : configurée si "nom" existe.
  bool mbConfiguree = false;
  bool mbAnnonceNonConfigureeEnvoyee = false;
  unsigned long mulDerniereAnnonceNonConfiguree = 0;

  // MQTT callback pour le deep-sleep
  std::function<int(const char*, const char*)> onMqttPublish;    
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT
  //std::function<void()> onMqttPowerDown;    
  //void setMqttPowerDownCallback(std::function<void()> cbMqttPowerDown); // Pour Power down MQTT

  void setup();
  void traiteReveil();
  void loop();
  void setWifi(CWifi  *wifiInfo) {mWifi = wifiInfo;}
  void setPrefixNVS(const char* pr) {mPrefixNVS = pr;}
  void loadFromWebServer (WebServer& server);
  void loadFromNVS();
  void saveToNVS();
  int parseSleepCommand(const String& msg);
  int parseSleepCommandSML(const String& msg);
  int  handleMqttCommand(const String& payload);
  void setSleepIntervalle(unsigned long st);
  void setWakeIntervalle(unsigned long st);
  void setDeepSleep(bool active);
  void enterDeepSleep();
  int  parseWdogCommand(const String& msg);
  void setWatchdog(bool active);
  void setWatchdogPeriod(unsigned long st);
  void noteEmission(const String& message, bool bAvecTopic = false);
  String getWatchdogStatus() const;
  bool loopWatchdog();
  bool verifieWatchdog(const String& nom, const String& topic);
  void envoieAlive(const String& nom, const String& topic);
  void annonceNonConfiguree();
  String getHTML();
  void print() const;
};

#endif