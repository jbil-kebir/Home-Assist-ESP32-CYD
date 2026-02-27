#ifndef CCONFIG_H
#define CCONFIG_H

#include <Preferences.h>
#include <Arduino.h>
#include <WebServer.h>
#include "MyRCDevice.h"
#ifdef __LOCAL_MODE__
#include "MyChaudiere.h"
#include "MyDS18B20.h"
#else
#include "RemoteChaudiere.h"
#include "RemoteRCDevice.h"
#endif
#include "global.h"
#include "MyWifi.h"
#include "RemoteTh.h"
#include "RemoteDHT20.h"
#include "RemoteBatAA.h"
#include "RemoteTor.h"
#include "MyDateTime.h"

class CWifi;
class CRemoteThermo;
class CRemoteDHT20;
class CRemoteBatterieAA;
class CRemoteTor;

#define CONFIG_SUB_TOPIC "configuration/"
#define CONFIG_SUB_TOPIC_MAITRE "configuration/"

#define NOM_EQUIPEMENT "Home Assistant"

class CConfig {
private:
  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;

public:
  CConfig(const String& nomEqu) {
    nomEquipement = nomEqu;
  };

#ifdef __LOCAL_MODE__
  CChaudiere *chaudiere=nullptr;
  // === APPAREILS 433 MHz ===
  CRCDevice *projecteur=nullptr;
  CRCDevice  *guirlande=nullptr;
  CRCDevice  *chauffageSb=nullptr;

  #ifdef __LOCAL_DS18B20__
  // Thermomètre
  MyDS18B20 *ds18b20=nullptr;
  #endif
#else // Chaudière, thermomètre principal et équipements 433 MHz distants
  CRemoteChaudiere *mRemoteChaudiere=nullptr;
  CRemoteRCDevice *mRemoteProjecteur=nullptr;
  CRemoteRCDevice *mRemoteGuirlande=nullptr;
  CRemoteRCDevice *mRemoteChauffage=nullptr;
  CRemoteThermo *mRemoteThMain=nullptr;
  CRemoteBatterieAA *mRemoteBatMain=nullptr;
#endif


  CRemoteThermo *mRemoteThCh1er=nullptr;
  CRemoteBatterieAA *mRemoteBatThCh1er=nullptr;

  CRemoteThermo *mRemoteThSdb=nullptr;
  CRemoteBatterieAA *mRemoteBatSdb=nullptr;

  CRemoteDHT20 *mRemoteThCave=nullptr;
  CRemoteTor *mRemoteTor=nullptr;
  CRemoteBatterieAA *mRemoteBatCave=nullptr;

  CRemoteDHT20 *mRemoteThNomade=nullptr;
  CRemoteBatterieAA *mRemoteBatNomade=nullptr;

  // === MQTT ===
  //MQTT_DEF  mqqtInfo;
  // === WIFI ===
  CWifi  *mWifi=nullptr;

  CMyDateTime *mDateTime=nullptr;
  String mPrefixNVS = "cfg_";  // Préfixe par défaut

  // === PRÉFIXE DOMOTIQUE (pour projecteur, guirlande, sdb) ===
  const char* default_domotique_topic_prefix = "home/";
  String domotique_prefix; // "home/"

  String nomEquipement = "Home Assistant";

  const bool default_boiler_state = false;
  const uint8_t default_enable_keep_alive = true;

  // Timings par défaut (10 cases, les dernières à 0)
  #ifdef __LOCAL_MODE__
  const uint32_t default_on_keepalive[MAX_KEEPALIVE] = {7, 14, 56, 85, 626, 0, 0, 0, 0, 0};
  const uint32_t default_off_keepalive[MAX_KEEPALIVE] = {30, 64, 77, 626, 0, 0, 0, 0, 0, 0};
  #endif

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
  //std::function<void()> onMqttPowerDown;    
  //void setMqttPowerDownCallback(std::function<void()> cbMqttPowerDown) {onMqttPowerDown = cbMqttPowerDown;}; // Pour Power down MQTT

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

#endif