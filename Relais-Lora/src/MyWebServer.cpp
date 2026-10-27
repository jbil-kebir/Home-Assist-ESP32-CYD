#include <WiFi.h>
#include <PubSubClient.h>
#include "global.h"
#include "MyConfig.h"
#include "MyMqtt.h"
#include "MyWebServer.h"
#include "FileCommandes.h"


void MyWebServer::handleRoot() {
  String html =
    "<!DOCTYPE html>"
    "<html lang=\"fr\">"
    "<head>"
      "<meta charset=\"UTF-8\">"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
      "<title>Relais Lora</title>"
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
  /*html +=  "<div class=\"control\">"
        "<form action=\"/chauffage_sb_on\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-cyan\">SB ON</button></form>"
        "<form action=\"/chauffage_sb_off\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-red\">SB OFF</button></form>"
        "<form action=\"/force_on\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-on\">Forcer ON</button></form>"
        "<form action=\"/force_off\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-off\">Forcer OFF</button></form>"
        "<form action=\"/toggle_p\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-orange\">Projecteur</button></form>"
        "<form action=\"/toggle_g\" method=\"POST\" style=\"display:inline;\"><button type=\"submit\" class=\"btn btn-yellow\">Guirlande</button></form>"
      "</div>"*/

    html +=    "<p class=\"status\"><a href=\"/capteurs\">Capteurs LoRa</a></p>"
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

  // === PRÉFIXE DOMOTIQUE ===
  html += "<h2>Préfixe domotique (projecteur, guirlande, sdb)</h2>"
        "<label>Préfixe (ex: home/)</label>"
        "<input type=\"text\" name=\"domo_prefix\" value=\"" + config.domotique_prefix + "\">";

  // === APPAREILS Lora 866 MHz ===
  html +=  "<h2>Appareils Lora 866 MHz</h2>"
        "<div class=\"device-grid\">";
  //html += config.mRemoteProjecteur->getHTML();

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

  // === PARAMÈTRES EQUIPEMENTS ===
//  config.mRemoteProjecteur->loadFromWebServer(server);

  // === SAUVEGARDE DES PARAMETRES GENERAUX ===
  config.saveToNVS();
  // === SAUVEGARDE DES PARAMETRES WIFI ===
  for(int i=0; i<MAX_WIFI_NETS;i++) {
    mWifi[i].saveToNVS();
  }
  // === SAUVEGARDE DES PARAMETRES MQTT ===
  mqtt.saveToNVS();
//  config.mRemoteProjecteur->saveToNVS();

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



// Page « Capteurs LoRa » : config de chaque capteur, rechargée toutes les 10 s sauf pendant une saisie
void MyWebServer::handleCapteurs() {
  String html =
    "<!DOCTYPE html>"
    "<html lang=\"fr\">"
    "<head>"
      "<meta charset=\"UTF-8\">"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
      "<title>Capteurs LoRa</title>"
      "<style>"
        "body { font-family: Arial, sans-serif; max-width: 800px; margin: 20px auto; padding: 20px; background: #f0f0f0; }"
        "h1 { text-align: center; color: #333; }"
        "h2 { margin-top: 0; color: #444; }"
        "label { font-weight: bold; margin: 8px 0 4px; display: block; }"
        "input[type=text] { width: 100%; padding: 8px; box-sizing: border-box; font-size: 16px; border: 1px solid #ccc; border-radius: 4px; }"
        ".btn { padding: 12px 25px; font-size: 16px; color: white; border: none; border-radius: 8px; cursor: pointer; margin: 15px 8px 0 0; }"
        ".btn-on { background: #28a745; }"
        ".btn-cyan { background: #17a2b8; }"
        ".btn-off { background: #dc3545; }"
        ".btn:disabled { opacity: 0.4; cursor: default; }"
        ".row { display: flex; flex-wrap: wrap; gap: 10px; margin-bottom: 10px; align-items: center; }"
        ".row > div { flex: 1; min-width: 200px; }"
        ".device { display: block; border: 1px solid #ddd; padding: 15px; border-radius: 8px; background: white; margin-bottom: 20px; }"
        ".attente { color: #fd7e14; font-weight: bold; }"
        ".aide-btn { width: 24px; height: 24px; border-radius: 50%; border: none; background: #007BFF; color: white; font-weight: bold; cursor: pointer; }"
        "#aide { display: none; position: fixed; inset: 0; background: rgba(0,0,0,0.5); }"
        "#aide > div { background: white; max-width: 640px; margin: 40px auto; padding: 20px; border-radius: 10px; max-height: 80vh; overflow-y: auto; }"
        "#aide table { border-collapse: collapse; width: 100%; }"
        "#aide td { border-bottom: 1px solid #ddd; padding: 6px; vertical-align: top; }"
        "#aide code { white-space: nowrap; }"
      "</style>"
      "<script>"
        "function aide(b) { document.getElementById('aide').style.display = b ? 'block' : 'none'; }"
        // Saisie = frappe réelle (évènement input), pas une valeur restaurée par le navigateur au rechargement
        "var saisie = false;"
        "document.addEventListener('input', function() { saisie = true; });"
        "setInterval(function() {"
          "if (saisie || document.getElementById('aide').style.display == 'block') return;"
          "var a = document.activeElement;"
          "if (a && a.tagName == 'INPUT') return;"
          "location.reload();"
        "}, 10000);"
      "</script>"
    "</head>"
    "<body>"
    "<h1>Capteurs LoRa</h1>"
    "<p><a href=\"/\">Configuration du relais</a></p>"
    "<p>Les capteurs dorment : une modification est envoyée au prochain réveil, puis la config est relue.</p>";
  if (config.mFileCommandes != nullptr) html += config.mFileCommandes->getHTML();

  // Aide de la commande libre : commandes du topic de config d'un capteur (Doc/mqtt.docx § 3.2)
  html +=
    "<div id=\"aide\" onclick=\"if (event.target == this) aide(false)\"><div>"
      "<h2>Commandes d'un capteur</h2>"
      "<p>Casse indifférente. Durées en secondes. Envoyée au prochain réveil du capteur.</p>"
      "<table>"
        "<tr><td><code>GET</code></td><td>Relit la configuration (réponse CONFIG)</td></tr>"
        "<tr><td><code>SET cle valeur ...</code><br><code>SET cle:valeur ...</code></td><td>Modifie un ou plusieurs paramètres et répond CONFIG "
          "(refusés listés après ERREUR). Clés : <code>sleepA</code> 0/1, <code>sleepI</code>, <code>sleepS</code>, <code>sleepM</code>, "
          "<code>sleepL</code> (1 à 86400), <code>wakeI</code> (1 à 3600)</td></tr>"
        "<tr><td><code>SLEEP ENABLE</code></td><td>Active le deep sleep</td></tr>"
        "<tr><td><code>SLEEP DISABLE</code><br><code>SLEEP -1</code></td><td>Désactive le deep sleep</td></tr>"
        "<tr><td><code>SLEEP n</code></td><td>Intervalle de sommeil = n s, et active le deep sleep</td></tr>"
        "<tr><td><code>SLEEPS</code> / <code>SLEEPM</code> / <code>SLEEPL</code></td><td>Applique la durée courte / moyenne / longue, et active le deep sleep</td></tr>"
        "<tr><td><code>SLEEPS n</code> (idem M, L)</td><td>Règle la durée courte (moyenne, longue) à n s, sans l'appliquer</td></tr>"
        "<tr><td><code>WDOG ENABLE</code><br><code>WDOG DISABLE</code></td><td>Active / désactive le watchdog ALIVE (ignoré en deep sleep)</td></tr>"
        "<tr><td><code>WDOG s</code></td><td>Période ALIVE = s secondes, et active le watchdog</td></tr>"
        "<tr><td><code>STATUS</code></td><td>Rapport d'état (bouton État)</td></tr>"
        "<tr><td><code>REBOOT</code></td><td>Redémarre le capteur (bouton Redémarrer)</td></tr>"
      "</table>"
      "<button type=\"button\" class=\"btn btn-cyan\" onclick=\"aide(false)\">Fermer</button>"
    "</div></div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void MyWebServer::handleCapteursSave() {
  if (config.mFileCommandes != nullptr) config.mFileCommandes->loadFromWebServer(server);
  server.sendHeader("Location", "/capteurs");
  server.send(303);
}

void MyWebServer::handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void MyWebServer::setup() {
  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/save", HTTP_POST, [this]() { handleSave(); });
  server.on("/capteurs", HTTP_GET, [this]() { handleCapteurs(); });
  server.on("/capteurs_save", HTTP_POST, [this]() { handleCapteursSave(); });
  server.onNotFound([this]() { handleNotFound(); });
  server.begin();
  Serial.println("Serveur web démarré");
}

void MyWebServer::loop() {
  server.handleClient();
}