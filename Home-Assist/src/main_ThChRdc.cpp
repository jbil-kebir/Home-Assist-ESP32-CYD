#include "global.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
#include "RemoteDHT20.h"
#include "RemoteBatAA.h"
#include "MyDateTime.h"

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;
extern CMyDateTime mDateTime;

CRemoteDHT20 mRemoteThChRdc(String("ThChRdc"), &ecran);
CRemoteBatterieAA mRemoteBatChRdc(String("BatChRdc"), &ecran);

extern int ajouterControleur(const String& nom, const String& ip);

void setup_ThChRdc() {
  mRemoteThChRdc.begin("thcui_"); // Préfixe NVS historique (ex-ThCuisine) conservé pour garder la config
  mRemoteBatChRdc.begin("batcui_"); // Préfixe NVS historique (ex-BatCuisine)
  config.mRemoteThChRdc = &mRemoteThChRdc; // Toujours remote
  config.mRemoteBatChRdc = &mRemoteBatChRdc; // Toujours remote
  // Thermomètre Chambre RDC - affichage température
  mRemoteThChRdc.setDisplayCallbackTemperature([](const String& exp, float temp) {
      ecran.updateRemoteDevice_ThChRdc(exp, temp);
  });
  // Thermomètre Chambre RDC - affichage humidité
  mRemoteThChRdc.setDisplayCallbackHumidite([](const String& exp, float temp) {
      ecran.updateRemoteDevice_ThChRdcH(exp, temp);
  });

  // Thermomètre Chambre RDC - publication MQTT
  mRemoteThChRdc.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });

  // Batterie du thermomètre Chambre RDC - affichage
  mRemoteBatChRdc.setDisplayCallback([](const String& exp, bool etatBatterie, float temp) {
      ecran.updateRemoteBat_ThChRdc(exp, etatBatterie, temp);
  });

  // Batterie du thermomètre Chambre RDC - publication MQTT
  mRemoteBatChRdc.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  mRemoteThChRdc.setonEquipementCallback([](const String& nom, const String& ip) -> int {
      return ajouterControleur(nom, ip);
  });

}

void loop_ThChRdc() {
int retThChRdc = mRemoteThChRdc.loop();
  static bool bWdogThChRdcErr = false;
  if (retThChRdc == -2) {
    //Serial.printf("%s Inactif\n", mRemoteThChRdc.nomEquipement.c_str());
    return;
  }
  else if (retThChRdc == -10) { // Watchdog error
    if (!bWdogThChRdcErr) {
      DBGLN(DBG_CAPTEURS, "void loop()" + String(" - ") + " - " + mRemoteThChRdc.nomEquipement+" n'a pas répondu depuis longtemps ==> KO " + mDateTime.getDate() + " " + mDateTime.getTime());
      ecran.updateRemoteDevice_ThChRdc(mRemoteThChRdc.nomEquipement, -254.0);
      bWdogThChRdcErr = true;
    }
  }
  else
    bWdogThChRdcErr = false;

}