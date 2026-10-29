#include "global.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
#include "RemoteDHT20.h"
#include "RemoteBatAA.h"
#include "RemoteTor.h"
#include "RemoteJourNuit.h"
#include "MyDateTime.h"

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;
extern CMyDateTime mDateTime;

CRemoteDHT20 mRemoteThCave(String("ThCave"), &ecran);
CRemoteBatterieAA mRemoteBatCave(String("BatCave"), &ecran);
CRemoteTor mRemoteTor(String("TorCave"), &ecran);
CRemoteJourNuit mRemoteJnCave(String("JnCave"), &ecran);

extern int ajouterControleur(const String& nom, const String& ip, unsigned long attenteMax = 0);

void setup_ThCave() {
  mRemoteThCave.begin("thCve_"); // Type, etage, lieu (thermomètre, 1er, chanmre)
  mRemoteBatCave.begin("batcve_");
  mRemoteTor.setup("torcve_");
  config.mRemoteThCave = &mRemoteThCave; // Toujours remote
  config.mRemoteTor = &mRemoteTor; // Toujours remote
  config.mRemoteBatCave = &mRemoteBatCave; // Toujours remote
  mRemoteJnCave.setup("jncve_");
  config.mRemoteJnCave = &mRemoteJnCave; // Toujours remote
  //--------------------------- Température et humidité ---------------------------
  // Thermomètre Cave - affichage température
  mRemoteThCave.setDisplayCallbackTemperature([](const String& exp, float temp) {
      ecran.updateRemoteDevice_ThCave(exp, temp);
  });
  // Thermomètre Cave - affichage humidité
  mRemoteThCave.setDisplayCallbackHumidite([](const String& exp, float temp) {
      ecran.updateRemoteDevice_ThCaveH(exp, temp);
  });

  // Thermomètre Cave - publication MQTT
  mRemoteThCave.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  //--------------------------- Tout Ou Rien ---------------------------
    // Tor - affichage
  mRemoteTor.setDisplayCallback([](const String& exp, int val) {
      ecran.updateRemoteDevice_ThCaveTor(exp, val);
  });

  // Tor - publication MQTT
  mRemoteTor.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });

  //--------------------------- Batterie ---------------------------
    // Batterie du thermomètre Cave - affichage
  mRemoteBatCave.setDisplayCallback([](const String& exp, bool etatBatterie, float temp) {
      ecran.updateRemoteBat_ThCave(exp, etatBatterie, temp);
  });

  // Batterie du thermomètre Cave - publication MQTT
  mRemoteBatCave.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  //--------------------------- Jour / nuit ---------------------------
  // Série 3 : à la place de la batterie
  mRemoteJnCave.setDisplayCallback([](const String& exp, int eclairement, int etat) {
      ecran.updateRemoteJourNuit_ThCave(eclairement, etat);
  });
  mRemoteJnCave.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  // Pastille du bouton Cave : jaune le jour, bleu nuit la nuit, gris si inconnu ou watchdog expiré
  ecran.setCouleurPastilleLumiereCave([]() -> int {
      if (mRemoteJnCave.loop() == -10) return TFT_LIGHTGREY;
      int etat = mRemoteJnCave.getLastEtat();
      return (etat == 1) ? TFT_YELLOW : (etat == 0) ? TFT_NAVY : TFT_LIGHTGREY;
  });

  mRemoteThCave.setonEquipementCallback([](const String& nom, const String& ip) -> int {
      return ajouterControleur(nom, ip, mRemoteThCave.getWatchdogIntervalle()); // Retiré de la série 5 au-delà de son watchdog
  });

}

void loop_ThCave() {
  // Jour / nuit : WDG affiché une fois le watchdog expiré, état réaffiché au premier message suivant
  static bool bWdogJnCaveErr = false;
  if (mRemoteJnCave.loop() == -10) {
    if (!bWdogJnCaveErr) {
      ecran.updateRemoteJourNuit_ThCave(mRemoteJnCave.getLastEclairement(), -2);
      bWdogJnCaveErr = true;
    }
  }
  else bWdogJnCaveErr = false;
  // Pastille du bouton Cave redessinée à chaque changement d'état (ou d'expiration du watchdog)
  static int iDernierEtatJn = -3; // Force le premier dessin
  int iEtatJn = bWdogJnCaveErr ? -2 : mRemoteJnCave.getLastEtat();
  if (iEtatJn != iDernierEtatJn) {
    iDernierEtatJn = iEtatJn;
    ecran.updatePastilleLumiereCave();
  }

int retThCave = mRemoteThCave.loop();
  static bool bWdogThCaveErr = false;
  if (retThCave == -2) {
    //Serial.printf("%s Inactif\n", mRemoteThCave.nomEquipement.c_str());
    return;
  }
  else if (retThCave == -10) { // Watchdog error
    if (!bWdogThCaveErr) {
      DBGLN(DBG_CAPTEURS, "void loop()" + String(" - ") + " - " + mRemoteThCave.nomEquipement+" n'a pas répondu depuis longtemps ==> KO " + mDateTime.getDate() + " " + mDateTime.getTime());
      ecran.updateRemoteDevice_ThCave(mRemoteThCave.nomEquipement, -254.0);
      bWdogThCaveErr = true;
    }
  }
  else
    bWdogThCaveErr = false;

}