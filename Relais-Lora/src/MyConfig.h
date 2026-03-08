#ifndef __MY_CONFIG_H__
#define __MY_CONFIG_H__

#include <Preferences.h>
#include <Arduino.h>
#include <WebServer.h>
#include "global.h"
#include "MyWifi.h"
#include "MyDateTime.h"

class CWifi;

#define CONFIG_SUB_TOPIC "configuration/"
#define CONFIG_SUB_TOPIC_MAITRE "configuration/"

#define NOM_EQUIPEMENT "Relais Lora"

class CConfig {
private:
  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;

public:
  CConfig(const String& nomEqu) {
    nomEquipement = nomEqu;
  };

  // === WIFI ===
  CWifi  *mWifi=nullptr;

  CMyDateTime *mDateTime=nullptr;
  String mPrefixNVS = "cfg_";  // Préfixe par défaut

  // === PRÉFIXE DOMOTIQUE (pour projecteur, guirlande, sdb) ===
  const char* default_domotique_topic_prefix = "home/";
  String domotique_prefix; // "home/"

  String nomEquipement = NOM_EQUIPEMENT;

  String mqttConfigSubTopic; // configuration/ ou configaux/ ...
  String  topic_config_command, // home/configuration/command
          topic_config_state; // home/configuration/state

  String mqttConfigMaitreSubTopic; // configuration/ Pour pouvoir s'adresser au maître
  String  topic_config_maitre_command; // home/configuration/command

  // Deep sleep
  const unsigned int DEFAULT_SLEEP_DURATION_SEC = 120;   // 2 minutes par défaut  
  const unsigned int DEFAULT_WAKE_DURATION_SEC = 20;   // 20 secondes par défaut  
  unsigned int mulSleepDuration = 120;  
  unsigned int mulWakeDuration = 20; // Durée avant un deep sleep. Permet de traiter d'éventuelles requêtes MQTT 
  bool mbDeepSleepActive = false;  
  bool mbWakeFromDeepSleep = false; // Variable qui indiquera qu'on s'est réveillé d'un deep sleep
  unsigned long mulDateMiseEnSommeil=0L; // en s. Enregistré dans le NVS. Valeur absolue depuis 01/01/1970.
  unsigned long mulDateReveil=0L; // en s. Enregistré dans le NVS. Valeur absolue depuis 01/01/1970.
  unsigned long mulNbSecondesDeSommeil=0L; // Différence entre les deux précédents = durée du dernier sommeil
  
  // MQTT callback pour le deep-sleep
  std::function<int(const char*, const char*)> onMqttPublish;    
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish) {onMqttPublish = cbMqttPublish;}; // Pour publication MQTT

  void setup(const String pref);
  void loop();
  void enterDeepSleep();
  void traiteReveil();
  void setWifi(CWifi  *wifiInfo) {mWifi = wifiInfo;}
  void loadFromWebServer (WebServer& server);
  void loadFromNVS();
  void saveToNVS();
  void handleMqttCommand(const String& payload);
  void setSleepTimeout(int32_t st);
  void print() const;
  String getHTML();
};

#endif // __MY_CONFIG_H__