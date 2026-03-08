// DHT20Sensor.h
#pragma once

#ifndef __DHT20SENSOR_H__
#define __DHT20SENSOR_H__
#include "global.h"
#include <functional>
#include <Preferences.h>
#include <Arduino.h>
#include <Wire.h>
#include <WebServer.h>
#include <DHT20.h>
#include "MyDateTime.h"
#include "MyRCSwitch.h"

#define DEFAULT_CAPTEUR_ID (random(0, 256))  // Aléatoire

#define TIMEOUT_WIRE    30000UL // Timeout de connexion Wire
#define TIMEOUT_DHT    30000UL // Timeout de connexion Wire

// Structure utilisée par handleMqttCommand()
struct MQTT_COMMAND_3 {
  String sExpediteur="";
  String sCommand="";
  String sArg="";
};


class DHT20Sensor {
private:
  DHT20 dht;
  bool _initialized = false;
  unsigned char mucSdaPin =  DEFAULT_SDA_PIN;
  unsigned char mucSclPin =  DEFAULT_SCL_PIN;
  unsigned long mulIntervalleMesure = 10L, mulDefaultIntervalleMesure = 10L; // en minutes : Intervalle entre deux mesures. En secondes pour les tests
  // en minutes : Intervalle de forcage de la rmontée de mesure, même si la valeur n'a pas changé. En secondes pour les tests
  unsigned long mulIntervalleForcageRemonteeMesure = 20L, mulDefaultIntervalleForcageRemonteeMesure = 20L; 
  unsigned char mucNbEnvois = 1; // Nombre de fois qu'il faut remonter les mesures à chaque réveil
  unsigned long mulIntervalleEnvoi = 10L, mulDefaultIntervalleEnvoi = 10L; // en secondes : Intervalle entre deux envois. 
  bool mbForce = true; // On force une remontée à l'initialisation
  Preferences prefs;
  // On mémorise la température pour n'afficher que les changements
  float lastTempC = -127.0;  // Valeur invalide par défaut
  float newTempC = -127.0;  // Valeur invalide par défaut
  // On mémorise la température pour n'afficher que les changements
  float lastHum = -1.0;  // Valeur invalide par défaut
  float newHum = -1.0;  // Valeur invalide par défaut
  const char* default_domotique_topic_prefix = "home/";
  const char* nvs_namespace = NVS_NAME_SPACE;  // 
  String mPrefixNVS = "thHum_";  // Préfixe par défaut
  CMyDateTime *mDateTime=nullptr;
  unsigned char mucCapteurID=0;
  unsigned int mDelai_Inter_Envoi = 350; // A mettre dans NVS et page Web
  

public:
    //DHT20Sensor() {}
    DHT20Sensor(CMyDateTime& dateTime,
        std::function<int(const char*, const char*)> cbonMqttPublish = nullptr,
        std::function<int(const char*)> cbonLoraP2PPublish = nullptr) : 
        mDateTime(&dateTime), 
        onMqttPublish(cbonMqttPublish), onLoraP2PPublish(cbonLoraP2PPublish) 
        {}

    String domotique_prefix;
    String nomEquipement = "ThCh1er";
    String mqttSubTopic = "thermometre";
    bool active = true;
    bool mbMesureRemontee = false; // Permet d'empêcher le deep sleep tant qu'une mesure n'a pas été remontée
    //unsigned char mucNbMesuresRemontees=0; // remplace mbMesureRemontee. On compte et on entre en deep-sleep au bout d'un certain de nombre de remontées
    //#ifdef _RCSWITCH_MODE_
    //#define DEFAULT_NB_MESURES_REMONTEES_AVANT_DEEP_SLEEP 3
    //#else
    //#define DEFAULT_NB_MESURES_REMONTEES_AVANT_DEEP_SLEEP 1
    //#endif
    //unsigned char mucNbMesuresRemonteesAvantDeepSleep = DEFAULT_NB_MESURES_REMONTEES_AVANT_DEEP_SLEEP;
    //bool bLocal = true; // true : Equipement local, commandé par l'ESP32. False : distant, commandé par Mqtt
    String mqttSubTopicCommand;
    String mqttSubTopicState;
  //  unsigned long mulIntervalleMesure = 10L, mulDefaultIntervalleMesure = 10L; // en minutes : Intervalle entre deux mesures. En secondes pour les tests
    // en minutes : Intervalle de forcage de la rmontée de mesure, même si la valeur n'a pas changé. En secondes pour les tests
  //  unsigned long mulIntervalleForcageRemonteeMesure = 20L, mulDefaultIntervalleForcageRemonteeMesure = 20L; 
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
    void setPrefixNVS(const char* pr) {mPrefixNVS = pr;}
    String getHTML();


    bool begin(const String pref);
    int loop();
    bool read();
    // Variante avec retry (utile en low-power, car parfois le premier read échoue)
    bool readWithRetry(uint8_t retries = 3);

    void print() const;
    float getLastTemperature() const;
    float getLastHumidite() const;

    int handleMqttCommand(const String& payload);
    bool publieSurMqtt(bool force=false);
    bool publieParCC1101(bool force=false);
    bool publieParLoraP2P(bool force=false);
    uint8_t computeCRC(const unsigned long* codes, int count);
    int readAndPublish(bool force=false, bool mesurer=false);
    int readAndPublishTEST(float t, float h); // Pour les tests

};
#endif // __DHT20SENSOR_H__