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
  //Serial.printf("void CMqtt::setup() - mConfig.mRemoteThCh1er->mqttSubTopicState : %s\n", mConfig.mRemoteThCh1er->mqttSubTopicState.c_str());
}

void CMqtt::begin(String sserv/*=""*/, uint16_t uiport/*=0*/, String user/*=""*/, String passwd/*=""*/) {

  if (!sserv.isEmpty() && uiport != 0 && !user.isEmpty() && !passwd.isEmpty()) {
    mqtt_server = sserv;
    mqtt_port = uiport;
    mqtt_user = user;
    mqtt_password = passwd;
  }
  //Serial.printf("void CMqtt::begin() %s %ld\n", mqtt_server.c_str(), mqtt_port);
  client.setServer(mqtt_server.c_str(), mqtt_port);
  client.setCallback([this](char* t, byte* p, unsigned int l) { this->callback(t, p, l); });

  //reconnect();
}

void CMqtt::reconnect() {
  //int tentatives = 5;
  while (!client.connected()) {
    Serial.print("Connexion MQTT...");
    String clientId = "CYD_AD200_" + String(random(0xffff), HEX);
    #ifdef __LOCAL_MODE__
    String topicStatus = mConfig.chaudiere->mqttSubTopicStatus;
    #else
    String topicStatus = mConfig.topic_config_state;//"home/configuration/state";
    #endif
    String sOut = mConfig.nomEquipement + " - Reconnect le " + mConfig.mDateTime->getDate() + " à " + mConfig.mDateTime->getTime();
    //Serial.printf("void CMqtt::reconnect() %s %s %s\n", clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str());
    if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str(),
                        topicStatus.c_str(), 0, false, sOut.c_str())) {
      Serial.println(sOut);


      // === ABONNEMENTS FORCÉS À CHAQUE CONNEXION ===
//      Serial.println("Abonnement à ad200/command");
//      client.subscribe(mConfig.chaudiere->mqttSubTopicCommand.c_str());
      Serial.println("Abonnement à " + mConfig.topic_config_command);
      client.subscribe(mConfig.topic_config_command.c_str()); //, 1);
      Serial.println("Abonnement à " + String("home/confthremise/command"));
      client.subscribe("home/confthremise/command"); //, 1);
      Serial.println("Abonnement à " + String("home/thermometre/command"));
      client.subscribe("home/thermometre/command"); //, 1);
      // Ajouter ici tous les équipements relayés par Lora
//      Serial.println("Abonnement à " + mConfig.mRemoteThCh1er->mqttSubTopicState);
//      client.subscribe(mConfig.mRemoteThCh1er->mqttSubTopicState.c_str());
    } else {
      Serial.print("échec, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  } // while()
}

  void CMqtt::loop() {
    if (!client.connected()) reconnect();
    client.loop();
  }


int CMqtt::publish(const char* topic, const char* payload, bool retained) {
  if (!active) return -1;
  if (client.connected()) {
    client.publish(topic, payload, retained);
    Serial.printf("MQTT publié (retained=%d) sur %s : %s\n", retained, topic, payload);
  } 
  else {
    Serial.println("MQTT non connecté — publication ignorée");
  }
  return 0;
}

// Garde aussi la version sans retained (pour compatibilité)
int CMqtt::publish(const char* topic, const char* payload) {
  if (!active) return -1;
  publish(topic, payload, false);
  return 0;
}

void CMqtt::callback(char* topic, byte* payload, unsigned int length) {
  if (!active) return;
  String message, messageOrg;
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  message.trim();
  messageOrg = message;
  message.toUpperCase();

  Serial.printf("void CMqtt::callback() - reçu : %s :  %s\n", topic, message.c_str());

  // Commandes de configuration
  if (String(topic) == mConfig.topic_config_command) {
    Serial.println("Commande configuration reçue : " + message);
    mConfig.handleMqttCommand(messageOrg);
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
  } // if (String(topic) == mConfig.topic_config_command) { 
  else if (String(topic) == "home/thermometre/command") {
    if (message.startsWith("THREMISE") ||
        message.startsWith("BATREMISE") ||
        message.startsWith("FLOTTEURREMISE") ||
        message.startsWith("THNOMADE") ||
        message.startsWith("BATNOMADE") ||
        message.startsWith("FLOTTEURNOMADE") ||
        message.startsWith("THCAVE") ||
        message.startsWith("BATCAVE")) {
      Serial.println("Commande thermomètre reçue : " + message);
      // Envoyer la trame par Lora
      String s = "home/thermometre/command " + message;
      Serial.println("Envoi de la commande par Lora : " + s);
      mConfig.onLoraPublish("home/thermometre/command", messageOrg.c_str());
    }
    else {
      Serial.printf("CMqtt::callback() - Equipement non traité ***%s*** : \n", message.c_str());
      
    }
  }
  else if (String(topic) == "home/confthremise/command") {
    mConfig.onLoraPublish("home/confthremise/command", messageOrg.c_str());
  }
  else {
    Serial.printf("CMqtt::callback() - Topic non traité ***%s*** : \n", topic);
  }
  
}

void CMqtt::loadFromNVS() {
  prefs.begin(nvs_namespace, true);
  mqtt_server = prefs.getString((mPrefixNVS + "server").c_str(), default_mqtt_server);
  mqtt_port = prefs.getUShort((mPrefixNVS + "port").c_str(), default_mqtt_port);
  mqtt_user = prefs.getString((mPrefixNVS + "user").c_str(), default_mqtt_user);
  mqtt_password = prefs.getString((mPrefixNVS + "password").c_str(), default_mqtt_password);

  nomEquipement = prefs.getString((mPrefixNVS + "nom").c_str(), "MQTT");
  active = prefs.getBool((mPrefixNVS + "active").c_str(), true);
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
  prefs.end();
}

void CMqtt::loadFromWebServer(WebServer& server) {
  if (server.hasArg("mqtt_server")) mqtt_server = server.arg((mPrefixNVS + "server").c_str());
  if (server.hasArg("mqtt_port")) mqtt_port = server.arg((mPrefixNVS + "port").c_str()).toInt();
  if (server.hasArg("mqtt_user")) mqtt_user = server.arg((mPrefixNVS + "user").c_str());
  if (server.hasArg("mqtt_password")) mqtt_password = server.arg((mPrefixNVS + "password").c_str());

  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
}

void CMqtt::print() const {
  Serial.println("[MQTT]");
  Serial.printf("  Nom          : %s\n", nomEquipement.c_str());
  Serial.printf("  Actif        : %s\n", active ? "OUI" : "NON");
  Serial.printf("  Server       : %s\n", mqtt_server.c_str());
  Serial.printf("  Port         : %d\n", mqtt_port);
  Serial.printf("  User         : %s\n", mqtt_user.c_str());
  Serial.printf("  Password     : %s\n", mqtt_password.c_str());
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
