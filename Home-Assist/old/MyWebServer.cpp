#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "global.h"
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyMqtt.h"
//#include "MyWifi.h"
#include "MyRadioTx.h"
#include "MyWebServer.h"

void MyWebServer::handleRoot() {
  String html = "<!DOCTYPE html>"
"<html lang=\"fr\">"
"<head>"
  "<meta charset=\"UTF-8\">"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
  "<title>Config AD200</title>"
  "<style>"
    "body { font-family: Arial, sans-serif; max-width: 600px; margin: 20px auto; padding: 20px; background: #f0f0f0; }"
    "h1 { text-align: center; color: #333; }"
    "form { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }"
    "label { display: block; margin: 10px 0 5px; font-weight: bold; }"
    "input[type=text], input[type=number], input[type=password] { width: 100%; padding: 8px; box-sizing: border-box; margin-bottom: 10px; }"
    "input[type=submit] { margin-top: 20px; padding: 12px 24px; background: #007BFF; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }"
    "input[type=submit]:hover { background: #0056b3; }"
    ".status { font-size: 18px; text-align: center; margin: 20px 0; }"
  "</style>"
"</head>"
"<body>"
  "<h1>Configuration AD200</h1>"
  "<div class=\"status\">État actuel : <strong>" + config.etatStr + "</strong></div>"
  "<form action=\"/save\" method=\"POST\">"
    "<h2>WiFi</h2>"
    "<label>SSID</label>"
    "<input type=\"text\" name=\"wifi_ssid\" value=\"" + config.wifi_ssid + "\">"
    "<label>Mot de passe</label>"
    "<input type=\"password\" name=\"wifi_password\" value=\"" + config.wifi_password + "\">"

    "<h2>MQTT</h2>"
    "<label>Serveur</label>"
    "<input type=\"text\" name=\"mqtt_server\" value=\"" + config.mqqtInfo.mqtt_server + "\">"
    "<label>Port</label>"
    "<input type=\"number\" name=\"mqtt_port\" value=\"" + String(config.mqqtInfo.mqtt_port) + "\">"
    "<label>Utilisateur</label>"
    "<input type=\"text\" name=\"mqtt_user\" value=\"" + config.mqqtInfo.mqtt_user + "\">"
    "<label>Mot de passe</label>"
    "<input type=\"password\" name=\"mqtt_password\" value=\"" + config.mqqtInfo.mqtt_password + "\">"

    "<h2>Topic prefix</h2>"
    "<label>Prefix (ex: ad200/)</label>"
    "<input type=\"text\" name=\"topic_prefix\" value=\"" + config.topic_prefix + "\">"

    "<input type=\"submit\" value=\"Sauvegarder et redémarrer\">"
  "</form>"

  "<hr style=\"margin: 40px 0;\">"

  "<h2>Contrôle forcé</h2>"
  "<div style=\"text-align:center;\">"
    "<form action=\"/force_on\" method=\"POST\" style=\"display:inline; margin:0 10px;\">"
      "<input type=\"submit\" value=\"Forcer ON\" style=\"padding:15px 30px; font-size:18px; background:#28a745; color:white; border:none; border-radius:8px; cursor:pointer;\">"
    "</form>"
    "<form action=\"/force_off\" method=\"POST\" style=\"display:inline; margin:0 10px;\">"
      "<input type=\"submit\" value=\"Forcer OFF\" style=\"padding:15px 30px; font-size:18px; background:#dc3545; color:white; border:none; border-radius:8px; cursor:pointer;\">"
    "</form>"
  "</div>"

"</body>"
"</html>";

  server.send(200, "text/html", html);
}

void MyWebServer::handleSave() {
  // Sauvegarde des paramètres
  if (server.hasArg("wifi_ssid")) config.wifi_ssid = server.arg("wifi_ssid");
  if (server.hasArg("wifi_password")) config.wifi_password = server.arg("wifi_password");
  if (server.hasArg("mqtt_server")) config.mqqtInfo.mqtt_server = server.arg("mqtt_server");
  if (server.hasArg("mqtt_port")) config.mqqtInfo.mqtt_port = server.arg("mqtt_port").toInt();
  if (server.hasArg("mqtt_user")) config.mqqtInfo.mqtt_user = server.arg("mqtt_user");
  if (server.hasArg("mqtt_password")) config.mqqtInfo.mqtt_password = server.arg("mqtt_password");
  if (server.hasArg("topic_prefix")) config.topic_prefix = server.arg("topic_prefix");

  // Reconstruction des topics
  config.topic_command = config.topic_prefix + mqqtInfo.subtopic_command;
  config.topic_state = config.topic_prefix + mqqtInfo.subtopic_state;
  config.topic_status = config.topic_prefix + mqqtInfo.subtopic_status;

  // Sauvegarde dans NVS
  Preferences prefs;
  prefs.begin("ad200", false);
  prefs.putString("wifi_ssid", config.wifi_ssid);
  prefs.putString("wifi_password", config.wifi_password);
  prefs.putString("mqtt_server", config.mqqtInfo.mqtt_server);
  prefs.putUShort("mqtt_port", config.mqqtInfo.mqtt_port);
  prefs.putString("mqtt_user", config.mqqtInfo.mqtt_user);
  prefs.putString("mqtt_password", config.mqqtInfo.mqtt_password);
  prefs.putString("topic_prefix", config.topic_prefix);
  prefs.end();

  server.send(200, "text/html", "<h1>Paramètres sauvegardés ! Redémarrage en cours...</h1>");
  delay(1000);
  ESP.restart();
}

void MyWebServer::handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void MyWebServer::setup() {
  server.on("/", HTTP_GET, [this]() { handleRoot(); });
  server.on("/save", HTTP_POST, [this]() { handleSave(); });
  server.onNotFound([this]() { handleNotFound(); });
  server.begin();
  Serial.println("Serveur web démarré sur http://192.168.x.x (voir IP dans serial)");

  server.on("/force_on", HTTP_POST, [this]() { handleForceOn(); });
  server.on("/force_off", HTTP_POST, [this]() { handleForceOff(); });
}

void MyWebServer::loop() {
  server.handleClient();
}

void MyWebServer::handleForceOn() {
  // Envoie la trame ON
    if (radioTX.bGetEnvoiEnCours()) {
        Serial.printf("Envoi déjà en cours\n");
    }
    else {
        radioTX.bSetEnvoyerTramesON(true);
        //mqtt.publishState(true); KJ
        config.saveBoilerState(true);
        ecran.updateBoilerStatus("ON");
    }

  // Page de confirmation
  String html = F("<!DOCTYPE html>"
  "<html lang=\"fr\">"
  "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta http-equiv=\"refresh\" content=\"2;url=/\">"
    "<title>Confirmation</title>"
    "<style>"
      "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background: #f0f0f0; }"
      "h1 { color: #28a745; font-size: 48px; }"
      "p { font-size: 24px; color: #333; }"
    "</style>"
  "</head>"
  "<body>"
    "<h1>✓ Chaudière FORCÉE ON</h1>"
    "<p>Trame envoyée avec succès.<br>Retour à la configuration dans 2 secondes...</p>"
  "</body>"
  "</html>");

  server.send(200, "text/html", html);
}

void MyWebServer::handleForceOff() {
  // Envoie la trame OFF
    if (radioTX.bGetEnvoiEnCours()) {
      Serial.printf("Envoi déjà en cours\n");
    }
    else {
      radioTX.bSetEnvoyerTramesOFF(true);
      mqtt.publishState(false);
      config.saveBoilerState(false);
      ecran.updateBoilerStatus("OFF");
    }

  // Page de confirmation
  String html = F("<!DOCTYPE html>"
  "<html lang=\"fr\">"
  "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta http-equiv=\"refresh\" content=\"2;url=/\">"
    "<title>Confirmation</title>"
    "<style>"
      "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background: #f0f0f0; }"
      "h1 { color: #dc3545; font-size: 48px; }"
      "p { font-size: 24px; color: #333; }"
    "</style>"
  "</head>"
  "<body>"
    "<h1>✕ Chaudière FORCÉE OFF</h1>"
    "<p>Trame envoyée avec succès.<br>Retour à la configuration dans 2 secondes...</p>"
  "</body>"
  "</html>");

  server.send(200, "text/html", html);
}