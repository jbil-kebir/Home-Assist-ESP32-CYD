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
#include "RemoteCapteurBase.h"
#include "MyDateTime.h"

extern CConfig config;
extern CEcran ecran;
extern CMqtt mqtt;
extern CMyDateTime mDateTime;

#ifdef __LOCAL_MODE__
#ifdef __LOCAL_DS18B20__
MyDS18B20 ds18b20(DEFAULT_DS18B20_PIN);  
#elif defined(__LOCAL_DHT20__)
MyDHT20 dht20(DEFAULT_DHT20_SDA_PIN, DEFAULT_DHT20_SCL_PIN);
#endif
#else
CRemoteThermo mRemoteThMain(String("ThMain"), &ecran); // ds18b20 distant sur la carte de commande (ThCYD)
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
  #elif defined(__LOCAL_DHT20__)
  DBG(DBG_CAPTEURS, "Initialisation DHT20...\n");
  dht20.begin("th_");
  config.dht20 = &dht20;
  dht20.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
    return mqtt.publishWithIP(topic, payload);
  });
  #endif
#else
  config.mRemoteThMain = &mRemoteThMain;
  config.mRemoteBatMain = &mRemoteBatMain;
  mRemoteThMain.begin("th_");
  mRemoteThMain.setDelaiSaisi(true);  // ThCYD (thermomètre du principal) : n'annonce pas de délai d'alerte
  mRemoteBatMain.begin("batmn_"); // Préfixe propre (partageait bat1er_ avec BatCh1er)
  mRemoteBatMain.setDelaiSaisi(true);
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

#ifdef __LOCAL_MODE__
#if defined(__LOCAL_DS18B20__) || defined(__LOCAL_DHT20__)
//
// Thermomètre local (ThCYD) en panne / revenu (LOCAL_THERMO_ECHECS_PANNE lectures ratées d'affilée) :
// - écran : WDG à la place de la température (CEcran::updateThermometreLocal()), redessiné à chaque changement ;
// - Home Assistant : « RETARD » / « OK » sur home/alerte/ThCYD, comme les capteurs distants (onAlerte, main.cpp).
//   Publié à chaque changement et une fois après le démarrage (1re lecture réussie ou panne), dès que MQTT est
//   connecté (efface un RETARD retenu d'avant le redémarrage). Thermomètre inactif : jamais en panne.
//
template <class T> static void surveillePanneThermoLocal(T& th) {
  static bool sbPanneAffichee = false;
  static int8_t sciEtatPublie = -1;          // -1 rien publié, 0 OK, 1 panne
  bool bPanne = th.active && th.estEnPanne();
  if (bPanne != sbPanneAffichee) {
    sbPanneAffichee = bPanne;
    DBGLN(DBG_CAPTEURS, th.nomEquipement + (bPanne ? " en panne (WDG)" : " revenu"));
    ecran.updateThermometreLocal();
  }
  int8_t ciEtat = bPanne ? 1 : ((th.aDejaLu() || !th.active) ? 0 : -1);
  if (ciEtat != -1 && ciEtat != sciEtatPublie && mqtt.isConnected() && CRemoteCapteurBase::onAlerte != nullptr) {
    CRemoteCapteurBase::onAlerte(th.nomEquipement, bPanne, th.getSilenceS());
    sciEtatPublie = ciEtat;
  }
}
#endif
#endif

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

  #elif defined(__LOCAL_DHT20__)
  static bool inactifAffiche = false;
  int ret = dht20.loop();
  if (ret == -1) {
    DBG(DBG_CAPTEURS, "dht20.readTemperature() - échec\n");
  }
  else if (ret == 0) {
    DBGLN(DBG_CAPTEURS, String(dht20.nomEquipement) + " - " + mDateTime.getDate() + " - " + mDateTime.getTime() + " - " + "Température " + String(dht20.getLastTemperature(), 1) + " °C inchangé");
  }
  else if (ret == 1 && dht20.active) { // Température changée
    DBGLN(DBG_CAPTEURS, String(dht20.nomEquipement) + " - " + mDateTime.getDate() + " - " + mDateTime.getTime() + " - " + "Température " + String(dht20.getLastTemperature(), 1) + " °C changée");
    ecran.updateThermometreLocal();
  }
  else if (ret == -9) { // Inactif
    if (!inactifAffiche) {
      String s = "Thermomètre " + dht20.nomEquipement + " inactif";
      DBGLN(DBG_CAPTEURS, s);
      inactifAffiche = true;
    }
  }

  if (ret != -9) inactifAffiche = false;

  #endif // __LOCAL_DS18B20__ / __LOCAL_DHT20__

  #ifdef __LOCAL_MODE__
  #ifdef __LOCAL_DS18B20__
  surveillePanneThermoLocal(ds18b20);
  #elif defined(__LOCAL_DHT20__)
  surveillePanneThermoLocal(dht20);
  #endif
  #endif
  
}