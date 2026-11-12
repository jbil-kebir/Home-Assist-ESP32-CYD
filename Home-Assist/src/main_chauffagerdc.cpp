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

// Chauffage de la chambre du rez-de-chaussée : prise 433 MHz pilotée comme la lumière de la cave, depuis Home Assistant
// (bouton + cadenas de la vue Actionneurs) et depuis l'écran (bouton « Ch.RdC » à droite de « SB », son cadenas à sa
// droite : BTN_CHAUFFAGE_RDC_*).
// CYD principal (__LOCAL_MODE__) : émet en 433 MHz. CYD auxiliaire : appareil distant (comme la guirlande),
// publie ON / OFF / ENABLE / DISABLE et suit les réponses du principal.
// Topic home/chauffagerdc/command : ON / OFF (réponse ONR / OFFR), ENABLE / DISABLE (réponse ENABLER / DISABLER).
#ifdef __LOCAL_MODE__
CRCDevice chauffageRdc(String("ChauffageRdc"), ecran);
#else
CRemoteRCDevice mRemoteChauffageRdc(String("ChauffageRdc"), ecran);
#endif

void setup_chauffageRdc() {
#ifdef __LOCAL_MODE__
  config.chauffageRdc = &chauffageRdc;
  chauffageRdc.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
    return mqtt.publish(topic, payload);
  });
  // Code 0x5B0E64 (24 bits), protocole 1, 306 µs (capture de l'interrupteur du 12/11/2026) : valeurs par défaut tant
  // que la page Web n'a rien enregistré
  chauffageRdc.setDefauts(0x5B0E64, 1, 306, "ChauffageRdc", "Chauffage RdC", "chauffagerdc/");
  chauffageRdc.setup("crdc_");
#else
  config.mRemoteChauffageRdc = &mRemoteChauffageRdc;
  mRemoteChauffageRdc.setDefauts("ChauffageRdc", "chauffagerdc/"); // Même topic que le principal tant que la page Web n'a rien enregistré
  mRemoteChauffageRdc.setup("crdc_");
  mRemoteChauffageRdc.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
#endif // __LOCAL_MODE__
}
