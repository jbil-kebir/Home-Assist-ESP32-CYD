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
#include "RemoteBatAA.h"
#include "MyDateTime.h"

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;
extern CMyDateTime mDateTime;

#ifdef __LOCAL_MODE__
#ifdef __LOCAL_DS18B20__
MyDS18B20 ds18b20(DEFAULT_DS18B20_PIN);  
#endif
#else
CRemoteThermo mRemoteThMain(String("ThMain"), &ecran); // ds18b20 distant sur la carte de commande
CRemoteBatterieAA mRemoteBatMain(String("BatMain"), &ecran); // Sera désactivé car la carte est alimentée en permanence par le secteur
#endif


void setup_ds18b20() {
#ifdef __LOCAL_MODE__
  #ifdef __LOCAL_DS18B20__
  Serial.println("Initialisation DS18B20...");
  ds18b20.begin("th_");
  config.ds18b20 = &ds18b20;
  ds18b20.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
    return ptr->publishWithIP(topic, payload);
  });
  #endif
#else
  config.mRemoteThMain = &mRemoteThMain;
  config.mRemoteBatMain = &mRemoteBatMain;
  mRemoteThMain.begin("th_");
  mRemoteBatMain.begin("bat1er_");
  // Affichage écran
  mRemoteThMain.setDisplayCallback([ptr = &ecran](const String& exp, float temp) {
      ptr->updateRemoteDevice_DS18B20(exp, temp);
  });

  // publication MQTT
  mRemoteThMain.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });
  // Batterie du thermomètre - Affichage écran
  mRemoteBatMain.setDisplayCallback([ptr = &ecran](const String& exp, bool etatBatterie, float temp) {
      ptr->updateRemoteBat_DS18B20(exp, etatBatterie, temp);
  });

  // Batterie du thermomètre - publication MQTT
  mRemoteBatMain.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });  

#endif
}

void loop_ds18b20() {
  #ifdef __LOCAL_DS18B20__
  static bool inactifAffiche = false;
  int ret = ds18b20.loop();
  if (ret == -1) {
    Serial.println("ds18b20.readTemperature() - échec");
  } 
  else if (ret == 0) {
    Serial.println(String(ds18b20.nomEquipement) + " - " + mDateTime.getDate() + " - " + mDateTime.getTime() + " - " + "Température " + String(ds18b20.getLastTemperature(), 1) + " °C inchangé");
  } 
  else if (ret == 1 && ds18b20.active) { // Température changée
    Serial.println(String(ds18b20.nomEquipement) + " - " + mDateTime.getDate() + " - " + mDateTime.getTime() + " - " + "Température " + String(ds18b20.getLastTemperature(), 1) + " °C changée");
    ecran.updateThermometreLocal();
  } 
  else if (ret == -9) { // Inactif
    if (!inactifAffiche) {
      String s = "Thermomètre " + ds18b20.nomEquipement + " inactif";
      Serial.println(s);
      inactifAffiche = true;
    }
  }

  if (ret != -9) inactifAffiche = false;

  #endif // __LOCAL_DS18B20__
  
}