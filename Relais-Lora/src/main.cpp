#include "global.h"
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "MyConfig.h"
#include "MyWifi.h"
#include "MyMqtt.h"
#include "MyWebServer.h"
#include "MyDateTime.h"
#include "MyLoraRxTx.h"


// === OBJETS GLOBAUX ===
CConfig config(NOM_EQUIPEMENT);
CWifi mWifi[MAX_WIFI_NETS];
WiFiClient wifiClient;
CMqtt mqtt(wifiClient, config);
MyWebServer webServer(config, mqtt, mWifi);

CMyDateTime mDateTime;

SX1262 radio = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RESET_PIN, LORA_BUSY_PIN);
CMyLoraRxTx mLoraRxTx(config, radio);

void print();

//----------------------------------------------------------------------
//            setup()
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while(!Serial && millis() - start < 10000); // Attend max 10 secondes

  Serial.printf("\n=== %s %s ===\n", config.nomEquipement.c_str(), VERSION);

  config.setup("cfg_");
  config.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  config.setLoraPublishCallback([](const char* topic, const char* payload) -> int {
      return mLoraRxTx.sendPacket(topic, payload);
  });
  
  int ret = mLoraRxTx.setup();
  if (ret != 0) {
    Serial.println("Erreur init LoRa RX/TX : " + String(ret));
  }
  mLoraRxTx.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  /*ret = radio.startReceive();
  if (ret == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(ret);
    while (true) { delay(10); }
  }*/


  #if defined(__FORCE_CONNEXION_WIFI__) || defined(__PREMIER_DEMARRAGE__)
  // Mettre votre SSID mais n'oubliez pas de le retirer
  for(int i=0; i< MAX_WIFI_NETS; i++) {
    mWifi[i].wifi_ssid = FORCE_WIFI_SSID; mWifi[i].wifi_password=FORCE_WIFI_PASSWD; // On met le SSI et le password par défaut dans toutes les connexions
    if (i==0) mWifi[i].active=true; else mWifi[i].active=false;           // On ne laisse active que la première
    String prefix = String("wifi")  + String("_") + String(i) + String("_"); // On met en place les préfixes NVS
    mWifi[i].setPrefixNVS(prefix.c_str());
  }
  #else
  for(int i=0; i< MAX_WIFI_NETS; i++) {
    String prefix = String("wifi")  + String("_") + String(i) + String("_");
    mWifi[i].setup(prefix.c_str());
  }
  #endif

  mqtt.setup("mqtt_");

  // Décommenter en mode Debug
  print(); // Affiche toute la config

  Serial.println("Initialisation WiFi...");
  int i = 0;
  while(true) {
    Serial.printf("WiFi %d : %s\n", i, mWifi[i].nomEquipement);
    if (mWifi[i].active) {
      mWifi[i].begin();
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nEchec de la connexion");
      }
      else break;
    }
    i = (i + 1) % MAX_WIFI_NETS;
  }
  config.setWifi(&mWifi[i]);
  Serial.println("...WiFi OK - IP : " + WiFi.localIP().toString());
  
  // Configurer et synchroniser l'heure
  Serial.println("\nSynchronisation de l'heure...");
  mDateTime.setup();
  
  
  // Afficher la date et l'heure
  Serial.println("\n========== DATE ET HEURE ==========");
  Serial.print("DateTime: "); Serial.println(mDateTime.getDateTime());
  Serial.print("Date seule: "); Serial.println(mDateTime.getDate());
  Serial.print("Heure seule: "); Serial.println(mDateTime.getTime());
  Serial.print("Format ISO: "); Serial.println(mDateTime.getDateTimeISO());
  Serial.println("===================================\n");

  config.mDateTime = &mDateTime;
  Serial.println("...Synchronisation de l'heure OK");
  
  

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Initiallisation Serveur Web...");
    webServer.setup();
    Serial.println("...Serveur Web OK");
  }

  #ifndef __PREMIER_DEMARRAGE__
  Serial.println("Initialisation MQTT...");
  mqtt.begin();
  mqtt.reconnect();
  Serial.println("...MQTT OK !");
  #endif

  config.traiteReveil();

  // On signale que nous venons 
  // d'accéder au réseau pour recevoir l'état des appareils
  // Sauf au premier démarrage car les topics MQTT ne sont 
  // pas encore valides. Il faudra les configurer par Web
  // puis redémarrer en commentant __PREMIER_DEMARRAGE__
  #ifndef __PREMIER_DEMARRAGE__
  String signalementMsg = "DEVICE RELAIS LORA "+ String(config.nomEquipement) + " ";
  signalementMsg += String(mDateTime.getDate()) + " " + String(mDateTime.getTime()) + "\n";
  mqtt.publish(config.topic_config_maitre_command.c_str(), signalementMsg.c_str(), false);
  #endif

  Serial.println("...Pret");  
}

//----------------------------------------------------------------------
//            Loop()
//----------------------------------------------------------------------
void loop() {
  config.loop();

  #ifndef __PREMIER_DEMARRAGE__
  mqtt.loop();  // Toujours actif, même écran éteint
  #endif
  webServer.loop(); // Toujours actif, même écran éteint
  

  int retRCS = mLoraRxTx.loop(); // On écoute sur la fréquence Lora

  // Envoi périodique aléatoire de l'ACK (5 à 20 secondes)
  /*static unsigned long ulTimerAck = 0;
  static unsigned long ulDelaiAck = 0;
  if (ulDelaiAck == 0) ulDelaiAck = random(5000, 20001); // Init au premier passage
  if (millis() - ulTimerAck >= ulDelaiAck) {
    ulTimerAck = millis();
    ulDelaiAck = random(5000, 20001);
    mLoraRxTx.sendPacket("==========home/thermometre/command FlotteurRemise ACK==========");
  }*/

  delay(10);
}

void print() {
  Serial.println("\n=== CONFIGURATION CHARGÉE DEPUIS NVS ===");
  
  config.print();
  Serial.println();

  Serial.println("[WiFi]");
  for (int i=0; i< MAX_WIFI_NETS; i++) {
    mWifi[i].print();
    Serial.println();
  }
  mqtt.print();
  Serial.println();

  //Serial.println("[Appareils Lora 866 MHz]");
  //Serial.println("   Appareil 1");
  //mRemoteProjecteur.print();


  Serial.println("============================================\n");
}