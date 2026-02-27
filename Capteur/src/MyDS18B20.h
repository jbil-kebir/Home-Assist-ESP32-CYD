#ifndef MYDS18B20_H
#define MYDS18B20_H

#include <functional>
#include <OneWire.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DallasTemperature.h>
#include "global.h"
#include "MyDateTime.h"

// Structure utilisée par handleMqttCommand()
struct MQTT_COMMAND {
  String sExpediteur="";
  String sCommand="";
  String sArg="";
};

class MyDS18B20 {
private:
  Preferences prefs;
  OneWire oneWire;
  DallasTemperature sensors;
  unsigned char mucPin;
  // On mémorise la température pour n'afficher que les changements
  float lastTempC = -127.0;  // Valeur invalide par défaut
  float newTempC = -127.0;  // Valeur invalide par défaut
  const char* default_domotique_topic_prefix = "home/";
  const char* nvs_namespace = NVS_NAME_SPACE;  // 
  String mPrefixNVS = "th_";  // Préfixe par défaut
  CMyDateTime *mDateTime=nullptr;


public:
  MyDS18B20(uint8_t gpioPin, CMyDateTime& dateTime,
    std::function<int(const char*, const char*)> cbonMqttPublish = nullptr,
    std::function<int(const char*)> cbonLoraP2PPublish = nullptr) :
    mucPin(gpioPin), oneWire(gpioPin), sensors(&oneWire), mDateTime(&dateTime), 
    onMqttPublish(cbonMqttPublish), 
    onLoraP2PPublish(cbonLoraP2PPublish)
     {} 

  String domotique_prefix;
  String nomEquipement = "ThCh1er";
  String mqttSubTopic = "thermometre";
  bool active = true;
  bool mbMesureRemontee = false; // Permet d'empêcher le deep sleep tant qu'une mesure n'a pas été remontée
  //bool bLocal = true; // true : Equipement local, commandé par l'ESP32. False : distant, commandé par Mqtt
  String mqttSubTopicCommand;
  String mqttSubTopicState;
  unsigned long mulIntervalleMesure = 10, mulDefaultIntervalleMesure = 10; // en minutes : Intervalle entre deux mesures. En secondes pour les tests
  // en minutes : Intervalle de forcage de la rmontée de mesure, même si la valeur n'a pas changé. En secondes pour les tests
  unsigned long mulIntervalleForcageRemonteeMesure = 20, mulDefaultIntervalleForcageRemonteeMesure = 20; 
 
  // MQTT callback
  std::function<int(const char*, const char*)> onMqttPublish;    
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT
  // Lora P2P callback
  std::function<int(const char*)> onLoraP2PPublish;    
  void setLoraP2PPublishCallback(std::function<int(const char* payload)> cbLoraP2PPublish); // Pour publication Lora P2O

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void setActive(bool state);
  void setPrefixNVS(const char* pr) {mPrefixNVS = pr;}
  String getHTML();

  int begin(const String pref);
  int loop();
  bool readTemperature();
  float getLastTemperature() const;
  void print() const;
  void printTemperature() const;
  int handleMqttCommand(const String& payload);
  bool publieSurMqtt(bool force=false);
  bool publieParLoraP2P(bool force=false);
  int readAndPublish(bool force=false);
};

#endif