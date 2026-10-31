#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "global.h"
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
#include "MyWebServer.h"
#include <nvs.h>
#include <vector>
#include <algorithm>


void MyWebServer::handleRoot() {
  String html =
    "<!DOCTYPE html>"
    "<html lang=\"fr\">"
    "<head>"
      "<meta charset=\"UTF-8\">"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
      "<title>Home Assistant</title>"
      "<style>"
        "body { font-family: Arial, sans-serif; max-width: 800px; margin: 20px auto; padding: 20px; background: #f0f0f0; }"
        "h1 { text-align: center; color: #333; }"
        "h2, h3 { margin-top: 30px; color: #444; }"
        "form { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }"
        "label { font-weight: bold; margin: 8px 0 4px; display: block; }"
        "input[type=text], input[type=number], input[type=password] { "
          "padding: 8px; box-sizing: border-box; font-size: 16px; border: 1px solid #ccc; border-radius: 4px; }"
        "input[type=checkbox] { margin-right: 8px; transform: scale(1.2); }"
        "input[type=submit] { width: 100%; padding: 15px; background: #007BFF; color: white; font-size: 18px; border: none; border-radius: 5px; cursor: pointer; margin-top: 30px; }"
        "input[type=submit]:hover { background: #0056b3; }"
        ".control { text-align: center; margin: 30px 0; }"
        ".btn { padding: 18px 35px; font-size: 18px; color: white; border: none; border-radius: 8px; cursor: pointer; margin: 8px; }"
        ".btn-on { background: #28a745; }"
        ".btn-off { background: #dc3545; }"
        ".btn-orange { background: #fd7e14; }"
        ".btn-yellow { background: #ffc107; color: #212529; }"
        ".btn-cyan { background: #17a2b8; }"
        ".btn-gray { background: #6c757d; }"
        ".status { font-size: 18px; text-align: center; margin: 15px 0; font-weight: bold; }"
        ".row { display: flex; flex-wrap: wrap; gap: 10px; margin-bottom: 10px; align-items: center; }"
        ".row > div { flex: 1; min-width: 200px; }"
        ".row input[type=text], .row input[type=number] { width: 100%; }"
        ".checkbox-row { display: flex; align-items: center; gap: 10px; }"
        ".device { border: 1px solid #ddd; padding: 15px; border-radius: 8px; background: #f9f9f9; margin-bottom: 20px; }"
        ".timing-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(100px, 1fr)); gap: 8px; margin: 15px 0; }"
      "</style>"
    "</head>"
    "<body>";
  #ifdef __LOCAL_MODE__
  html +=  "<div class=\"status\">Chaudière : <strong>" + config.chaudiere->etatStr + "</strong></div>"
      "<div class=\"status\">Salle de bain : <strong>" + config.chauffageSb->etatStr + "</strong></div>";
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  html +=  "<div class=\"status\">Chaudière : <strong>" + config.mRemoteChaudiere->etatStr + "</strong></div>"
      "<div class=\"status\">Salle de bain : <strong>" + config.mRemoteChauffage->etatStr + "</strong></div>";
  #endif // __LOCAL_MODE__
  // Bouton SB coloré comme sur le CYD : gris si inactif, vert si allumé, rouge si éteint
  #ifdef __LOCAL_MODE__
  CEquipementBase* pChauffageSb = config.chauffageSb;
  #else // Equipement distant
  CEquipementBase* pChauffageSb = config.mRemoteChauffage;
  #endif // __LOCAL_MODE__
  String sClasseSb = !pChauffageSb->active ? "btn-gray" : (pChauffageSb->etat ? "btn-on" : "btn-off");
  html +=  "<div class=\"control\">"
        "<form action=\"/chauffage_sb\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn " + sClasseSb + "\">SB</button></form>"
        "<form action=\"/force_on\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-on\">Forcer ON</button></form>"
        "<form action=\"/force_off\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-off\">Forcer OFF</button></form>"
        "<form action=\"/toggle_p\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-orange\">Projecteur</button></form>"
        "<form action=\"/toggle_g\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-yellow\">Guirlande</button></form>"
        "<form action=\"/logs\" method=\"GET\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-gray\">Journal</button></form>"
      "</div>"

      "<hr style=\"margin: 40px 0;\">"

      "<form action=\"/save\" method=\"POST\">";
  html +=  config.getHTML();

  // === WiFi (2 colonnes) ===
  html +=  "<h2>WiFi</h2>"
        "<div class=\"device-grid\">";
      for(int i=0; i<MAX_WIFI_NETS; i++) {
        html +=  mWifi[i].getHTML(i);

      }


  html +=  "</div>";

  // === MQTT (2 colonnes) ===
  html += mqtt.getHTML();
  #ifdef __CYD__
  html += ecran.getHTML();
  #endif
      // === CHAUDIÈRE ===
  #ifdef __LOCAL_MODE__
  html += config.chaudiere->getHTML();
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  html += config.mRemoteChaudiere->getHTML();
  #endif // __LOCAL_MODE__      

  // === PRÉFIXE DOMOTIQUE ===
  html += "<h2>Préfixe domotique (projecteur, guirlande, sdb)</h2>"
        "<label>Préfixe (ex: home/)</label>"
        "<input type=\"text\" name=\"domo_prefix\" value=\"" + config.domotique_prefix + "\">";

  // === APPAREILS 433 MHz ===
  html +=  "<h2>Appareils 433 MHz</h2>"
        "<div class=\"device-grid\">";
  #ifdef __LOCAL_MODE__
  html += config.projecteur->getHTML();
  html += config.guirlande->getHTML();
  html += config.lumiereCave->getHTML();
  html += config.chauffageSb->getHTML();

  #ifdef __LOCAL_DS18B20__
  // === Thermomètre local ===
  html += config.ds18b20->getHTML();
  #elif defined(__LOCAL_DHT20__)
  // === Thermomètre local ===
  html += config.dht20->getHTML();
  #endif
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  html += config.mRemoteProjecteur->getHTML();
  html += config.mRemoteGuirlande->getHTML();
  html += config.mRemoteLumiereCave->getHTML();
  html += config.mRemoteChauffage->getHTML();
  html += config.mRemoteThMain->getHTML();
  //html += config.mRemoteBatMain->getHTML();

  #endif // __LOCAL_MODE__  html += config.projecteur->getHTML();
  // === Thermomètres et autres remote ===
  html += config.mRemoteThCh1er->getHTML();
  html += config.mRemoteBatThCh1er->getHTML();

  html += config.mRemoteThSdb->getHTML();
  html += config.mRemoteCoulSdb->getHTML();
  html += config.mRemoteBatSdb->getHTML();

  html += config.mRemoteThCave->getHTML();
  html += config.mRemoteBatCave->getHTML();
  if (config.mRemoteJnCave != nullptr) html += config.mRemoteJnCave->getHTML();

  html += config.mRemoteThNomade->getHTML();
  html += config.mRemoteBatNomade->getHTML();
  html += config.mRemoteTorNomade->getHTML();

  html += config.mRemoteTorChauffageSb->getHTML();

  html += config.mRemoteThRemise->getHTML();
  html += config.mRemoteBatRemise->getHTML();
  html += config.mRemoteTor->getHTML();

  html += config.mRemoteThChRdc->getHTML();
  html += config.mRemoteBatChRdc->getHTML();

  html += config.mRemoteNewNas->getHTML();
  html += config.mRemoteBigNas->getHTML();

  html +=  "</div>";

  html +=  "<input type=\"submit\" value=\"Sauvegarder et redémarrer\">"
    "</form>"
  "</body>"
  "</html>";

  server.send(200, "text/html", html);
}

void MyWebServer::handleSave() {

  // === PARAMÈTRES CLASSIQUES ===
  config.loadFromWebServer(server);

  // === PARAMÈTRES WIFI ===
  for(int i=0; i<MAX_WIFI_NETS;i++) {
    mWifi[i].loadFromWebServer(server);
    DBGLN(DBG_RESEAU, "MyWebServer::handleSave() - mWifi[" + String(i) + "].print();");
    mWifi[i].print();
  }
  
  // === PARAMÈTRES MQTT ===
  mqtt.loadFromWebServer(server);

  // === PARAMÈTRES ECRAN ===
  ecran.loadFromWebServer(server);
  // === PARAMÈTRES EQUIPEMENTS ===
  #ifdef __LOCAL_MODE__
  config.chaudiere->loadFromWebServer(server);
  config.projecteur->loadFromWebServer(server);
  config.guirlande->loadFromWebServer(server);
  config.lumiereCave->loadFromWebServer(server);
  config.chauffageSb->loadFromWebServer(server);
  #ifdef __LOCAL_DS18B20__
  config.ds18b20->loadFromWebServer(server);
  #elif defined(__LOCAL_DHT20__)
  config.dht20->loadFromWebServer(server);
  #endif
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  config.mRemoteChaudiere->loadFromWebServer(server);
  config.mRemoteProjecteur->loadFromWebServer(server);
  config.mRemoteGuirlande->loadFromWebServer(server);
  config.mRemoteLumiereCave->loadFromWebServer(server);
  config.mRemoteChauffage->loadFromWebServer(server);
  config.mRemoteThMain->loadFromWebServer(server);
  //config.mRemoteBatMain->loadFromWebServer(server);

  #endif // __LOCAL_MODE__
  config.mRemoteThCh1er->loadFromWebServer(server);
  config.mRemoteBatThCh1er->loadFromWebServer(server);

  config.mRemoteThSdb->loadFromWebServer(server);
  config.mRemoteCoulSdb->loadFromWebServer(server);
  config.mRemoteBatSdb->loadFromWebServer(server);

  config.mRemoteThCave->loadFromWebServer(server);
  config.mRemoteTor->loadFromWebServer(server);
  config.mRemoteBatCave->loadFromWebServer(server);
  if (config.mRemoteJnCave != nullptr) config.mRemoteJnCave->loadFromWebServer(server);

  config.mRemoteThNomade->loadFromWebServer(server);
  config.mRemoteTorNomade->loadFromWebServer(server);
  config.mRemoteBatNomade->loadFromWebServer(server);

  config.mRemoteTorChauffageSb->loadFromWebServer(server);
  #ifdef __LOCAL_MODE__
  config.chauffageSb->verifieParametres(); // Après le watchdog du noeud, dont dépendent les bornes
  #endif

  config.mRemoteThRemise->loadFromWebServer(server);
  config.mRemoteBatRemise->loadFromWebServer(server);

  config.mRemoteThChRdc->loadFromWebServer(server);
  config.mRemoteBatChRdc->loadFromWebServer(server);

  config.mRemoteNewNas->loadFromWebServer(server);
  config.mRemoteBigNas->loadFromWebServer(server);

  // === SAUVEGARDE DES PARAMETRES GENERAUX ===
  config.saveToNVS();
  // === SAUVEGARDE DES PARAMETRES WIFI ===
  for(int i=0; i<MAX_WIFI_NETS;i++) {
    mWifi[i].saveToNVS();
  }
  // === SAUVEGARDE DES PARAMETRES MQTT ===
  mqtt.saveToNVS();
  // === SAUVEGARDE DES PARAMETRES ECRAN ===
  #ifdef __CYD__
  ecran.saveToNVS();
  #endif
  // === SAUVEGARDE DES PARAMETRES CHAUDIERE ===
  #ifdef __LOCAL_MODE__
   config.chaudiere->saveToNVS();
 // === SAUVEGARDE DES APPAREILS 433 MHz ===
  config.projecteur->saveToNVS();
  config.guirlande->saveToNVS();
  config.lumiereCave->saveToNVS();
  config.chauffageSb->saveToNVS();
  #ifdef __LOCAL_DS18B20__
  // === SAUVEGARDE DU THERMOMETRE LOCAL ===
  config.ds18b20->saveToNVS();
  #elif defined(__LOCAL_DHT20__)
  // === SAUVEGARDE DU THERMOMETRE LOCAL ===
  config.dht20->saveToNVS();
  #endif
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  config.mRemoteChaudiere->saveToNVS();
  config.mRemoteProjecteur->saveToNVS();
  config.mRemoteGuirlande->saveToNVS();
  config.mRemoteLumiereCave->saveToNVS();
  config.mRemoteChauffage->saveToNVS();
  config.mRemoteThMain->saveToNVS();
  //config.mRemoteBatMain->saveToNVS(); // Rien ne le modifie (pas dans la page Web) : valeurs par défaut, aucune entrée NVS

  #endif // __LOCAL_MODE__  config.chaudiere->saveToNVS();
  // === SAUVEGARDE DES APPAREILS REMOTE ===
  config.mRemoteThCh1er->saveToNVS();
  config.mRemoteBatThCh1er->saveToNVS();

  config.mRemoteThSdb->saveToNVS();
  config.mRemoteCoulSdb->saveToNVS();
  config.mRemoteBatSdb->saveToNVS();

  config.mRemoteThCave->saveToNVS();
  config.mRemoteTor->saveToNVS();
  config.mRemoteBatCave->saveToNVS();
  if (config.mRemoteJnCave != nullptr) config.mRemoteJnCave->saveToNVS();

  config.mRemoteThNomade->saveToNVS();
  config.mRemoteTorNomade->saveToNVS();
  config.mRemoteBatNomade->saveToNVS();

  config.mRemoteTorChauffageSb->saveToNVS();

  config.mRemoteThRemise->saveToNVS();
  config.mRemoteBatRemise->saveToNVS();

  config.mRemoteThChRdc->saveToNVS();
  config.mRemoteBatChRdc->saveToNVS();

  config.mRemoteNewNas->saveToNVS();
  config.mRemoteBigNas->saveToNVS();

  // Page de confirmation
  String html = F("<!DOCTYPE html>"
  "<html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"3;url=/\">"
  "<title>Sauvegarde</title>"
  "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
  "h1{color:#007BFF;font-size:48px;}p{font-size:24px;}</style></head>"
  "<body><h1>✓ Configuration sauvegardée !</h1>"
  "<p>Redémarrage en cours...<br>Retour à la page principale dans 3 secondes.</p></body></html>");

  server.send(200, "text/html", html);
  delay(1000);
  ESP.restart();
}

void MyWebServer::handleToggleP() {
  #ifdef __LOCAL_MODE__
  bool proc = config.projecteur->active;
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  bool proc = true;

  #endif // __LOCAL_MODE__
  if (proc) {
    #ifdef __LOCAL_MODE__
    config.projecteur->toggleDevice();
    config.projecteur->etat = !config.projecteur->etat;
    config.projecteur->saveState(config.projecteur->etat);
    mqtt.publish(config.projecteur->mqttSubTopicState.c_str(),
                  config.projecteur->etat ? "ON" : "OFF", false);
    #else // Chaudière, thermomètre principal et équipements 433 MHz

    #endif // __LOCAL_MODE__
     #ifdef __CYD__
     ecran.updateAllStates();
     #endif
    // Page de confirmation
    String html = F("<!DOCTYPE html>"
    "<html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"3;url=/\">"
    "<title>Chauffage SDB</title>"
    "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
    "h1{color:#007BFF;font-size:48px;}p{font-size:24px;}</style></head>"
    "<body><h1>✓ Requête acceptée</h1>"
    "<p>Envoi en cours...<br>Retour à la page principale dans un instant.</p></body></html>");
    server.send(200, "text/html", html);
    delay(1000);
  }
  else {
    // Page de confirmation
    String html = F("<!DOCTYPE html>"
    "<html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"3;url=/\">"
    "<title>Chauffage SDB</title>"
    "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
    "h1{color:#007BFF;font-size:48px;}p{font-size:24px;}</style></head>"
    "<body><h1>✓ Requête refusée</h1>"
    "<p>Equipement inactif...<br>Retour à la page principale dans un instant.</p></body></html>");

    server.send(200, "text/html", html);
    delay(1000);

  }  
}

void MyWebServer::handleToggleG() {
  #ifdef __LOCAL_MODE__
  bool proc = config.guirlande->active;
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  bool proc = true;

  #endif // __LOCAL_MODE__
  if (proc) {
    #ifdef __LOCAL_MODE__
    config.guirlande->toggleDevice();
    config.guirlande->etat = !config.guirlande->etat;
    config.guirlande->saveState(config.guirlande->etat);
    mqtt.publish(config.guirlande->mqttSubTopicState.c_str(),
                  config.guirlande->etat ? "ON" : "OFF", false);
    #else // Chaudière, thermomètre principal et équipements 433 MHz

    #endif // __LOCAL_MODE__
    #ifdef __CYD__
    ecran.updateAllStates();
    #endif
    // Page de confirmation
    String html = F("<!DOCTYPE html>"
    "<html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"3;url=/\">"
    "<title>Chauffage SDB</title>"
    "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
    "h1{color:#007BFF;font-size:48px;}p{font-size:24px;}</style></head>"
    "<body><h1>✓ Requête acceptée</h1>"
    "<p>Envoi en cours...<br>Retour à la page principale dans un instant.</p></body></html>");
    server.send(200, "text/html", html);
    delay(1000);
  }
  else {
    // Page de confirmation
    String html = F("<!DOCTYPE html>"
    "<html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"3;url=/\">"
    "<title>Chauffage SDB</title>"
    "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
    "h1{color:#007BFF;font-size:48px;}p{font-size:24px;}</style></head>"
    "<body><h1>✓ Requête refusée</h1>"
    "<p>Equipement inactif...<br>Retour à la page principale dans un instant.</p></body></html>");

    server.send(200, "text/html", html);
    delay(1000);

  }
}
//----------------------------------------------------------------------------------
// Bouton SB : marche ou arrêt selon l'état, comme le bouton SB du CYD (envoiOnOff())
//----------------------------------------------------------------------------------
void MyWebServer::handleChauffageSb() {
  int ret;
  #ifdef __LOCAL_MODE__
  ret = config.chauffageSb->envoiOnOff(); // 0 : RAS, -1 : inactif, -2 : séquence en cours, -3 : pas de noeud ou pas de MQTT
  #else // Equipement distant
  if (!config.mRemoteChauffage->active) ret = -1;
  else ret = (config.mRemoteChauffage->envoiOnOff() == 0) ? 0 : -3;
  #endif // __LOCAL_MODE__
  #ifdef __CYD__
  ecran.updateAllStates();
  #endif

  String sTitre = (ret == 0) ? "✓ Requête acceptée" : "✗ Requête refusée";
  String sMessage;
  switch (ret) {
    case 0:  sMessage = "Envoi en cours..."; break;
    case -1: sMessage = "Equipement inactif..."; break;
    case -2: sMessage = "Séquence marche/arrêt en cours..."; break;
    default: sMessage = "Noeud ou MQTT absent..."; break;
  }
  // Page de confirmation
  String html = "<!DOCTYPE html>"
    "<html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"3;url=/\">"
    "<title>Chauffage SDB</title>"
    "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
    "h1{color:#007BFF;font-size:48px;}p{font-size:24px;}</style></head>"
    "<body><h1>" + sTitre + "</h1>"
    "<p>" + sMessage + "<br>Retour à la page principale dans un instant.</p></body></html>";
  server.send(200, "text/html", html);
  delay(1000);
}




void MyWebServer::handleForceOn() {
  #ifdef __LOCAL_MODE__
  bool bEnvoiEnCours = config.chaudiere->bGetEnvoiEnCours();
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  bool bEnvoiEnCours = false;
  #endif // __LOCAL_MODE__  config.chaudiere->bSetEnvoyerTramesOFF(false);
  if (bEnvoiEnCours) {
    DBG(DBG_RESEAU, "Envoi déjà en cours (ON ignoré)\n");
  } 
  else {
    #ifdef __LOCAL_MODE__
    config.chaudiere->bSetEnvoyerTramesON(true);
    config.chaudiere->saveState(true);
    #else
    #endif
    mqtt.publishState(true);
    #ifdef __CYD__
    //ecran.updateBoilerStatus();
    #endif
  }

  String html = F("<!DOCTYPE html>"
  "<html lang=\"fr\">"
  "<head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"2;url=/\">"
  "<title>Confirmation</title>"
  "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
  "h1{color:#28a745;font-size:48px;}p{font-size:24px;color:#333;}</style></head>"
  "<body><h1>✓ Chaudière FORCÉE ON</h1>"
  "<p>Trame envoyée avec succès.<br>Retour dans 2 secondes...</p></body></html>");

  server.send(200, "text/html", html);
}

void MyWebServer::handleForceOff() {
  #ifdef __LOCAL_MODE__
  config.chaudiere->bSetEnvoyerTramesON(false);
  bool bEnvoiEnCours = config.chaudiere->bGetEnvoiEnCours();
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  bool bEnvoiEnCours = false;
  #endif // __LOCAL_MODE__  config.chaudiere->bSetEnvoyerTramesOFF(false);
  if (bEnvoiEnCours) {
    DBG(DBG_RESEAU, "Envoi déjà en cours (OFF ignoré)\n");
  } else {
    #ifdef __LOCAL_MODE__
    config.chaudiere->bSetEnvoyerTramesOFF(true);
    config.chaudiere->saveState(false);
    #else
    #endif
    mqtt.publishState(false);
    #ifdef __CYD__
    //ecran.updateBoilerStatus();
    #endif
  }

  String html = F("<!DOCTYPE html>"
  "<html lang=\"fr\">"
  "<head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"2;url=/\">"
  "<title>Confirmation</title>"
  "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
  "h1{color:#dc3545;font-size:48px;}p{font-size:24px;color:#333;}</style></head>"
  "<body><h1>✕ Chaudière FORCÉE OFF</h1>"
  "<p>Trame envoyée avec succès.<br>Retour dans 2 secondes...</p></body></html>");

  server.send(200, "text/html", html);
}

//------------------------------------------------------------------------------
// Journal : page /logs, rafraîchie toutes les 3 s depuis /logs/data (voir MyJournal.h).
// Filtre / Masquer : tri des lignes dans le navigateur (mémorisé dans le navigateur) ; le journal garde tout.
// Cases DBG : changent gDebugFlags en direct (/logs/dbg), sans NVS : retour à la valeur de main.cpp au redémarrage.
//------------------------------------------------------------------------------
void MyWebServer::handleLogs() {
  String html = F(
    "<!DOCTYPE html><html lang=\"fr\"><head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>Journal</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;max-width:900px;margin:20px auto;padding:20px;background:#f0f0f0;}"
    "h1{text-align:center;color:#333;}"
    ".controls{display:flex;gap:10px;margin-bottom:12px;align-items:center;flex-wrap:wrap;}"
    ".btn{padding:10px 22px;font-size:15px;color:white;border:none;border-radius:6px;cursor:pointer;text-decoration:none;display:inline-block;}"
    ".btn-gray{background:#6c757d;}.btn-red{background:#dc3545;}.btn-orange{background:#fd7e14;}"
    ".champ{flex:1;min-width:160px;padding:8px;font-size:14px;border:1px solid #ccc;border-radius:6px;}"
    "#logbox{background:#1e1e1e;color:#d4d4d4;font-family:monospace;font-size:13px;"
    "padding:15px;border-radius:8px;height:500px;overflow-y:auto;white-space:pre-wrap;word-break:break-all;}"
    "#status{font-size:12px;color:#888;margin-top:6px;display:flex;justify-content:space-between;}"
    "label{display:flex;align-items:center;gap:6px;cursor:pointer;font-size:15px;}"
    ".dbg{background:#fff;border:1px solid #ddd;border-radius:6px;padding:8px 12px;font-size:14px;}"
    ".dbg label{font-size:14px;}"
    "</style></head><body>"
    "<h1>Journal</h1>"
    "<div class=\"controls\">"
    "<a href=\"/\" class=\"btn btn-gray\">Retour</a>"
    "<button type=\"button\" id=\"pause\" class=\"btn btn-gray\">Pause</button>"
    "<form action=\"/logs/clear\" method=\"POST\" style=\"margin:0\">"
    "<button type=\"submit\" class=\"btn btn-red\">Vider</button></form>"
    "<label><input type=\"checkbox\" id=\"autoscroll\" checked> Auto-scroll</label>"
    "</div>"
    "<div class=\"controls\">"
    "<input type=\"text\" id=\"filtre\" class=\"champ\" placeholder=\"Filtre : lignes contenant… (a|b)\">"
    "<input type=\"text\" id=\"masque\" class=\"champ\" placeholder=\"Masquer : lignes contenant… (a|b)\">"
    "</div>"
  );
  // Catégories de traces DBG (global.h)
  static const struct { uint32_t bit; const char* nom; } dbg[] = {
    {DBG_MQTT, "MQTT"}, {DBG_CAPTEURS, "Capteurs"}, {DBG_ACTIONNEURS, "Actionneurs"}, {DBG_CHAUDIERE, "Chaudière"},
    {DBG_CONFIG, "Config"}, {DBG_RESEAU, "Réseau"}, {DBG_ECRAN, "Écran"}, {DBG_NOEUD, "Nœud SdB"}
  };
  html += F("<div class=\"controls dbg\" id=\"dbg\"><b>Traces DBG :</b>");
  for (const auto& d : dbg) {
    html += "<label><input type=\"checkbox\" data-b=\"" + String(d.bit) + "\"" + String((gDebugFlags & d.bit) ? " checked" : "") + "> " + d.nom + "</label>";
  }
  html += F("<span style=\"color:#888;font-size:12px\">(jusqu'au prochain redémarrage)</span></div>");
  html += F(
    "<div id=\"logbox\">(chargement...)</div>"
    "<div id=\"status\"><span id=\"nb\"></span><span id=\"maj\"></span></div>"
    "<script>"
    "const box=document.getElementById('logbox'),maj=document.getElementById('maj'),nb=document.getElementById('nb');"
    "const as=document.getElementById('autoscroll'),fi=document.getElementById('filtre'),ma=document.getElementById('masque'),pb=document.getElementById('pause');"
    "let brut='',pause=false;"
    "function lire(k){try{return localStorage.getItem(k)||'';}catch(e){return '';}}"
    "function ecrire(k,v){try{localStorage.setItem(k,v);}catch(e){}}"
    "fi.value=lire('jrnl_filtre');ma.value=lire('jrnl_masque');"
    "function motifs(s){return s.toLowerCase().split('|').map(x=>x.trim()).filter(x=>x);}"
    "function affiche(){"
    "const f=motifs(fi.value),m=motifs(ma.value);"
    "const lignes=brut.split('\\n').filter(l=>l);"
    "const g=lignes.filter(l=>{const b=l.toLowerCase();return (!f.length||f.some(x=>b.includes(x)))&&!m.some(x=>b.includes(x));});"
    "box.textContent=g.join('\\n');"
    "if(as.checked)box.scrollTop=box.scrollHeight;"
    "nb.textContent=g.length+' / '+lignes.length+' lignes';"
    "}"
    "fi.oninput=function(){ecrire('jrnl_filtre',fi.value);affiche();};"
    "ma.oninput=function(){ecrire('jrnl_masque',ma.value);affiche();};"
    "pb.onclick=function(){pause=!pause;pb.textContent=pause?'Reprendre':'Pause';pb.className='btn '+(pause?'btn-orange':'btn-gray');if(!pause)refresh();};"
    "async function refresh(){"
    "if(pause)return;"
    "try{"
    "const r=await fetch('/logs/data');"
    "brut=await r.text();affiche();"
    "maj.textContent='Mis à jour : '+new Date().toLocaleTimeString();"
    "}catch(e){maj.textContent='Erreur de connexion';}"
    "}"
    "document.querySelectorAll('#dbg input').forEach(function(c){c.onchange=async function(){"
    "let f=0;document.querySelectorAll('#dbg input:checked').forEach(function(x){f+=parseInt(x.dataset.b);});"
    "try{await fetch('/logs/dbg',{method:'POST',body:new URLSearchParams({f:f})});maj.textContent='Traces DBG modifiées';}"
    "catch(e){maj.textContent='Erreur de connexion';}"
    "};});"
    "refresh();setInterval(refresh,3000);"
    "</script></body></html>"
  );
  server.send(200, "text/html", html);
}

void MyWebServer::handleLogsData() {
  server.send(200, "text/plain; charset=utf-8", gJournal.getContenu());
}

void MyWebServer::handleLogsClear() {
  gJournal.vide();
  server.send(200, "text/html",
    F("<html><head><meta http-equiv='refresh' content='0;url=/logs'></head><body></body></html>"));
}

// Cases DBG de la page /logs : nouvelle valeur de gDebugFlags (non sauvegardée)
void MyWebServer::handleLogsDbg() {
  if (server.hasArg("f")) {
    gDebugFlags = (uint32_t)strtoul(server.arg("f").c_str(), nullptr, 10);
    Serial.printf("Traces DBG : gDebugFlags = 0x%08lX\n", (unsigned long)gDebugFlags);
  }
  server.send(200, "text/plain", String(gDebugFlags));
}

//------------------------------------------------------------------------------
// Page /nvs : occupation de la partition NVS (20 Ko par défaut, ≈ 500 entrées utiles) et liste des clés.
// Une entrée fait 32 octets : 1 par nombre, 1 + 1 par tranche de 32 octets pour une chaîne, ≈ 2 + 1 par tranche
// de 32 octets pour un blob (float, journal). Quand il n'y a plus de place, les put…() échouent (NOT_ENOUGH_SPACE)
// et le réglage est perdu au redémarrage.
// Valeurs affichées pour nos espaces de noms seulement (pas pour le Wi-Fi du système), mots de passe masqués.
//------------------------------------------------------------------------------
struct SEntreeNvs { String ns; String cle; nvs_type_t type; };

static const char* nomTypeNvs(nvs_type_t t) {
  switch (t) {
    case NVS_TYPE_U8:  return "u8";   case NVS_TYPE_I8:  return "i8";
    case NVS_TYPE_U16: return "u16";  case NVS_TYPE_I16: return "i16";
    case NVS_TYPE_U32: return "u32";  case NVS_TYPE_I32: return "i32";
    case NVS_TYPE_U64: return "u64";  case NVS_TYPE_I64: return "i64";
    case NVS_TYPE_STR: return "str";  case NVS_TYPE_BLOB: return "blob";
    default: return "?";
  }
}

static String echappeHtml(const String& s) {
  String r;
  r.reserve(s.length());
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '<') r += "&lt;"; else if (c == '>') r += "&gt;"; else if (c == '&') r += "&amp;"; else if (c == '"') r += "&quot;";
    else r += c;
  }
  return r;
}

// Valeur lisible et nombre d'entrées (≈) occupées par la clé
static String valeurNvs(nvs_handle_t h, const SEntreeNvs& e, bool bAfficheValeur, int& nbEntrees) {
  nbEntrees = 1;
  const char* k = e.cle.c_str();
  String v;
  switch (e.type) {
    case NVS_TYPE_U8:  { uint8_t x;  if (nvs_get_u8(h, k, &x) == ESP_OK)  v = String(x); break; }
    case NVS_TYPE_I8:  { int8_t x;   if (nvs_get_i8(h, k, &x) == ESP_OK)  v = String(x); break; }
    case NVS_TYPE_U16: { uint16_t x; if (nvs_get_u16(h, k, &x) == ESP_OK) v = String(x); break; }
    case NVS_TYPE_I16: { int16_t x;  if (nvs_get_i16(h, k, &x) == ESP_OK) v = String(x); break; }
    case NVS_TYPE_U32: { uint32_t x; if (nvs_get_u32(h, k, &x) == ESP_OK) v = String(x); break; }
    case NVS_TYPE_I32: { int32_t x;  if (nvs_get_i32(h, k, &x) == ESP_OK) v = String(x); break; }
    case NVS_TYPE_U64: { uint64_t x; if (nvs_get_u64(h, k, &x) == ESP_OK) v = String((unsigned long)x); break; }
    case NVS_TYPE_I64: { int64_t x;  if (nvs_get_i64(h, k, &x) == ESP_OK) v = String((long)x); break; }
    case NVS_TYPE_STR: {
      size_t lg = 0;
      if (nvs_get_str(h, k, nullptr, &lg) == ESP_OK) {
        nbEntrees = 1 + (lg + 31) / 32;
        if (bAfficheValeur && lg <= 200) {
          char buf[201];
          if (nvs_get_str(h, k, buf, &lg) == ESP_OK) v = "\"" + String(buf) + "\"";
        } else {
          v = "(" + String(lg) + " octets)";
        }
      }
      break;
    }
    case NVS_TYPE_BLOB: {
      size_t lg = 0;
      if (nvs_get_blob(h, k, nullptr, &lg) == ESP_OK) {
        nbEntrees = 2 + (lg + 31) / 32;
        float f;
        if (bAfficheValeur && lg == sizeof(float) && nvs_get_blob(h, k, &f, &lg) == ESP_OK) v = String(f, 3) + " (float)";
        else v = "(" + String(lg) + " octets)";
      }
      break;
    }
    default: break;
  }
  if (!bAfficheValeur && e.type != NVS_TYPE_STR && e.type != NVS_TYPE_BLOB) v = "";
  return v;
}

void MyWebServer::handleNvs() {
  nvs_stats_t st;
  bool bStats = (nvs_get_stats(NULL, &st) == ESP_OK);

  // Toutes les clés, triées par espace de noms puis par clé
  std::vector<SEntreeNvs> entrees;
  nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL, NVS_TYPE_ANY);
  while (it != NULL) {
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    entrees.push_back({String(info.namespace_name), String(info.key), info.type});
    it = nvs_entry_next(it); // libère l'itérateur en fin de liste
  }
  std::sort(entrees.begin(), entrees.end(), [](const SEntreeNvs& a, const SEntreeNvs& b) {
    return (a.ns != b.ns) ? (a.ns < b.ns) : (a.cle < b.cle);
  });

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  String html = F(
    "<!DOCTYPE html><html lang=\"fr\"><head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>NVS</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;max-width:900px;margin:20px auto;padding:20px;background:#f0f0f0;}"
    "h1{text-align:center;color:#333;}"
    ".btn{padding:10px 22px;font-size:15px;color:white;border:none;border-radius:6px;text-decoration:none;display:inline-block;background:#6c757d;}"
    ".resume{background:#fff;border:1px solid #ddd;border-radius:6px;padding:10px 14px;margin:12px 0;}"
    "table{border-collapse:collapse;width:100%;background:#fff;margin-bottom:16px;font-size:13px;}"
    "th,td{border:1px solid #ddd;padding:4px 8px;text-align:left;}th{background:#e9ecef;}"
    "td.n{text-align:right;}code{font-size:13px;}"
    ".plein{color:#dc3545;font-weight:bold;}"
    "</style></head><body>"
    "<h1>Mémoire NVS</h1>"
    "<a href=\"/\" class=\"btn\">Retour</a>"
  );

  // Résumé
  html += F("<div class=\"resume\">");
  if (bStats) {
    size_t utiles = st.total_entries > 126 ? st.total_entries - 126 : st.total_entries; // une page reste libre
    int pct = utiles ? (int)(100UL * st.used_entries / utiles) : 0;
    html += "<b>Entrées utilisées :</b> " + String(st.used_entries) + " / " + String(utiles) + " utiles (" + String(pct) + " %)";
    if (st.free_entries <= 126) html += F(" <span class=\"plein\">— PLEINE</span>");
    html += "<br><b>Libres :</b> " + String(st.free_entries) + " (dont 126 réservées)"
            "<br><b>Total :</b> " + String(st.total_entries) + " entrées de 32 octets"
            "<br><b>Espaces de noms :</b> " + String(st.namespace_count);
  } else {
    html += F("nvs_get_stats() en échec");
  }
  html += "<br><b>Clés :</b> " + String(entrees.size()) + "</div>";
  html += F("<form action=\"/nvs/purge\" method=\"POST\" style=\"margin:0 0 12px\" "
            "onsubmit=\"return confirm('Supprimer les clés obsolètes et les espaces de noms deep_sleep et cydmon ?')\">"
            "<button type=\"submit\" class=\"btn\" style=\"background:#dc3545;cursor:pointer\">Supprimer les clés obsolètes</button></form>");
  server.sendContent(html);

  // Une table par espace de noms
  size_t i = 0;
  while (i < entrees.size()) {
    const String ns = entrees[i].ns;
    bool bNotre = (ns == NVS_NAME_SPACE) || (ns == "journal");
    nvs_handle_t h;
    bool bOuvert = (nvs_open(ns.c_str(), NVS_READONLY, &h) == ESP_OK);
    String corps = "<h3>" + echappeHtml(ns) + "</h3><table><tr><th>Clé</th><th>Type</th><th>Entrées ≈</th><th>Valeur</th></tr>";
    int total = 0, nbCles = 0;
    for (; i < entrees.size() && entrees[i].ns == ns; i++) {
      const SEntreeNvs& e = entrees[i];
      String cleMin = e.cle;
      cleMin.toLowerCase();
      bool bSecret = cleMin.indexOf("pass") >= 0 || cleMin.indexOf("pwd") >= 0;
      int nb = 1;
      String v = bOuvert ? valeurNvs(h, e, bNotre && !bSecret, nb) : String("");
      if (bSecret && bNotre) v = "(masqué)";
      total += nb;
      nbCles++;
      corps += "<tr><td><code>" + echappeHtml(e.cle) + "</code></td><td>" + nomTypeNvs(e.type) + "</td><td class=\"n\">" +
               String(nb) + "</td><td>" + echappeHtml(v) + "</td></tr>";
      if (corps.length() > 3000) { // envoi par morceaux (tas limité)
        server.sendContent(corps);
        corps = "";
      }
    }
    if (bOuvert) nvs_close(h);
    corps += "<tr><th colspan=\"2\">" + String(nbCles) + " clés</th><th class=\"n\">" + String(total) + "</th><th></th></tr></table>";
    server.sendContent(corps);
  }

  server.sendContent(F("</body></html>"));
  server.sendContent("");
}

// Bouton de la page /nvs : suppression des clés qu'aucune version actuelle du code ne lit (relevé du 30/10/2026)
// et des espaces de noms d'autres programmes (deep_sleep, cydmon : CydMonitor). Une clé absente est ignorée.
void MyWebServer::handleNvsPurge() {
  static const char* espacesObsoletes[] = { "deep_sleep", "cydmon" };
  static const char* clesObsoletes[] = {
    "wifi_active", "wifi_nom", "wifi_password", "wifi_ssid", "wifi_subtopic",              // avant wifi_0_ / wifi_1_ / wifi_2_
    "thnmd_active", "thnmd_id", "thnmd_nom", "thnmd_st", "thnmd_subtopic", "thnmd_wdog",   // remplacé par thnmf_
    "pre_active", "pre_nom", "pre_subtopic", "pre_wdog",
    "thCh1er_active", "thCh1er_nom",
    "ecraff", "ecraff_max", "ecror_ecran", "ecrsleep_tout",                               // MyEcran : aff, or_ecran… sans préfixe
    "wdog", "on_keep_size", "off_keep_size",
    "Chactive", "Chst", "guirl_st", "guirlande_state", "csdb_st", "pr_active",
    "Pr_active", "Pr_st", "Gu_active", "Gu_st",                                            // préfixe provisoire du constructeur
    "bat_active", "bat_nom", "bat_st", "bat_subtopic", "bat_wdog",
    "ecr_act", "ecr_max", "ecr_ecr_act", "ecr_ecr_max", "ecr_or_ecran", "ecr_sleep_tout",
    "ecrecr_act", "ecrecr_max", "ecrihm"
  };
  int nbSupprimees = 0;
  nvs_handle_t h;
  if (nvs_open(NVS_NAME_SPACE, NVS_READWRITE, &h) == ESP_OK) {
    for (const char* k : clesObsoletes) {
      if (nvs_erase_key(h, k) == ESP_OK) nbSupprimees++;
    }
    nvs_commit(h);
    nvs_close(h);
  }
  String sEspaces;
  for (const char* ns : espacesObsoletes) {
    // Ouverture en lecture d'abord : en écriture, un espace de noms absent serait créé (une entrée de plus)
    bool bPresent = (nvs_open(ns, NVS_READONLY, &h) == ESP_OK);
    if (bPresent) nvs_close(h);
    bool bVide = false;
    if (bPresent && nvs_open(ns, NVS_READWRITE, &h) == ESP_OK) {
      bVide = (nvs_erase_all(h) == ESP_OK);
      nvs_commit(h);
      nvs_close(h);
    }
    sEspaces += String(ns) + " : " + (bVide ? "vidé" : (bPresent ? "ÉCHEC" : "absent")) + "<br>";
    Serial.printf("NVS : espace de noms %s %s\n", ns, bVide ? "vidé" : (bPresent ? "ECHEC" : "absent"));
  }
  Serial.printf("NVS : %d clés obsolètes supprimées\n", nbSupprimees);
  server.send(200, "text/html",
    "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"3;url=/nvs\"></head>"
    "<body style=\"font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;\">"
    "<h1>" + String(nbSupprimees) + " clés supprimées</h1><p>" + sEspaces +
    "Retour à la page NVS dans 3 secondes.</p></body></html>");
}

void MyWebServer::handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void MyWebServer::setup() {
  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/save", HTTP_POST, [this]() { handleSave(); });
  server.on("/force_on", HTTP_POST, [this]() { handleForceOn(); });
  server.on("/force_off", HTTP_POST, [this]() { handleForceOff(); });
  server.on("/toggle_p", HTTP_POST, [this]() { handleToggleP(); });
  server.on("/toggle_g", HTTP_POST, [this]() { handleToggleG(); });  
  server.on("/chauffage_sb", HTTP_POST, [this]() { handleChauffageSb(); });
  server.on("/logs",       HTTP_GET,  [this]() { handleLogs(); });
  server.on("/logs/data",  HTTP_GET,  [this]() { handleLogsData(); });
  server.on("/logs/clear", HTTP_POST, [this]() { handleLogsClear(); });
  server.on("/logs/dbg",   HTTP_POST, [this]() { handleLogsDbg(); });
  server.on("/nvs",        HTTP_GET,  [this]() { handleNvs(); });
  server.on("/nvs/purge",  HTTP_POST, [this]() { handleNvsPurge(); });
  server.onNotFound([this]() { handleNotFound(); });
  server.begin();
  DBG(DBG_RESEAU, "Serveur web démarré\n");
}

void MyWebServer::loop() {
  server.handleClient();
}