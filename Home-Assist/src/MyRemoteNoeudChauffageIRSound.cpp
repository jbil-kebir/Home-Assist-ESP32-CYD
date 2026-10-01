#include "MyRCDevice.h"
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyRemoteNoeudChauffageIRSound.h"

//----------------------------------------------------------------------------------
// CRemoteNoeudChauffageIRSound::setEtatReelOnOff()
//
// Etat réel du chauffage, appelé depuis CEcran
//  Eteint : 0
//  Allumé : 1
// Pas de publication MQTT : c'est le CYD principal (__LOCAL_MODE__) qui publie
// ONR/OFFR (sinon risque de boucle MQTT, cf. CRemoteChaudiere::setEtatReelOnOff())
//----------------------------------------------------------------------------------
void CRemoteNoeudChauffageIRSound::setEtatReelOnOff(bool state) {
  DBG(DBG_ACTIONNEURS, "void CRemoteNoeudChauffageIRSound::setEtatReelOnOff() - state=%d - etat=%d\n", state, etat);
  if (etat == state) return;

  saveState(state); // Sauvegarde NVS
  if (mEcran != nullptr)
    mEcran->updateAllStates();
}



/*void CRemoteNoeudChauffageIRSound::setup() {
  // On charge les infos de config depuis le NVS
  loadFromNVS();

}
*/
/*int CRemoteNoeudChauffageIRSound::activeEquipement() {
  int ret = 0;
  //setActive(!active);
  //ret = onMqttPublish(mqttSubTopicCommand.c_str(), active ? "ENABLER" : "DISABLER");

  return ret;
}*/

/*void CRemoteNoeudChauffageIRSound::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }*/
/*void CRemoteNoeudChauffageIRSound::loop() {
  //loop_envoi_trames();        
}*/

/*void CRemoteNoeudChauffageIRSound::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  domotique_prefix = prefs.getString("domo_prefix", default_domotique_topic_prefix);
  // Chaudiere
  nomEquipement = prefs.getString("chaud_nom", "XXX");
  //nomBoutonON = prefs.getString("chaud_nameBtnON", "Ch. ON");
  //nomBoutonOFF = prefs.getString("chaud_nameBtnOF", "Ch. OFF");
  frequency = prefs.getFloat("chaud_freq", 433.92);
  active = prefs.getBool("chaud_active", false);

  mqttSubTopic = prefs.getString("topic_prefix", default_topic_prefix);

  // On forme les subtopic MQTT pour la chaudière
  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";
  mqttSubTopicStatus = domotique_prefix + mqttSubTopic + "status"; //mqqtInfo.subtopic_status;

  // État chaudière
  etat = prefs.getBool("boiler_state", default_boiler_state);
  etatStr = etat ? "ON" : "OFF";


  prefs.end();
}*/

/*void CRemoteNoeudChauffageIRSound::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  prefs.putString("chaud_nom", nomEquipement);
  prefs.putString("chaud_nameBtnON", nomBoutonON);
  prefs.putString("chaud_nameBtnOF", nomBoutonOFF);
  prefs.putFloat("chaud_freq", frequency);
  prefs.putBool("chaud_active", active);
  prefs.putBool("boiler_state", etat);

  prefs.putString("topic_prefix", mqttSubTopic);//topic_prefix);
  // Keep alive flag
  prefs.putBool("en_keep_alive", enableKeepAlive);

  

  prefs.end();
}*/

/*void CRemoteNoeudChauffageIRSound::print() const {
  DBG(DBG_CHAUDIERE, "[CHAUDIERE]\n");
  DBG(DBG_CHAUDIERE, "  [GENERAL]\n");
  DBG(DBG_CHAUDIERE, "     Nom            : %s\n", nomEquipement.c_str());
  DBG(DBG_CHAUDIERE, "     Actif          : %s\n", active ? "OUI" : "NON");
  DBG(DBG_CHAUDIERE, "     Nom btn ON     : %s\n", nomBoutonON.c_str());
  DBG(DBG_CHAUDIERE, "     Nom btn OFF    : %s\n", nomBoutonOFF.c_str());

  DBG(DBG_CHAUDIERE, "  [Protocole]\n");
  DBG(DBG_CHAUDIERE, "     Fréquence      : %f\n", frequency);

  DBG(DBG_CHAUDIERE, "  [MQTT]==\n");
  DBG(DBG_CHAUDIERE, "     MQTTSubTopic   : %s\n", mqttSubTopic.c_str());
  DBG(DBG_CHAUDIERE, "     Topic prefix   : %s\n", mqttSubTopic.c_str());//topic_prefix.c_str());
  DBG(DBG_CHAUDIERE, "     Command        : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_CHAUDIERE, "     State          : %s\n", mqttSubTopicState.c_str());
  DBG(DBG_CHAUDIERE, "     Status         : %s\n", mqttSubTopicStatus.c_str());

  DBG(DBG_CHAUDIERE, "  [État chaudière]\n");
  DBG(DBG_CHAUDIERE, "     Dernier état   : %s\n", etatStr.c_str());



}*/

/*void CRemoteNoeudChauffageIRSound::saveState(bool state) {
  if (etat == state) return;

  prefs.begin(nvs_namespace, false);
  prefs.putBool("boiler_state", state);
  prefs.end();

  etat = state;
  etatStr = state ? "ON" : "OFF";

  prefs.end();
}*/
/*void CRemoteNoeudChauffageIRSound::setActive(bool state) {
  if (active == state) return;

  prefs.begin(nvs_namespace, false);
  prefs.putBool("chaud_active", state);
  prefs.end();

  active = state;
}*/

// Envoie l'état actif/inactif ainsi que ON/OFF
/*int CRemoteNoeudChauffageIRSound::remonteStatusParMqtt() {
  int ret = 0;
  DBG(DBG_CHAUDIERE, "CRemoteNoeudChauffageIRSound::remonteStatusParMqtt() - Envoi %s sur %s\n", this->active ? "ENABLER" : "DISABLER", this->mqttSubTopicCommand.c_str());
  onMqttPublish(this->mqttSubTopicCommand.c_str(), this->active ? "ENABLER" : "DISABLER");
  delay(200);
  DBG(DBG_CHAUDIERE, "CRemoteNoeudChauffageIRSound::remonteStatusParMqtt() - Envoi %s sur %s\n", this->etat ? "ONR" : "OFFR", this->mqttSubTopicCommand.c_str());
  onMqttPublish(this->mqttSubTopicCommand.c_str(), this->etat ? "ONR" : "OFFR");
  delay(200);
  return ret;
} */

//
// Etat réel de la chaudière
// Vient de l'observatoion de la DEL rouge
//  Eteint : 0
//  Allulmée : 1
//
/*void CRemoteNoeudChauffageIRSound::setEtatReelOnOff(bool state) {
  // On prévient les appareils distants
  //Serial.printf("void CRemoteNoeudChauffageIRSound::setEtatReelOnOff() - state=%d - etat=%d\n", state, etat);
  if (etat == state) return;
  String sEnvoi = state ? "ONR" : "OFFR";
  if (onMqttPublish != nullptr)
    onMqttPublish(this->mqttSubTopicCommand.c_str(), sEnvoi.c_str());

  saveState(state); // Sauvegarde NVS
  if (mEcran != nullptr) 
    mEcran->updateAllStates();

  // Si on a reçu OFF (state = false), et si bForcerDisable est à true, alors on remet l'équipement dans un état inactif (forcé)
  if (!state && mbForcerDisable) {
    setActive(false);
    if (onMqttPublish != nullptr)
      onMqttPublish(this->mqttSubTopicCommand.c_str(), "DISABLER");
    String s = nomEquipement + " : désactivee (force OFF)";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    DBGLN(DBG_CHAUDIERE, s);
    mbForcerDisable = false; // Réinitialisation du flag après utilisation
  }

}*/

/*void CRemoteNoeudChauffageIRSound::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
        
  // ON_FORCE et OFF_FORCE permettent de forcer ON ou OFF même si la chaudière est désactivée.
  if (cmd == "ON" || cmd == "OFF" || cmd == "ON_FORCE" || cmd == "OFF_FORCE") {
    bool state = (cmd == "ON" || cmd == "ON_FORCE") ? true : false; // 
    bool bForceEnable = (cmd == "ON_FORCE") ? true : false;
    bool bForceDisable = (cmd == "OFF_FORCE") ? true : false;
    bool mbOldActif = active; // Sauvegarde de l'état actif avant changement
    // Si c'est une commande ON ou OFF forcée, on active l'équipement même s'il est désactivé
    if (bForceEnable || bForceDisable) {
      setActive(true);
      if (onMqttPublish != nullptr)
        onMqttPublish(this->mqttSubTopicCommand.c_str(), "ENABLER");
      String s = nomEquipement + " : active (force ON)";
      if (mEcran != nullptr) mEcran->updateStatus(s, true);
      DBGLN(DBG_CHAUDIERE, s);
    }
    if (active) {    
      saveState(state); // Ajout 2.0
      if (state) { // On doit allumer la chaudière

        bSetEnvoyerTramesOFF(false);
        bSetEnvoyerTramesON(true);
        if (onMqttPublish != nullptr)
          onMqttPublish(this->mqttSubTopicCommand.c_str(), "ONR");
        //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      }
      else { // On doit éteindre la chaudière
        bSetEnvoyerTramesON(false);
        bSetEnvoyerTramesOFF(true);
        if (onMqttPublish != nullptr)
          onMqttPublish(this->mqttSubTopicCommand.c_str(), "OFFR");
        //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      }
      saveState(state);
      if (mEcran != nullptr) mEcran->updateAllStates();
      // Si l'équipement était inactif avant un OFF forcé, il faudra le remettre dans l'atat inactif au retour du résultat du OFF
      if (bForceDisable) {
        mbForcerDisable=true;
      }
    } // if (active) {
    else {
      String s = "L'equipement " + String(nomEquipement) + " n'est pas actif";
      if (mEcran != nullptr) mEcran->updateStatus(s);
      DBGLN(DBG_CHAUDIERE, s);
    }

  }
  else if (cmd == "ENABLE") {
    setActive(true);
    if (onMqttPublish != nullptr)
      onMqttPublish(this->mqttSubTopicCommand.c_str(), "ENABLER");
    String s = nomEquipement + " : active";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    DBGLN(DBG_CHAUDIERE, s);
  }
  else if (cmd == "DISABLE") {
    setActive(false);
    if (onMqttPublish != nullptr)
      onMqttPublish(this->mqttSubTopicCommand.c_str(), "DISABLER");
    String s = nomEquipement + " : désactivee";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    DBGLN(DBG_CHAUDIERE, s);
  }
}
*/
/*void CRemoteNoeudChauffageIRSound::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
        
  if (cmd == "ON" || cmd == "OFF") {
    bool state = cmd == "ON" ? true : false;
    if (active) {    
      saveState(state); // Ajout 2.0
      if (state) {

        bSetEnvoyerTramesOFF(false);
        bSetEnvoyerTramesON(true);
        if (onMqttPublish != nullptr)
          onMqttPublish(this->mqttSubTopicCommand.c_str(), "ONR");
        //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      }
      else {
        bSetEnvoyerTramesON(false);
        bSetEnvoyerTramesOFF(true);
        if (onMqttPublish != nullptr)
          onMqttPublish(this->mqttSubTopicCommand.c_str(), "OFFR");
        //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      }
      saveState(state);
      if (mEcran != nullptr) mEcran->updateAllStates();
    }
    else {
      String s = "L'equipement " + String(nomEquipement) + " n'est pas actif";
      if (mEcran != nullptr) mEcran->updateStatus(s);
      Serial.println(s);
    }

  }
  else if (cmd == "ENABLE") {
    setActive(true);
    onMqttPublish(this->mqttSubTopicCommand.c_str(), "ENABLER");
    String s = nomEquipement + " : active";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    Serial.println(s);
  }
  else if (cmd == "DISABLE") {
    setActive(false);
    onMqttPublish(this->mqttSubTopicCommand.c_str(), "DISABLER");
    String s = nomEquipement + " : désactivee";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    Serial.println(s);
  }
}*/
   

/*void CRemoteNoeudChauffageIRSound::loadFromWebServer (WebServer& server) {
  if (server.hasArg("chaud_nom")) nomEquipement = server.arg("chaud_nom");
  if (server.hasArg("chaud_active")) active = true; else active = false;
  if (server.hasArg("en_keep_alive")) enableKeepAlive = true; else enableKeepAlive = false;//server.arg("en_keep_alive").toInt();
  if (server.hasArg("chaud_nameBtnON")) nomBoutonON = server.arg("chaud_nameBtnON");
  if (server.hasArg("chaud_nameBtnOF")) nomBoutonOFF = server.arg("chaud_nameBtnOF");
  if (server.hasArg("chaud_freq")) frequency = server.arg("chaud_freq").toFloat();

  // Lecture des 10 timings ON
  for (uint8_t i = 0; i < 10; i++) {
    String arg_name = "on_keep_" + String(i);
    if (server.hasArg(arg_name)) {
      onKeepalive[i] = server.arg(arg_name).toInt();
    }
  }

  // Lecture des 10 timings OFF
  for (uint8_t i = 0; i < 10; i++) {
    String arg_name = "off_keep_" + String(i);
    if (server.hasArg(arg_name)) {
      offKeepalive[i] = server.arg(arg_name).toInt();
    }
  }
}*/

/*String CRemoteNoeudChauffageIRSound::getHTML() {
  String html = "";
  html = "<h2>Configuration de la chaudière</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=\"chaud_nom\" value=\"" + nomEquipement + "\"></div>"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=\"topic_prefix\" value=\"" + mqttSubTopic + "\"></div>"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=\"chaud_active\" value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Nom bouton ON</label><input type=\"text\" name=\"chaud_nameBtnON\" value=\"" + nomBoutonON + "\"></div>"
        "<div><label>Nom bouton OFF</label><input type=\"text\" name=\"chaud_nameBtnOF\" value=\"" + nomBoutonOFF + "\"></div>"
      "</div>"
      "<label>Fréquence (MHz)</label><input type=\"number\" step=\"0.01\" name=\"chaud_freq\" value=\"" + String(frequency, 2) + "\">"

      // === KEEP-ALIVE ===
      "<h2>Keep-Alive Chaudière</h2>"
      "<div class=\"checkbox-row\"><label>Activer le keep-alive</label><input type=\"checkbox\" name=\"en_keep_alive\" value=\"1\"" + String(enableKeepAlive ? " checked" : "") + "></div>"
      "<h3>Timings ON (secondes)</h3><div class=\"timing-grid\">";
  for (uint8_t i = 0; i < 10; i++) {
    html += "<input type=\"number\" name=\"on_keep_" + String(i) + "\" value=\"" + String(onKeepalive[i]) + "\">";
  }
  html += "</div><h3>Timings OFF (secondes)</h3><div class=\"timing-grid\">";
  for (uint8_t i = 0; i < 10; i++) {
    html += "<input type=\"number\" name=\"off_keep_" + String(i) + "\" value=\"" + String(offKeepalive[i]) + "\">";
  }
  html += "</div>";

  return html;
}*/


