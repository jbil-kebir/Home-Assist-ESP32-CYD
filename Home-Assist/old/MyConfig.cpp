/*#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "global.h"*/
#include "MyConfig.h"


void CConfig::setup() {
  prefs.begin(nvs_namespace, false);  // read/write

  // Chargement WiFi/MQTT
  wifi_ssid = prefs.getString("wifi_ssid", default_wifi_ssid);
  wifi_password = prefs.getString("wifi_password", default_wifi_password);
  mqtt_server = prefs.getString("mqtt_server", default_mqtt_server);
  mqtt_port = prefs.getUShort("mqtt_port", default_mqtt_port);
  mqtt_user = prefs.getString("mqtt_user", default_mqtt_user);
  mqtt_password = prefs.getString("mqtt_password", default_mqtt_password);
  topic_prefix = prefs.getString("topic_prefix", default_topic_prefix);

  topic_command = topic_prefix + mqqtInfo.subtopic_command;
  topic_state = topic_prefix + mqqtInfo.subtopic_state;
  topic_status = topic_prefix + mqqtInfo.subtopic_status;

  // État chaudière
  etat = prefs.getBool("boiler_state", default_boiler_state);
  etatStr = etat ? "ON" : "OFF";

  // Flag keep-alive
  enable_keep_alive = prefs.getUChar("enable_keep_alive", default_enable_keep_alive);

  // Timings keep-alive
  on_keepalive_count = 5;
  for (uint8_t i = 0; i < 5; i++) {
    String key = "on_keep_" + String(i);
    on_keepalive[i] = prefs.getULong(key.c_str(), default_on_keepalive[i]);
  }

  off_keepalive_count = 4;
  for (uint8_t i = 0; i < 4; i++) {
    String key = "off_keep_" + String(i);
    off_keepalive[i] = prefs.getULong(key.c_str(), default_off_keepalive[i]);
  }

  // Timeout veille
  sleep_timeout = prefs.getInt("sleep_timeout", default_sleep_timeout);

  prefs.end();

  Serial.println("\n=== Configuration NVS chargée ===");
  Serial.printf("Keep-alive : %s\n", enable_keep_alive ? "ACTIVÉ" : "DÉSACTIVÉ");
  Serial.printf("Veille timeout : %ld s (%s)\n", sleep_timeout, (sleep_timeout <= 0 ? "Désactivée" : "Activée"));
}

void CConfig::saveBoilerState(bool state) {
  prefs.begin(nvs_namespace, false);
  prefs.putBool("boiler_state", state);
  prefs.end();

  etat = state;
  etatStr = state ? "ON" : "OFF";
}