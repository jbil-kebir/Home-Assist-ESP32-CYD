#ifndef __FILE_COMMANDES_H__
#define __FILE_COMMANDES_H__

#include <Arduino.h>
#include <functional>
#include <WebServer.h>
#include "global.h"

//
// File d'attente des commandes de configuration vers les capteurs LoRa.
// Un capteur LoRa dort la plupart du temps : une commande réémise dès sa réception MQTT est perdue.
// Le relais mémorise donc la dernière commande de chaque capteur et la lui envoie quand il se manifeste
// (FILE_CMD_SILENCE_MS après sa dernière trame, pendant sa fenêtre d'écoute), jusqu'à son accusé
// « COMMANDE reçue » ou FILE_CMD_MAX_ENVOIS réveils sans accusé (abandon signalé sur MQTT).
//
struct SCapteurLora {
  const char* topicConfig;   // ex : "home/confthremise/command"
  const char* suffixe;       // fin des noms d'équipement du capteur, ex : "Remise" (ThRemise, BatRemise, FlotteurRemise)
};

class CFileCommandes {
private:
  struct SEntree {
    String commande;                    // Commande en attente ("" = rien)
    unsigned long ulDerniereTrame = 0;  // millis() de la dernière trame reçue du capteur
    bool bDejaVu = false;               // Au moins une trame reçue depuis le démarrage
    bool bEnvoiArme = false;            // Trame reçue : envoyer après FILE_CMD_SILENCE_MS de silence
    int nbEnvois = 0;                   // Envois faits pour la commande en attente
    String nomCapteur;                  // Nom annoncé dans la réponse CONFIG (ex : ThRemise)
    String ip;                          // IP en fin de la dernière trame ("" en LoRa seul : 0.0.0.0)
    String config;                      // Dernière config reçue : « sleepA:1 sleepI:30 ... »
    unsigned long ulConfigRecue = 0;    // millis() de sa réception
    bool bConfigConnue = false;
    bool bGetAutoFait = false;          // GET automatique déjà demandé depuis le démarrage
    String etat;                        // Dernière réponse STATUS : « <date> <heure> | Wdog : ... | Uptime : ... »
    unsigned long ulEtatRecu = 0;
  };
  static const SCapteurLora mCapteurs[];
  static const int NB_CAPTEURS;
  SEntree mEntrees[FILE_CMD_NB_MAX_CAPTEURS];

  int indexTopic(const String& topicConfig) const;
  int indexEquipement(const String& nomEquipement) const;
  void noteConfig(SEntree& e, const String& payload, int iConfig);
  void retire(int i);         // Vide la commande en attente du capteur i
  void publieAttente(int i);  // Commande en attente sur home/<conf>/attente (retenu), pour Home Assistant

public:
  // Envoi LoRa (topic, payload) et publication MQTT (topic, payload)
  std::function<int(const char*, const char*)> onLoraPublish;
  std::function<int(const char*, const char*)> onMqttPublish;
  std::function<int(const char*, const char*, bool)> onMqttPublishRetenu;
  String topicEtatRelais; // Où signaler les abandons

  int nbCapteurs() const { return NB_CAPTEURS; }
  const char* topicConfig(int i) const { return mCapteurs[i].topicConfig; }
  bool estTopicConfig(const String& topic) const { return indexTopic(topic) >= 0; }

  void memorise(const String& topicConfig, const String& commande);  // « ANNULER » vide la file au lieu d'attendre
  void publieAttentes();  // Toutes les files (au démarrage, une fois MQTT connecté)
  void noteTrame(const String& topic, const String& nomEquipement, const String& payload);
  void loop();
  String getEtat() const;  // Une ligne par capteur (dernière trame, commande en attente) pour STATUS

  // Page Web « Capteurs LoRa » : config de chaque capteur, modifiable (SET) ou à relire (GET) au prochain réveil
  String getHTML() const;
  void loadFromWebServer(WebServer& server);
};

#endif // __FILE_COMMANDES_H__
