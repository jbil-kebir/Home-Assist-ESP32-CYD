// DHT20Sensor.h
#pragma once

#ifndef __CTOR_H__
#define __CTOR_H__
#include "global.h"
#include <functional>
#include <Preferences.h>
#include <Arduino.h>
//#include <Wire.h>
#include <WebServer.h>
//#include <DHT20.h>
#include "MyDateTime.h"
#include "MyRCSwitch.h"

#ifdef FLOTTEUR_VERTICAL

#define DEFAULT_CAPTEUR_ID (random(0, 256))  // Aléatoire


// Structure utilisée par handleMqttCommand()
struct MQTT_COMMAND_4 {
  String sExpediteur="";
  String sCommand="";
  String sArg="";
};


class CTor {
private:
  //DHT20 dht;
  bool _initialized = false;
  //unsigned char mucSdaPin =  DEFAULT_SDA_PIN;
  //unsigned char mucSclPin =  DEFAULT_SCL_PIN;
  unsigned char mucPin =  DEFAULT_TOR_PIN;
  unsigned char mucPinMode = DEFAULT_TOR_PULL_UP_DOWN_MODE;
  unsigned long mulIntervalleMesure = 10L, mulDefaultIntervalleMesure = 10L; // en minutes : Intervalle entre deux mesures. En secondes pour les tests
  // en minutes : Intervalle de forcage de la rmontée de mesure, même si la valeur n'a pas changé. En secondes pour les tests
  unsigned long mulIntervalleForcageRemonteeMesure = 20L, mulDefaultIntervalleForcageRemonteeMesure = 20L; 
  unsigned char mucNbEnvois = 1; // Nombre de fois qu'il faut remonter les mesures à chaque réveil
  unsigned long mulIntervalleEnvoi = 10L, mulDefaultIntervalleEnvoi = 10L; // en secondes : Intervalle entre deux envois. 
  bool mbForce = true; // On force une remontée à l'initialisation
  Preferences prefs;
  // On mémorise la température pour n'afficher que les changements
  //float lastTempC = -127.0;  // Valeur invalide par défaut
  //float newTempC = -127.0;  // Valeur invalide par défaut
  // On mémorise la température pour n'afficher que les changements
  //float lastHum = -1.0;  // Valeur invalide par défaut
  //float newHum = -1.0;  // Valeur invalide par défaut
  int miLastVal = -1; // Même en tout ou rien, on déclare la mesure en int pour y intégrer les erreurs éventuelles
  int miNewVal = -1; // Même en tout ou rien, on déclare la mesure en int pour y intégrer les erreurs éventuelles
  const char* default_domotique_topic_prefix = "home/";
  const char* nvs_namespace = NVS_NAME_SPACE;  // 
  String mPrefixNVS = "thTor_";  // Préfixe par défaut
  CMyDateTime *mDateTime=nullptr;
  unsigned char mucCapteurID=0;
  unsigned int mDelai_Inter_Envoi = 350; // A mettre dans NVS et page Web

public:
    CTor(CMyDateTime& dateTime,
        std::function<int(const char*, const char*)> cbonMqttPublish = nullptr,
        std::function<int(const char*)> cbonLoraP2PPublish = nullptr) : 
        mDateTime(&dateTime), 
        onMqttPublish(cbonMqttPublish), 
        onLoraP2PPublish(cbonLoraP2PPublish)
        {/*bLocal = true;*/} 
  String domotique_prefix;
  String nomEquipement = "Tor";
  String mqttSubTopic = "tor";
  bool active = false;
  bool mbMesureRemontee = false; // Permet d'empêcher le deep sleep tant qu'une mesure n'a pas été remontée
  bool mbAckNeeded = false; // Indique si un ACK est nécessaire (dépend du capteur). A mettre dans NVS et page Web
  bool mbAckReceived = false; // Pour le suivi des ACK en Lora P2P
  String mqttSubTopicCommand;
  String mqttSubTopicState;
  #ifdef _RCSWITCH_MODE_
  CMyRCSwitch *mRCSwitch = nullptr;
  #endif

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
  void setAck(bool state);
  void setpinMode(unsigned char mode = INPUT_PULLUP);
  void setPrefixNVS(const char* pr) {mPrefixNVS = pr;}
  String getHTML();


  bool setup(const String pref);
  int loop();
  bool read();
  // Variante avec retry (utile en low-power, car parfois le premier read échoue)
  bool readWithRetry(uint8_t retries = 3);

  void print() const;
  //float getLastTemperature() const;
  //float getLastHumidite() const;
  int getLastMesure() const;

  int handleMqttCommand(const String& payload);
  bool publieSurMqtt(bool force=false);
  bool publieParLoraP2P(bool force=false);
  bool publieParCC1101(bool force=false);
  uint8_t computeCRC(const unsigned long* codes, int count);
  int readAndPublish(bool force=false, bool mesurer=false);
  int readAndPublishTEST(float t, float h); // Pour les tests

};
#endif // FLOTTEUR_VERTICAL
#endif // __CTOR_H__