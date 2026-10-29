#ifndef __REMOTE_JOUR_NUIT_H__
#define __REMOTE_JOUR_NUIT_H__

#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "global.h"
#include "MyEcran.h"
#include "EquipementBase.h"

//------------------------------------------------------------------------------
// Détecteur jour / nuit distant (classe CJourNuit du projet Capteur). Trames reçues sur home/thermometre/state :
//   « <nom> Jour <date> <heure> 1|0 [FORCE] [IP] »
//   « <nom> Eclairement <date> <heure> <pct> [FORCE] [IP] »   (si le capteur remonte l'éclairement)
//------------------------------------------------------------------------------
#define JOUR_NUIT_INCONNU  -1  // État ou éclairement pas encore reçu

class CRemoteJourNuit : public CEquipementBase {
private:
  CEcran *mEcran=nullptr;
  int miLastEclairement = JOUR_NUIT_INCONNU; // En %
  int miLastEtat = JOUR_NUIT_INCONNU;        // 0 : nuit, 1 : jour
  //====================== Commandes MQTT ======================
  String sMqttCommandMesure = "MESURE"; // Force une mesure

  // En secondes. Au-delà sans message, le capteur est considéré KO. Plus long que le forçage du capteur
  // (600 s par défaut) : sans remontée de l'éclairement, il n'émet qu'aux bascules et au forçage.
  unsigned long mulWatchdogIntervalle = 1500UL;
  unsigned long mulWatchdogDefaultIntervalle = 1500UL;
  unsigned long mulWatchDog;
  bool mbMessageRecu = false; // Au moins un message reçu depuis le démarrage

public:
  CRemoteJourNuit(const String& nomEqu, CEcran* ecran) : mEcran(ecran) {
      nomEquipement = nomEqu;
    }

  // Callback : void(String expediteur, int eclairement, int etat)
  std::function<void(const String&, int, int)> onMesureChanged = nullptr;
  // MQTT
  std::function<int(const char*, const char*)> onMqttPublish = nullptr;

  void setDisplayCallback(std::function<void(const String&, int, int)> cb) { onMesureChanged = cb; }
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cb) { onMqttPublish = cb; }

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  String getHTML();

  int setup(const String pref);
  int loop();
  bool readMesure();
  int getLastEclairement() const { return miLastEclairement; }
  int getLastEtat() const { return miLastEtat; }
  void print() const;
  void handleMqttState(const String& payload);
  bool remonteStatusParMqtt();
  unsigned long getWatchdogIntervalle() const { return mulWatchdogIntervalle; } // en secondes
};

#endif // __REMOTE_JOUR_NUIT_H__
