#ifndef MYDHT20_H
#define MYDHT20_H

#include "global.h"

#ifdef __LOCAL_DHT20__

#include <functional>
#include <Wire.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DHT20.h>
#include "EquipementBase.h"

//----------------------------------------------------------------------------
// Thermomètre local DHT20 (I2C). Même interface que MyDS18B20 :
// seule la température est remontée (MQTT, écran). L'humidité est lue
// et mémorisée, mais pas encore exploitée.
//----------------------------------------------------------------------------
class MyDHT20 : public CEquipementBase {
private:
  DHT20 dht;
  unsigned char mucSdaPin;
  unsigned char mucSclPin;
  bool mbInitialise = false;
  // On mémorise la température pour n'afficher que les changements
  float lastTempC = -127.0;  // Valeur invalide par défaut
  float newTempC = -127.0;  // Valeur invalide par défaut
  float lastHum = -1.0;     // Valeur invalide par défaut

public:
  MyDHT20(uint8_t sdaPin, uint8_t sclPin) : dht(&Wire), mucSdaPin(sdaPin), mucSclPin(sclPin) {}

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
  float getLastHumidite() const;
  void print() const;
  void printTemperature() const;
  bool publieSurMqtt(bool force=false);
  int readAndPublish(bool force=false);
  bool remonteStatusParMqtt();

};

#endif
#endif
