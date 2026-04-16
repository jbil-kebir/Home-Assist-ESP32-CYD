#include "global.h"
#include <Arduino.h>
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
#include "RemoteTor.h"

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;
extern CMyDateTime mDateTime;

extern int ajouterControleur(const String& nom, const String& ip);

CRemoteTor mRemoteNewNas(String("NewNas"), &ecran);
CRemoteTor mRemoteBigNas(String("BigNas"), &ecran);

void setup_homeassistant() {
  mRemoteNewNas.setup("ha1_");
  mRemoteNewNas.nomEquipement = "NewNas";
  mRemoteNewNas.mqttSubTopicState   = "home/homeassistant/state";
  mRemoteNewNas.mqttSubTopicCommand = "home/homeassistant/command";

  mRemoteBigNas.setup("ha2_");
  mRemoteBigNas.nomEquipement = "BigNas";
  mRemoteBigNas.mqttSubTopicState   = "home/homeassistant/state";
  mRemoteBigNas.mqttSubTopicCommand = "home/homeassistant/command";

  config.mRemoteNewNas = &mRemoteNewNas;
  config.mRemoteBigNas = &mRemoteBigNas;

  mRemoteNewNas.setDisplayCallback([](const String& exp, int val) {
    ecran.updateRemoteDevice_NewNas(exp, val);
  });
  mRemoteNewNas.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
    return mqtt.publish(topic, payload);
  });
  mRemoteNewNas.setonEquipementCallback([](const String& nom, const String& ip) -> int {
    return ajouterControleur(nom, ip);
  });

  mRemoteBigNas.setDisplayCallback([](const String& exp, int val) {
    ecran.updateRemoteDevice_BigNas(exp, val);
  });
  mRemoteBigNas.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
    return mqtt.publish(topic, payload);
  });
  mRemoteBigNas.setonEquipementCallback([](const String& nom, const String& ip) -> int {
    return ajouterControleur(nom, ip);
  });

}

void loop_NewNas() {
int retNewNas = mRemoteNewNas.loop();
  static bool bWdogNewNasErr = false;
  if (retNewNas == -2) {
    DBG(DBG_CONFIG, "%s Inactif\n", mRemoteNewNas.nomEquipement.c_str());
    return;
  }
  else if (retNewNas == -10) { // Watchdog error
    if (!bWdogNewNasErr) {
      DBGLN(DBG_CONFIG, "void loop()" + String(" - ") + " - " + mRemoteNewNas.nomEquipement+" n'a pas répondu depuis longtemps ==> KO " + mDateTime.getDate() + " " + mDateTime.getTime());
      ecran.updateRemoteDevice_NewNas(mRemoteNewNas.nomEquipement, -1.0);
      bWdogNewNasErr = true;
    } 
  }
  else
    bWdogNewNasErr = false;

}
void loop_BigNas() {
int retBigNas = mRemoteBigNas.loop();
  static bool bWdogBigNasErr = false;
  if (retBigNas == -2) {
    DBG(DBG_CONFIG, "%s Inactif\n", mRemoteBigNas.nomEquipement.c_str());
    return;
  }
  else if (retBigNas == -10) { // Watchdog error
    if (!bWdogBigNasErr) {
      DBGLN(DBG_CONFIG, "void loop()" + String(" - ") + " - " + mRemoteBigNas.nomEquipement+" n'a pas répondu depuis longtemps ==> KO " + mDateTime.getDate() + " " + mDateTime.getTime());
      ecran.updateRemoteDevice_BigNas(mRemoteBigNas.nomEquipement, -1.0);
      bWdogBigNasErr = true;
    } 
  }
  else
    bWdogBigNasErr = false;

}

void loop_homeassistant() {
  loop_NewNas();
  loop_BigNas();  
}