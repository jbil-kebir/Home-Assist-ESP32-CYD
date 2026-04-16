#ifndef __REMOTE_TOR_H__
#define __REMOTE_TOR_H__

#include <OneWire.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "global.h"
#include "MyEcran.h"
#include "EquipementBase.h"


class CRemoteTor : public CEquipementBase {
private:
  CEcran *mEcran=nullptr;
  // On mémorise la Mesure pour n'afficher que les changements
  // La mesure est en int même si sa vraie valeur est booléenne.
  // Cela permet d'utiliser des valeurs pour indiquer des états autres que ON/OFF
  int lastMesure = -1;  // Valeur invalide par défaut
  int newMesure = -1;  // Valeur invalide par défaut
  //====================== Commandes MQTT ======================
  String sMqttCommandMesure = "MESURE"; // Force une mesure

  // En secondes pour les tests. Si l'appareil ne répond pas au delà de cet intervalle, 
  // il est considéré comme KO
  unsigned long mulWatchdogIntervalle = 60UL; 
  unsigned long mulWatchdogDefaultIntervalle = 60UL; 
  unsigned long mulWatchDog;

public:
  CRemoteTor(const String& nomEqu, CEcran* ecran, 
      std::function<void(const String&, int)> callback = nullptr,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(ecran), onMesureChanged(callback), onMqttPublish(cbonMqttPublish) {
      nomEquipement = nomEqu;
    } 

  
  // Callback : void(String expediteur, float temperature)
  std::function<void(const String&, int)> onMesureChanged;    
  // MQTT
  std::function<int(const char*, const char*)> onMqttPublish;    

  void setDisplayCallback(std::function<void(const String&, int)> cb); // Pour mise à jour de l'affichage
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT


  //int miLastEtatBatterie = -1; // -1 : état inconnu, 0 : DECHARGEE, 1 CHARGEE

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  String getHTML();

  int setup(const String pref);
  int loop();
  bool readMesure();
  float getLastMesure() const;
  void print() const;
  void printMesure() const;
  void handleMqttCommand(const String& payload);
  void handleMqttState(const String& payload);
  bool remonteStatusParMqtt();
};

#endif // __REMOTE_TOR_H__