#include <Arduino.h>
#include <WiFi.h>
#include "MyWifi.h"

void CWifi::setup(const String pref) { //const String pref) {
  mPrefixNVS = pref;
  loadFromNVS();
  //active = true; // A supprimer
}

// Dernière raison de déconnexion renvoyée par la pile WiFi (diagnostic)
static volatile uint8_t sDerniereRaison = 0;
static uint8_t sDernierBssid[6] = {0};
static bool sEvtInstalle = false;

static const char* raisonTexte(uint8_t r) {
  switch (r) {
    case 2:   return "AUTH_EXPIRE";
    case 3:   return "AUTH_LEAVE";
    case 4:   return "ASSOC_EXPIRE";
    case 8:   return "ASSOC_LEAVE";
    case 15:  return "4WAY_HANDSHAKE_TIMEOUT (mot de passe ?)";
    case 200: return "BEACON_TIMEOUT (signal trop faible ?)";
    case 201: return "NO_AP_FOUND (SSID introuvable / 5 GHz / canal)";
    case 202: return "AUTH_FAIL (mot de passe ou WPA3 ?)";
    case 203: return "ASSOC_FAIL (box refuse l'association)";
    case 204: return "HANDSHAKE_TIMEOUT (mot de passe ?)";
    case 205: return "CONNECTION_FAIL";
    case 210: return "NO_AP_FOUND_W_COMPATIBLE_SECURITY (WPA3 seul ?)";
    case 211: return "NO_AP_FOUND_IN_AUTHMODE_THRESHOLD";
    case 212: return "NO_AP_FOUND_IN_RSSI_THRESHOLD";
    default:  return "?";
  }
}

static const char* authTexte(wifi_auth_mode_t a) {
  switch (a) {
    case WIFI_AUTH_OPEN:            return "OPEN";
    case WIFI_AUTH_WEP:             return "WEP";
    case WIFI_AUTH_WPA_PSK:         return "WPA";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-ENT";
    case WIFI_AUTH_WPA3_PSK:        return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2/WPA3";
    default:                        return "?";
  }
}

// Affiche systématiquement (indépendamment de gDebugFlags) le diagnostic d'échec
void CWifi::diagnostic() {
  Serial.printf("\n--- Diagnostic WiFi '%s' ---\n", wifi_ssid.c_str());
  Serial.printf("  Statut WiFi    : %d\n", (int)WiFi.status());
  Serial.printf("  Raison déconnexion : %u %s\n", sDerniereRaison, raisonTexte(sDerniereRaison));
  Serial.printf("  Point d'accès refusant : %02X:%02X:%02X:%02X:%02X:%02X\n",
                sDernierBssid[0], sDernierBssid[1], sDernierBssid[2], sDernierBssid[3], sDernierBssid[4], sDernierBssid[5]);
  Serial.printf("  MAC ESP32      : %s\n", WiFi.macAddress().c_str());
  Serial.println("  Scan des réseaux 2,4 GHz...");
  WiFi.disconnect();   // Arrête les tentatives de reconnexion, sinon le scan échoue
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.printf("  %d réseau(x) trouvé(s)\n", n);
  bool trouve = false;
  for (int k = 0; k < n; k++) {
    bool cible = (WiFi.SSID(k) == wifi_ssid);
    trouve |= cible;
    Serial.printf("  %s %-32s %s  RSSI %4d  canal %2d  %s\n", cible ? "=>" : "  ",
                  WiFi.SSID(k).c_str(), WiFi.BSSIDstr(k).c_str(), WiFi.RSSI(k), WiFi.channel(k), authTexte(WiFi.encryptionType(k)));
  }
  if (!trouve) Serial.printf("  SSID '%s' NON VU par l'ESP32 (box en 5 GHz seul, canal 12/13, SSID masqué ?)\n", wifi_ssid.c_str());
  WiFi.scanDelete();
  Serial.println("------------------------------");
}

void CWifi::begin() {
  if (!sEvtInstalle) {
    WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t info) {
      sDerniereRaison = info.wifi_sta_disconnected.reason;
      memcpy(sDernierBssid, info.wifi_sta_disconnected.bssid, 6);
    }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    sEvtInstalle = true;
  }
  sDerniereRaison = 0;
  // Plusieurs points d'accès au même SSID (répéteurs) : scanner tous les canaux et prendre le plus fort
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  DBG(DBG_RESEAU, "Connexion au WiFi %s", wifi_ssid.c_str());
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    DBG(DBG_RESEAU, ".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    DBGLN(DBG_RESEAU, "\nWiFi connecté - IP : " + WiFi.localIP().toString());
  } else {
    DBG(DBG_RESEAU, "\nÉchec connexion WiFi\n");
    diagnostic();
  }
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
  DBG(DBG_RESEAU, "   WiFi Config\n");
  DBG(DBG_RESEAU, "     Nom            : %s\n", nomEquipement.c_str());
  DBG(DBG_RESEAU, "     Actif          : %s\n", active ? "OUI" : "NON");
  DBG(DBG_RESEAU, "     SSID           : %s\n", wifi_ssid.c_str());
  DBG(DBG_RESEAU, "     MQTT SubTopic  : %s\n", mqttSubTopic.c_str());
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
  return html;
}

