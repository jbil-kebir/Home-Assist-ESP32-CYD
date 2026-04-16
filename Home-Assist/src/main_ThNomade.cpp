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
#include "MyDateTime.h"

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;
extern CMyDateTime mDateTime;

CRemoteDHT20 mRemoteThNomade(String("ThNomade"), &ecran);
CRemoteBatterieAA mRemoteBatNomade(String("BatNomade"), &ecran);
CRemoteTor mRemoteTorNomade(String("TorNomade"), &ecran);

extern int ajouterControleur(const String& nom, const String& ip);

void setup_ThNomade() {
  mRemoteThNomade.begin("thnmf_"); // Type, etage, lieu (thermomètre, 1er, chanmre)
  mRemoteBatNomade.begin("batnmd_");
  mRemoteTorNomade.setup("tornmd_");
  config.mRemoteThNomade = &mRemoteThNomade; // Toujours remote
  config.mRemoteTorNomade = &mRemoteTorNomade; // Toujours remote
  config.mRemoteBatNomade = &mRemoteBatNomade; // Toujours remote
  //--------------------------- Température et humidité ---------------------------
  // Thermomètre Nomade - affichage température
  mRemoteThNomade.setDisplayCallbackTemperature([](const String& exp, float temp) {
      ecran.updateRemoteDevice_ThNomade(exp, temp);
  });
  // Thermomètre Nomade - affichage humidité
  mRemoteThNomade.setDisplayCallbackHumidite([](const String& exp, float temp) {
      ecran.updateRemoteDevice_ThNomadeH(exp, temp);
  });

  // Thermomètre Nomade - publication MQTT
  mRemoteThNomade.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  //--------------------------- Tout Ou Rien ---------------------------
  // Tor - affichage
  DBG(DBG_CAPTEURS, "void setup_ThNomade() - Equipement = %s\n", mRemoteTorNomade.nomEquipement.c_str());
  mRemoteTorNomade.setDisplayCallback([](const String& exp, int val) {
      ecran.updateRemoteDevice_ThNomadeTor(exp, val);
  });

  // Tor - publication MQTT
  mRemoteTorNomade.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });

  //--------------------------- Batterie ---------------------------
    // Batterie du thermomètre Nomade - affichage
  mRemoteBatNomade.setDisplayCallback([](const String& exp, bool etatBatterie, float temp) {
      ecran.updateRemoteBat_ThNomade(exp, etatBatterie, temp);
  });

  // Batterie du thermomètre Nomade - publication MQTT
  mRemoteBatNomade.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  mRemoteThNomade.setonEquipementCallback([](const String& nom, const String& ip) -> int {
      return ajouterControleur(nom, ip);
  });
  mRemoteTorNomade.setonEquipementCallback([](const String& nom, const String& ip) -> int {
      return ajouterControleur(nom, ip);
  });

}

void loop_ThNomade() {
int retThNomade = mRemoteThNomade.loop();
  static bool bWdogThNomadeErr = false;
  if (retThNomade == -2) {
    //Serial.printf("%s Inactif\n", mRemoteThNomade.nomEquipement.c_str());
    return;
  }
  else if (retThNomade == -10) { // Watchdog error
    if (!bWdogThNomadeErr) {
      DBGLN(DBG_CAPTEURS, "void loop()" + String(" - ") + " - " + mRemoteThNomade.nomEquipement+" n'a pas répondu depuis longtemps ==> KO " + mDateTime.getDate() + " " + mDateTime.getTime());
      ecran.updateRemoteDevice_ThNomade(mRemoteThNomade.nomEquipement, -254.0);
      bWdogThNomadeErr = true;
    }
  }
  else
    bWdogThNomadeErr = false;

}