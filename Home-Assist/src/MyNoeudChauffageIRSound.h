#ifndef MYNOEUDCHAUFFAGEIRSOUND_H
#define MYNOEUDCHAUFFAGEIRSOUND_H

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "MyRCDevice.h"
class CRemoteTor; // Déclaration anticipée (RemoteTor.h inclut MyEcran.h : inclusion circulaire)
#include "global.h"


class CEcran;
class CRadioTX;

class CNoeudChauffageIRSound : public CRCDevice {
public:
  using CRCDevice::CRCDevice;   // hérite des constructeurs de CRCDevice

  int envoiOnOff() override;
  void setEtatReelOnOff(bool state);

  //------------------------------------------------------------------------------
  // Marche / arrêt du chauffage (voir demandeOn() et demandeOff())
  // Le relais 433 MHz n'alimente que le noeud (C3) et n'a qu'un code, qui bascule :
  // on ne connaît son état que par la présence du noeud (watchdog ALIVE).
  //------------------------------------------------------------------------------
  CRemoteTor *mNoeud = nullptr; // Noeud IR/son : présence et commandes ON/OFF
  void setup(const String pref);
  void loop();
  int demandeOn();
  int demandeOff();
  void signaleIrko();
  void handleMqttCommand(const String& payload);
  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  String getHTML();
  void verifieParametres(); // Bornes des délais (dépend du watchdog du noeud : appeler une fois mNoeud chargé)

private:
  enum EtatSequence {
    SEQ_REPOS,
    ARRET_ATTENTE_ETAT0, ARRET_ATTENTE_DISPARITION,  // Arrêt (demandeOff())
    MARCHE_ATTENTE_APPARITION, MARCHE_ATTENTE_ETAT1  // Marche (demandeOn())
  };
  EtatSequence mEtatSequence = SEQ_REPOS;
  unsigned long mulDebutEtape = 0;
  unsigned int muiEnvoisCode = 0;  // Nb d'envois du code relais pour la séquence en cours
  // Paramètres (NVS + page Web)
  unsigned long mulDelaiCoupureForcee = 20;  // min : coupure du relais si OnOff 0 n'arrive pas
  unsigned long mulDelaiVerification = 90;   // s : attente de la disparition du noeud après le code (> watchdog du noeud)
  unsigned long mulDelaiApparition = 90;     // s : attente du démarrage du noeud après le code (> durée de démarrage du C3)
  unsigned long mulDelaiConfirmationOn = 60; // s : attente de OnOff 1 une fois le noeud présent
  unsigned int muiNbEnvoisCode = 3;          // Nb max d'envois du code relais
  bool mbNoeudPresent = false;     // Présence au tour de loop() précédent (détection de la disparition)
  bool mbNoeudPerdu = false;       // Arrêt : noeud disparu avant l'envoi du code relais (C3 planté ou déconnecté)
  unsigned int muiRenvoisOff = 0;  // Arrêt : nb de renvois de la commande OFF au noeud
  bool mbCoupureDiffereeArmee = false;     // Coupure du relais programmée (chauffage éteint)
  bool mbOffConfirme = false;              // OFF obtenu par une séquence d'arrêt (IR + bip), pas par un bip spontané
  bool mbCoupureImmediate = false;         // Arrêt de vérification : coupure du relais dès OnOff 0
  void lanceArret(bool bCoupureImmediate);
  void armeCoupureNonConfirmee();
  unsigned long mulDebutCoupureDifferee = 0;
  unsigned long mulDelaiCoupureApresOff = 15; // min : coupure du relais après OnOff 0 (rallumage rapide par IR d'ici là)
  void publieCommandeNoeud(const char* cmd);
  bool noeudPresent() const;
  void appliqueEtat(bool state);
  void coupeRelais();
  void fermeRelais();
  String etapeSequence() const;
  void message(const String& s);
public:
  /*CNoeudChauffageIRSound() = default;
  CNoeudChauffageIRSound(const String& nomEqu, CEcran& ecran,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(&ecran), onMqttPublish(cbonMqttPublish) {
    nomEquipement = nomEqu;
    mPrefixNVS = nomEquipement.substring(0, 2);
    #ifndef __CYD__
    mEcran = nullptr;
    #endif
   
  }

  // Publication MQTT
  std::function<int(const char*, const char*)> onMqttPublish; 
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  //const bool default_boiler_state = false;
  //const char* default_topic_prefix = "chaudiere/";

  //String nomBoutonON = "CHAUD. ON";
  //String nomBoutonOFF = "CHAUD. OFF";

  //float frequency = 434.038;
  //String etatStr = "OFF";


  void setup();
  void loop();
  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void setEtatReelOnOff(bool state);
  void saveState(bool state);
  void setActive(bool state);
  int activeEquipement();
  String getHTML();

  ///void handleMqttCommand(const String& payload);
  void print() const;*/


};

#endif // MYNOEUDCHAUFFAGEIRSOUND_H