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

CRemoteDHT20 mRemoteThCave(String("ThCave"), &ecran);
CRemoteBatterieAA mRemoteBatCave(String("BatCave"), &ecran);
CRemoteTor mRemoteTor(String("TorCave"), &ecran);

extern int ajouterControleur(const String& nom, const String& ip);

void setup_ThCave() {
  mRemoteThCave.begin("thCve_"); // Type, etage, lieu (thermomètre, 1er, chanmre)
  mRemoteBatCave.begin("batcve_");
  mRemoteTor.setup("torcve_");
  config.mRemoteThCave = &mRemoteThCave; // Toujours remote
  config.mRemoteTor = &mRemoteTor; // Toujours remote
  config.mRemoteBatCave = &mRemoteBatCave; // Toujours remote
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
  mRemoteThCave.setonEquipementCallback([](const String& nom, const String& ip) -> int {
      return ajouterControleur(nom, ip);
  });

}

void loop_ThCave() {
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