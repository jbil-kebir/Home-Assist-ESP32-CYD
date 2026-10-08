#include "global.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#ifdef __LOCAL_MODE__
#endif
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
#include "RemoteTor.h"
#ifdef __LOCAL_MODE__
#else
#include "RemoteRCDevice.h"
#endif

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;
extern CMyDateTime mDateTime;

extern int ajouterControleur(const String& nom, const String& ip);

#ifdef __LOCAL_MODE__
//CRCDevice  chauffageSb(String("Chauffage"), ecran);
CNoeudChauffageIRSound  chauffageSb(String("Chauffage"), ecran);
#else
//CRemoteRCDevice mRemoteChauffage(String("Chauffage"), ecran);
CRemoteNoeudChauffageIRSound mRemoteChauffage(String("Chauffage"), ecran);
#endif
// Etat réel du chauffage SdB, remonté par le noeud IR/son
CRemoteTor mRemoteTorChauffageSb(String("NoeudChauffageSb"), &ecran);

void setup_chauffageSb() {
#ifdef __LOCAL_MODE__
  config.chauffageSb = &chauffageSb;
  chauffageSb.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
    return mqtt.publish(topic, payload);
  });
  chauffageSb.setup("sb_");
#else
  config.mRemoteChauffage = &mRemoteChauffage;
  mRemoteChauffage.setup("sb_");
  mRemoteChauffage.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
#endif // __LOCAL_MODE__

  //--------------------------- Etat réel (Tout Ou Rien) ---------------------------
  mRemoteTorChauffageSb.setup("torsb_");
  // Premier démarrage (rien dans le NVS) : nom et topic initiaux, au lieu des valeurs
  // par défaut de CEquipementBase ("Thermomètre", "thermometre/").
  // Ensuite, ils sont modifiables depuis la page Web.
  mRemoteTorChauffageSb.prefs.begin(mRemoteTorChauffageSb.nvs_namespace, true);
  bool bDejaConfigure = mRemoteTorChauffageSb.prefs.isKey("torsb_nom");
  mRemoteTorChauffageSb.prefs.end();
  if (!bDejaConfigure) {
    mRemoteTorChauffageSb.nomEquipement = "NoeudChauffageSb";
    mRemoteTorChauffageSb.mqttSubTopic = "noeud/";
    mRemoteTorChauffageSb.saveToNVS();
    mRemoteTorChauffageSb.loadFromNVS(); // Recalcule les topics state et command
  }
  config.mRemoteTorChauffageSb = &mRemoteTorChauffageSb; // Toujours remote
  // Tor - affichage et mise à jour de l'état réel
  mRemoteTorChauffageSb.setDisplayCallback([](const String& exp, int val) {
      ecran.updateRemoteDevice_ChauffageSb(exp, val);
  });
  // Tor - publication MQTT
  mRemoteTorChauffageSb.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  mRemoteTorChauffageSb.setonEquipementCallback([](const String& nom, const String& ip) -> int {
      return ajouterControleur(nom, ip);
  });
  // Echec de l'actionneur IR du noeud
  mRemoteTorChauffageSb.onIrko = []() {
    #ifdef __LOCAL_MODE__
    chauffageSb.signaleIrko();
    #else
    ecran.updateStatus(mRemoteTorChauffageSb.nomEquipement + " : échec IR");
    #endif
  };
  #ifdef __LOCAL_MODE__
  chauffageSb.mNoeud = &mRemoteTorChauffageSb; // Présence du noeud et commande OFF
  chauffageSb.verifieParametres(); // Valeurs NVS antérieures aux bornes (dépend du watchdog du noeud)
  #endif
  // Pastille du bouton SB : présence du noeud (C3), d'après son watchdog
  ecran.setEtatPastilleChauffageSb([]() -> int {
    return mRemoteTorChauffageSb.etatPresence();
  });
}

//--------------------------------------------------------------------------
// Watchdog du noeud chauffage SdB
// Le noeud n'est alimenté que lorsque le relais 433 MHz est fermé : il est présent
// tant que ses messages (mesures ou ALIVE) arrivent avant l'expiration du watchdog.
// Le -10 ne doit pas passer par le callback d'affichage : setEtatReelOnOff(bool) le prendrait pour ON.
//--------------------------------------------------------------------------
void loop_chauffageSb() {
  int retChauffageSb = mRemoteTorChauffageSb.loop();
  static bool bWdogChauffageSbErr = false;
  if (retChauffageSb == -10) { // Watchdog error
    if (!bWdogChauffageSbErr) {
      String s = mRemoteTorChauffageSb.nomEquipement + " absent " + mDateTime.getTime();
      DBG(DBG_NOEUD, "[%s] %s\n", mDateTime.getTime().c_str(), s.c_str());
      ecran.updateStatus(s);
      bWdogChauffageSbErr = true;
    }
  }
  else if (retChauffageSb != -2 && bWdogChauffageSbErr) { // -2 : inactif
    String s = mRemoteTorChauffageSb.nomEquipement + " present " + mDateTime.getTime();
    DBG(DBG_NOEUD, "[%s] %s\n", mDateTime.getTime().c_str(), s.c_str());
    ecran.updateStatus(s);
    bWdogChauffageSbErr = false;
  }
  // Pastille du bouton SB redessinée à chaque changement de présence du noeud
  static int iDernierePresence = -2; // Force le premier dessin
  int iPresence = mRemoteTorChauffageSb.etatPresence();
  if (iPresence != iDernierePresence) {
    iDernierePresence = iPresence;
    ecran.updatePastilleChauffageSb();
  }
  #ifdef __LOCAL_MODE__
  // Séquences marche/arrêt et disparition du noeud, après les messages du watchdog
  // pour que les messages de la séquence ne soient pas écrasés à l'écran
  chauffageSb.loop();
  #endif
}
