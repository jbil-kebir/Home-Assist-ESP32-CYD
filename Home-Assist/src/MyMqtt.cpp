#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include <WiFi.h>
#include <PubSubClient.h>
#include "global.h"
#include "MyConfig.h"
#include "MyWifi.h"
#include "MyEcran.h"
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
  client.setServer(mqtt_server.c_str(), mqtt_port);
  client.setCallback([this](char* t, byte* p, unsigned int l) { this->callback(t, p, l); });

  reconnect();
}

void CMqtt::reconnect() {
  while (!client.connected()) {
    Serial.print("Connexion MQTT...");
    String clientId = "CYD_AD200_" + String(random(0xffff), HEX);
    #ifdef __LOCAL_MODE__
    String topicStatus = mConfig.chaudiere->mqttSubTopicStatus;
    #else
    String topicStatus = mConfig.topic_config_state;//"home/configuration/state";
    #endif
    String sOut = mConfig.nomEquipement + " - Reconnect le " + mConfig.mDateTime->getDate() + " à " + mConfig.mDateTime->getTime();
    if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str(),
                        topicStatus.c_str(), 0, false, sOut.c_str())) {
      Serial.println(sOut);

// Nettoyage retained chaudière (à faire une seule fois, puis commenter)
/*client.publish("home/chaudiere/command", "", true);
client.publish("home/chaudiere/state",   "", true);
client.publish("home/chaudiere/status",  "", true);*/

      #ifdef __LOCAL_MODE__
      // === ABONNEMENTS FORCÉS À CHAQUE CONNEXION ===
      Serial.println("Abonnement à ad200/command");
      client.subscribe(mConfig.chaudiere->mqttSubTopicCommand.c_str());

      Serial.println("Abonnement à " + mConfig.projecteur->mqttSubTopicCommand);
      client.subscribe(mConfig.projecteur->mqttSubTopicCommand.c_str());

      Serial.println("Abonnement à " + mConfig.guirlande->mqttSubTopicCommand);
      client.subscribe(mConfig.guirlande->mqttSubTopicCommand.c_str());

      Serial.println("Abonnement à " + mConfig.chauffageSb->mqttSubTopicCommand);
      client.subscribe(mConfig.chauffageSb->mqttSubTopicCommand.c_str());

      // Publication des états retained
      //client.publish(mConfig.chaudiere->mqttSubTopicStatus.c_str(), "online", false);
      //publishState(currentState);
      #else
      Serial.println("Abonnement à " + mConfig.mRemoteChaudiere->mqttSubTopicCommand);
      client.subscribe(mConfig.mRemoteChaudiere->mqttSubTopicCommand.c_str());
      Serial.println("Abonnement à " + mConfig.mRemoteProjecteur->mqttSubTopicCommand);
      client.subscribe(mConfig.mRemoteProjecteur->mqttSubTopicCommand.c_str());
      Serial.println("Abonnement à " + mConfig.mRemoteGuirlande->mqttSubTopicCommand);
      client.subscribe(mConfig.mRemoteGuirlande->mqttSubTopicCommand.c_str());
      Serial.println("Abonnement à " + mConfig.mRemoteChauffage->mqttSubTopicCommand);
      client.subscribe(mConfig.mRemoteChauffage->mqttSubTopicCommand.c_str());
      #endif
      Serial.println("Abonnement à " + mConfig.topic_config_command);
      client.subscribe(mConfig.topic_config_command.c_str()); //, 1);
      Serial.println("Abonnement à " + mConfig.topic_config_state);
      client.subscribe(mConfig.topic_config_state.c_str()); //, 1);
      Serial.println("Abonnement à " + mConfig.mRemoteThCh1er->mqttSubTopicState);
      client.subscribe(mConfig.mRemoteThCh1er->mqttSubTopicState.c_str());
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
    #ifdef __LOCAL_MODE__
    String topic = mConfig.chaudiere->mqttSubTopicState;
    #else
    String topic = "";
    #endif
    //client.publish(topic.c_str(), state ? "ON" : "OFF", false); KJ
    return 0;
  }

int CMqtt::publish(const char* topic, const char* payload, bool retained) {
  if (!active) return -1;
  String s = String(payload);// + String(" ") + WiFi.localIP().toString();
  if (client.connected()) {
    client.publish(topic, s.c_str(), retained);
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

int CMqtt::publishWithIP(const char* topic, const char* payload, bool retained) {
  if (!active) return -1;
  String s = payload + String(" ") + WiFi.localIP().toString();
  if (client.connected()) {
    client.publish(topic, s.c_str(), retained);
    Serial.printf("MQTT publié (retained=%d) sur %s : %s\n", retained, topic, payload);
  } 
  else {
    Serial.println("MQTT non connecté — publication ignorée");
  }
  return 0;
}


// Garde aussi la version sans retained (pour compatibilité)
int CMqtt::publishWithIP(const char* topic, const char* payload) {
  if (!active) return -1;
  publishWithIP(topic, payload, false);
  return 0;
}
/*void CMqtt::setonEquipementCallback(std::function<int(const String& nom, const String& ip)> onEqu) {
  onEquipement = onEqu;
}*/
void CMqtt::callback(char* topic, byte* payload, unsigned int length) {
  if (!active) return;
  String message, messageOrg;
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  message.trim();
  messageOrg = message;
  message.toUpperCase();

  //Serial.printf("void CMqtt::callback() - reçu : %s :  %s\n", topic, message.c_str());

  #ifdef __LOCAL_MODE__
  if (String(topic) == mConfig.chaudiere->mqttSubTopicCommand) {
    Serial.println("CMqtt::callback() - Chaudière - Topic : " + String(topic) + " - Msg : " + message);
    mConfig.chaudiere->handleMqttCommand(messageOrg);
  } // Projecteur
  else if (String(topic) == mConfig.projecteur->mqttSubTopicCommand) {
    //Serial.println("CMqtt::callback() - Projecteur - Topic : " + String(topic) + " - Msg : " + message);
    mConfig.projecteur->handleMqttCommand(messageOrg);
  } 
  else if (String(topic) == mConfig.guirlande->mqttSubTopicCommand) {
    //Serial.println("CMqtt::callback() - Guirlande - Topic : " + String(topic) + " - Msg : " + message);
    mConfig.guirlande->handleMqttCommand(messageOrg);
  } 
  else if (String(topic) == mConfig.chauffageSb->mqttSubTopicCommand) {
    //Serial.println("CMqtt::callback() - Chauffage - Topic : " + String(topic) + " - Msg : " + message);

    mConfig.chauffageSb->handleMqttCommand(messageOrg);
  } 

  #else // Mettre ici les tests pour équipements remote
  if (String(topic) == mConfig.mRemoteChaudiere->mqttSubTopicCommand) { 
    mConfig.mRemoteChaudiere->handleMqttCommand(messageOrg);
  }
  else if (String(topic) == mConfig.mRemoteProjecteur->mqttSubTopicCommand) { 
    mConfig.mRemoteProjecteur->handleMqttCommand(messageOrg);
  }
  else if (String(topic) == mConfig.mRemoteGuirlande->mqttSubTopicCommand) { 
    mConfig.mRemoteGuirlande->handleMqttCommand(messageOrg);
  }
  else if (String(topic) == mConfig.mRemoteChauffage->mqttSubTopicCommand) { 
    mConfig.mRemoteChauffage->handleMqttCommand(messageOrg);
  }
  /*else if (String(topic) == mConfig.mRemoteThCh1er->mqttSubTopicState) {  // Tous les thermomètres utilisent le même canal que mRemoteThCh1er->mqttSubTopicState...
    if (message.startsWith("THCHRDC")) {                                  // On les différencie par le nom. A améliorer ?
      mConfig.mRemoteThMain->handleMqttState(message);
    }
    
  }*/ 
  #endif
  else if (String(topic) == mConfig.mRemoteThCh1er->mqttSubTopicState) {  // Tous les thermomètres utilisent le même canal que mRemoteThCh1er->mqttSubTopicState... ()"home/thermometre/state")
    if (message.startsWith("THCHRDC")) {                                              // On les différencie par le nom. A améliorer ?
      #ifdef __LOCAL_MODE__
      // On est en local. Le thermomètre correspondant dialogue directement avec i'interface
      #else
      mConfig.mRemoteThMain->handleMqttState(messageOrg);
      #endif
    }

    else if (message.startsWith("THSDB")) {
      mConfig.mRemoteThSdb->handleMqttState(messageOrg);
    }
    else if (message.startsWith("COULSDB")) {
      mConfig.mRemoteCoulSdb->handleMqttState(messageOrg);
    }
    else if (message.startsWith("BATSDB")) {
      mConfig.mRemoteBatSdb->handleMqttState(messageOrg);
    }
    else if (message.startsWith("THCH1ER")) {
      mConfig.mRemoteThCh1er->handleMqttState(messageOrg);
    }
    else if (message.startsWith("BATCH1ER")) {
      mConfig.mRemoteBatThCh1er->handleMqttState(messageOrg);
    }
    else if (message.startsWith("THCAVE")) {
      mConfig.mRemoteThCave->handleMqttState(messageOrg);
    }
    else if (message.startsWith("FLOTTEURNOMADE")) {
      mConfig.mRemoteTor->handleMqttState(messageOrg);
    }
    else if (message.startsWith("BATCAVE")) {
      mConfig.mRemoteBatCave->handleMqttState(messageOrg);
    }
    else if (message.startsWith("THNOMADE")) {
      mConfig.mRemoteThNomade->handleMqttState(messageOrg);
    }
    else if (message.startsWith("BATNOMADE")) {
      mConfig.mRemoteBatNomade->handleMqttState(messageOrg);
    }
    else {
      Serial.printf("CMqtt::callback() message non traité ***%s*** : \n", message.c_str());
    }
    
  } 
  // Traitement des commandes de configuration
  else if (String(topic) == mConfig.topic_config_command) {
    message.trim();  // Supprime espaces début/fin
    message.toUpperCase();
    //mConfig.handleMqttCommand(message);

    Serial.println("Commande configuration reçue : " + message);
  if (message == "STATUS") {
      String statusMsg = "=== ÉTAT Home Assisstant "+ String(mConfig.nomEquipement) +" ===\n";
      statusMsg += String(mConfig.mDateTime->getDate()) + " " + String(mConfig.mDateTime->getTime()) + "\n";
      #ifdef __LOCAL_MODE__
      statusMsg += "Chaudière     : " + String(mConfig.chaudiere->active ? "ACTIVE" : "INACTIVE") + " - "  + mConfig.chaudiere->etatStr + "\n";
      statusMsg += "Keep-alive    : " + String(mConfig.chaudiere->enableKeepAlive ? "ACTIVÉ" : "DÉSACtivé") + "\n";
      statusMsg += "Projecteur    : " + String(mConfig.projecteur->active ? "ACTIF" : "INACTIF") + " - " + mConfig.projecteur->etatStr + "\n";
      statusMsg += "Guirlande     : " + String(mConfig.guirlande->active ? "ACTIVE" : "INACTIVE") + " - " + mConfig.guirlande->etatStr + "\n";
      statusMsg += "Salle de bain : " + String(mConfig.chauffageSb->active ? "ACTIF" : "INACTIF") + " - " + mConfig.chauffageSb->etatStr + "\n";
      #else // Chaudière, thermomètre principal et équipements 433 MHz
      statusMsg += "Chaudière     : " + String(mConfig.mRemoteChaudiere->active ? "ACTIVE" : "INACTIVE") + " - "  + mConfig.mRemoteChaudiere->etatStr + "\n";
      //statusMsg += "Keep-alive    : " + String(mConfig.mRemoteChaudiere->enableKeepAlive ? "ACTIVÉ" : "DÉSACtivé") + "\n";
      statusMsg += "Projecteur    : " + String(mConfig.mRemoteProjecteur->active ? "ACTIF" : "INACTIF") + " - " + mConfig.mRemoteProjecteur->etatStr + "\n";
      statusMsg += "Guirlande     : " + String(mConfig.mRemoteGuirlande->active ? "ACTIVE" : "INACTIVE") + " - " + mConfig.mRemoteGuirlande->etatStr + "\n";
      statusMsg += "Salle de bain : " + String(mConfig.mRemoteChauffage->active ? "ACTIF" : "INACTIF") + " - " + mConfig.mRemoteChauffage->etatStr + "\n";
      #endif
      String statusMsgBis = "=== ÉTAT Home Assisstant "+ String(mConfig.nomEquipement) +" (Suite) ===\n";
      #ifdef __CYD__
      statusMsg += "Veille écran  : " + String(mEcran.sleep_timeout > 0 ? String(mEcran.sleep_timeout) + "s" : "Désactivée") + "\n";
      #endif
      statusMsg += "IP     : " + WiFi.localIP().toString() + "\n";
      statusMsg += "Uptime : " + String(millis() / 1000) + "s";

      std::vector<CIPModule> *mvsControleurs = mConfig.mvsControleurs;
      if (mvsControleurs != nullptr) {
        statusMsg += "\n=== Contrôleurs et capteurs connus ===\n";
        for (const auto& ctrl : *mvsControleurs) {
            String sCtrl = ctrl.getNom();
            String sIp = ctrl.getIP();
            statusMsg += sCtrl + " ==> " + sIp + "\n";
        }
      }


      publish(mConfig.topic_config_state.c_str(), statusMsg.c_str(), false);
      Serial.println("STATUS publié sur " + mConfig.topic_config_state + " :");
      Serial.println(statusMsg);
      Serial.printf("Taille : %d\n", statusMsg.length());

    }
    else if (message.startsWith("VEILLE ")) {
      #ifdef __CYD__
      int32_t veilleTimeout = (int32_t)message.substring(6).toDouble();
      mEcran.setSleepTimeout(veilleTimeout);
      #endif
    }
    else if (message == "ECRAN SLEEP") {
      #ifdef __CYD__
      mEcran.forceSleep();  
      #endif
    }
    else if (message == "ECRAN WAKE") {
      #ifdef __CYD__
      mEcran.forceWake();
      #endif
    }
    else if (message.startsWith("ECRAN SERIE ")) {
      #ifdef __CYD__
      int serie = message.substring(12).toInt();
      mEcran.setSerieAffichageEnCours(serie);
      #endif
    }
    else if (message.startsWith("ECRAN ")) {
      #ifdef __CYD__
      int rotation = message.substring(6).toInt();
      mEcran.setOrientationEcran(rotation);
      #endif
    }
    else if (message.startsWith("DEVICE ")) { // Un nouveau module (CYD) vient d'apparaître sur le réseau, si on est en __LOCAL_MODE__, on lui envoie les états des appareils
      #ifdef __LOCAL_MODE__
      mConfig.chaudiere->remonteStatusParMqtt();
      mConfig.projecteur->remonteStatusParMqtt();
      mConfig.guirlande->remonteStatusParMqtt();
      mConfig.chauffageSb->remonteStatusParMqtt();

      mConfig.ds18b20->remonteStatusParMqtt();

      mConfig.mRemoteThCh1er->remonteStatusParMqtt();
      mConfig.mRemoteBatThCh1er->remonteStatusParMqtt();

      mConfig.mRemoteThCave->remonteStatusParMqtt();
      mConfig.mRemoteBatCave->remonteStatusParMqtt();

      mConfig.mRemoteThSdb->remonteStatusParMqtt();
      mConfig.mRemoteCoulSdb->remonteStatusParMqtt();
      mConfig.mRemoteBatSdb->remonteStatusParMqtt();

      mConfig.mRemoteThNomade->remonteStatusParMqtt();
      mConfig.mRemoteBatNomade->remonteStatusParMqtt();
      mConfig.mRemoteTor->remonteStatusParMqtt();

      mConfig.remonteCYDInfoToEcran(messageOrg);
      #endif
    }
    else if (message == "REBOOT") {
      ESP.restart();
    }
    // Tu pourras ajouter d’autres commandes plus tard
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
