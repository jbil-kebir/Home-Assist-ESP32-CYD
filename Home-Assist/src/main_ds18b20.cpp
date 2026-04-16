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
  DBG(DBG_CAPTEURS, "Initialisation DS18B20...\n");
  ds18b20.begin("th_");
  config.ds18b20 = &ds18b20;
  ds18b20.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
    return mqtt.publishWithIP(topic, payload);
  });
  #endif
#else
  config.mRemoteThMain = &mRemoteThMain;
  config.mRemoteBatMain = &mRemoteBatMain;
  mRemoteThMain.begin("th_");
  mRemoteBatMain.begin("bat1er_");
  // Affichage écran
  mRemoteThMain.setDisplayCallback([](const String& exp, float temp) {
      ecran.updateRemoteDevice_DS18B20(exp, temp);
  });

  // publication MQTT
  mRemoteThMain.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  // Batterie du thermomètre - Affichage écran
  mRemoteBatMain.setDisplayCallback([](const String& exp, bool etatBatterie, float temp) {
      ecran.updateRemoteBat_DS18B20(exp, etatBatterie, temp);
  });

  // Batterie du thermomètre - publication MQTT
  mRemoteBatMain.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });  

#endif
}

void loop_ds18b20() {
  #ifdef __LOCAL_DS18B20__
  static bool inactifAffiche = false;
  int ret = ds18b20.loop();
  if (ret == -1) {
    DBG(DBG_CAPTEURS, "ds18b20.readTemperature() - échec\n");
  }
  else if (ret == 0) {
    DBGLN(DBG_CAPTEURS, String(ds18b20.nomEquipement) + " - " + mDateTime.getDate() + " - " + mDateTime.getTime() + " - " + "Température " + String(ds18b20.getLastTemperature(), 1) + " °C inchangé");
  }
  else if (ret == 1 && ds18b20.active) { // Température changée
    DBGLN(DBG_CAPTEURS, String(ds18b20.nomEquipement) + " - " + mDateTime.getDate() + " - " + mDateTime.getTime() + " - " + "Température " + String(ds18b20.getLastTemperature(), 1) + " °C changée");
    ecran.updateThermometreLocal();
  }
  else if (ret == -9) { // Inactif
    if (!inactifAffiche) {
      String s = "Thermomètre " + ds18b20.nomEquipement + " inactif";
      DBGLN(DBG_CAPTEURS, s);
      inactifAffiche = true;
    }
  }

  if (ret != -9) inactifAffiche = false;

  #endif // __LOCAL_DS18B20__
  
}