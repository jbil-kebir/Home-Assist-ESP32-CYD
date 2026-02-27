#include "global.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#ifdef __LOCAL_MODE__
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#endif
#include <WiFi.h>
#include <PubSubClient.h>
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyWifi.h"
#include "MyMqtt.h"
#include "MyWebServer.h"
#ifdef __LOCAL_MODE__
#include "MyCC1101.h"
#include "MyRCSwitch.h"
#include "MyDS18B20.h"
//#include "MyRadioTherm.h"
#else
#include "RemoteChaudiere.h"
#include "RemoteRCDevice.h"
#endif
#include "MyDateTime.h"
#include "RemoteTh.h"
#include "RemoteDHT20.h"
#include "RemoteBatAA.h"





// === OBJETS GLOBAUX ===
CConfig config(NOM_EQUIPEMENT);
CWifi mWifi[MAX_WIFI_NETS];
CEcran ecran(config);
WiFiClient wifiClient;
CMqtt mqtt(wifiClient, config, ecran);
MyWebServer webServer(config, ecran, mqtt, mWifi);
CMyRCSwitch mGeneralRCSwitch(config); // Pourrait faire doublon avec l'héritage "class CRCDevice : public CMyRCSwitch"

#ifdef __LOCAL_MODE__
CCC1101 mCC1101;
#ifdef __LOCAL_DS18B20__
extern MyDS18B20 ds18b20;
#endif
extern CChaudiere chaudiere;
extern CRCDevice projecteur;
extern CRCDevice guirlande;
extern CRCDevice  chauffageSb;
#else // Chaudière, thermomètre principal et équipements 433 MHz distants
extern CRemoteChaudiere mRemoteChaudiere;
extern CRemoteThermo mRemoteThMain;
extern CRemoteBatterieAA mRemoteBatMain;
extern CRemoteRCDevice mRemoteProjecteur;
extern CRemoteRCDevice mRemoteGuirlande;
extern CRemoteRCDevice mRemoteChauffage;
#endif // __LOCAL_MODE__

extern CRemoteThermo mRemoteThCh1er;
extern CRemoteBatterieAA mRemoteBatThCh1er;
extern CRemoteThermo mRemoteThSdb;
extern CRemoteBatterieAA mRemoteBatSdb;
extern CRemoteDHT20 mRemoteThCave;
extern CRemoteTor mRemoteTor;
extern CRemoteBatterieAA mRemoteBatCave;
extern CRemoteDHT20 mRemoteThNomade;
extern CRemoteBatterieAA mRemoteBatNomade;


CMyDateTime mDateTime;
void print();

extern void setup_chaudiere();
extern void setup_ds18b20();
extern void setup_projecteur();
extern void setup_guirlande();
extern void setup_chauffageSb();

extern void setup_ThCave();
extern void setup_ThSdb();
extern void setup_ThCh1er();
extern void setup_ThNomade();

extern void loop_ds18b20();
extern void loop_ThCave();
extern void loop_ThSdb();
extern void loop_ThCh1er();
extern void loop_ThNomade();

//-------------------------------------------------------------------------------------------
//      SETUP
//-------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.printf("\n=== Home Assistant %s ===\n", VERSION);

  #ifdef __LOCAL_MODE__
  Serial.println("Initialisation CC1101...");
  mCC1101.setup();
  #endif
  setup_chaudiere();
  setup_ds18b20();
  setup_projecteur();
  setup_guirlande();
  setup_chauffageSb();

  #ifdef __LOCAL_MODE__

  //therm.setup();
  mGeneralRCSwitch.setup();

  #else // Chaudière, thermomètre principal et équipements 433 MHz
  #endif
  setup_ThCh1er();
  setup_ThSdb();
  setup_ThCave();
  setup_ThNomade();

  
  config.setup("cfg_");
  #ifndef __LOCAL_MODE__
  config.setMqttPublishCallback([ptr = &mqtt](const char* topic, const char* payload) -> int {
      return ptr->publish(topic, payload);
  });
  #endif

  #ifdef __CYD__
  ecran.setup("ecr");
  //ecran.eteindreEcran();
  //ecran.updateBoilerStatus(); // On affiche le dernier état de la chaudière
  ecran.drawMainInterface();
  #endif
 
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
  for(int i=0; i< MAX_WIFI_NETS; i++) {
    String prefix = String("wifi")  + String("_") + String(i) + String("_");
    mWifi[i].setup(prefix.c_str());
  }
  #endif

  mqtt.setup("mqtt_");

  // Décommenter en mode Debug
  print(); // Affiche toute la config
  
  ecran.updateStatus("Initialisation WiFi...");
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
  ecran.updateTitleWithIP(WiFi.localIP().toString());
  ecran.updateStatus("...WiFi OK");
  
  // Configurer et synchroniser l'heure
  //Serial.println("\nSynchronisation de l'heure...");
  ecran.updateStatus("Synchronisation de l'heure...");
  mDateTime.setup();
  
  
  // Afficher la date et l'heure
  Serial.println("\n========== DATE ET HEURE ==========");
  Serial.print("DateTime: "); Serial.println(mDateTime.getDateTime());
  Serial.print("Date seule: "); Serial.println(mDateTime.getDate());
  Serial.print("Heure seule: "); Serial.println(mDateTime.getTime());
  Serial.print("Format ISO: "); Serial.println(mDateTime.getDateTimeISO());
  Serial.println("===================================\n");

  config.mDateTime = &mDateTime;
  ecran.updateDateHeure();
  ecran.updateStatus("...Synchronisation de l'heure OK");
  
  

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Initiallisation Serveur Web...");
    webServer.setup();
    Serial.println("...Serveur Web OK");
  }
  /*#ifdef __CYD__
  ecran.setup("ecr");
  //ecran.eteindreEcran();
  //ecran.updateBoilerStatus(); // On affiche le dernier état de la chaudière
  ecran.drawMainInterface();
  #endif*/
  #ifdef DISABLE_EFFECTEUR
  #ifndef __LOCAL_MODE__
  String sWarning = "WARNING : DISABLE_EFFECTEUR défini sans __LOCAL_MODE__";
  Serial.println(sWarning);
  ecran.updateStatus(sWarning);
  #endif
  #endif

  ecran.updateStatus("Initialisation MQTT...");
  #ifdef FORCE_CONNEXION_WIFI
  mqtt.begin(FORCE_MQTT_SERVER, FORCE_MQTT_PORT, FORCE_MQTT_USER, FORCE_MQTT_PWD);
  #else
  mqtt.begin();
  #endif
  mqtt.reconnect(); // KJ
  ecran.updateStatus("...MQTT OK !");

  #ifndef __LOCAL_MODE__
  // Nécessite wifi et mqtt
  config.traiteReveil();
  #endif

  // Au démarrage du Maître, on envoie les états des appareils 
  // pour synchroniser les autres CYD
  ecran.updateStatus("Initialisation Interface...");

  #ifdef __LOCAL_MODE__
  config.chaudiere->remonteStatusParMqtt();
  config.projecteur->remonteStatusParMqtt();
  config.guirlande->remonteStatusParMqtt();
  config.chauffageSb->remonteStatusParMqtt();
  #else
  // Si on n'est pas en __LOCAL_MODE__, on signale que nous venons 
  // d'accéder au réseau pour recevoir l'état des appareils
  String signalementMsg = "DEVICE CYD "+ String(config.nomEquipement) + " ";
  signalementMsg += String(mDateTime.getDate()) + " " + String(mDateTime.getTime()) + "\n";
  mqtt.publish(config.topic_config_maitre_command.c_str(), signalementMsg.c_str(), false);
  #endif
  ecran.updateStatus("...Pret");
  //ecran.allumerEcran();
}


//-------------------------------------------------------------------------------------------
//      LOOP
//-------------------------------------------------------------------------------------------
void loop() {
  config.loop();

  #ifdef __CYD__
  ecran.loop();
  #endif
  mqtt.loop();  // Toujours actif, même écran éteint
  webServer.loop(); // Toujours actif, même écran éteint
  

  #ifdef __LOCAL_MODE__
  int retRCS = mGeneralRCSwitch.loop(); // On écoute sur la fréquence RCSwitch
  chaudiere.loop();
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  #endif

  loop_ds18b20();
  loop_ThCave();
  loop_ThCh1er();
  loop_ThSdb();
  loop_ThNomade();

  
  #ifdef __CYD__  
  int btnNum = ecran.getPressedButton();

  if (btnNum > -1) {
    // Gestion des boutons
    if (btnNum == BTN_PROJECTEUR_ACTIVE) { // Bouton Activation Projecteur appuyé
      #ifdef __LOCAL_MODE__
      projecteur.activeEquipement();
      #ifdef __CYD__
      ecran.activeProjecteur(projecteur.active);
      #endif
      #else // Equipement distant
      mRemoteProjecteur.activeEquipement();
      #endif // __LOCAL_MODE__
    }
    else if (btnNum == BTN_GUIRLANDE_ACTIVE) { // Bouton Activation Guirlande appuyé
      #ifdef __LOCAL_MODE__
      guirlande.activeEquipement();
      #ifdef __CYD__
      ecran.activeGuirlande(guirlande.active);
      #endif
      #else // Equipement distant
      mRemoteGuirlande.activeEquipement();
      #ifdef __CYD__
      ecran.activeGuirlande(mRemoteGuirlande.active);
      #endif
      #endif // __LOCAL_MODE__
    }
    else if (btnNum == BTN_CHAUFFAGE_ON_ACTIVE) { // Bouton Activation chauffage appuyé
      #ifdef __LOCAL_MODE__
      chauffageSb.activeEquipement();
      #ifdef __CYD__
      ecran.activeChauffageSb(chauffageSb.active);
      #endif
      #else // Equipement distant
      mRemoteChauffage.activeEquipement();
      #endif // __LOCAL_MODE__
     }
    else if (btnNum == BTN_CHAUFFAGE_OFF_ACTIVE) { // Bouton Activation chauffage appuyé
      #ifdef __LOCAL_MODE__
      chauffageSb.activeEquipement();
      #ifdef __CYD__
      ecran.activeChauffageSb(chauffageSb.active);
      #endif
      #else // Equipement distant
      mRemoteChauffage.activeEquipement();
      #endif
     }
    else if (btnNum == BTN_CHAUDIERE_ON_ACTIVE) { // Bouton Activation chaudière appuyé
      #ifdef __LOCAL_MODE__
      chaudiere.activeEquipement();
      #else // Equipement distant
      mRemoteChaudiere.activeEquipement();
      #endif //__LOCAL_MODE__
      ecran.drawMainInterface();
    }
    else if (btnNum == BTN_CHAUDIERE_OFF_ACTIVE) { // Bouton Activation chaudière appuyé
      #ifdef __LOCAL_MODE__
      chaudiere.activeEquipement();
      #else // Equipement distant
      mRemoteChaudiere.activeEquipement();
      #endif //__LOCAL_MODE__
      ecran.drawMainInterface();
    }
    else if (btnNum == BTN_SERIE) { // Bouton Activation Serie appuyé
      ecran.serieAffichagePlusUn();
    }
    else if (btnNum == BTN_CHAUDIERE_ON) { // Bouton ON appuyé
      #ifdef __LOCAL_MODE__
      int retChLoc = chaudiere.allumer();
      #else // Equipement distant
      int retChDis = mRemoteChaudiere.allumer();
      #endif
    }
    else if (btnNum == BTN_CHAUDIERE_OFF) { // Bouton OFF appuyé
      #ifdef __LOCAL_MODE__
      int retChLocExt = chaudiere.eteindre();
      #else // Equipement distant
      int retChDisExt = mRemoteChaudiere.eteindre();
      #endif
    }
    else if (btnNum == BTN_PROJECTEUR_TOGGLE) { // Bouton Projecteur TOGGLE appuyé
      #ifdef __LOCAL_MODE__
      int ret = projecteur.envoiOnOff();
      #else // Equipement distant
      mRemoteProjecteur.envoiOnOff();
      #endif
    }
    else if (btnNum == BTN_GUIRLANDE_TOGGLE) { // Bouton Guirlande TOGGLE appuyé
      #ifdef __LOCAL_MODE__
      int ret = guirlande.envoiOnOff();
      #else // Equipement distant
      mRemoteGuirlande.envoiOnOff();
      #endif
    }
    else if (btnNum == BTN_CHAUFFAGE_ON) { // Bouton Chauffage ON appuyé
      #ifdef __LOCAL_MODE__
      int ret = chauffageSb.envoiOnOff();
      #else // Equipement distant
      mRemoteChauffage.envoiOnOff();
      #endif
    }
    else if (btnNum == BTN_CHAUFFAGE_OFF) { // Bouton Chauffage ON appuyé
      #ifdef __LOCAL_MODE__
      int ret = chauffageSb.envoiOnOff();
      #else // Equipement distant
      mRemoteChauffage.envoiOnOff();
      #endif
    }

  } // if (btnNum > -1)
  else {
  }
  #endif




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

  #ifdef __CYD__
  ecran.print();
  Serial.println();
  #endif

  #ifdef __LOCAL_MODE__
  chaudiere.print();
  Serial.println();

  Serial.println("[Appareils 433 MHz]");
  Serial.println("   Appareil 1");
  projecteur.print();

  Serial.println("   Appareil 2");
  guirlande.print();

  Serial.println("   Appareil 3");
  chauffageSb.print();

  #ifdef __LOCAL_DS18B20__
  Serial.println("[DS18B20]");
  ds18b20.print();
  #endif
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  mRemoteChaudiere.print();
  Serial.println();

  Serial.println("[Appareils 433 MHz]");
  Serial.println("   Appareil 1");
  mRemoteProjecteur.print();

  Serial.println("   Appareil 2");
  mRemoteGuirlande.print();

  Serial.println("   Appareil 3");
  mRemoteChauffage.print();

  Serial.println("[DS18B20]");
  mRemoteThMain.print();

  #endif // __LOCAL_MODE__


  Serial.println("[Remote]");
  mRemoteThCh1er.print();
  mRemoteBatThCh1er.print();
  mRemoteThSdb.print();
  mRemoteBatSdb.print();
  mRemoteThCave.print();
  mRemoteTor.print();
  mRemoteBatCave.print();
  mRemoteThNomade.print();
  mRemoteBatNomade.print();




  Serial.println("============================================\n");
}