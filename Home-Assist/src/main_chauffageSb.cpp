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
}
