#include "global.h" // Serial -> journal web (voir MyJournal.h)
#include <Arduino.h>
#include <WiFi.h>
#include "MyWifi.h"
#include "Diagnostic.h"

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

// Canal et BSSID de la box, conservés pendant le deep sleep (perdus à la mise sous tension)
struct WifiMemo {
  bool bValide;
  uint32_t ulHashSsid;  // Le canal mémorisé ne sert qu'au même réseau
  int32_t iCanal;
  uint8_t bssid[6];
};
RTC_DATA_ATTR static WifiMemo gWifiMemo = {};

static uint32_t hashSsid(const String& s) {
  uint32_t h = 2166136261UL; // FNV-1a
  for (size_t k = 0; k < s.length(); k++) { h ^= (uint8_t)s[k]; h *= 16777619UL; }
  return h;
}

bool CWifi::ipFixeValide() const {
  IPAddress ip, passerelle, masque;
  return ip.fromString(msIp) && passerelle.fromString(msPasserelle) && masque.fromString(msMasque);
}

// IP fixe : appelée avant WiFi.begin(). Adresse invalide → DHCP (l'adresse reçue sera apprise, voir apprendIp).
void CWifi::configureIp() {
  if (!mbIpFixe) return;
  IPAddress ip, passerelle, masque, dns;
  if (!ipFixeValide()) {
    Serial.println("IP fixe absente ou invalide : DHCP, l'adresse reçue sera apprise");
    return;
  }
  ip.fromString(msIp); passerelle.fromString(msPasserelle); masque.fromString(msMasque);
  if (!dns.fromString(msDns)) dns = passerelle;
  WiFi.config(ip, passerelle, masque, dns);
}

// Retient l'adresse obtenue en DHCP (IP, passerelle, masque, DNS). Écriture NVS seulement si elle a changé.
void CWifi::apprendIp() {
  String sIp = WiFi.localIP().toString(), sGw = WiFi.gatewayIP().toString();
  String sMasque = WiFi.subnetMask().toString(), sDns = WiFi.dnsIP().toString();
  if (WiFi.localIP() == IPAddress((uint32_t)0)) return; // Pas d'adresse : rien à apprendre
  bool bChange = !mbIpApprise || sIp != msIp || sGw != msPasserelle || sMasque != msMasque || sDns != msDns;
  msIp = sIp; msPasserelle = sGw; msMasque = sMasque; msDns = sDns;
  mbIpApprise = true;
  if (!bChange) return;
  prefs.begin(nvs_namespace, false);
  prefs.putString((mPrefixNVS+"ip").c_str(), msIp);
  prefs.putString((mPrefixNVS+"gw").c_str(), msPasserelle);
  prefs.putString((mPrefixNVS+"mask").c_str(), msMasque);
  prefs.putString((mPrefixNVS+"dns").c_str(), msDns);
  prefs.putBool((mPrefixNVS+"ipap").c_str(), true);
  prefs.end();
  Serial.printf("IP fixe apprise : %s (passerelle %s, masque %s, DNS %s)\n",
                msIp.c_str(), msPasserelle.c_str(), msMasque.c_str(), msDns.c_str()); Serial.flush();
}

// Garde-fou de l'IP apprise : appelé sur échec de connexion MQTT. Une IP saisie à la main n'est jamais touchée.
bool CWifi::repliDhcp() {
  if (!mbIpFixe || !mbIpApprise || mbRepliDhcpFait) return false;
  mbRepliDhcpFait = true;
  Serial.printf("MQTT injoignable avec l'IP apprise %s : reconnexion en DHCP\n", msIp.c_str()); Serial.flush();
  WiFi.disconnect();
  WiFi.config(IPAddress((uint32_t)0), IPAddress((uint32_t)0), IPAddress((uint32_t)0)); // Retour au DHCP
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  WiFi.setTxPower((wifi_power_t)mcTxPower);
  if (!attendConnexion(WIFI_TIMEOUT_MS)) {
    Serial.println("Reconnexion DHCP : échec"); Serial.flush();
    return false;
  }
  apprendIp(); // Réécrit la NVS seulement si la box a donné une autre adresse
  return true;
}

bool CWifi::attendConnexion(unsigned long ulTimeoutMs) {
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < ulTimeoutMs) delay(20);
  return WiFi.status() == WL_CONNECTED;
}

void CWifi::begin() {
  configureIp();
  bool bRapide = mbMemoCanal && gWifiMemo.bValide && gWifiMemo.ulHashSsid == hashSsid(wifi_ssid);
  if (bRapide) WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str(), gWifiMemo.iCanal, gWifiMemo.bssid);
  else WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  // Puissance d'émission du profil (page Web), appliquée après WiFi.begin().
  // ESP32-C3 Super Mini : à pleine puissance, démarrages de 15 s à plus de 90 s constatés (voir MyWifi.h).
  WiFi.setTxPower((wifi_power_t)mcTxPower);
  Serial.printf("Connexion au WiFi %s (puissance %s dBm%s%s)", wifi_ssid.c_str(), puissanceTexte(mcTxPower).c_str(),
                mbIpFixe ? ", IP fixe" : "", bRapide ? ", canal mémorisé" : ""); Serial.flush();
  unsigned long start = millis();
  gDiag.ucWifiRapide = bRapide ? 1 : 0;
  if (bRapide && !attendConnexion(WIFI_TIMEOUT_RAPIDE_MS)) {
    // La box a changé de canal (redémarrage...) : connexion normale
    Serial.println("\nCanal mémorisé : échec, connexion normale"); Serial.flush();
    gDiag.ucWifiRapide = 2;
    gWifiMemo.bValide = false;
    WiFi.disconnect();
    configureIp();
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    WiFi.setTxPower((wifi_power_t)mcTxPower);
  }
  attendConnexion(WIFI_TIMEOUT_MS);
  if (WiFi.status() == WL_CONNECTED) {
    if (mbIpFixe && !ipFixeValide()) apprendIp(); // Connecté en DHCP faute d'adresse : on retient celle de la box
    if (mbMemoCanal) {
      gWifiMemo.iCanal = WiFi.channel();
      memcpy(gWifiMemo.bssid, WiFi.BSSID(), sizeof(gWifiMemo.bssid));
      gWifiMemo.ulHashSsid = hashSsid(wifi_ssid);
      gWifiMemo.bValide = true;
    }
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
  mbIpFixe = prefs.getBool((mPrefixNVS + "ipfx").c_str(), false);
  mbIpApprise = prefs.getBool((mPrefixNVS + "ipap").c_str(), false);
  msIp = prefs.getString((mPrefixNVS + "ip").c_str(), msIp);
  msPasserelle = prefs.getString((mPrefixNVS + "gw").c_str(), msPasserelle);
  msMasque = prefs.getString((mPrefixNVS + "mask").c_str(), msMasque);
  msDns = prefs.getString((mPrefixNVS + "dns").c_str(), msDns);
  mbMemoCanal = prefs.getBool((mPrefixNVS + "memo").c_str(), false);
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
  prefs.putBool((mPrefixNVS+"ipfx").c_str(), mbIpFixe);
  prefs.putString((mPrefixNVS+"ip").c_str(), msIp);
  prefs.putString((mPrefixNVS+"gw").c_str(), msPasserelle);
  prefs.putString((mPrefixNVS+"mask").c_str(), msMasque);
  prefs.putString((mPrefixNVS+"dns").c_str(), msDns);
  prefs.putBool((mPrefixNVS+"ipap").c_str(), mbIpApprise);
  prefs.putBool((mPrefixNVS+"memo").c_str(), mbMemoCanal);
  prefs.end();
}

void CWifi::print() const {
  Serial.println("   WiFi Config");
  Serial.printf("     Nom            : %s\n", nomEquipement.c_str());
  Serial.printf("     Actif          : %s\n", active ? "OUI" : "NON");
  Serial.printf("     SSID           : %s\n", wifi_ssid.c_str());
  Serial.printf("     MQTT SubTopic  : %s\n", mqttSubTopic.c_str());
  Serial.printf("     Puissance TX   : %s dBm\n", puissanceTexte(mcTxPower).c_str());
  Serial.printf("     IP fixe        : %s %s%s (passerelle %s, masque %s, DNS %s)\n", mbIpFixe ? "OUI" : "NON",
                msIp.c_str(), mbIpApprise ? " apprise" : "", msPasserelle.c_str(), msMasque.c_str(), msDns.c_str());
  Serial.printf("     Canal mémorisé : %s\n", mbMemoCanal ? "OUI" : "NON");
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
  mbIpFixe = server.hasArg((mPrefixNVS+"ipfx").c_str());
  if (server.hasArg((mPrefixNVS+"ip").c_str())) {
    String sIp = server.arg(mPrefixNVS+"ip"); sIp.trim();
    // Adresse modifiée sur la page : saisie à la main (jamais retouchée par le capteur) ; vidée : sera réapprise
    if (sIp != msIp) mbIpApprise = false;
    msIp = sIp;
  }
  if (server.hasArg((mPrefixNVS+"gw").c_str())) msPasserelle = server.arg(mPrefixNVS+"gw");
  if (server.hasArg((mPrefixNVS+"mask").c_str())) msMasque = server.arg(mPrefixNVS+"mask");
  if (server.hasArg((mPrefixNVS+"dns").c_str())) msDns = server.arg(mPrefixNVS+"dns");
  mbMemoCanal = server.hasArg((mPrefixNVS+"memo").c_str());
  if (!mbMemoCanal) gWifiMemo.bValide = false;
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
            // Connexion rapide : IP fixe (adresse à réserver dans la box) et canal mémorisé
            "<div class=\"row\"><div class=\"checkbox-row\"><label>IP fixe</label><input type=\"checkbox\" name=" + (mPrefixNVS+"ipfx").c_str() + " value=\"1\"" + String(mbIpFixe ? " checked" : "") + "></div>"
            "<div class=\"checkbox-row\"><label>Mémoriser le canal</label><input type=\"checkbox\" name=" + (mPrefixNVS+"memo").c_str() + " value=\"1\"" + String(mbMemoCanal ? " checked" : "") + "></div></div>"
            "<div class=\"row\"><div><label>Adresse IP" + String(mbIpApprise ? " (apprise de la box)" : (mbIpFixe && !ipFixeValide() ? " (vide : apprise de la box au prochain démarrage)" : "")) + "</label><input type=\"text\" name=" + (mPrefixNVS+"ip").c_str() + " value=\"" + msIp + "\" placeholder=\"vide (actuelle : " + WiFi.localIP().toString() + ")\"></div>"
            "<div><label>Passerelle</label><input type=\"text\" name=" + (mPrefixNVS+"gw").c_str() + " value=\"" + msPasserelle + "\"></div></div>"
            "<div class=\"row\"><div><label>Masque</label><input type=\"text\" name=" + (mPrefixNVS+"mask").c_str() + " value=\"" + msMasque + "\"></div>"
            "<div><label>DNS</label><input type=\"text\" name=" + (mPrefixNVS+"dns").c_str() + " value=\"" + msDns + "\"></div></div>"
          "</div>";
//Serial.println("String CWifi::getHTML(int i) : " + html);
  return html;
}

