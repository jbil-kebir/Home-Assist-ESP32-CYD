#ifndef __MYWIFI_H__
#define __MYWIFI_H__

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include "global.h"
#include "MyWebServer.h"

#define MAX_WIFI_NETS 3 // Nombre de connexions Wifi Max

class CWifi {
private:
  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;  

public:
  CWifi() = default;  // Constructeur par défaut → OK pour CWifi mWifi;

  String wifi_ssid;
  String wifi_password;
  const char* default_wifi_ssid = "Simpson";
  const char* default_wifi_password = "al177SOLO$*";
  String nomEquipement = "Wifi";
  String mqttSubTopic = "wifi";
  bool active = false;
  String mPrefixNVS = "wifi_";
  // Puissance d'émission (valeur de wifi_power_t, en quarts de dBm : 34 = 8,5 dBm, 78 = 19,5 dBm)
  #ifdef __ESP32_C3__
  // ESP32-C3 Super Mini : antenne mal adaptée, la connexion échoue souvent à pleine puissance.
  // Réduire améliore la stabilité de près, mais réduit la portée : à régler selon l'emplacement.
  static const int8_t default_tx_power = WIFI_POWER_8_5dBm;
  #else
  static const int8_t default_tx_power = WIFI_POWER_19_5dBm;
  #endif
  int8_t mcTxPower = default_tx_power;

  void setup(const String pref); //const String pref);
  void begin();
  void loadFromNVS();
  void saveToNVS();
  void setPrefixNVS(const char* pr) { mPrefixNVS = pr; }
  String getHTML(int i);  
  void loadFromWebServer(WebServer& server);  
  void print() const;
};

#endif // __MYWIFI_H__