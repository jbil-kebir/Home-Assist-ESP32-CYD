#ifndef __MYPRESENCERESEAUELECTRIQUE_H__
#define __MYPRESENCERESEAUELECTRIQUE_H__

#include <functional>
#include <WebServer.h>
#include <Preferences.h>
#include "global.h"
#include "MyDateTime.h"

//
// Présence sur le réseau électrique : nœud alimenté par la prise RF 433 MHz d'un appareil (guirlande...) dont il
// remonte l'état réel (nœud alimenté = appareil allumé). Nom au préfixe Pr (ex. PrGuirlande).
// Trame « <nom> PRESENT <date> <heure> » sur le topic de configuration du nœud (home/conf<nom>/state), MQTT et LoRa :
//  - à chaque démarrage (retentée tant qu'elle n'a pas été publiée : MQTT pas encore connecté) ;
//  - en réponse à la commande PING (topic de configuration, voir CConfig::handleMqttCommand()).
// L'extinction ne peut pas être annoncée (plus de courant) : le CYD principal la déduit du silence du nœud
// (CRemotePresenceReseauElectrique). Signe de vie : watchdog ALIVE de CConfig, période à régler à 600 s.
//
#define PRESENCE_RESEAU_RETENTE_MS 2000UL // Intervalle entre deux tentatives d'annonce de démarrage

class CPresenceReseauElectrique {
private:
  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;
  String mPrefixNVS = "pr_";
  CMyDateTime *mDateTime=nullptr;
  bool mbAnnonceEnvoyee = false;          // Annonce de démarrage publiée
  unsigned long mulDerniereTentative = 0; // millis() de la dernière tentative d'annonce
  unsigned long mulNbPresent = 0;         // Trames PRESENT publiées depuis le démarrage

public:
  CPresenceReseauElectrique(CMyDateTime& dateTime) : mDateTime(&dateTime) {}

  String nomEquipement = "Pr";
  bool active = false; // Voir loadFromNVS()
  bool mbMesureRemontee = false; // Annonce de démarrage publiée : le deep sleep l'attend (CConfig::mesuresToutesRemontees())
  String mqttSubTopicState; // Topic de configuration du nœud (topic_config_state), fixé par CConfig::setup()

  std::function<int(const char*, const char*)> onMqttPublish = nullptr;
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cb) { onMqttPublish = cb; }
  std::function<int(const char*)> onLoraP2PPublish = nullptr;
  void setLoraP2PPublishCallback(std::function<int(const char* payload)> cb) { onLoraP2PPublish = cb; }

  void loadFromNVS();
  void loadFromWebServer(WebServer& server);
  void saveToNVS();
  String getHTML();

  int setup(const String pref);
  int loop();
  int publiePresent();
  int handleMqttCommand(const String& payload);
  void print() const;
};

#endif // __MYPRESENCERESEAUELECTRIQUE_H__
