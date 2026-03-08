#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "MyConfig.h"
#include "MyWifi.h"
#include "MyMqtt.h"

  void CMqtt::setup(CommandCallback cb) {
    onCommand = cb;
    client.setServer(mConfig.mqqtInfo.mqtt_server.c_str(), mConfig.mqqtInfo.mqtt_port);
    client.setCallback([this](char* topic, byte* payload, unsigned int length) {
      this->callback(topic, payload, length);
    });
  }

  void CMqtt::reconnect() {
    while (!client.connected()) {
      Serial.print("Connexion MQTT...");
      String clientId = "CYD_AD200_" + String(random(0xffff), HEX);
      if (client.connect(clientId.c_str(), mConfig.mqqtInfo.mqtt_user.c_str(), mConfig.mqqtInfo.mqtt_password.c_str(), mConfig.topic_status.c_str(), 0, true, "offline")) {
        Serial.println("connecté");
        client.publish(mConfig.topic_status.c_str(), "Contrôleur principal online", false);
        client.subscribe(mConfig.topic_command.c_str());
        //client.publish(mConfig.topic_state.c_str(), currentState ? "ON" : "OFF", false); KJ
      } 
      else {
        Serial.print("échec, rc=");
        Serial.println(client.state());
        delay(3000);
      }
    }
  }

  void CMqtt::loop() {
    if (!client.connected()) reconnect();
    client.loop();
  }

  void CMqtt::publishState(bool state) {
    currentState = state;
    //client.publish(mConfig.topic_state.c_str(), state ? "ON" : "OFF", true);
  }

