#ifndef __MYMQTT_H__
#define __MYMQTT_H__

#include <PubSubClient.h>
#include <Preferences.h>
#include "global.h"
#include "MyWebServer.h"

#ifdef MQTT_MAX_PACKET_SIZE
#undef MQTT_MAX_PACKET_SIZE
#endif
#define MQTT_MAX_PACKET_SIZE 1024

class CEcran;
class CConfig;

class CMqtt {
private:
  PubSubClient client;
  bool currentState = false;
  CConfig& mConfig;
  CEcran& mEcran;

  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;  

public:
  CMqtt(WiFiClient& wifiClient, CConfig& config, CEcran& ecran/*, CRemoteThermo& remTh1*/) : client(wifiClient), mConfig(config), mEcran(ecran)/*, remoteTh1(&remTh1)*/ {
    
  }
  
  void setup(const String pref);
  void begin(String serv="", uint16_t uiport=0, String user="", String passwd="");
  void reconnect();
  void loop();
  int publishState(bool state);
  int publish(const char* topic, const char* payload);
  int publish(const char* topic, const char* payload, bool retained);
  bool getCurrentState() const {
    return currentState;
  }
  /*
  std::function<int(const String&, const String&)> onEquipement;    // Verifie et le cas échéant ajoute l'équipement
  void setonEquipementCallback(std::function<int(const String& nom, const String& ip)> onEqu); 
  */
  const char* default_mqtt_server = "51.77.244.19";
  const uint16_t default_mqtt_port = 1883;
  const char* default_mqtt_user = "ubuntu";
  const char* default_mqtt_password = "1234567890";
  const char* default_topic_prefix = "home/";

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
