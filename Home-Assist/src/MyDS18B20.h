#ifndef MYDS18B20_H
#define MYDS18B20_H

#ifdef __LOCAL_DS18B20__

#include <functional>
#include <OneWire.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DallasTemperature.h>
#include "global.h"
#include "EquipementBase.h"

class MyDS18B20 : public CEquipementBase {
private:
  OneWire oneWire;
  DallasTemperature sensors;
  unsigned char mucPin;
  // On mémorise la température pour n'afficher que les changements
  float lastTempC = -127.0;  // Valeur invalide par défaut
  float newTempC = -127.0;  // Valeur invalide par défaut

public:
  MyDS18B20(uint8_t gpioPin) : mucPin(gpioPin), oneWire(gpioPin), sensors(&oneWire) {/*bLocal = true;*/} 

  unsigned long mulIntervalleMesure = 10, mulDefaultIntervalleMesure = 10; // en minutes : Intervalle entre deux mesures. En secondes pour les tests
  // en minutes : Intervalle de forcage de la rmontée de mesure, même si la valeur n'a pas changé. En secondes pour les tests
  unsigned long mulIntervalleForcageRemonteeMesure = 20, mulDefaultIntervalleForcageRemonteeMesure = 20; 
 
  // MQTT callback
  std::function<int(const char*, const char*)> onMqttPublish;    
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  String getHTML();

  int begin(const String pref);
  int loop();
  bool readTemperature();
  float getLastTemperature() const;
  void print() const;
  void printTemperature() const;
  bool publieSurMqtt(bool force=false);
  int readAndPublish(bool force=false);
  bool remonteStatusParMqtt();

};

#endif
#endif