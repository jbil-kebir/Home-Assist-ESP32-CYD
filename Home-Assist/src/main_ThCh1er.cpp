#include "global.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
#include "RemoteTh.h"
#include "RemoteBatAA.h"
#include "MyDateTime.h"

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;
extern CMyDateTime mDateTime;

CRemoteThermo mRemoteThCh1er(String("ThCh1er"), &ecran);
CRemoteBatterieAA mRemoteBatThCh1er(String("BatCh1er"), &ecran);


void setup_ThCh1er() {
  mRemoteThCh1er.begin("t1C_"); // Type, etage, lieu (thermomètre, 1er, chanmre)
  mRemoteBatThCh1er.begin("bat1er_");
  config.mRemoteThCh1er = &mRemoteThCh1er; // Toujours remote
  config.mRemoteBatThCh1er = &mRemoteBatThCh1er; // Toujours remote
  mRemoteThCh1er.setDisplayCallback([ptr = &ecran](const String& exp, float temp) {
      ptr->updateRemoteDevice_ThCh1er(exp, temp);
  });

  // Thermomètre Chambre 1er - publication MQTT
  mRemoteThCh1er.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });
  // Thermomètre Chambre 1er - Affichage
  mRemoteBatThCh1er.setDisplayCallback([ptr = &ecran](const String& exp, bool etatBatterie, float temp) {
      ptr->updateRemoteBat_ThCh1er(exp, etatBatterie, temp);
  });

  // Batterie du thermomètre Salle de bain - publication MQTT
  mRemoteBatThCh1er.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });
}

void loop_ThCh1er() {
  int retThCh1er = mRemoteThCh1er.loop();
  static bool bWdogThCh1erErr = false;
  if (retThCh1er == -2) {
    //Serial.printf("%s Inactif\n", mRemoteThCh1er.nomEquipement.c_str());
    return;
  }
  else if (retThCh1er == -10) { // Watchdog error
    if (!bWdogThCh1erErr) {
      Serial.println("void loop()" + String(" - ") + " - " + mRemoteThCh1er.nomEquipement+" n'a pas répondu depuis longtemps ==> KO " + mDateTime.getDate() + " " + mDateTime.getTime());
      ecran.updateRemoteDevice_ThCh1er(mRemoteThCh1er.nomEquipement, -254.0);
      bWdogThCh1erErr = true;
    }
  }
  else
    bWdogThCh1erErr = false;
}