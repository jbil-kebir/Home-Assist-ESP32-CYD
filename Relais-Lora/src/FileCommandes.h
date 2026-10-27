#ifndef __FILE_COMMANDES_H__
#define __FILE_COMMANDES_H__

#include <Arduino.h>
#include <functional>
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
  };
  static const SCapteurLora mCapteurs[];
  static const int NB_CAPTEURS;
  SEntree mEntrees[FILE_CMD_NB_MAX_CAPTEURS];

  int indexTopic(const String& topicConfig) const;
  int indexEquipement(const String& nomEquipement) const;

public:
  // Envoi LoRa (topic, payload) et publication MQTT (topic, payload)
  std::function<int(const char*, const char*)> onLoraPublish;
  std::function<int(const char*, const char*)> onMqttPublish;
  String topicEtatRelais; // Où signaler les abandons

  int nbCapteurs() const { return NB_CAPTEURS; }
  const char* topicConfig(int i) const { return mCapteurs[i].topicConfig; }
  bool estTopicConfig(const String& topic) const { return indexTopic(topic) >= 0; }

  void memorise(const String& topicConfig, const String& commande);
  void noteTrame(const String& topic, const String& nomEquipement, const String& payload);
  void loop();
  String getEtat() const;  // Une ligne par capteur (dernière trame, commande en attente) pour STATUS
};

#endif // __FILE_COMMANDES_H__
