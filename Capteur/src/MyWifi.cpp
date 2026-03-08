#include <Arduino.h>
#include <WiFi.h>
#include "MyWifi.h"

void CWifi::setup(const String pref) { //const String pref) {
  mPrefixNVS = pref;
  loadFromNVS();
  //active = true; // A supprimer
}

void CWifi::begin() {
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  Serial.printf("Connexion au WiFi %s", wifi_ssid.c_str()); Serial.flush();
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print("."); Serial.flush();
  }
  if (WiFi.status() == WL_CONNECTED) {
    //Serial.println("\nWiFi connecté - IP : " + WiFi.localIP().toString()); Serial.flush();
    Serial.printf("\nWiFi connecté - IP : %s\n", WiFi.localIP().toString().c_str()); Serial.flush();
  } else {
    //Serial.println("\nÉchec connexion WiFi\n"); Serial.flush();
    Serial.printf("\nÉchec connexion WiFi\n"); Serial.flush();
  }
  Serial.flush();
}


void CWifi::loadFromNVS() {
  prefs.begin(nvs_namespace, true);
  nomEquipement = prefs.getString((mPrefixNVS + "nom").c_str(), "Wifi");
  active = prefs.getBool((mPrefixNVS + "active").c_str(), true);
  wifi_ssid = prefs.getString((mPrefixNVS + "ssid").c_str(), default_wifi_ssid);
  wifi_password = prefs.getString((mPrefixNVS + "password").c_str(), default_wifi_password);
  mqttSubTopic = prefs.getString((mPrefixNVS + "subtopic").c_str(), "wifi");
  prefs.end();
}

void CWifi::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putString((mPrefixNVS + "nom").c_str(), nomEquipement);
  prefs.putBool((mPrefixNVS + "active").c_str(), active);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putString((mPrefixNVS+"ssid").c_str(), wifi_ssid);
  prefs.putString((mPrefixNVS+"password").c_str(), wifi_password);
  prefs.end();
}

void CWifi::print() const {
  Serial.println("   WiFi Config");
  Serial.printf("     Nom            : %s\n", nomEquipement.c_str());
  Serial.printf("     Actif          : %s\n", active ? "OUI" : "NON");
  Serial.printf("     SSID           : %s\n", wifi_ssid.c_str());
  Serial.printf("     MQTT SubTopic  : %s\n", mqttSubTopic.c_str());
}

void CWifi::loadFromWebServer(WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"ssid").c_str())) wifi_ssid = server.arg(mPrefixNVS+"ssid");
  if (server.hasArg((mPrefixNVS+"password").c_str())) wifi_password = server.arg(mPrefixNVS+"password");
}

String CWifi::getHTML(int i) {
  String html = "";
  html =  "<div class=\"device\">"
            "<h3>Wifi " + String(i) +"</h3>"
            "<div class=\"row\"><div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom").c_str() + " value=\"" + nomEquipement + "\"></div>"
            "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active").c_str() + " value=\"1\"" + String(active ? " checked" : "") + "></div></div>"
            "<div class=\"row\"><div><label>WiFi SSID</label><input type=\"text\" name=" + (mPrefixNVS+"ssid").c_str() + " value=\"" + wifi_ssid + "\"></div>"
            "<div><label>WiFi Mot de passe</label><input type=\"text\" name=" + (mPrefixNVS+"password").c_str() + " value=\"" + wifi_password + "\"></div>"
            "<div class=\"row\"><div><label>MQTT subtopic</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic").c_str() + " value=\"" + mqttSubTopic + "\"></div>"
            "</div>"
          "</div>";
//Serial.println("String CWifi::getHTML(int i) : " + html);
  return html;
}

