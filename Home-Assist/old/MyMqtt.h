#ifndef __MYMQTT_H__
#define __MYMQTT_H__

// Topics MQTT
#define MQTT_TOPIC_COMMAND "ad200/command"   // Envoie "ON" ou "OFF" ici
#define MQTT_TOPIC_STATE   "ad200/state"     // État publié (ON/OFF)
#define MQTT_TOPIC_STATUS  "ad200/status"    // online/offline

class CMqtt {
private:
  PubSubClient client;
  bool currentState = false;
  using CommandCallback = std::function<void(bool)>;
  CommandCallback onCommand;
  CConfig &mConfig;

public:
  CMqtt(WiFiClient& wifiClient, CConfig& config) : client(wifiClient), mConfig(config) {}
  
  void setup(CommandCallback cb);
  void reconnect();
  void loop();
  void publishState(bool state);
  bool getCurrentState() const {
    return currentState;
  }

private:
  void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
    message.toUpperCase();
    Serial.println("MQTT reçu : " + message);

    if (message == "ON" && onCommand) {
      onCommand(true);
    } else if (message == "OFF" && onCommand) {
      onCommand(false);
    }
  }
};
#endif // __MYMQTT_H__
