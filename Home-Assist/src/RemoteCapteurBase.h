#ifndef MYDS18B20_H
#define MYDS18B20_H

#include <functional>
#include <OneWire.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DallasTemperature.h>
#include "global.h"
#include "EquipementBase.h"

class CRemoteCapteurBase : public CEquipementBase {
private:
  //OneWire oneWire;
  //DallasTemperature sensors;
  //unsigned char mucPin;
  // On mémorise la température pour n'afficher que les changements
  float lastMesure = -127.0;  // Valeur invalide par défaut
  float newMesure = -127.0;  // Valeur invalide par défaut*/

public:
  //CRemoteCapteurBase(uint8_t gpioPin) : mucPin(gpioPin), oneWire(gpioPin), sensors(&oneWire) {} 

  unsigned long mulWatchdogIntervalle = 60; 
  unsigned long mulWatchdogDefaultIntervalle = 60; 
  unsigned long mulWatchDog;
 
  // Callback : void(String expediteur, float temperature)
  std::function<void(const String&, float)> onMesureChanged;    
  // MQTT
  std::function<int(const char*, const char*)> onMqttPublish;    

  void setDisplayCallback(std::function<void(const String&, float)> cb); // Pour mise à jour de l'affichage
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT


  //void loadFromNVS();
  //void loadFromWebServer (WebServer& server);
  //void saveToNVS();
  //String getHTML();

  //int begin(const String pref);
  //int loop();
  //bool readMesure();
  //float getLastMesure() const;
  //void print() const;
  //void printMesure() const;
  //bool publieSurMqtt(bool force=false);
  //int readAndPublish(bool force=false);

};

#endif