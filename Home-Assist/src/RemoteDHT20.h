#ifndef __REMOTEDHT20_H__
#define __REMOTEDHT20_H__

#include <OneWire.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "global.h"
#include "MyEcran.h"
#include "RCSwitchCodeAnalyzer.h"
#include "EquipementBase.h"

class CRemoteDHT20 : public CRCSwitchCodeAnalyzer, public CEquipementBase  {
private:
  CEcran *mEcran=nullptr;
  // On mémorise la température pour n'afficher que les changements
  float lastTempC = -127.0;  // Valeur invalide par défaut
  float newTempC = -127.0;  // Valeur invalide par défaut
  // On mémorise l'humidité pour n'afficher que les changements
  float lastHum = -1.0;  // Valeur invalide par défaut
  float newHum = -1.0;  // Valeur invalide par défaut
  //====================== Commandes MQTT ======================
  String sMqttCommandMesure = "MESURE"; // Force une mesure

  // En secondes pour les tests. Si l'appareil ne répond pas au delà de cet intervalle, 
  // il est considéré comme KO
  unsigned long mulWatchdogIntervalle = 60; 
  unsigned long mulWatchdogDefaultIntervalle = 60; 
  unsigned long mulWatchDog;
  unsigned char mucCapteurID=0;
  // Codes RCSwitch
  #define NB_MESURES  2 // Température et humidité
  #define NB_CODES_RCS  (2*NB_MESURES) // MSB et LSB ==> donc 2 codes
  unsigned long mulCodes[NB_CODES_RCS];
  _ST_MESURE_ mStMesures[NB_CODES_RCS]; // Mesures décodées
  unsigned char mucNbCodesRecus = 0; // Nombre de codes reçus

public:
  CRemoteDHT20(const String& nomEqu, CEcran* ecran, 
      std::function<void(const String&, float)> callbackTemerature = nullptr, 
      std::function<void(const String&, float)> callbackHumidite = nullptr,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(ecran), onTemperatureChanged(callbackTemerature), onHumiditeChanged(callbackHumidite), onMqttPublish(cbonMqttPublish) {
      nomEquipement = nomEqu;
    } 

  
  // Callback : void(String expediteur, float temperature)
  std::function<void(const String&, float)> onTemperatureChanged;    
  std::function<void(const String&, float)> onHumiditeChanged;    
  // MQTT
  std::function<int(const char*, const char*)> onMqttPublish;    

  void setDisplayCallbackTemperature(std::function<void(const String&, float)> cb); // Pour mise à jour de l'affichage
  void setDisplayCallbackHumidite(std::function<void(const String&, float)> cb); // Pour mise à jour de l'affichage
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  String getHTML();

  int begin(const String pref);
  int loop();
  bool readMesures();
  float getLastTemperature() const;
  float getLastHumidite() const;
  void print() const;
  void printMesures() const;
  void handleMqttCommand(const String& payload);
  void handleMqttState(const String& payload);
  int handleRCSwitchCode(unsigned long code);
  bool remonteStatusParMqtt();
  bool remonteTemperatureParMqtt();
  bool remonteHumiditeParMqtt();
//  bool remonteTensionParMqtt();

};

#endif // __REMOTEDHT20_H__