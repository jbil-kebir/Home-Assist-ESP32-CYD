#include "global.h"
#include <Arduino.h>
#include <vector>

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
#include "MyIPModule.h"





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
extern CRemoteTor mRemoteTorNomade;
extern CRemoteBatterieAA mRemoteBatNomade;

extern CRemoteTor mRemoteNewNas;
extern CRemoteTor mRemoteBigNas;

extern CRemoteDHT20 mRemoteThRemise;
extern CRemoteBatterieAA mRemoteBatRemise;


// DBG_NONE, 
// DBG_MQTT, _CAPTEURS, DBG_ACTIONNEURS, DBG_CHAUDIERE, DBG_CONFIG, DBG_RESEAU, DBG_ECRAN, 
// DBG_ALL         
uint32_t gDebugFlags = DBG_CHAUDIERE|DBG_MQTT;

CMyDateTime mDateTime;


std::vector<CIPModule> mvsControleurs; // Liste des controleurs du réseau
std::vector<CIPModule> mvsEsclaves; // Liste des modules capteurs et effecteurs (cartes ESP32 C3/S3)
void initControleursEtEsclaves() {
    mvsControleurs.clear();
    mvsEsclaves.clear();
}
//
// Retour
// 0 : RAS, on ajoute
// -1 : Equipement existant, on ne fait rien
// -2 : Un champ est vide
//
int ajouterControleur(const String& nom, const String& ip) {
  //Serial.printf("int ajouterControleur() - %s %s\n", nom.c_str(), ip.c_str());
  if (nom.isEmpty() || ip.isEmpty()) return -2;
  for (const auto& ctrl : mvsControleurs) {
      String sCtrl = ctrl.getNom();
      String sIp = ctrl.getIP();
      if (sCtrl == nom && sIp == ip) return -1;
  }
  mvsControleurs.emplace_back(nom, ip);
  ecran.drawMainInterface();
  return 0;
}
int ajouterEsclave(const String& nom, const String& ip) {
    mvsEsclaves.emplace_back(nom, ip);
    return 0;
}
// Retour
// -1 : non connecté. Ne devrait jamais arriver
// >= 0 : N° de la connexion WiFi ayant réussi à se connecter au réseau
int connecte_wifi() {
  int i = 0;
  for(i=0; i< MAX_WIFI_NETS; i++) {
    if (mWifi[i].active) {
      mWifi[i].begin();
      if (WiFi.status() == WL_CONNECTED) {
        config.setWifi(&mWifi[i]);
        return i;
      }
    }
  }
  return -1;
}
void printControleurs();
void printEsclaves();

void print();

extern void setup_chaudiere();
extern void setup_ds18b20();
extern void setup_projecteur();
extern void setup_guirlande();
extern void setup_chauffageSb();

extern void setup_ThCave();
extern void setup_ThNomade();
extern void setup_ThSdb();
extern void setup_ThCh1er();
extern void setup_ThRemise();
extern void setup_homeassistant();

extern void loop_ds18b20();
extern void loop_ThCave();
extern void loop_ThNomade();
extern void loop_ThSdb();
extern void loop_ThCh1er();
extern void loop_ThRemise();

extern void loop_homeassistant();


//#define NOM_EQUIPEMENT  "CYD HA"
//-------------------------------------------------------------------------------------------
//      SETUP
//-------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while(!Serial && millis() - start < 3000); // Attend max 3 secondes
  Serial.printf("\n=== %s %s ===\n", NOM_EQUIPEMENT, VERSION);

  mvsControleurs.clear();
  mvsEsclaves.clear();
  
  #ifdef __LOCAL_MODE__
  Serial.println("Initialisation CC1101...");
  mCC1101.setup();
  #endif
  setup_chaudiere();
  setup_ds18b20();
  setup_projecteur();
  setup_guirlande();
  setup_chauffageSb();
  //ecran.mvsControleurs = &mvsControleurs;
  //ecran.mvsEsclaves = &mvsEsclaves;
  
  #ifdef __LOCAL_MODE__

  //therm.setup();
  mGeneralRCSwitch.setup();

  #else // Chaudière, thermomètre principal et équipements 433 MHz
  #endif
  setup_ThCh1er();
  setup_ThSdb();
  setup_ThCave();
  setup_ThNomade();
  setup_ThRemise();
  setup_homeassistant();
  
  config.setup("cfg_", &mvsControleurs, &mvsEsclaves);
  config.setonEquipementCallback([](const String& nom, const String& ip) -> int {
      return ajouterControleur(nom, ip);
  });

  //#ifndef __LOCAL_MODE__
  config.setMqttPublishCallback([](const char* topic, const char* payload) -> int {
      return mqtt.publish(topic, payload);
  });
  //#endif

  #ifdef __CYD__
  ecran.setup("ecr", &mvsControleurs, &mvsEsclaves);

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
  int i = connecte_wifi();
  /*while(true) {
    Serial.printf("WiFi %d : %s\n", i, mWifi[i].nomEquipement);
    if (mWifi[i].active) {
      mWifi[i].begin();
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nEchec de la connexion");
      }
      else break;
    }
    i = (i + 1) % MAX_WIFI_NETS;
  }*/
 if (i == -1) { // Ne devrait jamais arriver car on boucle dans connecte_wifi() jusqu'à trouver une connexion, mais on gère le cas au cas où
    String sError = "ERREUR : Impossible de se connecter à un réseau WiFi. Vérifiez la configuration.";
    Serial.println(sError);
    ecran.updateStatus(sError);
    while(true); // On bloque tout
  }
  config.setWifi(&mWifi[i]);
  ecran.updateTitleWithIP(WiFi.localIP().toString());
  ecran.updateStatus("...WiFi OK");
  // On enregistre l'IP
  ajouterControleur(NOM_EQUIPEMENT, WiFi.localIP().toString());
  
  printControleurs();
  printEsclaves();
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
  signalementMsg += String(mDateTime.getDate()) + " " + String(mDateTime.getTime()) + " 0 " + WiFi.localIP().toString()+ "\n";
  mqtt.publish(config.topic_config_maitre_command.c_str(), signalementMsg.c_str(), false);
  #endif
  ecran.remonteStatusParMqtt();
  ecran.updateStatus("...Pret");
  //ecran.allumerEcran();
}


//-------------------------------------------------------------------------------------------
//      LOOP
//-------------------------------------------------------------------------------------------
void loop() {
  // Avant toute chose, on vérifie la connexion WiFi. Si elle est perdue, on tente de se reconnecter. Tant que la connexion n'est pas rétablie, on ne fait rien d'autre.
  if (WiFi.status() != WL_CONNECTED) {
    ecran.updateStatus("Connexion WiFi perdue. Reconnexion...");
    int i = connecte_wifi();
    if (i == -1) { // Ne devrait jamais arriver car on boucle dans connecte_wifi() jusqu'à trouver une connexion, mais on gère le cas au cas où
      String sError = "ERREUR : Impossible de se reconnecter à un réseau WiFi. Vérifiez la configuration.";
      DBGLN(DBG_CONFIG, sError);
      ecran.updateStatus(sError);
      while(true); // On bloque tout
    }
    config.setWifi(&mWifi[i]);
    ecran.updateTitleWithIP(WiFi.localIP().toString());
    ecran.updateStatus("...Reconnexion WiFi réussie !");
    // Au réveil, on envoie les états des appareils pour synchroniser les autres CYD
    /*#ifdef __LOCAL_MODE__
    config.chaudiere->remonteStatusParMqtt();
    config.projecteur->remonteStatusParMqtt();
    config.guirlande->remonteStatusParMqtt();
    config.chauffageSb->remonteStatusParMqtt();     
    #endif*/
  } // if (WiFi.status() != WL_CONNECTED) {

  int retCfg = config.loop();
  if (retCfg == -10) {
    // Watchdog Alive Time : il est temps d'envoyer un Watchdog
    String s = "ALIVEWDOG " + String(mDateTime.getDateTime());
    //Serial.println(s);
    mqtt.publishWithIP(config.topic_config_state.c_str(), s.c_str());
  } 

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
  loop_ThNomade();
  loop_ThCh1er();
  loop_ThSdb();
  loop_ThRemise();
  loop_homeassistant();
  
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

void printControleurs() {
    DBG(DBG_CONFIG, "\n=== Contrôleurs connus : %d ===\n", mvsControleurs.size());
    for (const auto& ctrl : mvsControleurs) {
        DBG(DBG_CONFIG, " - ");
        DBG(DBG_CONFIG, "%s", ctrl.getNom().c_str());
        DBG(DBG_CONFIG, " → ");
        DBG(DBG_CONFIG, "%s\n", ctrl.getIP().c_str());
    }
}

void printEsclaves() {
    DBG(DBG_CONFIG, "\n=== Esclaves connus : %d ===\n", mvsControleurs.size());
    for (const auto& ctrl : mvsEsclaves) {
        DBG(DBG_CONFIG, " - ");
        DBG(DBG_CONFIG, "%s", ctrl.getNom().c_str());
        DBG(DBG_CONFIG, " → ");
        DBG(DBG_CONFIG, "%s\n", ctrl.getIP().c_str());
    }
}


void print() {
  DBG(DBG_CONFIG, "\n=== CONFIGURATION CHARGÉE DEPUIS NVS ===\n");

  config.print();
  DBG(DBG_CONFIG, "\n");

  DBG(DBG_CONFIG, "[WiFi]\n");
  for (int i=0; i< MAX_WIFI_NETS; i++) {
    mWifi[i].print();
    DBG(DBG_CONFIG, "\n");
  }
  mqtt.print();
  DBG(DBG_CONFIG, "\n");

  #ifdef __CYD__
  ecran.print();
  DBG(DBG_CONFIG, "\n");
  #endif

  #ifdef __LOCAL_MODE__
  chaudiere.print();
  DBG(DBG_CONFIG, "\n");

  DBG(DBG_CONFIG, "[Appareils 433 MHz]\n");
  DBG(DBG_CONFIG, "   Appareil 1\n");
  projecteur.print();

  DBG(DBG_CONFIG, "   Appareil 2\n");
  guirlande.print();

  DBG(DBG_CONFIG, "   Appareil 3\n");
  chauffageSb.print();

  #ifdef __LOCAL_DS18B20__
  DBG(DBG_CONFIG, "[DS18B20]\n");
  ds18b20.print();
  #endif
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  mRemoteChaudiere.print();
  DBG(DBG_CONFIG, "\n");

  DBG(DBG_CONFIG, "[Appareils 433 MHz]\n");
  DBG(DBG_CONFIG, "   Appareil 1\n");
  mRemoteProjecteur.print();

  DBG(DBG_CONFIG, "   Appareil 2\n");
  mRemoteGuirlande.print();

  DBG(DBG_CONFIG, "   Appareil 3\n");
  mRemoteChauffage.print();

  DBG(DBG_CONFIG, "[DS18B20]\n");
  mRemoteThMain.print();

  #endif // __LOCAL_MODE__


  DBG(DBG_CONFIG, "[Remote]\n");

  mRemoteThCh1er.print();
  mRemoteBatThCh1er.print();

  mRemoteThSdb.print();
  mRemoteBatSdb.print();

  mRemoteThCave.print();
  mRemoteTor.print();
  mRemoteBatCave.print();

  mRemoteThNomade.print();
  mRemoteTorNomade.print();
  mRemoteBatNomade.print();

  mRemoteThRemise.print();
  mRemoteBatRemise.print();

  printControleurs();
  printEsclaves();



  DBG(DBG_CONFIG, "============================================\n\n");
}