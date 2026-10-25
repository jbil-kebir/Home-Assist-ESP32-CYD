#ifndef __MYWIFI_H__
#define __MYWIFI_H__

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include "global.h"
#include "MyWebServer.h"

#define MAX_WIFI_NETS 3 // Nombre de connexions Wifi Max
#define WIFI_TIMEOUT_MS 15000UL        // Attente maximale d'une connexion normale
#define WIFI_TIMEOUT_RAPIDE_MS 3000UL  // Attente maximale d'une connexion sur le canal mémorisé (sinon : connexion normale)

class CWifi {
private:
  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;  

public:
  CWifi() = default;  // Constructeur par défaut → OK pour CWifi mWifi;

  String wifi_ssid;
  String wifi_password;
  // Wi-Fi d'une puce neuve (NVS vide) : identifiants de secret.h, hors Git
  const char* default_wifi_ssid = FORCE_WIFI_SSID;
  const char* default_wifi_password = FORCE_WIFI_PASSWD;
  String nomEquipement = "Wifi";
  String mqttSubTopic = "wifi";
  bool active = false;
  String mPrefixNVS = "wifi_";
  // Puissance d'émission (valeur de wifi_power_t, en quarts de dBm : 34 = 8,5 dBm, 78 = 19,5 dBm)
  #if defined(__ESP32_C3__) && !defined(BOARD_XIAO_C3)
  // ESP32-C3 Super Mini (le XIAO ESP32-C3, antenne externe, garde 19,5 dBm) : antenne mal adaptée, la connexion échoue souvent à pleine puissance.
  // Réduire améliore la stabilité de près, mais réduit la portée : à régler selon l'emplacement.
  static const int8_t default_tx_power = WIFI_POWER_8_5dBm;
  #else
  static const int8_t default_tx_power = WIFI_POWER_19_5dBm;
  #endif
  int8_t mcTxPower = default_tx_power;

  // Connexion rapide (réveil de deep sleep) : voir doc/Adresse_IP_Fixe.docx
  // IP fixe : pas de DHCP. L'adresse doit être réservée dans la box (bail statique) pour l'adresse MAC du capteur.
  bool mbIpFixe = false;
  String msIp = "";
  String msPasserelle = "192.168.1.254"; // Freebox
  String msMasque = "255.255.255.0";
  String msDns = "192.168.1.254";
  // Mémorisation du canal et du BSSID de la box en mémoire RTC : pas de recherche sur tous les canaux au réveil
  bool mbMemoCanal = false;

  void setup(const String pref); //const String pref);
  void begin();
  void loadFromNVS();
  void saveToNVS();
  void setPrefixNVS(const char* pr) { mPrefixNVS = pr; }
  String getHTML(int i);  
  void loadFromWebServer(WebServer& server);  
  void print() const;

private:
  void configureIp();
  bool attendConnexion(unsigned long ulTimeoutMs);
};

#endif // __MYWIFI_H__