  
#include <Arduino.h>
#include <WiFi.h>
#include "MyConfig.h"
#include "MyWifi.h"

  
  void CWifi::setup() {
    WiFi.begin(mConfig.wifi_ssid.c_str(), mConfig.wifi_password.c_str());
    Serial.print("Connexion WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi connecté - IP : " + WiFi.localIP().toString());
  }
