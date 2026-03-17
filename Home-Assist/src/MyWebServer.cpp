#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "global.h"
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
#include "MyWebServer.h"


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
      "<div class=\"status\">Salle de bain : <strong>" + config.mRemoteChaudiere->etatStr + "</strong></div>";
  #endif // __LOCAL_MODE__
  html +=  "<div class=\"control\">"
        "<form action=\"/chauffage_sb_on\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-cyan\">SB ON</button></form>"
        "<form action=\"/chauffage_sb_off\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-red\">SB OFF</button></form>"
        "<form action=\"/force_on\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-on\">Forcer ON</button></form>"
        "<form action=\"/force_off\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-off\">Forcer OFF</button></form>"
        "<form action=\"/toggle_p\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-orange\">Projecteur</button></form>"
        "<form action=\"/toggle_g\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-yellow\">Guirlande</button></form>"
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
  html += config.chauffageSb->getHTML();

  #ifdef __LOCAL_DS18B20__
  // === Thermomètre local ===
  html += config.ds18b20->getHTML();
  #endif
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  html += config.mRemoteProjecteur->getHTML();
  html += config.mRemoteGuirlande->getHTML();
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
  html += config.mRemoteThNomade->getHTML();
  html += config.mRemoteBatNomade->getHTML();
  html += config.mRemoteTor->getHTML();

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
    Serial.println("MyWebServer::handleSave() - mWifi[" + String(i) + "].print();");
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
  config.chauffageSb->loadFromWebServer(server);
  #ifdef __LOCAL_DS18B20__
  config.ds18b20->loadFromWebServer(server);
  #endif
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  config.mRemoteChaudiere->loadFromWebServer(server);
  config.mRemoteProjecteur->loadFromWebServer(server);
  config.mRemoteGuirlande->loadFromWebServer(server);
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
  config.mRemoteThNomade->loadFromWebServer(server);
  config.mRemoteBatNomade->loadFromWebServer(server);

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
  config.chauffageSb->saveToNVS();
  #ifdef __LOCAL_DS18B20__
  // === SAUVEGARDE DU THERMOMETRE LOCAL ===
  config.ds18b20->saveToNVS();
  #endif
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  config.mRemoteChaudiere->saveToNVS();
  config.mRemoteProjecteur->saveToNVS();
  config.mRemoteGuirlande->saveToNVS();
  config.mRemoteChauffage->saveToNVS();
  config.mRemoteThMain->saveToNVS();
  config.mRemoteBatMain->saveToNVS();

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
  config.mRemoteThNomade->saveToNVS();
  config.mRemoteBatNomade->saveToNVS();

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
void MyWebServer::handleChauffageSbOn() {
  #ifdef __LOCAL_MODE__
  bool proc = config.chauffageSb->active;
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  bool proc = true;

  #endif // __LOCAL_MODE__
  if (proc) {
    #ifdef __LOCAL_MODE__
    config.chauffageSb->toggleDevice();
    // KJ mqtt.publish(config.chauffageSb->mqttSubTopicState.c_str(), "ON", false);
    config.chauffageSb->etat = true;
    config.chauffageSb->saveState(true);
    #else
    #endif
    #ifdef __CYD__
    ecran.updateAllStates();
    #endif
    // Page de confirmation
    String html = F("<!DOCTYPE html>"
    "<html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"3;url=/\">"
    "<title>Chauffage SDB</title>"
    "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
    "h1{color:#007BFF;font-size:48px;}p{font-size:24px;}</style></head>"
    "<body><h1>✓ Requête ON acceptée</h1>"
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
    "<body><h1>✓ Requête ON refusée</h1>"
    "<p>Equipement inactif...<br>Retour à la page principale dans un instant.</p></body></html>");

    server.send(200, "text/html", html);
    delay(1000);

  }
}

void MyWebServer::handleChauffageSbOff() {
  #ifdef __LOCAL_MODE__
  bool proc = config.chauffageSb->active;
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  bool proc = true;

  #endif // __LOCAL_MODE__
  if (proc) {
    #ifdef __LOCAL_MODE__
    config.chauffageSb->toggleDevice();
    // KJ mqtt.publish(config.chauffageSb->mqttSubTopicState.c_str(), "OFF", false);
    config.chauffageSb->etat = false;
    config.chauffageSb->saveState(false);
    #else
    #endif
    #ifdef __CYD__
    ecran.updateAllStates();
    #endif
    // Page de confirmation
    String html = F("<!DOCTYPE html>"
    "<html><head><meta charset=\"UTF-8\"><meta http-equiv=\"refresh\" content=\"3;url=/\">"
    "<title>Chauffage SDB</title>"
    "<style>body{font-family:Arial;text-align:center;padding:50px;background:#f0f0f0;}"
    "h1{color:#007BFF;font-size:48px;}p{font-size:24px;}</style></head>"
    "<body><h1>✓ Requête OFF acceptée</h1>"
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
    "<body><h1>✓ Requête OFF refusée</h1>"
    "<p>Equipement inactif...<br>Retour à la page principale dans un instant.</p></body></html>");

    server.send(200, "text/html", html);
    delay(1000);

  }

}



void MyWebServer::handleForceOn() {
  #ifdef __LOCAL_MODE__
  bool bEnvoiEnCours = config.chaudiere->bGetEnvoiEnCours();
  #else // Chaudière, thermomètre principal et équipements 433 MHz
  bool bEnvoiEnCours = false;
  #endif // __LOCAL_MODE__  config.chaudiere->bSetEnvoyerTramesOFF(false);
  if (bEnvoiEnCours) {
    Serial.println("Envoi déjà en cours (ON ignoré)");
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
    Serial.println("Envoi déjà en cours (OFF ignoré)");
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
  server.on("/chauffage_sb_on", HTTP_POST, [this]() { handleChauffageSbOn(); });
  server.on("/chauffage_sb_off", HTTP_POST, [this]() { handleChauffageSbOff(); });
  server.onNotFound([this]() { handleNotFound(); });
  server.begin();
  Serial.println("Serveur web démarré");
}

void MyWebServer::loop() {
  server.handleClient();
}