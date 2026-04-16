#include "global.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
#ifdef __LOCAL_MODE__
#else
#include "RemoteRCDevice.h"
#endif

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;

#ifdef __LOCAL_MODE__
CChaudiere chaudiere(String("Chaudiere"), ecran);
#else
CRemoteChaudiere mRemoteChaudiere(String("Chaudiere"), ecran);
#endif

void setup_chaudiere() {
#ifdef __LOCAL_MODE__
  config.chaudiere = &chaudiere;
  chaudiere.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  chaudiere.setup();
#else
  config.mRemoteChaudiere = &mRemoteChaudiere;
  mRemoteChaudiere.setup();
  mRemoteChaudiere.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
#endif
}