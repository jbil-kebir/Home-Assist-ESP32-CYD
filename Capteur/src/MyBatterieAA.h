#ifndef __MYBATTERIE_H__
#define __MYBATTERIE_H__

#include <functional>
#include <WebServer.h>
#include <Preferences.h>
#include "global.h"
#include "MyDateTime.h"

// Structure utilisée par handleMqttCommand()
struct MQTT_COMMAND_2 {
  String sExpediteur="";
  String sCommand="";
  String sArg="";
};


class CBatterieAA {
private:
  Preferences prefs;
  unsigned char mucPin;
  // On mémorise la température pour n'afficher que les changements
  float mfTensionMin = -1.0;  // Seuil en dessous duquel la batterie est considérée HS
  float mLastTension = -1.0;  // Valeur invalide par défaut
  float newTension = -2.0;  // Valeur invalide par défaut
  float mCalibre = 4.38; // Entre 4.35 et 4.45
  float mDefaultCalibre = (100.0+330.0)/100.0; // Pont diviseur Vcc -> 330 K --> GPIO --> 100 K --> GND
  const char* default_domotique_topic_prefix = "home/";
  const char* nvs_namespace = NVS_NAME_SPACE;  // 
  String mPrefixNVS = "bt_";  // Préfixe par défaut
  CMyDateTime *mDateTime=nullptr;


public:
  CBatterieAA(uint8_t gpioPin, CMyDateTime& dateTime,
    std::function<int(const char*, const char*)> cbonMqttPublish = nullptr,
    std::function<int(const char*)> cbonLoraP2PPublish = nullptr) : 
    mucPin(gpioPin), mDateTime(&dateTime), 
    onMqttPublish(cbonMqttPublish), 
    onLoraP2PPublish(cbonLoraP2PPublish) 
    {} 

  String domotique_prefix;
  String nomEquipement = "Batt";
  String mqttSubTopic = "bat";
  bool active = true;
  bool mbMesureRemontee = false; // Permet d'empêcher le deep sleep tant qu'une mesure n'a pas été remontée
  //bool bLocal = true; // true : Equipement local, commandé par l'ESP32. False : distant, commandé par Mqtt
  String mqttSubTopicCommand;
  String mqttSubTopicState;
  unsigned long mulIntervalleMesure = 30UL, mulDefaultIntervalleMesure = 30UL; // en secondes : Intervalle entre deux mesures. En secondes pour les tests
  // en minutes : Intervalle de forcage de la rmontée de mesure, même si la valeur n'a pas changé. En secondes pour les tests
  unsigned long mulIntervalleForcageRemonteeMesure = 60UL, mulDefaultIntervalleForcageRemonteeMesure = 60UL; // En secondes
 
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

  int setup(const String pref);
  int loop();
  bool readTension();
  float getLastTension() const;
  void print() const;
  void printTension() const;
  int handleMqttCommand(const String& payload);
  bool publieSurMqtt(bool force=false);
  //bool publieParCC1101(bool force=false);
  bool publieParLoraP2P(bool force=false);
  int readAndPublish(bool force=false);
};

#endif // __MYBATTERIE_H__