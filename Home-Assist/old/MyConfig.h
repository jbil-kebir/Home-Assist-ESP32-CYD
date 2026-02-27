#ifndef CCONFIG_H
#define CCONFIG_H

#include <Preferences.h>
#include <Arduino.h>

class CConfig {
private:
  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;

public:
  // === VALEURS PAR DÉFAUT ===
  const char* default_wifi_ssid = "TON_SSID";
  const char* default_wifi_password = "TON_PASSWORD";
  const char* default_mqtt_server = "51.77.244.19";
  const uint16_t default_mqtt_port = 1883;
  const char* default_mqtt_user = "ubuntu";
  const char* default_mqtt_password = "1234567890";
  const char* default_topic_prefix = "ad200/";

  const bool default_boiler_state = false;
  const uint8_t default_enable_keep_alive = 1;  // 1 = activé, 0 = désactivé

  // Timings keep-alive en secondes
  const uint32_t default_on_keepalive[5] = {7, 14, 56, 85, 626};
  const uint32_t default_off_keepalive[4] = {30, 64, 77, 626};

  const int32_t default_sleep_timeout = 300;  // 5 minutes, -1 = désactivé

  // === VALEURS CHARGÉES ===
  String wifi_ssid;
  String wifi_password;
  String mqtt_server;
  uint16_t mqtt_port;
  String mqtt_user;
  String mqtt_password;
  String topic_prefix;
  String topic_command;
  String topic_state;
  String topic_status;

  bool etat = false;
  String etatStr = "OFF";

  uint8_t enable_keep_alive = 1;

  uint32_t on_keepalive[5];
  uint32_t off_keepalive[4];
  uint8_t on_keepalive_count = 5;
  uint8_t off_keepalive_count = 4;

  int32_t sleep_timeout = 300;  // en secondes, -1 = désactivé

  void setup();
  void saveBoilerState(bool state);
  void print() const;  // Optionnel pour debug
};

#endif