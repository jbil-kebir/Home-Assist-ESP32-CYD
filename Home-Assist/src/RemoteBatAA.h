#ifndef __REMOTEBATAA_H__
#define __REMOTEBATAA_H__

#include <OneWire.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "global.h"
#include "MyEcran.h"
#include "EquipementBase.h"

struct MQTTMessage_2 {
  String sExpediteur;
  String sAction; // TENSION = remontée de tension, STATUS = remontée état (online, actif/inactif)
  String sArg; // Chargée, déchargée
  String sDate;
  String sTime;
  float fVal; 
};

class CRemoteBatterieAA : public CEquipementBase {
private:
  CEcran *mEcran=nullptr;
  // On mémorise la température pour n'afficher que les changements
  float lastTension = -1.0;  // Valeur invalide par défaut
  float newTension = -1.0;  // Valeur invalide par défaut
  //====================== Commandes MQTT ======================
  String sMqttCommandMesure = "MESURE"; // Force une mesure

  // En secondes pour les tests. Si l'appareil ne répond pas au delà de cet intervalle, 
  // il est considéré comme KO
  unsigned long mulWatchdogIntervalle = 60UL; 
  unsigned long mulWatchdogDefaultIntervalle = 60UL; 
  unsigned long mulWatchDog;

public:
  CRemoteBatterieAA(const String& nomEqu, CEcran* ecran, 
      std::function<void(const String&, bool, float)> callback = nullptr,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(ecran), onTensionChanged(callback), onMqttPublish(cbonMqttPublish) {
      nomEquipement = nomEqu;
    } 

  
  // Callback : void(String expediteur, float temperature)
  std::function<void(const String&, bool, float)> onTensionChanged;    
  // MQTT
  std::function<int(const char*, const char*)> onMqttPublish;    

  void setDisplayCallback(std::function<void(const String&, bool, float)> cb); // Pour mise à jour de l'affichage
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT


  int miLastEtatBatterie = -1; // -1 : état inconnu, 0 : DECHARGEE, 1 CHARGEE

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  String getHTML();

  int begin(const String pref);
  int loop();
  bool readTension();
  float getLastTension() const;
  void print() const;
  void printTension() const;
  void mqttMessageToStruct_split(MQTTMessage_2& st, const String& s);
  void handleMqttCommand(const String& payload);
  void handleMqttState(const String& payload);
  bool remonteStatusParMqtt();
};

#endif // __REMOTEBATAA