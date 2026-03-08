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

CRemoteThermo mRemoteThSdb(String("ThSdb"), &ecran);
CRemoteBatterieAA mRemoteBatSdb(String("BatSdb"), &ecran);

extern int ajouterControleur(const String& nom, const String& ip);

void setup_ThSdb() {
  mRemoteThSdb.begin("t0Sdb_"); // Type, etage, lieu (thermomètre, 1er, chanmre)
  mRemoteBatSdb.begin("batsdb_");
  config.mRemoteThSdb = &mRemoteThSdb; // Toujours remote
  config.mRemoteBatSdb = &mRemoteBatSdb; // Toujours remote

  // Thermomètre Salle de bain - affichage
  mRemoteThSdb.setDisplayCallback([ptr = &ecran](const String& exp, float temp) {
      ptr->updateRemoteDevice_ThSdb(exp, temp);
  });

  // Thermomètre Salle de bain - publication MQTT
  mRemoteThSdb.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });
  // Batterie du thermomètre Salle de bain - affichage
  mRemoteBatSdb.setDisplayCallback([ptr = &ecran](const String& exp, bool etatBatterie, float temp) {
      ptr->updateRemoteBat_ThSdb(exp, etatBatterie, temp);
  });

  // Batterie du thermomètre Salle de bain - publication MQTT
  mRemoteBatSdb.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });
  mRemoteThSdb.setonEquipementCallback([](const String& nom, const String& ip) -> int {
      return ajouterControleur(nom, ip);
  });
}

void loop_ThSdb() {
  int retThSdb = mRemoteThSdb.loop();
  static bool bWdogThSdbErr = false;
  if (retThSdb == -2) {
    //Serial.printf("%s Inactif\n", mRemoteThSdb.nomEquipement.c_str());
    return;
  }
  else if (retThSdb == -10) { // Watchdog error
    if (!bWdogThSdbErr) {
      Serial.println("void loop()" + String(" - ") + " - " + mRemoteThSdb.nomEquipement+" n'a pas répondu depuis longtemps ==> KO " + mDateTime.getDate() + " " + mDateTime.getTime());
      ecran.updateRemoteDevice_ThSdb(mRemoteThSdb.nomEquipement, -254.0);
      bWdogThSdbErr = true;
    }
  }
  else
    bWdogThSdbErr = false;

}