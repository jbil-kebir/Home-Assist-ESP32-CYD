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

CRemoteDHT20 mRemoteThNomade(String("ThNomade"), &ecran);
CRemoteBatterieAA mRemoteBatNomade(String("BatNomade"), &ecran);


void setup_ThNomade() {
  mRemoteThNomade.begin("thnmd_"); // Type, etage, lieu (thermomètre, 1er, chanmre)
  mRemoteBatNomade.begin("batnmd_");
  config.mRemoteThNomade = &mRemoteThNomade; // Toujours remote
  config.mRemoteBatNomade = &mRemoteBatNomade; // Toujours remote
  // Thermomètre Nomade - affichage température
  mRemoteThNomade.setDisplayCallbackTemperature([ptr = &ecran](const String& exp, float temp) {
      ptr->updateRemoteDevice_ThNomade(exp, temp);
  });
  // Thermomètre Nomade - affichage humidité
  mRemoteThNomade.setDisplayCallbackHumidite([ptr = &ecran](const String& exp, float temp) {
      ptr->updateRemoteDevice_ThNomadeH(exp, temp);
  });

  // Thermomètre Nomade - publication MQTT
  mRemoteThNomade.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });

  // Batterie du thermomètre Nomade - affichage
  mRemoteBatNomade.setDisplayCallback([ptr = &ecran](const String& exp, bool etatBatterie, float temp) {
      ptr->updateRemoteBat_ThNomade(exp, etatBatterie, temp);
  });

  // Batterie du thermomètre Nomade - publication MQTT
  mRemoteBatNomade.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
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
      Serial.println("void loop()" + String(" - ") + " - " + mRemoteThNomade.nomEquipement+" n'a pas répondu depuis longtemps ==> KO " + mDateTime.getDate() + " " + mDateTime.getTime());
      ecran.updateRemoteDevice_ThNomade(mRemoteThNomade.nomEquipement, -254.0);
      bWdogThNomadeErr = true;
    }
  }
  else
    bWdogThNomadeErr = false;

}