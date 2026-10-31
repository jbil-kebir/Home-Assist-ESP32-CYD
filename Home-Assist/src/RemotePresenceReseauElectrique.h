#ifndef __REMOTE_PRESENCE_RESEAU_ELECTRIQUE_H__
#define __REMOTE_PRESENCE_RESEAU_ELECTRIQUE_H__

#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "global.h"
#include "EquipementBase.h"

//------------------------------------------------------------------------------
// Présence sur le réseau électrique d'un nœud distant (classe CPresenceReseauElectrique du projet Capteur), branché
// derrière la prise RF 433 MHz d'un appareil du CYD principal (CRCDevice : guirlande...) : nœud présent = appareil
// allumé. Trames reçues sur le topic de configuration du nœud (home/conf<nom>/state) :
//   « <nom> PRESENT ... » (démarrage, réponse à PING), « <nom> ALIVE ... » (toutes les 600 s) et toute autre trame
//   du nœud (VERSION, CONFIG, accusé...) : présent ;  « <nom> offline » (testament Wi-Fi) : absent (conclut
//   aussitôt une vérification d'extinction) ;
//   « <nom> DEEPSLEEP ON ... » ou « <nom> CONFIG ... sleepA:1 ... » : mode non vérifié (le silence d'un nœud
//   endormi n'est pas une extinction) ; « sleepA:0 » ou ALIVE (jamais émis en deep sleep) : de nouveau vérifié.
// Commande (état voulu, demandeEtat()) : attente, puis PING répétés sur home/conf<nom>/command ; conclusion
// présent / absent ; si elle diffère de l'état voulu, renvoi du code (qui bascule) jusqu'à N essais, puis échec.
// Sans commande : un nœud muet plus de 2 périodes ALIVE (+ marge) est absent ; une trame le rend présent.
//------------------------------------------------------------------------------
#define PRESENCE_INCONNUE             -1
#define PRESENCE_ABSENT                0
#define PRESENCE_PRESENT               1
#define PRESENCE_VERIF_SANS_COMMANDE  -1  // demandeEtat() : vérification seule, sans renvoi du code
#define PRESENCE_DELAI_DEFAUT_S        8  // Entre l'envoi du code et la 1re question (démarrage du nœud ~5 s)
#define PRESENCE_ATTENTE_DEFAUT_S      5  // Attente de la réponse à chaque PING
#define PRESENCE_NB_PING_DEFAUT        3  // PING sans réponse avant de conclure à l'absence
#define PRESENCE_NB_ESSAIS_DEFAUT      3  // Envois du code avant de signaler l'échec
#define PRESENCE_ALIVE_DEFAUT_S      600  // Période ALIVE du nœud
#define PRESENCE_MARGE_ALIVE_S        60  // Absent après 2 périodes ALIVE + cette marge de silence
#define PRESENCE_DELAI_DEMARRAGE_S    15  // Vérification de l'état réel après le démarrage du CYD (MQTT connecté)

class CRemotePresenceReseauElectrique : public CEquipementBase {
private:
  enum EPhase { PHASE_REPOS, PHASE_ATTENTE, PHASE_QUESTION };
  EPhase mPhase = PHASE_REPOS;
  int miEtat = PRESENCE_INCONNUE;
  int miEtatVoulu = PRESENCE_VERIF_SANS_COMMANDE;
  bool mbNonVerifie = false;
  bool mbSignalPhase = false;          // Trame du nœud reçue depuis le début de la phase en cours
  unsigned long mulDebutPhase = 0;
  unsigned long mulDernierSignal = 0;  // millis() de la dernière trame du nœud (0 : aucune)
  int miNbQuestions = 0;
  int miEssai = 0;
  bool mbVerifInitialeFaite = false;

  void calculeTopics();
  void signalRecu();
  void changeEtat(int etat);
  void changeNonVerifie(bool nonVerifie);
  void debutePhase(EPhase phase);
  void pose();                         // PING au nœud
  void conclut(int etat);
  void finCommande();

public:
  CRemotePresenceReseauElectrique() { nomEquipement = ""; active = false; }

  // Réglages (page Web du CYD principal, section de l'appareil)
  unsigned long mulDelaiS = PRESENCE_DELAI_DEFAUT_S;
  unsigned long mulAttenteS = PRESENCE_ATTENTE_DEFAUT_S;
  int miNbPing = PRESENCE_NB_PING_DEFAUT;
  int miNbEssais = PRESENCE_NB_ESSAIS_DEFAUT;
  unsigned long mulAliveS = PRESENCE_ALIVE_DEFAUT_S;

  // Callbacks vers l'appareil RF
  std::function<int(const char*, const char*)> onMqttPublish = nullptr;
  std::function<void(int)> onEtatChange = nullptr;          // État mesuré changé (PRESENCE_ABSENT / PRESENCE_PRESENT)
  std::function<void(int, int)> onRenvoi = nullptr;         // Renvoyer le code (essai, nb d'essais)
  std::function<void(int, int)> onEchec = nullptr;          // État voulu non atteint après N essais (voulu, essais)
  std::function<void(bool)> onNonVerifieChange = nullptr;   // Entrée / sortie du mode non vérifié
  std::function<void()> onFinCommande = nullptr;            // Fin de la vérification d'une commande (réussite, échec, abandon)
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cb) { onMqttPublish = cb; }

  void loadFromNVS();
  void loadFromWebServer(WebServer& server);
  void saveToNVS();
  String getHTML();

  int setup(const String pref);
  void loop();
  void handleMqttState(const String& payload);
  bool estConfiguree() const { return active; }   // Nœud associé (nom non vide)
  bool estNonVerifie() const { return mbNonVerifie; }
  bool verificationEnCours() const { return mPhase != PHASE_REPOS; }
  int getEtat() const { return miEtat; }
  void demandeEtat(int voulu);                    // Après l'envoi du code (voulu : 0 / 1) ou vérification seule (-1)
  void print() const;
};

#endif // __REMOTE_PRESENCE_RESEAU_ELECTRIQUE_H__
