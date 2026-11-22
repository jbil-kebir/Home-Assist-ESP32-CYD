#ifndef __MYBATTERIE_H__
#define __MYBATTERIE_H__

#include <functional>
#include <WebServer.h>
#include <Preferences.h>
#include "global.h"
#include "MyDateTime.h"
#include "CapteurBase.h"

// Structure utilisée par handleMqttCommand()
struct MQTT_COMMAND_2 {
  String sExpediteur="";
  String sCommand="";
  String sArg="";
};


class CBatterieAA : public CCapteurBase {
private:
  Preferences prefs;
  unsigned char mucPin;
  // On mémorise la température pour n'afficher que les changements
  float mfTensionMin = -1.0;  // Seuil en dessous duquel la batterie est considérée HS
  float mLastTension = -1.0;  // Valeur invalide par défaut
  float newTension = -2.0;  // Valeur invalide par défaut
  float mCalibre = 4.38; // Entre 4.35 et 4.45
  float mDefaultCalibre = (220.0+100.0)/100.0; // Pont diviseur Vpiles -> 220 K (R2) --> GPIO --> 100 K (R3) --> GND (pas de 200 K en stock)
  const char* default_domotique_topic_prefix = "home/";
  const char* nvs_namespace = NVS_NAME_SPACE;  // 
  String mPrefixNVS = "bt_";  // Préfixe par défaut
  CMyDateTime *mDateTime=nullptr;


public:
  CBatterieAA(uint8_t gpioPin, CMyDateTime& dateTime,
    std::function<int(const char*, const char*)> cbonMqttPublish = nullptr,
    std::function<int(const char*)> cbonLoraP2PPublish = nullptr) :
    CCapteurBase(60UL),
    mucPin(gpioPin), mDateTime(&dateTime),
    onMqttPublish(cbonMqttPublish),
    onLoraP2PPublish(cbonLoraP2PPublish)
    {}
  String getPrefixNVS() const override { return mPrefixNVS; }

  String domotique_prefix;
  String nomEquipement = "Batt";
  String mqttSubTopic = "bat";
  bool active = false; // Voir loadFromNVS()
  bool mbMesureRemontee = false; // Permet d'empêcher le deep sleep tant qu'une mesure n'a pas été remontée
  //bool bLocal = true; // true : Equipement local, commandé par l'ESP32. False : distant, commandé par Mqtt
  String mqttSubTopicCommand;
  String mqttSubTopicState;
  unsigned long mulIntervalleMesure = 30UL, mulDefaultIntervalleMesure = 30UL; // en secondes : Intervalle entre deux mesures. En secondes pour les tests
  // Forçage de la remontée : voir CCapteurBase (60 s par défaut)

  // MQTT callback
  std::function<int(const char*, const char*)> onMqttPublish;    
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT
  // Lora P2P callback
  std::function<int(const char*)> onLoraP2PPublish;    
  void setLoraP2PPublishCallback(std::function<int(const char* payload)> cbLoraP2PPublish); // Pour publication Lora P2O

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  // Réglages à chaud (Capteur 5.3, MyWebServer::handleSave) : réglages qui demandent un redémarrage
  String reglagesRedemarrage() const { return String(active) + "/" + String(mucPin) + "/" + mqttSubTopic; }
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