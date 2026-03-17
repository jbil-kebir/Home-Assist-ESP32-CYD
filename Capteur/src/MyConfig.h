#ifndef CCONFIG_H
#define CCONFIG_H

#include "global.h"
#include <Preferences.h>
#include <Arduino.h>
#include <WebServer.h>
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
#include "MyBatterieAA.h"
#include "MyWifi.h"
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
 CBatterieAA *mBatterieAA=nullptr;
  String nomEquipement = "ThCh1er";
  // === MQTT ===
  //MQTT_DEF  mqqtInfo;
  // === WIFI ===
  CWifi  *mWifi=nullptr;
  CMyDateTime *mDateTime=nullptr;


  // === PRÉFIXE DOMOTIQUE (pour projecteur, guirlande, sdb) ===
  const char* default_domotique_topic_prefix = "home/";
  String domotique_prefix;




  String mqttSubTopic; // configuration
  String  topic_config_command, // home/configuration/
          topic_config_state; 
  // Temps entre deux mesures (en secondes)
  const unsigned int DEFAULT_SLEEP_DURATION_SEC = 120;   // 2 minutes par défaut  
  const unsigned int DEFAULT_WAKE_DURATION_SEC = 20;   // 20 secondes par défaut  
  unsigned int mulSleepDuration = 120;  // Durée deep-sleep
  unsigned int mulWakeDuration = 20; // Durée avant un deep sleep. Permet de traiter d'éventuelles requêtes MQTT 
  bool mbDeepSleepActive = false;  
  bool mbWakeFromDeepSleep = false; // Variable qui indiquera qu'on s'est réveillé d'un deep sleep
  unsigned long mulDateMiseEnSommeil=0; // en s. Enregistré dans le NVS. Valeur absolue depuis 01/01/1970.
  unsigned long mulDateReveil=0L; // en s. Enregistré dans le NVS. Valeur absolue depuis 01/01/1970.
  unsigned long mulNbSecondesDeSommeil=0L; // Différence entre les deux précédents = durée du dernier sommeil
 
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
  int  handleMqttCommand(const String& payload);
  void setSleepIntervalle(unsigned long st);
  void setWakeIntervalle(unsigned long st);
  void setDeepSleep(bool active);
  void enterDeepSleep();
  String getHTML();
  void print() const;
};

#endif