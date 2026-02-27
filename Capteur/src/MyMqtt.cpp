#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "global.h"
#include "MyConfig.h"
#include "MyWifi.h"
#include "MyMqtt.h"

void CMqtt::setup(const String pref) {
  mPrefixNVS = pref;
  loadFromNVS();
  Serial.println("===========void CMqtt::setup(const String pref)===========");
  print();
}

void CMqtt::begin() {
  client.setServer(mqtt_server.c_str(), mqtt_port);
  client.setCallback([this](char* t, byte* p, unsigned int l) { this->callback(t, p, l); });

  reconnect();
}

void CMqtt::powerDown() {
  if (client.connected()) client.disconnect();
}
void CMqtt::reconnect() {
  while (!client.connected()) {
    String clientId = "Thermometre_" + String(random(0xffff), HEX);
    //Serial.print("Connexion MQTT Id : " + clientId + " - User : " + mqtt_user + " - Passwd : " + mqtt_password + " ...");
    String sPayload = mConfig.nomEquipement + String(" ") + String("offline");
    if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str(),
                        mConfig.topic_config_command.c_str(), 0, true, sPayload.c_str())) {
      Serial.println("connecté");

      // === ABONNEMENTS FORCÉS À CHAQUE CONNEXION ===
      #ifdef CAPTEUR_DS18B20
      client.subscribe(mConfig.ds18b20->mqttSubTopicCommand.c_str(), 1);
      Serial.println("Abonnement à : " + mConfig.ds18b20->mqttSubTopicCommand);
      //client.subscribe(mConfig.ds18b20->mqttSubTopicState.c_str(), 1);
      //Serial.println("Abonnement à : " + mConfig.ds18b20->mqttSubTopicState);
      #endif
      #ifdef CAPTEUR_DHT20
      client.subscribe(mConfig.dht20->mqttSubTopicCommand.c_str(), 1);
      Serial.println("Abonnement à : " + mConfig.dht20->mqttSubTopicCommand);
      //client.subscribe(mConfig.dht20->mqttSubTopicState.c_str(), 1);
      //Serial.println("Abonnement à : " + mConfig.dht20->mqttSubTopicState);
      #endif


      client.subscribe(mConfig.topic_config_command.c_str(), 1);
      Serial.println("Abonnement à : " + mConfig.topic_config_command);
 
 
      sPayload = mConfig.nomEquipement + String(" ") + String("online") + " le " + mConfig.mDateTime->getDate() + " à " + mConfig.mDateTime->getTime();
      // Publication des états retained
      #ifdef CAPTEUR_DS18B20
      client.publish(mConfig.ds18b20->mqttSubTopicState.c_str(), sPayload.c_str(), false);
      #endif
      #ifdef CAPTEUR_DHT20
      client.publish(mConfig.dht20->mqttSubTopicState.c_str(), sPayload.c_str(), false);
      #endif
      //publishState(currentState);


    } else {
      Serial.print("échec, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

  void CMqtt::loop() {
    if (!client.connected()) reconnect();
    client.loop();
  }

  int CMqtt::publishState(bool state) {
    if (!active) return -1;
    currentState = state;
    #ifdef CAPTEUR_DS18B20
    client.publish(mConfig.ds18b20->mqttSubTopicState.c_str(), state ? "ON" : "OFF", false);
    #endif
    #ifdef CAPTEUR_DHT20
    client.publish(mConfig.dht20->mqttSubTopicState.c_str(), state ? "ON" : "OFF", false);
    #endif
    return 0;
  }
//
// Retour :
// 0 : RAS
// -1 : MQTT inactif
// -2 : tentative de publication échouée
// -3 : MQTT non connecté
//
int CMqtt::publish(const char* topic, const char* payload, bool retained) {
  int ret = 0;
  if (!active) return -1;
  String s = String(payload) + String(" ") + WiFi.localIP().toString();
  if (client.connected()) {
    if (client.publish(topic, s.c_str(), retained)) {
      Serial.printf("MQTT publié (retained=%d) sur %s : %s\n", retained, topic, s.c_str());
      ret = 0;
    }
    else {
      Serial.println("MQTT — tentative de publication échouée");
      ret = -2;
    }
  } 
  else {
    Serial.println("MQTT non connecté — publication ignorée");
    ret = -3;
  }
  return ret;
}

// Garde aussi la version sans retained (pour compatibilité)
int CMqtt::publish(const char* topic, const char* payload) {
  if (!active) return -1;
  publish(topic, payload, false);
  return 0;
}

void CMqtt::callback(char* topic, byte* payload, unsigned int length) {
  if (!active) return;
  String message;
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  message.toUpperCase();
  Serial.println("MQTT reçu sur " + String(topic) + " : " + message);

  #ifdef CAPTEUR_DS18B20
  if (String(topic) == mConfig.ds18b20->mqttSubTopicCommand) { // 
  #endif
  #ifdef CAPTEUR_DHT20
  if (String(topic) == mConfig.dht20->mqttSubTopicCommand) { // 
  #endif
  #if defined (CAPTEUR_DS18B20) || defined (CAPTEUR_DHT20)
    //mConfig.ds18b20->handleMqttCommand(message);
    int idx = 0;
    String sExpediteur = message.substring(0, message.indexOf(' ', idx)); // ex : "ThSdb xxx yyyy"
    idx = message.indexOf(' ', idx) + 1;
    if (idx >= message.length()) {
      Serial.println("Commande incomplète longueur : " + String(message.length()) + " index : " + String(idx));
      return ; // Commande incomplète
    }
    String sCommand = message.substring(idx, message.indexOf(' ', idx));
    idx = message.indexOf(' ', idx) + 1;
    if (idx >= message.length()) {
      Serial.println("Commande sans argument. RAS");
      return; // Commande sans argument. RAS
    }
    else {
      if (sCommand == "SLEEP") {
        String sArg = message.substring(idx, message.indexOf(' ', idx));
        int sleep = (int)sArg.substring(0).toDouble();
        Serial.println("Sleep : "+String(sleep));
        if (sleep!=0L) {
          if (sleep == -1) {
            mConfig.setDeepSleep(false);
            //Serial.println("Deep Sleep Désactivé");
          }
          else {
            mConfig.setSleepIntervalle((unsigned long)sArg.substring(0).toDouble());
            //Serial.println("Deep Sleep - Nouvelle durée : " + String((unsigned long)sArg.substring(0).toDouble()));
          }
        } // if (sleep!=0L)
      } // if (sCommand == "SLEEP") 
    }
  } 
  // Traitement des commandes de configuration
  else if (String(topic) == mConfig.topic_config_command) {
    message.trim();  // Supprime espaces début/fin
    message.toUpperCase();

    Serial.println("Commande configuration reçue : " + message);
    mConfig.handleMqttCommand(message);
  if (message == "STATUS") {
      String statusMsg = "=== ÉTAT "+ String(mConfig.nomEquipement) +" ===\n";
      statusMsg += String(mConfig.mDateTime->getDate()) + " " + String(mConfig.mDateTime->getTime()) + "\n";
      statusMsg += "IP     : " + WiFi.localIP().toString() + "\n";
      statusMsg += "Uptime : " + String(millis() / 1000) + "s";

      client.publish(mConfig.topic_config_state.c_str(), statusMsg.c_str(), false);
      Serial.println("STATUS publié sur " + mConfig.topic_config_state);
      Serial.println(statusMsg); Serial.flush();
    }
    else if (message == "REBOOT") {
      ESP.restart();
    }
   /* else if(message.startsWith("SLEEP")) {
      // On regarde s'il y a un argujment
      int idx = 0;
      String sCmd = message.substring(0, message.indexOf(' ', idx));
      idx = message.indexOf(' ', idx) + 1;
      Serial.println("Long : " + String(message.length()) + " Index : "+String(idx));
      String sArg = message.substring(idx, message.indexOf(' ', idx));

      Serial.println("============================================");
      Serial.println("cmd : "+sCmd+" Arg : " + sArg) + " index : " + String(idx);

    }*/
    // On peut ajouter d’autres commandes plus tard
  }
  #endif
  
}

void CMqtt::loadFromNVS() {
  prefs.begin(nvs_namespace, true);
  mqtt_server = prefs.getString((mPrefixNVS + "server").c_str(), default_mqtt_server);
  mqtt_port = prefs.getUShort((mPrefixNVS + "port").c_str(), default_mqtt_port);
  mqtt_user = prefs.getString((mPrefixNVS + "user").c_str(), default_mqtt_user);
  mqtt_password = prefs.getString((mPrefixNVS + "password").c_str(), default_mqtt_password);

  nomEquipement = prefs.getString((mPrefixNVS + "nom").c_str(), "MQTT");
  active = prefs.getBool((mPrefixNVS + "active").c_str(), true);
  /*mqttSubTopic = prefs.getString((mPrefixNVS + "subtopic").c_str(), "wifi");*/
  prefs.end();
}

void CMqtt::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putString((mPrefixNVS + "server").c_str(), mqtt_server);
  prefs.putUShort((mPrefixNVS + "port").c_str(), mqtt_port);
  prefs.putString((mPrefixNVS + "user").c_str(), mqtt_user);
  prefs.putString((mPrefixNVS + "password").c_str(), mqtt_password);

  prefs.putString((mPrefixNVS + "nom").c_str(), nomEquipement);
  prefs.putBool((mPrefixNVS + "active").c_str(), active);
  /*prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);*/
  prefs.end();
}

void CMqtt::loadFromWebServer(WebServer& server) {
  if (server.hasArg("mqtt_server")) mqtt_server = server.arg((mPrefixNVS + "server").c_str());
  if (server.hasArg("mqtt_port")) mqtt_port = server.arg((mPrefixNVS + "port").c_str()).toInt();
  if (server.hasArg("mqtt_user")) mqtt_user = server.arg((mPrefixNVS + "user").c_str());
  if (server.hasArg("mqtt_password")) mqtt_password = server.arg((mPrefixNVS + "password").c_str());

  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  /*if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());*/
}

void CMqtt::print() const {
  Serial.println("[MQTT]");
  Serial.printf("  Nom          : %s\n", nomEquipement.c_str());
  Serial.printf("  Actif        : %s\n", active ? "OUI" : "NON");
  Serial.printf("  Server       : %s\n", mqtt_server.c_str());
  Serial.printf("  Port         : %d\n", mqtt_port);
  Serial.printf("  User         : %s\n", mqtt_user.c_str());
  Serial.printf("  Password     : %s\n", mqtt_password.c_str());
  /*Serial.printf("     MQTT SubTopic  : %s\n", mqttSubTopic.c_str());*/
}

String CMqtt::getHTML() {
  String html = "";
  html =  "<h2>MQTT</h2>"
    "<div class=\"row\">"
      "<div class=\"row\"><div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+String("nom")) + " value=\"" + nomEquipement + "\"></div>"
      "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active") + " value=\"1\"" + String(active ? " checked" : "") + "></div></div>"
    "</div>"
    "<div class=\"row\">"
      "<div><label>MQTT Serveur</label><input type=\"text\" name=\"mqtt_server\" value=\"" + mqtt_server + "\"></div>"
      "<div><label>MQTT Port</label><input type=\"number\" name=\"mqtt_port\" value=\"" + String(mqtt_port) + "\"></div>"
    "</div>"
    "<div class=\"row\">"
      "<div><label>MQTT Utilisateur</label><input type=\"text\" name=\"mqtt_user\" value=\"" + mqtt_user + "\"></div>"
      "<div><label>MQTT Mot de passe</label><input type=\"password\" name=\"mqtt_password\" value=\"" + mqtt_password + "\"></div>"
    "</div>";


  return html;
}
