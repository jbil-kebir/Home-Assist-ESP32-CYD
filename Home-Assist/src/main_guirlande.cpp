#include "global.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#ifdef __LOCAL_MODE__
#endif
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
CRCDevice  guirlande(String("Guirlande"), ecran);
#else
CRemoteRCDevice mRemoteGuirlande(String("Guirlande"), ecran);
#endif


void setup_guirlande() {
#ifdef __LOCAL_MODE__
  config.guirlande = &guirlande;
  guirlande.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
    return mqtt.publish(topic, payload);
  });
  guirlande.setup("guir_");
#else
  config.mRemoteGuirlande = &mRemoteGuirlande;
  mRemoteGuirlande.setup("guir_");
  mRemoteGuirlande.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
#endif // __LOCAL_MODE__
}
