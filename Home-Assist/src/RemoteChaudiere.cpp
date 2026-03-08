#include "MyConfig.h"
#include "MyEcran.h"
#include "RemoteChaudiere.h"



void CRemoteChaudiere::setup_envoi_trames() {
    // Aucune transmission n'est en cours
    mbEnvoyerTramesON = false; 
    mbEnvoyerTramesOFF = false;
    // Obsolète
    miCompteurEnvois = 0;
    // Keep alive
    mbKeepAliveActive = false;  // Ce paramètre sera ajusté après envoi de la première trame ON ou OFF
    mbTrameCommandeEnvoyee = false;

    // On regarde l'état de la chaudière dans la config
    // car s'il y a eu un redémarrage inoppiné de notre
    // microcontrolleur, il faudra repartir du dernier
    // état
    if (etat) { // ON
      mbEnvoyerTramesON = true; // On demande l'envoi d'une trame ON
      mbEnvoyerTramesOFF = false;
      mbTrameCommandeEnvoyee = false; // La trame de commande n'a pas encore été envoyée
      muiKeepAliveIndexEnCours = 0; // Pour les keep-alive, on recommence à 0
      mbKeepAliveActive = false;  // Ce paramètre sera ajusté après envoi de la première trame ON ou OFF
    }
    else {
      mbEnvoyerTramesON = false;
      mbEnvoyerTramesOFF = true;
      mbTrameCommandeEnvoyee = false;
      muiKeepAliveIndexEnCours = 0;
      mbKeepAliveActive = false;
    }      
}

void CRemoteChaudiere::setup() {
  nomEquipement = "AD200Device";
  mqttSubTopic = "chaudiere/";
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  // Doit être fait après le chargement des paramètres
  setup_envoi_trames();
}

void CRemoteChaudiere::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }


//-------------------------------------------------------------------------------- 
// Bouton ON appuyé
// Retour
//  0 : RAS (chaudière active, aucun envoi en cours, on envoie une trame ON)
//  -1 : Chadière inactive, on ne fait rien
//  -2 : L'envoi est en cours (trame initiale ou de keep alive), on ne fait rien
//    
//-------------------------------------------------------------------------------- 
int CRemoteChaudiere::envoiTrameON() {
    int ret = 0;
    // Mettre un délai avant de pouvoir arrêter l'envoi du OFF 
    // après demande OFF
    if (active) {
      bool b = bGetEnvoiEnCours();
      Serial.printf("main loop - ON appuye - Envoi en cours : % d\n", b);
      bSetEnvoyerTramesOFF(false);
      if (bGetEnvoiEnCours()) {
        ret = -2;
      }
      else {
        bSetEnvoyerTramesON(true);
        saveState(true);
        ret = 0;
      }
    }
      else {
      ret = -1;
    }
    return ret;
} 
//-------------------------------------------------------------------------------- 
// Bouton OF appuyé
// Retour
//  0 : RAS (chaudière active, aucun envoi en cours, on envoie une trame ON)
//  -1 : Chadière inactive, on ne fait rien
//  -2 : L'envoi est en cours (trame initiale ou de keep alive), on ne fait rien
//    
//-------------------------------------------------------------------------------- 
int CRemoteChaudiere::envoiTrameOFF() {
  int ret = 0;
  // Mettre un délai avant de pouvoir arrêter l'envoi du ON 
  // après demande ON
  if (active) {
    bool b = bGetEnvoiEnCours();
    Serial.printf("main loop - OFF appuye - Envoi en cours : % d\n", b);
    bSetEnvoyerTramesON(false);
    if (bGetEnvoiEnCours()) {
      ret = -2;
    }
    else {
      bSetEnvoyerTramesOFF(true);
      saveState(false);
      ret = 0;
    }
  }
    else {
    ret = -1;
  }
  return ret;
}

bool CRemoteChaudiere::bSetEnvoyerTramesON(bool b) {
    bool bLastVal = mbEnvoyerTramesON;
    mbEnvoyerTramesON = b;
    mbTrameCommandeEnvoyee = false;
    mbKeepAliveActive = false;
    muiKeepAliveIndexEnCours = 0;
    return bLastVal;
}
bool CRemoteChaudiere::bSetEnvoyerTramesOFF(bool b) {
    bool bLastVal = mbEnvoyerTramesOFF;
    mbEnvoyerTramesOFF = b;
    mbTrameCommandeEnvoyee = false;
    mbKeepAliveActive = false;
    muiKeepAliveIndexEnCours = 0;
    return bLastVal;
}
bool CRemoteChaudiere::bGetEnvoiEnCours(void) { 
  return mbEnvoyerTramesON || mbEnvoyerTramesOFF;
}

int CRemoteChaudiere::allumer() {
  int ret = onMqttPublish(mqttSubTopicCommand.c_str(),"ON");
  if (ret == 0) { // RAS
    saveState(true);

    mEcran->updateAllStates();
    }

    return ret;
}

int CRemoteChaudiere::eteindre() {
  int ret = onMqttPublish(mqttSubTopicCommand.c_str(),"OFF");
  String sEq = nomEquipement;
  if (ret == 0) { // RAS
    saveState(false);

    mEcran->updateAllStates();
    }

    return ret;
}
int CRemoteChaudiere::activeEquipement() {
  int ret = 0;
  setActive(!active);
  ret = onMqttPublish(mqttSubTopicCommand.c_str(), active ? "ENABLE" : "DISABLE");

  return ret;
}

void CRemoteChaudiere::envoiCommande(const uint16_t* pulses, int nbPulses, const char* action) {
  if (mEcran != nullptr) mEcran->updateStatus(String("Emission ") + action + "...");
  if (mEcran != nullptr) mEcran->updateStatus(String("Emission ") + action + " terminee");
}

void CRemoteChaudiere::loop() {
}

void CRemoteChaudiere::loadFromNVS() {
    prefs.begin(nvs_namespace, true);

    domotique_prefix = prefs.getString("domo_prefix", default_domotique_topic_prefix);
  // Chaudiere
  nomEquipement = prefs.getString("chaud_nom", "XXX");
  nomBoutonON = prefs.getString("chaud_nameBtnON", "Ch. ON");
  nomBoutonOFF = prefs.getString("chaud_nameBtnOF", "Ch. OFF");

  mqttSubTopic = prefs.getString("topic_prefix", default_topic_prefix);

  // On forme les subtopic MQTT pour la chaudière*/
  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";
  mqttSubTopicStatus = domotique_prefix + mqttSubTopic + "status"; //mqqtInfo.subtopic_status;


  // État chaudière
  etat = prefs.getBool("boiler_state", default_boiler_state);
  etatStr = etat ? "ON" : "OFF";


  prefs.end();
}

void CRemoteChaudiere::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  prefs.putString("chaud_nom", nomEquipement);
  prefs.putString("chaud_nameBtnON", nomBoutonON);
  prefs.putString("chaud_nameBtnOF", nomBoutonOFF);
  prefs.putBool("boiler_state", etat);

  prefs.putString("topic_prefix", mqttSubTopic);//topic_prefix);

  prefs.end();
}

void CRemoteChaudiere::print() const {
  Serial.println("[CHAUDIERE]");
  Serial.println("  [GENERAL]");
  Serial.printf("     Nom            : %s\n", nomEquipement);
  Serial.printf("     Actif          : %s\n", active ? "OUI" : "NON");
  Serial.printf("     Nom btn ON     : %s\n", nomBoutonON.c_str());
  Serial.printf("     Nom btn OFF    : %s\n", nomBoutonOFF.c_str());

  Serial.println("  [MQTT]==");
  Serial.printf("     MQTTSubTopic   : %s\n", mqttSubTopic.c_str());

  Serial.printf("     Command        : %s\n", mqttSubTopicCommand.c_str());
  Serial.printf("     State          : %s\n", mqttSubTopicState.c_str());
  Serial.printf("     Status         : %s\n", mqttSubTopicStatus.c_str());
  
  Serial.println("  [État chaudière]");
  Serial.printf("     Dernier état   : %s\n", etatStr.c_str());

}

void CRemoteChaudiere::saveState(bool state) {
  CEquipementBase::saveState(state);
  etatStr = state ? "ON" : "OFF";
}

void CRemoteChaudiere::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
        
  Serial.printf("void CRemoteChaudiere::handleMqttCommand() - reçu : %s\n", cmd.c_str());

  if (cmd == "ONR") {
    bool state = true;
    if (active) {    
      saveState(state); // Ajout 2.0
      //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      if (mEcran != nullptr) mEcran->updateAllStates();
    }
    else {
      String s = "L'equipement " + String(nomEquipement) + " n'est pas actif";
      if (mEcran != nullptr) mEcran->updateStatus(s);
      //Serial.println(s);
    }

  }
  else if (cmd == "OFFR") {
    bool state = false;
    if (active) {    
      saveState(state); // Ajout 2.0
      //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      if (mEcran != nullptr) mEcran->updateAllStates();
    }
    else {
      String s = "L'equipement " + String(nomEquipement) + " n'est pas actif";
      if (mEcran != nullptr) mEcran->updateStatus(s);
      //Serial.println(s);
    }

  }
  else if (cmd == "ENABLER") {
    setActive(true);
    String s = nomEquipement + " : active";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    //Serial.println(s);
  }
  else if (cmd == "DISABLER") {
    setActive(false);
    String s = nomEquipement + " : désactivee";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    //Serial.println(s);
  }
}
    
void CRemoteChaudiere::loadFromWebServer (WebServer& server) {
  if (server.hasArg("chaud_nom")) nomEquipement = server.arg("chaud_nom");
  if (server.hasArg("chaud_active")) active = true; else active = false;
  if (server.hasArg("topic_prefix")) mqttSubTopic = server.arg("topic_prefix");

  if (server.hasArg("chaud_nameBtnON")) nomBoutonON = server.arg("chaud_nameBtnON");
  if (server.hasArg("chaud_nameBtnOF")) nomBoutonOFF = server.arg("chaud_nameBtnOF");

}

String CRemoteChaudiere::getHTML() {
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
      "</div>";
  html += "</div>";

  return html;
}


