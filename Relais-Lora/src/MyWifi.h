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
  const char* default_wifi_ssid = "xxxx";
  const char* default_wifi_password = "xxxxxxxx";
  String nomEquipement = "Wifi";
  String mqttSubTopic = "wifi";
  bool active = false;
  String mPrefixNVS = "wifi_";  

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