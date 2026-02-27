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
CRCDevice projecteur(String("Projecteur"), ecran);
#else
CRemoteRCDevice mRemoteProjecteur(String("Projecteur"), ecran);
#endif

void setup_projecteur() {
#ifdef __LOCAL_MODE__
  config.projecteur = &projecteur;
  projecteur.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
    return ptr->publish(topic, payload);
  });
  projecteur.setup("proj_");
#else
  config.mRemoteProjecteur = &mRemoteProjecteur;
  mRemoteProjecteur.setup("proj_");
  mRemoteProjecteur.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });
#endif // __LOCAL_MODE__
}