#include "global.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
#ifndef __LOCAL_MODE__
#include "RemoteRCDevice.h"
#endif

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;

// Lumière de la cave : prise 433 MHz pilotée comme le projecteur, depuis Home Assistant (bouton + cadenas de la vue
// Actionneurs) et depuis l'écran (bouton « Cave » à droite de « SB », son cadenas à sa droite : BTN_LUMIERE_CAVE_*).
// CYD principal (__LOCAL_MODE__) : émet en 433 MHz. CYD auxiliaire : appareil distant (comme la guirlande),
// publie ON / OFF / ENABLE / DISABLE et suit les réponses du principal.
// Topic home/lumierecave/command : ON / OFF (réponse ONR / OFFR), ENABLE / DISABLE (réponse ENABLER / DISABLER).
#ifdef __LOCAL_MODE__
CRCDevice lumiereCave(String("LumiereCave"), ecran);
#else
CRemoteRCDevice mRemoteLumiereCave(String("LumiereCave"), ecran);
#endif

void setup_lumiereCave() {
#ifdef __LOCAL_MODE__
  config.lumiereCave = &lumiereCave;
  lumiereCave.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
    return mqtt.publish(topic, payload);
  });
  // Code 0x1FCF24, protocole 1, 301 µs : valeurs par défaut tant que la page Web n'a rien enregistré
  lumiereCave.setDefauts(0x1FCF24, 1, 301, "LumiereCave", "Lumière cave", "lumierecave/");
  lumiereCave.setup("lcav_");
#else
  config.mRemoteLumiereCave = &mRemoteLumiereCave;
  mRemoteLumiereCave.setDefauts("LumiereCave", "lumierecave/"); // Même topic que le principal tant que la page Web n'a rien enregistré
  mRemoteLumiereCave.setup("lcav_");
  mRemoteLumiereCave.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
#endif // __LOCAL_MODE__
}
