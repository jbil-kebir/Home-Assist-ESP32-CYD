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

CRemoteDHT20 mRemoteThRemise(String("ThRemise"), &ecran);
CRemoteBatterieAA mRemoteBatRemise(String("BatRemise"), &ecran);

extern int ajouterControleur(const String& nom, const String& ip);

void setup_ThRemise() {
  mRemoteThRemise.begin("threm_"); // Type, etage, lieu (thermomètre, 1er, chanmre)
  mRemoteBatRemise.begin("batrem_");
  config.mRemoteThRemise = &mRemoteThRemise; // Toujours remote
  config.mRemoteBatRemise = &mRemoteBatRemise; // Toujours remote
  // Thermomètre Remise - affichage température
  mRemoteThRemise.setDisplayCallbackTemperature([](const String& exp, float temp) {
      ecran.updateRemoteDevice_ThRemise(exp, temp);
  });
  // Thermomètre Remise - affichage humidité
  mRemoteThRemise.setDisplayCallbackHumidite([](const String& exp, float temp) {
      ecran.updateRemoteDevice_ThRemiseH(exp, temp);
  });

  // Thermomètre Remise - publication MQTT
  mRemoteThRemise.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });

  // Batterie du thermomètre Remise - affichage
  mRemoteBatRemise.setDisplayCallback([](const String& exp, bool etatBatterie, float temp) {
      ecran.updateRemoteBat_ThRemise(exp, etatBatterie, temp);
  });

  // Batterie du thermomètre Remise - publication MQTT
  mRemoteBatRemise.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  mRemoteThRemise.setonEquipementCallback([](const String& nom, const String& ip) -> int {
      return ajouterControleur(nom, ip);
  });

}

void loop_ThRemise() {
int retThRemise = mRemoteThRemise.loop();
  static bool bWdogThRemiseErr = false;
  if (retThRemise == -2) {
    //Serial.printf("%s Inactif\n", mRemoteThRemise.nomEquipement.c_str());
    return;
  }
  else if (retThRemise == -10) { // Watchdog error
    if (!bWdogThRemiseErr) {
      DBGLN(DBG_CAPTEURS, "void loop()" + String(" - ") + " - " + mRemoteThRemise.nomEquipement+" n'a pas répondu depuis longtemps ==> KO " + mDateTime.getDate() + " " + mDateTime.getTime());
      ecran.updateRemoteDevice_ThRemise(mRemoteThRemise.nomEquipement, -254.0);
      bWdogThRemiseErr = true;
    }
  }
  else
    bWdogThRemiseErr = false;

}