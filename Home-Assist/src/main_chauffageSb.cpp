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
CRCDevice  chauffageSb(String("Chauffage"), ecran);
#else
CRemoteRCDevice mRemoteChauffage(String("Chauffage"), ecran);
#endif

void setup_chauffageSb() {
#ifdef __LOCAL_MODE__
  config.chauffageSb = &chauffageSb;
  chauffageSb.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
    return ptr->publish(topic, payload);
  });
  chauffageSb.setup("sb_");
#else
  config.mRemoteChauffage = &mRemoteChauffage;
  mRemoteChauffage.setup("sb_");
  mRemoteChauffage.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });
#endif // __LOCAL_MODE__
}