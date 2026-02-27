#ifndef __REMOTETH_H__
#define __REMOTETH_H__

#include <OneWire.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "global.h"
#include "MyEcran.h"
#include "EquipementBase.h"

class CRemoteThermo : public CEquipementBase {
private:
  CEcran *mEcran=nullptr;
  // On mémorise la température pour n'afficher que les changements
  float lastTempC = -127.0;  // Valeur invalide par défaut
  float newTempC = -127.0;  // Valeur invalide par défaut
  //====================== Commandes MQTT ======================
  String sMqttCommandMesure = "MESURE"; // Force une mesure

  // En secondes pour les tests. Si l'appareil ne répond pas au delà de cet intervalle, 
  // il est considéré comme KO
  unsigned long mulWatchdogIntervalle = 60; 
  unsigned long mulWatchdogDefaultIntervalle = 60; 
  unsigned long mulWatchDog;

public:
  CRemoteThermo(const String& nomEqu, CEcran* ecran, 
      std::function<void(const String&, float)> callback = nullptr,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(ecran), onTemperatureChanged(callback), onMqttPublish(cbonMqttPublish) {
      nomEquipement = nomEqu;
    } 

  
  // Callback : void(String expediteur, float temperature)
  std::function<void(const String&, float)> onTemperatureChanged;    
  // MQTT
  std::function<int(const char*, const char*)> onMqttPublish;    

  void setDisplayCallback(std::function<void(const String&, float)> cb); // Pour mise à jour de l'affichage
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
  void handleMqttCommand(const String& payload);
  void handleMqttState(const String& payload);
  bool remonteStatusParMqtt();
};

#endif // __REMOTETH_H__