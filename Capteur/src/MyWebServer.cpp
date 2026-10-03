#include <WiFi.h>
#include <PubSubClient.h>
#include "global.h"
#include "MyConfig.h"
#include "MyMqtt.h"
#include "MyWebServer.h"


void MyWebServer::handleRoot() {
  String html =
    "<!DOCTYPE html>"
    "<html lang=\"fr\">"
    "<head>"
      "<meta charset=\"UTF-8\">"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
      "<title>Config " + config.nomEquipement + "</title>"
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
    "<body>"
      "<h1>Configuration " + config.nomEquipement + "</h1>"
      "<div class=\"control\"><a href=\"/logs\" class=\"btn btn-cyan\" style=\"text-decoration:none;\">Journal</a></div>"

      "<hr style=\"margin: 40px 0;\">"

      "<form action=\"/save\" method=\"POST\">";

  // === WiFi (2 colonnes) ===
  html +=  "<h2>WiFi</h2>"
        "<div class=\"device-grid\">";
      for(int i=0; i<MAX_WIFI_NETS; i++) {
        html +=  mWifi[i].getHTML(i);
      }


  html +=  "</div>";

  // === MQTT (2 colonnes) ===
  #ifndef __DESACTIVE_ENVOI_MQTT__
  html += mqtt.getHTML();
  #endif

  // === PRÉFIXE DOMOTIQUE ===
  /*html += "<h2>Préfixe domotique (projecteur, guirlande, sdb)</h2>"
        "<label>Préfixe (ex: home/)</label>"
        "<input type=\"text\" name=\"domo_prefix\" value=\"" + config.domotique_prefix + "\">";*/
  html += config.getHTML();

  // === APPAREILS 433 MHz ===
  html +=  "<h2>Appareils</h2>"
        "<div class=\"device-grid\">";

  // === Thermomètre local ===
  #ifdef CAPTEUR_DS18B20
  if (config.ds18b20 != nullptr) html += config.ds18b20->getHTML();
  #endif
  #ifdef CAPTEUR_DHT20
  if (config.dht20 != nullptr) html += config.dht20->getHTML();
  #endif
  #ifdef FLOTTEUR_VERTICAL
  if (config.mFlotteurVertical != nullptr) html += config.mFlotteurVertical->getHTML();
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  if (config.mCapteurRGB != nullptr) html += config.mCapteurRGB->getHTML();
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
  if (config.mMicro != nullptr) html += config.mMicro->getHTML();
  #endif
  #ifdef ACTIONNEUR_IR
  if (config.mActionneurIR != nullptr) html += config.mActionneurIR->getHTML();
  #endif
  // === Batterie ===
  #ifdef CAPTEUR_BATTERIE
  if (config.mBatterieAA != nullptr) html += config.mBatterieAA->getHTML();
  #endif

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
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mqtt.loadFromWebServer(server);
  #endif

  #ifdef CAPTEUR_DS18B20
  if (config.ds18b20 != nullptr) config.ds18b20->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_DHT20
  if (config.dht20 != nullptr) config.dht20->loadFromWebServer(server);
  #endif
  #ifdef FLOTTEUR_VERTICAL
  if (config.mFlotteurVertical != nullptr) config.mFlotteurVertical->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  if (config.mCapteurRGB != nullptr) config.mCapteurRGB->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
  if (config.mMicro != nullptr) config.mMicro->loadFromWebServer(server);
  #endif
  #ifdef ACTIONNEUR_IR
  if (config.mActionneurIR != nullptr) config.mActionneurIR->loadFromWebServer(server);
  #endif
  #ifdef CAPTEUR_BATTERIE
  if (config.mBatterieAA != nullptr) config.mBatterieAA->loadFromWebServer(server);
  #endif

  // === SAUVEGARDE DES PARAMETRES GENERAUX ===
  config.saveToNVS();
  // === SAUVEGARDE DES PARAMETRES WIFI ===
  for(int i=0; i<MAX_WIFI_NETS;i++) {
    mWifi[i].saveToNVS();
  }
  // === SAUVEGARDE DES PARAMETRES MQTT ===
  #ifndef __DESACTIVE_ENVOI_MQTT__
  mqtt.saveToNVS();
  #endif
  
  // === SAUVEGARDE DU THERMOMETRE LOCAL ===
  #ifdef CAPTEUR_DS18B20
  if (config.ds18b20 != nullptr) config.ds18b20->saveToNVS();
  #endif
  #ifdef CAPTEUR_DHT20
  if (config.dht20 != nullptr) config.dht20->saveToNVS();
  #endif
  #ifdef FLOTTEUR_VERTICAL
  if (config.mFlotteurVertical != nullptr) config.mFlotteurVertical->saveToNVS();
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  if (config.mCapteurRGB != nullptr) config.mCapteurRGB->saveToNVS();
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
  if (config.mMicro != nullptr) config.mMicro->saveToNVS();
  #endif
  #ifdef ACTIONNEUR_IR
  if (config.mActionneurIR != nullptr) config.mActionneurIR->saveToNVS();
  #endif
  #ifdef CAPTEUR_BATTERIE
  if (config.mBatterieAA != nullptr) config.mBatterieAA->saveToNVS();
  #endif

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


void MyWebServer::handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

//------------------------------------------------------------------------------
// Journal : page /logs, rafraîchie toutes les 3 s depuis /logs/data (voir MyJournal.h)
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
    ".controls{display:flex;gap:10px;margin-bottom:15px;align-items:center;flex-wrap:wrap;}"
    ".btn{padding:10px 22px;font-size:15px;color:white;border:none;border-radius:6px;cursor:pointer;text-decoration:none;display:inline-block;}"
    ".btn-gray{background:#6c757d;}.btn-red{background:#dc3545;}"
    "#logbox{background:#1e1e1e;color:#d4d4d4;font-family:monospace;font-size:13px;"
    "padding:15px;border-radius:8px;height:500px;overflow-y:auto;white-space:pre-wrap;word-break:break-all;}"
    "#status{font-size:12px;color:#888;margin-top:6px;text-align:right;}"
    "label{display:flex;align-items:center;gap:6px;cursor:pointer;font-size:15px;}"
    "</style></head><body>"
    "<h1>Journal</h1>"
    "<div class=\"controls\">"
    "<a href=\"/\" class=\"btn btn-gray\">Retour</a>"
    "<form action=\"/logs/clear\" method=\"POST\" style=\"margin:0\">"
    "<button type=\"submit\" class=\"btn btn-red\">Vider</button></form>"
    "<label><input type=\"checkbox\" id=\"autoscroll\" checked> Auto-scroll</label>"
    "</div>"
    "<div id=\"logbox\">(chargement...)</div>"
    "<div id=\"status\"></div>"
    "<script>"
    "const box=document.getElementById('logbox');"
    "const status=document.getElementById('status');"
    "const as=document.getElementById('autoscroll');"
    "async function refresh(){"
    "try{"
    "const r=await fetch('/logs/data');"
    "box.textContent=await r.text();"
    "if(as.checked)box.scrollTop=box.scrollHeight;"
    "status.textContent='Mis à jour : '+new Date().toLocaleTimeString();"
    "}catch(e){status.textContent='Erreur de connexion';}"
    "}"
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

void MyWebServer::setup() {
  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/save", HTTP_POST, [this]() { handleSave(); });
  server.on("/logs",       HTTP_GET,  [this]() { handleLogs(); });
  server.on("/logs/data",  HTTP_GET,  [this]() { handleLogsData(); });
  server.on("/logs/clear", HTTP_POST, [this]() { handleLogsClear(); });
  server.onNotFound([this]() { handleNotFound(); });
  server.begin();
  Serial.printf("Serveur web démarré sur %s\n", WiFi.localIP().toString().c_str()); Serial.flush();
}

void MyWebServer::loop() {
  server.handleClient();
}