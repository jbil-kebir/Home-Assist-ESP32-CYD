#ifndef __MYMQTT_H__
#define __MYMQTT_H__

#include <PubSubClient.h>
#include <Preferences.h>
#include "global.h"
#include "MyWebServer.h"

class CConfig;

#define MQTT_RECONNECT_INTERVAL_MS 30000UL // Délai entre deux tentatives de connexion au broker
// Juste après le boot (réveil de deep sleep), tentatives rapprochées pour publier avant la fin de l'éveil
#define MQTT_RECONNECT_INTERVAL_BOOT_MS 2000UL
#define MQTT_RECONNECT_DUREE_BOOT_MS 60000UL // Durée après le boot pendant laquelle l'intervalle court s'applique
#define MQTT_SOCKET_TIMEOUT_S 3             // Attente maximale d'une réponse du broker (CONNACK...)
#define MQTT_ECHO_TIMEOUT_MS 1500UL         // powerDown() : attente maximale de l'écho
#define MQTT_FERMETURE_TIMEOUT_MS 500UL     // powerDown() : attente maximale de la fermeture par le broker

class CMqtt {
private:
  PubSubClient client;
  WiFiClient& mWifiClient;  // Socket du client MQTT : utilisée par powerDown() pour la déconnexion
  bool mbEchoRecu = false;  // powerDown() : écho reçu
  bool currentState = false;
  CConfig& mConfig;
  unsigned long mulDerniereTentative = 0; // millis() de la dernière tentative de connexion au broker

  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;  

public:
  CMqtt(WiFiClient& wifiClient, CConfig& config) : client(wifiClient), mWifiClient(wifiClient), mConfig(config) {}
  
  void setup(const String pref);
  void begin();
  void reconnect();
  void powerDown(); // Appelée pour deep-sleep
  void loop();
  int publishState(bool state);
  int publish(const char* topic, const char* payload);
  int publish(const char* topic, const char* payload, bool retained);
  bool getCurrentState() const {
    return currentState;
  }

  const char* default_mqtt_server = "1.1.1.1";
  const uint16_t default_mqtt_port = 1883;
  const char* default_mqtt_user = "xxxxxxx";
  const char* default_mqtt_password = "xxxxxxxxxxxx";
  const char* default_topic_prefix = "ad200/";

  String nomEquipement = "Mqtt";
  String mqttSubTopic = "mqtt";
  bool active = true;
  String mPrefixNVS = "mqtt_";  
  String mqtt_server;
  uint16_t mqtt_port;
  String mqtt_user;
  String mqtt_password;
  String subtopic_command = "command";      // Dernier élément de l'arborescence MQTT ==> Commande
  String subtopic_state = "state";          // Dernier élément de l'arborescence MQTT ==> Etat
  String subtopic_status = "status";       // Dernier élément de l'arborescence MQTT ==> Status

  void loadFromNVS();
  void saveToNVS();
  void setPrefixNVS(const char* pr) { mPrefixNVS = pr; }
  String getHTML();  
  void loadFromWebServer(WebServer& server);  
  void print() const;

private:
    void callback(char* topic, byte* payload, unsigned int length);
};
#endif // __MYMQTT_H__
