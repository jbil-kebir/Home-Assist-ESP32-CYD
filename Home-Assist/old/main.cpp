#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "global.h"
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyWifi.h"
#include "MyRadioTx.h"
#include "MyMqtt.h"
#include "MyWebServer.h"


// === OBJETS GLOBAUX ===
CConfig config;
CEcran ecran(config);
CWifi wifi(config);
WiFiClient wifiClient;
CMqtt mqtt(wifiClient, config);
CRadioTX radioTX(&ecran,config);
MyWebServer webServer(config, radioTX, ecran, mqtt);

// === SETUP & LOOP ===
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== AD200 " + String(VERSION) + " TX - Classes OOP ===");

  config.setup();
  ecran.setup();
  ecran.updateBoilerStatus(config.etatStr); // On affiche le dernier état de la chaudière
//  bool bEtatChaudiere = config.etat;
//  radioTX.isetUpStatus(bEtatChaudiere);

  radioTX.setup();
  wifi.setup();
  
  mqtt.setup([&](bool on) {
    if (on) {
      radioTX.bSetEnvoyerTramesOFF(false);
      if (radioTX.bGetEnvoiEnCours()) {
        Serial.printf("Envoi déjà en cours");
      }
      else {
        radioTX.bSetEnvoyerTramesON(true);
      }
      config.saveBoilerState(true);
      //mqtt.publishState(true);
      ecran.updateBoilerStatus("ON");
      Serial.printf("mqtt request - ON\n");
    } 
    else {
      radioTX.bSetEnvoyerTramesON(false);
      if (radioTX.bGetEnvoiEnCours()) {
        Serial.printf("Envoi déjà en cours");
      }
      else {
        radioTX.bSetEnvoyerTramesOFF(true);
      }
      config.saveBoilerState(false);
      //mqtt.publishState(false); KJ
      ecran.updateBoilerStatus("OFF");
      Serial.printf("mqtt request - OFF\n");
    }
    //mqtt.publishState(on); KJ
  });

  if (WiFi.status() == WL_CONNECTED) {
    webServer.setup();
  }

  ecran.drawMainInterface();

  

}


void loop() {
  mqtt.loop();  // Toujours actif, même écran éteint
  
  radioTX.loop();

  webServer.loop();

  int x, y;
  if (ecran.isTouched(x, y)) {

    // Gestion des boutons
    if (x >= 20 && x <= 110 && y >= 50 && y <= 110) {
      ecran.updateStatus("Capture non utilisee");
    }
    else if (x >= 115 && x <= 205 && y >= 50 && y <= 110) { // Bouton ON appuyé ?
      // Mettre un délai avant de pouvoir arrêter l'envoi du OFF 
      // après demande OFF
      bool b = radioTX.bGetEnvoiEnCours();
      Serial.printf("main loop - ON appuye - Envoi en cours : % d\n", b);
      radioTX.bSetEnvoyerTramesOFF(false);
      if (radioTX.bGetEnvoiEnCours()) {
        Serial.printf("Envoi déjà en cours\n");
      }
      else {
        radioTX.bSetEnvoyerTramesON(true);
        //mqtt.publishState(true); KJ
        config.saveBoilerState(true);
        ecran.updateBoilerStatus("ON");
      }
  
    }
    else if (x >= 210 && x <= 300 && y >= 50 && y <= 110) { // Bouton OFF appuyé ?
      // Mettre un délai avant de pouvoir arrêter l'envoi du ON 
      // après demande ON
      bool b = radioTX.bGetEnvoiEnCours();
      Serial.printf("main loop - OFF appuye - Envoi en cours : % d\n", b);
      radioTX.bSetEnvoyerTramesON(false);
      if (radioTX.bGetEnvoiEnCours()) {
        Serial.printf("Envoi déjà en cours\n");
      }
      else {
        radioTX.bSetEnvoyerTramesOFF(true);
        //mqtt.publishState(false); KJ
        config.saveBoilerState(false);
        ecran.updateBoilerStatus("OFF");
      }
    }

    while (ecran.isTouched(x, y)) delay(10);
  }


  ecran.loop();


  delay(10);
}