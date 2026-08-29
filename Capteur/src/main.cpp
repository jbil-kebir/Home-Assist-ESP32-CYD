#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "global.h"
#include "MyConfig.h"
#ifdef _WIFI_MODE_
#include "MyWifi.h"
#ifndef __DESACTIVE_ENVOI_MQTT__
#include "MyMqtt.h"
#endif
#include "MyWebServer.h"
#endif
#ifdef _RCSWITCH_MODE_
#include "MyRCSwitch.h"
#include "MyCC1101.h"
#endif
#ifdef _LORA_P2P_MODE_
#include "MyLoraRxTx.h"
#endif
#ifdef CAPTEUR_DS18B20
#include "MyDS18B20.h"
#endif
#ifdef CAPTEUR_DHT20
#include "DHT20Sensor.h"
#endif
#ifdef FLOTTEUR_VERTICAL
#include "MyTor.h"
#endif
#ifdef CAPTEUR_RGB_TCS34725
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "DetecteurRGB_TCS34725.h"
#endif

#ifdef CAPTEUR_BATTERIE
#include "MyBatterieAA.h"
#endif
#include "MyDateTime.h"




// === OBJETS GLOBAUX ===
CMyDateTime mDateTime;
CConfig config("Thermomètre ThCh1er", mDateTime);
#ifdef _WIFI_MODE_ 
CWifi mWifi[MAX_WIFI_NETS];
WiFiClient wifiClient;
#ifndef __DESACTIVE_ENVOI_MQTT__
CMqtt mqtt(wifiClient, config/*, mRmoteTH1*/);
#endif
#ifndef __DESACTIVE_ENVOI_MQTT__
MyWebServer webServer(config, mqtt, mWifi);
#else
MyWebServer webServer(config, mWifi);
#endif
#endif
#ifdef _RCSWITCH_MODE_
CCC1101 mCC1101;
CMyRCSwitch mRCSwitch(config);
#endif
#ifdef _LORA_P2P_MODE_
CMyLoraRxTx mLoraRxTx(config); //, radio);
#endif
#ifdef CAPTEUR_DS18B20
MyDS18B20 ds18b20(DS18B20_PIN, mDateTime);  
#endif
#ifdef CAPTEUR_DHT20
DHT20Sensor dht20(mDateTime);  
#endif
#ifdef CAPTEUR_BATTERIE
CBatterieAA mBatterie(BATTERIE_PIN, mDateTime);
#endif
#ifdef FLOTTEUR_VERTICAL
CTor mFlotteurVertical(mDateTime);  
#endif
#ifdef CAPTEUR_RGB_TCS34725
CDetecteurRGB_TCS34725 mCapteurRGB(mDateTime);
#endif



void print();
void printBoardInfo();





//-------------------------------------------------------------------------------------------
//      SETUP
//-------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while(!Serial && millis() - start < 5000); // Attend max 5 secondes (USB CDC : re-énumération après reboot)

  Serial.println("\n\n=== Démarrage de " + String(config.nomEquipement) + " (version " + String(VERSION) + ") ===");

  randomSeed(esp_random());  // Initialisation aléatoire (esp_random = RNG matériel)
  printBoardInfo();  // affiche "Carte détectée : ESP32-C3 Super Mini" ou S3

  //--------------------------------------------------------------------------------------------
  // Neutralisation des broches des capteurs/actionneurs non utilisés
  //--------------------------------------------------------------------------------------------
  // But : un capteur/actionneur désactivé en commentant son #define dans global.h (ex :
  // CAPTEUR_DS18B20) ne doit pas rester "alimenté" via une broche flottante ou tirée par une
  // pull-up externe. On la repasse en INPUT_PULLDOWN (comme le fait déjà le nettoyage GPIO avant
  // deep-sleep dans MyConfig.cpp) plutôt qu'en OUTPUT/LOW : ça évite tout conflit si la broche
  // est en réalité câblée à autre chose, tout en la ramenant à un niveau bas défini.
  //
  // Certaines broches sont réutilisées d'une fonctionnalité à l'autre selon la config active
  // (ex : DS18B20_PIN et BATTERIE_PIN partagent la broche 2 quand CAPTEUR_DS18B20 est désactivé ;
  // LED_CAPTEUR_RGB_PIN et DEFAULT_TOR_PIN partagent la broche 1 sur ESP32-C3). On ne peut donc
  // pas neutraliser "la broche du capteur X" seulement parce que X est désactivé : il faut
  // vérifier qu'aucune fonctionnalité active ne la réclame. D'où l'approche par listes : on
  // recense les broches réellement utilisées (usedPins, dépend des #ifdef actifs) et on neutralise
  // toute broche candidate (candidatePins, toujours définie quelle que soit la config) qui n'y
  // figure pas.
  {
    // Broches réellement utilisées par les fonctionnalités actives.
    // Sentinelle 255 en fin de tableau : garantit un tableau non vide (donc valide en C++) même
    // si aucun capteur/actionneur n'est actif ; 255 n'est jamais une broche GPIO valide sur ces cartes.
    const uint8_t usedPins[] = {
      #ifdef CAPTEUR_DS18B20
      DS18B20_PIN,
      #endif
      #ifdef CAPTEUR_BATTERIE
      BATTERIE_PIN,
      #endif
      #ifdef FLOTTEUR_VERTICAL
      DEFAULT_TOR_PIN,
      #endif
      #if defined(CAPTEUR_DHT20) || defined(CAPTEUR_RGB_TCS34725)
      DEFAULT_SDA_PIN, DEFAULT_SCL_PIN, // bus I2C partagé entre DHT20 et le capteur RGB
      #endif
      #ifdef CAPTEUR_RGB_TCS34725
      CAPTEUR_RGB_TCS34725_INTERRUPT,
      #ifdef LED_CAPTEUR_RGB
      LED_CAPTEUR_RGB_PIN,
      #endif
      #endif
      #if IS_ESP32_C3 && defined(_RCSWITCH_MODE_)
      CC1101_GDO0, CC1101_CS, CC1101_MOSI, CC1101_MISO, CC1101_SCK, CC1101_POWER_GND_GPIO,
      #endif
      #if IS_ESP32_S3 && defined(_LORA_P2P_MODE_)
      LORA_MISO_PIN, LORA_SCK_PIN, LORA_MOSI_PIN, LORA_CS_PIN,
      LORA_DIO2_PIN, LORA_DIO1_PIN, LORA_RESET_PIN, LORA_BUSY_PIN,
      #endif
      255
    };

    // Toutes les broches candidates du montage (capteurs + actionneurs prévus sur cette carte),
    // utilisées ou non par la config active.
    const uint8_t candidatePins[] = {
      DS18B20_PIN, BATTERIE_PIN, DEFAULT_TOR_PIN,
      DEFAULT_SDA_PIN, DEFAULT_SCL_PIN,
      CAPTEUR_RGB_TCS34725_INTERRUPT, LED_CAPTEUR_RGB_PIN,
      #if IS_ESP32_C3
      CC1101_GDO0, CC1101_CS, CC1101_MOSI, CC1101_MISO, CC1101_SCK, CC1101_POWER_GND_GPIO,
      #endif
      #if IS_ESP32_S3
      LORA_MISO_PIN, LORA_SCK_PIN, LORA_MOSI_PIN, LORA_CS_PIN,
      LORA_DIO2_PIN, LORA_DIO1_PIN, LORA_RESET_PIN, LORA_BUSY_PIN,
      #endif
    };

    for (uint8_t candidate : candidatePins) {
      bool used = false;
      for (uint8_t u : usedPins) {
        if (u == candidate) { used = true; break; }
      }
      if (!used) {
        pinMode(candidate, INPUT_PULLDOWN);
      }
    }
  }

  #ifdef _RCSWITCH_MODE_
  pinMode(CC1101_POWER_GND_GPIO, OUTPUT);
  digitalWrite(CC1101_POWER_GND_GPIO, CC1101_ON);  // Allume le CC1101
  #endif
  
  
  config.setPrefixNVS("cfg_");

  #ifdef _LORA_P2P_MODE_
  int ret = mLoraRxTx.setup();
  if (ret != 0) {
    Serial.println("Erreur init LoRa RX/TX : " + String(ret));
  }
  #endif


  #ifdef _LORA_P2P_MODE_
  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mLoraRxTx.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  #endif
  #endif
  #endif // _LORA_P2P_MODE_

  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  config.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  #endif
  #endif
  Serial.printf("\n=== %s %s ===\n",config.nomEquipement.c_str(), VERSION);

  //================================================== DS18B20 ==================================================
  #ifdef CAPTEUR_DS18B20
  Serial.println("Initialisation DS18B20...");
  ds18b20.begin("th_");
  config.ds18b20 = &ds18b20;
  #ifdef _WIFI_MODE_
  // Thermomètre Chambre 1er - publication MQTT
  #ifndef __DESACTIVE_ENVOI_MQTT__
  ds18b20.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  #endif
  #endif
  #ifdef _RCSWITCH_MODE_
  ds18b20.mRCSwitch = &mRCSwitch;
  #endif
  #ifdef _LORA_P2P_MODE_
  ds18b20.setLoraP2PPublishCallback([](const char* payload) -> int {
      return mLoraRxTx.sendPacket(payload);
  });
  #endif
  #endif // CAPTEUR_DS18B20
  //================================================== Batterie ==================================================
  #ifdef CAPTEUR_BATTERIE
  mBatterie.setup("bat_");
  config.mBatterieAA = &mBatterie;

  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mBatterie.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  #endif
  #endif
  #ifdef _RCSWITCH_MODE_
  //mBatterie.mRCSwitch = &mRCSwitch;
  #endif
  #ifdef _LORA_P2P_MODE_
  config.mLoraRxTx = &mLoraRxTx;
  mBatterie.setLoraP2PPublishCallback([](const char* payload) -> int {
      return mLoraRxTx.sendPacket(payload);
  });
  #endif
  #endif // CAPTEUR_BATTERIE
  //================================================== DHT20 ==================================================
  #ifdef CAPTEUR_DHT20
  Serial.println("Initialisation DHT20...");
  dht20.begin("thhum_");
  config.dht20 = &dht20;
  dht20.domotique_prefix = "home/";
  Serial.printf("void loop() - dht20.domotique_prefix : %s\n", dht20.domotique_prefix.c_str()); Serial.flush();
  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  dht20.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  #endif
  #endif
  #ifdef _RCSWITCH_MODE_
  dht20.mRCSwitch = &mRCSwitch;
  #endif
  #ifdef _LORA_P2P_MODE_
  dht20.setLoraP2PPublishCallback([](const char* payload) -> int {
      return mLoraRxTx.sendPacket(payload);
  });
  #endif
  #endif // CAPTEUR_DHT20

  //================================================== FLOTTEUR ==================================================
  #ifdef FLOTTEUR_VERTICAL
  Serial.println("Initialisation FLOTTEUR...");
  mFlotteurVertical.setup("mfv_");
  config.mFlotteurVertical = &mFlotteurVertical;
  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mFlotteurVertical.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  #endif
  #endif // _WIFI_MODE_
  #ifdef _RCSWITCH_MODE_
  mFlotteurVertical.mRCSwitch = &mRCSwitch;
  #endif
  #ifdef _LORA_P2P_MODE_
  mFlotteurVertical.setLoraP2PPublishCallback([](const char* payload) -> int {
      return mLoraRxTx.sendPacket(payload);
  });
  #endif
  #endif // FLOTTEUR_VERTICAL
//================================================== Capteur RGB ==================================================
  #ifdef CAPTEUR_RGB_TCS34725
  Serial.println("Initialisation CAPTEUR_RGB_TCS34725...");
  mCapteurRGB.begin("thhum_"); 
  config.mCapteurRGB = &mCapteurRGB;
  mCapteurRGB.domotique_prefix = "home/";
  Serial.printf("void loop() - mCapteurRGB.domotique_prefix : %s\n", mCapteurRGB.domotique_prefix.c_str()); Serial.flush();
  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mCapteurRGB.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  #endif
  #endif
  #ifdef _RCSWITCH_MODE_
  mCapteurRGB.mRCSwitch = &mRCSwitch;
  #endif
  #ifdef _LORA_P2P_MODE_
  mCapteurRGB.setLoraP2PPublishCallback([](const char* payload) -> int {
      return mLoraRxTx.sendPacket(payload);
  });
  #endif
  #endif // CAPTEUR_RGB_TCS34725

  
  config.setup();

  #ifdef _RCSWITCH_MODE_
  mCC1101.setup();
  mRCSwitch.setup();
  #endif

  #ifdef _WIFI_MODE_
  #ifdef FORCE_CONNEXION_WIFI
  // Mettre votre SSID mais n'oubliez pas de le retirer
  for(int i=0; i< MAX_WIFI_NETS; i++) {
    mWifi[i].wifi_ssid = FORCE_WIFI_SSID; mWifi[i].wifi_password=FORCE_WIFI_PASSWD; // On met le SSI et le password par défaut dans toutes les connexions
    if (i==0) mWifi[i].active=true; else mWifi[i].active=false;           // On ne laisse active que la première
    String prefix = String("wifi")  + String("_") + String(i) + String("_"); // On met en place les préfixes NVS
    mWifi[i].setPrefixNVS(prefix.c_str());
  }
  //#undef MAX_WIFI_NETS
  //#define MAX_WIFI_NETS 1
  #else
  //#ifndef _RCSWITCH_MODE_
  for(int i=0; i< MAX_WIFI_NETS; i++) {
    String prefix = String("wifi")  + String("_") + String(i) + String("_");
    mWifi[i].setup(prefix.c_str());
  }
  //#endif
  #endif
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mqtt.setup("mqtt_");
  #endif
  #endif //_WIFI_MODE_

  print(); // Affiche toute la config
  
  #ifdef _WIFI_MODE_
  int i = 0;
  while(true) {
    Serial.printf("WiFi %d : %s\n", i, mWifi[i].nomEquipement); Serial.flush();
    if (mWifi[i].active) {
//      mWifi[i].setup();
      mWifi[i].begin();
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nEchec de la connexion");
      }
      else break;
    }
    i = (i + 1) % MAX_WIFI_NETS;
  }
  config.setWifi(&mWifi[i]);

  // Configurer et synchroniser l'heure
  Serial.println("\nSynchronisation de l'heure...");
  mDateTime.setup();
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Attendre la synchronisation
  delay(2000);
  
  // Afficher la date et l'heure
  Serial.println("\n========== DATE ET HEURE ==========");
  Serial.print("DateTime: ");
  Serial.println(mDateTime.getDateTime());
  Serial.print("Date seule: ");
  Serial.println(mDateTime.getDate());
  Serial.print("Heure seule: ");
  Serial.println(mDateTime.getTime());
  Serial.print("Format ISO: ");
  Serial.println(mDateTime.getDateTimeISO());
  Serial.println("===================================\n");

  if (WiFi.status() == WL_CONNECTED) {
    webServer.setup();
  }
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mqtt.begin();
  #endif
  #endif

  #ifndef TEST_EMISSION_RCS
  // Nécessite wifi et mqtt
  config.traiteReveil();
  #ifdef CAPTEUR_DS18B20
  int retTemp = ds18b20.readAndPublish(true); // Force une première remontée
  #endif
  #ifdef CAPTEUR_BATTERIE
  int retBatt = mBatterie.readAndPublish(true); // Force une première remontée
  #endif
  #ifdef CAPTEUR_DHT20
  int retTempHum = dht20.readAndPublish(true, true); // Force une première remontée
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  int retRGBLux = mCapteurRGB.readAndPublish(true, true); // Force une première remontée
  #endif
  #endif // TEST_EMISSION_RCS


}



//-------------------------------------------------------------------------------------------
//      LOOP
//-------------------------------------------------------------------------------------------
void loop() {
  // Test de la commande du MosFet - DEBUT
  /*unsigned long temps = 10000UL;
  pinMode(CC1101_POWER_GPIO, OUTPUT);
  digitalWrite(CC1101_POWER_GPIO, CC1101_ON);  // Allume le CC1101
  delay(temps);
  pinMode(CC1101_POWER_GPIO, OUTPUT);
  digitalWrite(CC1101_POWER_GPIO, CC1101_OFF);  // Eteint le CC1101
  delay(temps);
  return;*/
  // Test de la commande du MosFet - FIN

  #ifdef TEST_EMISSION_RCS
  //-------------------------------------
  // TEST
  //-------------------------------------
  static unsigned long ttt=0L;
  static int index=0;
  #define nbMes 4
  static float mes[nbMes][2] = {
                          {10.0,90.0},
                          {20.0,80.0},
                          {40.0,60.0},
                          {-12.0,2.0}
                        };
  if (millis() - ttt > 5000 || ttt == 0L) {
    dht20.readAndPublishTEST(mes[index][0], mes[index][1]); 
    //dht20.mRCSwitch->envoieCode(nullptr, 0);
    ttt = millis();
    index = (index+1) % nbMes;
    delay(10);
    return;
  }
  else {
    delay(10);
    return;
  }
  //-------------------------------------
  // TEST FIN
  //-------------------------------------
  #endif // TEST_EMISSION_RCS
  
  config.loop();
  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mqtt.loop();  // Toujours actif, même écran éteint
  #endif
  webServer.loop();
  #endif
 
  #ifdef _LORA_P2P_MODE_
  int retLora = mLoraRxTx.loop();
  switch(retLora) {
    case -1 : 
    Serial.printf("void loop() - mLoraRxTx.loop() : Erreur de lecture des données reçues\n");
    break;
    case -9 : 
    Serial.printf("void loop() - mLoraRxTx : Module Lora P2P non initialisé\n");
    break;
    default:
    break;
  }
  #endif

  String sDate = "DATE";
  String sHeure = "HEURE";
  #ifdef _WIFI_MODE_
  sDate = mDateTime.getDate();
  sHeure = mDateTime.getTime();
  #endif

  //------------------------------------------------------------------------------------
  // DS18B20
  //------------------------------------------------------------------------------------
  #ifdef CAPTEUR_DS18B20
  static bool dsDejaAffiche = false;
 
  int ret = ds18b20.loop();
  if (ret == -1) {
    dsDejaAffiche = false;
    Serial.println("ds18b20.readTemperature() - échec");
  } 
  else if (ret == 0) {
    dsDejaAffiche = false;
    Serial.println(String(ds18b20.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Température " + String(ds18b20.getLastTemperature(), 1) + " °C inchangé");
  } 
  else if (ret == 1 && ds18b20.active) { // Température changée
    Serial.println(String(ds18b20.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Température " + String(ds18b20.getLastTemperature(), 1) + " °C changée");
    dsDejaAffiche = false;
  } 
  else if (ret == -9) { // Inactif
    if (!dsDejaAffiche) {
      String s = "Thermomètre " + ds18b20.nomEquipement + " inactif";
      Serial.println(s);
      dsDejaAffiche = true;
    }
    }
  #endif


  
  #ifdef CAPTEUR_BATTERIE
  int retTension = mBatterie.loop();
  static bool BatInactifDejaAffiche = false;

  if (retTension == -1) {
    Serial.println("mBatterie.readTension() - échec");
    BatInactifDejaAffiche = false;
  }
  else if (retTension == 0) {
    Serial.println(String(mBatterie.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Tension " + String(mBatterie.getLastTension(), 1) + " V inchangé");
    BatInactifDejaAffiche = false;
  }
  else if (retTension == 1 && mBatterie.active) { // Température changée
    Serial.println(String(mBatterie.nomEquipement) + " - " + sDate + " - " + sHeure  + " - " + "Tension " + String(mBatterie.getLastTension(), 1) + " V changée");
    BatInactifDejaAffiche = false;
  }
  else if (retTension == -9) { // Inactif
    if (!BatInactifDejaAffiche) {
      String s = "[Main] Tension " + mBatterie.nomEquipement + " inactif";
      Serial.println(s);
      BatInactifDejaAffiche = true;
    }
  }
  #endif // CAPTEUR_BATTERIE

  //------------------------------------------------------------------------------------
  // DHT20
  //------------------------------------------------------------------------------------
  #ifdef CAPTEUR_DHT20
  static bool DHT20InactifDejaAffiche = false;
  int retdht20 = dht20.loop();
  if (retdht20 == -1) {
    Serial.println("[Main] dht20.readTemperature() - échec");
    DHT20InactifDejaAffiche = false;
  } 
  else if (retdht20 == 0) {
    Serial.println("[Main] " + String(dht20.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Température " + String(dht20.getLastTemperature(), 1) + " °C inchangé");
    Serial.println("[Main] " + String(dht20.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Humidite " + String(dht20.getLastHumidite(), 1) + " % inchangé");
    DHT20InactifDejaAffiche = false;
  } 
  else if (retdht20 == 1 && dht20.active) { // Température changée
    Serial.println("[Main] " + String(dht20.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Température " + String(dht20.getLastTemperature(), 1) + " °C changée");
    Serial.println("[Main] " + String(dht20.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Humidite " + String(dht20.getLastHumidite(), 1) + " % changée");
    DHT20InactifDejaAffiche = false;
  } 
  else if (retdht20 == -9) { // Inactif
    if (!DHT20InactifDejaAffiche) {
      String s = "[Main] Thermomètre " + dht20.nomEquipement + " inactif";
      Serial.println(s);
      DHT20InactifDejaAffiche = true;
    }
  }
  #endif


  //------------------------------------------------------------------------------------
  // Flotteur vertical
  //------------------------------------------------------------------------------------
  #ifdef FLOTTEUR_VERTICAL
  static bool FVInactifDejaAffiche = false;
  int retFV = mFlotteurVertical.loop();
  if (retFV == -1) {
    Serial.println("[Main] mFlotteurVertical.readMesure() - échec");
    FVInactifDejaAffiche = false;
  } 
  else if (retFV == 0) {
    Serial.println("[Main] " + String(mFlotteurVertical.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Etat " + String(mFlotteurVertical.getLastMesure()) + " inchangé");
    FVInactifDejaAffiche = false;
  } 
  else if (retFV == 1 && mFlotteurVertical.active) { // Mesure changée
    Serial.println("[Main] " + String(mFlotteurVertical.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Etat " + String(mFlotteurVertical.getLastMesure()) + " changé");
    FVInactifDejaAffiche = false;
  } 
  else if (retFV == -9) { // Inactif
    if (!FVInactifDejaAffiche) {
      String s = "[Main] Capteur " + mFlotteurVertical.nomEquipement + " inactif";
      Serial.println(s);
      FVInactifDejaAffiche = true;
      }
    }
  #endif

  //------------------------------------------------------------------------------------
  // Capteur RGB
  //------------------------------------------------------------------------------------
  #ifdef CAPTEUR_RGB_TCS34725
  static bool RGBInactifDejaAffiche = false;
  int retRGB = mCapteurRGB.loop();
  if (retRGB == -1) {
    Serial.println("mCapteurRGB.readTemperature() - échec");
    RGBInactifDejaAffiche = false;
  } 
  else if (retRGB == 0) {
    Serial.println(String(mCapteurRGB.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Couleur 0x" + String(mCapteurRGB.getLastR(), HEX) + " inchangé");
    Serial.println(String(mCapteurRGB.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Luminosité 0x" + String(mCapteurRGB.getLastLuminosite(), HEX) + " inchangé");
    RGBInactifDejaAffiche = false;
  } 
  else if (retRGB == 1 && mCapteurRGB.active) { // Température changée
    Serial.println(String(mCapteurRGB.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Couleur 0x" + String(mCapteurRGB.getLastR(), HEX) + " changée");
    Serial.println(String(mCapteurRGB.nomEquipement) + " - " + sDate + " - " + sHeure + " - " + "Luminosité 0x" + String(mCapteurRGB.getLastLuminosite(), HEX) + " changée");
    RGBInactifDejaAffiche = false;
  } 
  else if (retRGB == -9) { // Inactif
    if (!RGBInactifDejaAffiche) {
      String s = "Equipement " + mCapteurRGB.nomEquipement + " inactif";
      Serial.println(s);
      RGBInactifDejaAffiche = true;
    }
  }
  #endif  

    delay(10);
}



void print() {
  Serial.println("\n=== CONFIGURATION CHARGÉE DEPUIS NVS ===");
  
  config.print();
  Serial.println();

  #ifdef _WIFI_MODE_
  Serial.println("[WiFi]");
  for (int i=0; i< MAX_WIFI_NETS; i++) {
    mWifi[i].print();
    Serial.println();
  }
  
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mqtt.print();
  Serial.println();
  #endif
  #endif

  #ifdef CAPTEUR_DS18B20
  Serial.println("[DS18B20]");
  ds18b20.print();
  #endif

  #ifdef CAPTEUR_DHT20
  Serial.println("[DHT20]");
  dht20.print();
  #endif

  #ifdef FLOTTEUR_VERTICAL
  Serial.println("[FLOTTEUR VERTICAL]");
  mFlotteurVertical.print();
  #endif

  #ifdef CAPTEUR_RGB_TCS34725
  Serial.println("[CAPTEUR RGB]");
  mCapteurRGB.print();
  #endif

  #ifdef CAPTEUR_BATTERIE
  Serial.println("[Batterie AA]");
  mBatterie.print();
  #endif

  Serial.println("============================================\n");
  Serial.flush();
}

// -----------------------------------------------------------------------------
// Affichage au démarrage pour vérifier la détection
// -----------------------------------------------------------------------------
void printBoardInfo() {
  Serial.print("Carte détectée : ");
  Serial.println(BOARD_NAME);
#if IS_ESP32_C3
  Serial.println("Mode ESP32-C3 activé");
#elif IS_ESP32_S3
  Serial.println("Mode ESP32-S3 activé");
#endif
}