#include "global.h" // Serial -> journal web (voir MyJournal.h)
#include <Arduino.h>
#include <WiFi.h>
#include "MyWifi.h"

void CWifi::setup(const String pref) { //const String pref) {
  mPrefixNVS = pref;
  loadFromNVS();
  //active = true; // A supprimer
}

// Puissances proposées sur la page Web (valeurs de wifi_power_t, en quarts de dBm)
static const int8_t PUISSANCES_TX[] = {
  WIFI_POWER_19_5dBm, WIFI_POWER_19dBm, WIFI_POWER_18_5dBm, WIFI_POWER_17dBm, WIFI_POWER_15dBm,
  WIFI_POWER_13dBm, WIFI_POWER_11dBm, WIFI_POWER_8_5dBm, WIFI_POWER_7dBm, WIFI_POWER_5dBm, WIFI_POWER_2dBm
};
static const size_t NB_PUISSANCES_TX = sizeof(PUISSANCES_TX) / sizeof(PUISSANCES_TX[0]);

static bool puissanceValide(int v) {
  for (size_t k = 0; k < NB_PUISSANCES_TX; k++)
    if (PUISSANCES_TX[k] == v) return true;
  return false;
}

// Quarts de dBm -> "8.5"
static String puissanceTexte(int8_t v) {
  return String(v / 4.0f, 1);
}

void CWifi::begin() {
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  // Puissance d'émission du profil (page Web), appliquée après WiFi.begin().
  // ESP32-C3 Super Mini : à pleine puissance, démarrages de 15 s à plus de 90 s constatés (voir MyWifi.h).
  WiFi.setTxPower((wifi_power_t)mcTxPower);
  Serial.printf("Connexion au WiFi %s (puissance %s dBm)", wifi_ssid.c_str(), puissanceTexte(mcTxPower).c_str()); Serial.flush();
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print("."); Serial.flush();
  }
  if (WiFi.status() == WL_CONNECTED) {
    //Serial.println("\nWiFi connecté - IP : " + WiFi.localIP().toString()); Serial.flush();
    Serial.printf("\nWiFi connecté en %lu ms - IP : %s - RSSI : %d dBm\n", millis() - start, WiFi.localIP().toString().c_str(), WiFi.RSSI()); Serial.flush();
  } else {
    //Serial.println("\nÉchec connexion WiFi\n"); Serial.flush();
    Serial.printf("\nÉchec connexion WiFi après %lu ms (statut %d)\n", millis() - start, (int)WiFi.status()); Serial.flush();
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
  mcTxPower = prefs.getChar((mPrefixNVS + "txpw").c_str(), default_tx_power);
  if (!puissanceValide(mcTxPower)) mcTxPower = default_tx_power;
  prefs.end();
}

void CWifi::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putString((mPrefixNVS + "nom").c_str(), nomEquipement);
  prefs.putBool((mPrefixNVS + "active").c_str(), active);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putString((mPrefixNVS+"ssid").c_str(), wifi_ssid);
  prefs.putString((mPrefixNVS+"password").c_str(), wifi_password);
  prefs.putChar((mPrefixNVS+"txpw").c_str(), mcTxPower);
  prefs.end();
}

void CWifi::print() const {
  Serial.println("   WiFi Config");
  Serial.printf("     Nom            : %s\n", nomEquipement.c_str());
  Serial.printf("     Actif          : %s\n", active ? "OUI" : "NON");
  Serial.printf("     SSID           : %s\n", wifi_ssid.c_str());
  Serial.printf("     MQTT SubTopic  : %s\n", mqttSubTopic.c_str());
  Serial.printf("     Puissance TX   : %s dBm\n", puissanceTexte(mcTxPower).c_str());
}

void CWifi::loadFromWebServer(WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"ssid").c_str())) wifi_ssid = server.arg(mPrefixNVS+"ssid");
  if (server.hasArg((mPrefixNVS+"password").c_str())) wifi_password = server.arg(mPrefixNVS+"password");
  if (server.hasArg((mPrefixNVS+"txpw").c_str())) {
    int v = server.arg(mPrefixNVS+"txpw").toInt();
    if (puissanceValide(v)) mcTxPower = v;
  }
}

String CWifi::getHTML(int i) {
  String options = "";
  for (size_t k = 0; k < NB_PUISSANCES_TX; k++)
    options += "<option value=\"" + String(PUISSANCES_TX[k]) + "\"" + (PUISSANCES_TX[k] == mcTxPower ? " selected" : "") + ">" + puissanceTexte(PUISSANCES_TX[k]) + "</option>";
  String html = "";
  html =  "<div class=\"device\">"
            "<h3>Wifi " + String(i) +"</h3>"
            "<div class=\"row\"><div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom").c_str() + " value=\"" + nomEquipement + "\"></div>"
            "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active").c_str() + " value=\"1\"" + String(active ? " checked" : "") + "></div></div>"
            "<div class=\"row\"><div><label>WiFi SSID</label><input type=\"text\" name=" + (mPrefixNVS+"ssid").c_str() + " value=\"" + wifi_ssid + "\"></div>"
            "<div><label>WiFi Mot de passe</label><input type=\"text\" name=" + (mPrefixNVS+"password").c_str() + " value=\"" + wifi_password + "\"></div>"
            "<div class=\"row\"><div><label>Puissance d'émission (dBm, défaut " + puissanceTexte(default_tx_power) + ")</label><select name=" + (mPrefixNVS+"txpw").c_str() + ">" + options + "</select></div>"
            "<div><label>MQTT subtopic</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic").c_str() + " value=\"" + mqttSubTopic + "\"></div>"
            "</div>"
          "</div>";
//Serial.println("String CWifi::getHTML(int i) : " + html);
  return html;
}

