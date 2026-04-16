#include "MyRCDevice.h"
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyChaudiere.h"

// === VALEURS MOYENNES (µs) ===
#define A 210,205
#define B 210,453
#define C 449,205
#define D 449,453
#define LAST_PULSE_HIGH 449

// === TRAMES ===
#define NB_CARACTERES_ON_1 73
#define TAILLE_BUFFER_ON_1 (2 * NB_CARACTERES_ON_1 + 1)
const uint16_t pulseBufferON[TAILLE_BUFFER_ON_1] = {
  A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
  D,A,D,A,A,A,A,A,B,A,A,C,A,B,A,D,
  C,A,B,A,C,B,A,C,A,A,A,A,A,
  A,A,A,A,A,A,A,
  B,C,A,B,C,A,A,A,A,A,A,A,
  B,C,A,B,C,A,A,A,B,
  LAST_PULSE_HIGH
};

#define NB_CARACTERES_ACK_ON 73
#define TAILLE_BUFFER_ACK_ON (2 * NB_CARACTERES_ACK_ON + 1)
const uint16_t pulseBufferACK_ON[TAILLE_BUFFER_ACK_ON] = {
  A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
  D,A,D,A,A,A,A,A,B,A,A,C,A,B,A,D,
  C,A,B,A,C,B,A,C,A,A,A,A,A,
  B,C,A,A,A,A,
  B,C,A,B,C,A,A,A,A,A,A,A,
  B,C,A,B,C,A,A,A,A,B,
  LAST_PULSE_HIGH
};

#define NB_CARACTERES_OFF_1 74
#define TAILLE_BUFFER_OFF_1 (2 * NB_CARACTERES_OFF_1 + 1)
const uint16_t pulseBufferOFF[TAILLE_BUFFER_OFF_1] = {
  A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
  D,A,D,A,A,A,A,A,B,A,A,C,A,B,A,D,
  C,A,B,A,C,B,A,C,A,A,A,A,A,
  A,A,A,A,A,A,A,
  B,C,A,B,C,A,A,A,A,A,A,A,
  B,A,C,A,B,A,A,A,A,A,
  LAST_PULSE_HIGH
};

#define NB_CARACTERES_ACK_OFF 74
#define TAILLE_BUFFER_ACK_OFF (2 * NB_CARACTERES_ACK_OFF + 1)
const uint16_t pulseBufferACK_OFF[TAILLE_BUFFER_ACK_OFF] = {
  A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
  D,A,D,A,A,A,A,A,B,A,A,C,A,B,A,D,
  C,A,B,A,C,B,A,C,A,A,A,A,A,
  B,C,A,A,A,A,
  B,C,A,B,C,A,A,A,A,A,A,A,
  B,A,C,A,B,A,A,A,A,D,
  LAST_PULSE_HIGH
};

void CChaudiere::setup_envoi_trames() {
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

void CChaudiere::setup() {
  nomEquipement = "AD200Device";
  mqttSubTopic = "chaudiere/";
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  // Doit être fait après le chargement des paramètres
  setup_envoi_trames();
}

void CChaudiere::loop_envoi_trames() {
  static unsigned long delaiAvantEnvoi = millis();
    static unsigned long lastKeepAliveTime = 0;
    if (active) {
      if (  mbEnvoyerTramesON && // Si envoi trame ON demandé
            !mbTrameCommandeEnvoyee // et pas encore envoyée
      ) {
        DBG(DBG_CHAUDIERE, "Demande ON et pas encore envoyee...\n");
        // On envoi la trame ON
        envoiCommande(pulseBufferON, TAILLE_BUFFER_ON_1, "Commande ON");
        mbTrameCommandeEnvoyee = true; // On signale que la première trame a été envoyée
        DBG(DBG_CHAUDIERE, "\t...Demande ON envoyee\n");
        // Activation keep-alive après première trame ON
        mbKeepAliveActive = enableKeepAlive == 0 ? false : true;
        muiKeepAliveIndexEnCours = 0;
        lastKeepAliveTime = millis();
      }
      else if ( mbEnvoyerTramesOFF && // Si envoi trame OFF demandé
                !mbTrameCommandeEnvoyee // et pas encore envoyée
      ) {
        DBG(DBG_CHAUDIERE, "Demande OFF et pas encore envoyee\n");
        // On envoi la trame ON
        envoiCommande(pulseBufferOFF, TAILLE_BUFFER_OFF_1, "Commande OFF");
        mbTrameCommandeEnvoyee = true; // On signale que la première trame a été envoyée
        DBG(DBG_CHAUDIERE, "\t...Demande OFF envoyee\n");
        // Activation keep-alive après première trame OFF
        mbKeepAliveActive = enableKeepAlive == 0 ? false : true;
        muiKeepAliveIndexEnCours = 0;
        lastKeepAliveTime = millis();
      }
      else if (  mbEnvoyerTramesON && // Si envoi trame ON demandé
            mbTrameCommandeEnvoyee && // et trame déjà envoyée
            mbKeepAliveActive   // et Keep active activé
      ) {
        if (millis() - lastKeepAliveTime > onKeepalive[muiKeepAliveIndexEnCours] * 1000UL) { // S'il est temps d'envoyer un keep alive
          DBG(DBG_CHAUDIERE, "Demande ON deja envoye - Keep alive actif\n");
          String s = "Keep-Alive ON " + String(onKeepalive[muiKeepAliveIndexEnCours]) + "s";
          DBG(DBG_CHAUDIERE, "..Envoi ON Keep-Alive : %d - %s\n", muiKeepAliveIndexEnCours, s.c_str());
          envoiCommande(pulseBufferON, TAILLE_BUFFER_ACK_ON, s.c_str());
          muiKeepAliveIndexEnCours++;
          if (muiKeepAliveIndexEnCours + 1 >= on_keepalive_count) { // Si le dernier Keppe alive du tableau a été envoyé, on le renvoit
            muiKeepAliveIndexEnCours = on_keepalive_count - 1;  // Répète le dernier (626s)
          }
          lastKeepAliveTime = millis();

        }
      }
      else if (  mbEnvoyerTramesOFF && // Si envoi trame OFF demandé
            mbTrameCommandeEnvoyee && // et trame déjà envoyée
            mbKeepAliveActive   // et Keep active activé
      ) {
        if (millis() - lastKeepAliveTime > offKeepalive[muiKeepAliveIndexEnCours] * 1000UL) { // S'il est temps d'envoyer un keep alive
          DBG(DBG_CHAUDIERE, "Demande OFF deja envoye - Keep alive actif : %d\n", offKeepalive[muiKeepAliveIndexEnCours]);
          String s = "Keep-Alive OFF " + String(offKeepalive[muiKeepAliveIndexEnCours]) + "s";
          DBG(DBG_CHAUDIERE, "..Envoi OFF Keep-Alive : %d - %s\n", muiKeepAliveIndexEnCours, s.c_str());
          envoiCommande(pulseBufferOFF, TAILLE_BUFFER_ACK_OFF, s.c_str());
          muiKeepAliveIndexEnCours++;
          if (muiKeepAliveIndexEnCours + 1 >= off_keepalive_count) { // Si le dernier Keep alive du tableau a été envoyé, on le renvoit
            muiKeepAliveIndexEnCours = off_keepalive_count - 1;  // Répète le dernier (626s)
          }
          lastKeepAliveTime = millis();

        }

      }
    }    
}
//-------------------------------------------------------------------------------- 
// Bouton ON appuyé
// Retour
//  0 : RAS (chaudière active, aucun envoi en cours, on envoie une trame ON)
//  -1 : Chaudière inactive, on ne fait rien
//  -2 : L'envoi est en cours (trame initiale ou de keep alive), on ne fait rien
//    
//-------------------------------------------------------------------------------- 
int CChaudiere::envoiTrameON() {
    int ret = 0;
    // Mettre un délai avant de pouvoir arrêter l'envoi du OFF 
    // après demande OFF
    if (active) {
      bool b = bGetEnvoiEnCours();
      DBG(DBG_CHAUDIERE, "main loop - ON appuye - Envoi en cours : % d\n", b);
      bSetEnvoyerTramesOFF(false);
      if (bGetEnvoiEnCours()) {
        ret = -2;
      }
      else {
        bSetEnvoyerTramesON(true);
        //mqtt.publishState(true);
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
int CChaudiere::envoiTrameOFF() {
  int ret = 0;
  // Mettre un délai avant de pouvoir arrêter l'envoi du ON 
  // après demande ON
  if (active) {
    bool b = bGetEnvoiEnCours();
    DBG(DBG_CHAUDIERE, "main loop - OFF appuye - Envoi en cours : % d\n", b);
    bSetEnvoyerTramesON(false);
    if (bGetEnvoiEnCours()) {
      ret = -2;
    }
    else {
      bSetEnvoyerTramesOFF(true);
      //mqtt.publishState(false);
      saveState(false);
      ret = 0;
    }
  }
    else {
    ret = -1;
  }
  return ret;
}

bool CChaudiere::bSetEnvoyerTramesON(bool b) {
    bool bLastVal = mbEnvoyerTramesON;
    mbEnvoyerTramesON = b;
    mbTrameCommandeEnvoyee = false;
    mbKeepAliveActive = false;
    muiKeepAliveIndexEnCours = 0;
    return bLastVal;
}
bool CChaudiere::bSetEnvoyerTramesOFF(bool b) {
    bool bLastVal = mbEnvoyerTramesOFF;
    mbEnvoyerTramesOFF = b;
    mbTrameCommandeEnvoyee = false;
    mbKeepAliveActive = false;
    muiKeepAliveIndexEnCours = 0;
    return bLastVal;
}
bool CChaudiere::bGetEnvoiEnCours(void) { 
  return mbEnvoyerTramesON || mbEnvoyerTramesOFF;
}
int CChaudiere::activeEquipement() {
  int ret = 0;
  setActive(!active);
  ret = onMqttPublish(mqttSubTopicCommand.c_str(), active ? "ENABLER" : "DISABLER");

  return ret;
}
int CChaudiere::allumer() {
  int ret = envoiTrameON();
  if (ret == 0) { // RAS
    saveState(true);
    if (mEcran != nullptr) {
      //mEcran->updateBoilerStatus();
      mEcran->updateAllStates();
    }
  }
  else if (ret == -1) { // Equipement inactif
    String nomEq = nomEquipement;
    String s = "L'equipement " + nomEq + " n'est pas actif";
    if (mEcran != nullptr) {
      mEcran->updateStatus(s);
    }
    DBGLN(DBG_CHAUDIERE, s);
  }
  else if (ret == -2) { // Envoi en cours, on ne fait rien
    DBG(DBG_CHAUDIERE, "Envoi en cours\n");
  }
  if (onMqttPublish != nullptr)
    onMqttPublish(mqttSubTopicCommand.c_str(),"ONR"); // Mise à jour de l'IHM des CYD auxilliaires
  else DBG(DBG_CHAUDIERE, "int CChaudiere::allumer() - onMqttPublish == nullptr\n");
    
  return ret;
}
int CChaudiere::eteindre() {
  int ret = envoiTrameOFF();
  if (ret == 0) { // RAS
    saveState(false);
    if (mEcran != nullptr) {
      //mEcran->updateBoilerStatus();
      mEcran->updateAllStates();
    }
  }
  else if (ret == -1) {
    String sEq = nomEquipement;
    String s = "L'equipement " + sEq + " n'est pas actif";
    if (mEcran != nullptr) {
      mEcran->updateStatus(s);
    }
      DBGLN(DBG_CHAUDIERE, s);
  }
  else if (ret == -2) {
    DBG(DBG_CHAUDIERE, "Envoi déjà en cours\n");
  }
  if (onMqttPublish != nullptr)
    ret = onMqttPublish(mqttSubTopicCommand.c_str(),"OFFR");
  else DBG(DBG_CHAUDIERE, "int CChaudiere::eteindre() - onMqttPublish == nullptr\n");
    
 return ret;
}
void CChaudiere::envoiCommande(const uint16_t* pulses, int nbPulses, const char* action) {
  if (mEcran != nullptr) mEcran->updateStatus(String("Emission ") + action + "...");
  transmitPulses(pulses, nbPulses, action);
  if (mEcran != nullptr) mEcran->updateStatus(String("Emission ") + action + " terminee");
}

void CChaudiere::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }
void CChaudiere::loop() {
  loop_envoi_trames();        
}

void CChaudiere::loadFromNVS() {
    prefs.begin(nvs_namespace, true);

    domotique_prefix = prefs.getString("domo_prefix", default_domotique_topic_prefix);
  // Chaudiere
  nomEquipement = prefs.getString("chaud_nom", "XXX");
  nomBoutonON = prefs.getString("chaud_nameBtnON", "Ch. ON");
  nomBoutonOFF = prefs.getString("chaud_nameBtnOF", "Ch. OFF");
  frequency = prefs.getFloat("chaud_freq", 433.92);
  active = prefs.getBool("chaud_active", false);

  mqttSubTopic = prefs.getString("topic_prefix", default_topic_prefix);

  // On forme les subtopic MQTT pour la chaudière*/
  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";
  mqttSubTopicStatus = domotique_prefix + mqttSubTopic + "status"; //mqqtInfo.subtopic_status;

  // État chaudière
  etat = prefs.getBool("boiler_state", default_boiler_state);
  etatStr = etat ? "ON" : "OFF";

  // Flag keep-alive
  enableKeepAlive = prefs.getBool("en_keep_alive", default_enable_keep_alive);

  // Chargement des compteurs
  on_keepalive_count = prefs.getUChar("on_keep_count", 5);
  off_keepalive_count = prefs.getUChar("off_keep_count", 4);

  // On charge seulement les cases existantes
  for (uint8_t i = 0; i < on_keepalive_count && i < MAX_KEEPALIVE; i++) {
    String key = "on_keep_" + String(i);
    onKeepalive[i] = prefs.getULong(key.c_str(), 0);  // 0 si clé absente
  }
  // Les cases restantes restent à 0 (pas besoin de les toucher)

  for (uint8_t i = 0; i < off_keepalive_count && i < MAX_KEEPALIVE; i++) {
    String key = "off_keep_" + String(i);
    offKeepalive[i] = prefs.getULong(key.c_str(), 0);
  }

  prefs.end();
}

void CChaudiere::saveToNVS() {
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

  
  // Timings keep alive compactés
  // Compaction ON
  uint8_t real_count_on = 0;
  for (uint8_t i = 0; i < MAX_KEEPALIVE; i++) {
    if (onKeepalive[i] > 0) real_count_on = i + 1;
  }
  on_keepalive_count = real_count_on;
  prefs.putUChar("on_keep_count", on_keepalive_count);

  for (uint8_t i = 0; i < MAX_KEEPALIVE; i++) {
    String key = "on_keep_" + String(i);
    if (i < real_count_on) {
      prefs.putULong(key.c_str(), onKeepalive[i]);
    } else {
      prefs.remove(key.c_str());  // Nettoyage des anciennes valeurs
    }
  }
  // Même chose pour OFF
  uint8_t real_count_off = 0;
  for (uint8_t i = 0; i < MAX_KEEPALIVE; i++) {
    if (offKeepalive[i] > 0) real_count_off = i + 1;
  }
  off_keepalive_count = real_count_off;
  prefs.putUChar("off_keep_count", off_keepalive_count);

  for (uint8_t i = 0; i < MAX_KEEPALIVE; i++) {
    String key = "off_keep_" + String(i);
    if (i < real_count_off) {
      prefs.putULong(key.c_str(), offKeepalive[i]);
    } else {
      prefs.remove(key.c_str());
    }
  }

  prefs.end();
}

void CChaudiere::print() const {
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

  DBG(DBG_CHAUDIERE, "  [Keep-Alive]\n");
  DBG(DBG_CHAUDIERE, "     Activé         : %s\n", enableKeepAlive ? "OUI" : "NON");

  DBG(DBG_CHAUDIERE, "     Timings ON   : ");
  for (uint8_t i = 0; i < MAX_KEEPALIVE; i++) {
    if (onKeepalive[i] > 0) {
      DBGLN(DBG_CHAUDIERE, onKeepalive[i]);
      DBG(DBG_CHAUDIERE, " ");
    }
  }
  DBG(DBG_CHAUDIERE, "\n");

  DBG(DBG_CHAUDIERE, "     Timings OFF  : ");
  for (uint8_t i = 0; i < MAX_KEEPALIVE; i++) {
    if (offKeepalive[i] > 0) {
      DBGLN(DBG_CHAUDIERE, offKeepalive[i]);
      DBG(DBG_CHAUDIERE, " ");
    }
  }
}

void CChaudiere::saveState(bool state) {
  if (etat == state) return;

  prefs.begin(nvs_namespace, false);
  prefs.putBool("boiler_state", state);
  prefs.end();

  etat = state;
  etatStr = state ? "ON" : "OFF";

  prefs.end();
}
void CChaudiere::setActive(bool state) {
  if (active == state) return;

  prefs.begin(nvs_namespace, false);
  prefs.putBool("chaud_active", state);
  prefs.end();

  active = state;
}

// Envoie l'état actif/inactif ainsi que ON/OFF
int CChaudiere::remonteStatusParMqtt() {
  int ret = 0;
  DBG(DBG_CHAUDIERE, "CChaudiere::remonteStatusParMqtt() - Envoi %s sur %s\n", this->active ? "ENABLER" : "DISABLER", this->mqttSubTopicCommand.c_str());
  onMqttPublish(this->mqttSubTopicCommand.c_str(), this->active ? "ENABLER" : "DISABLER");
  delay(200);
  DBG(DBG_CHAUDIERE, "CChaudiere::remonteStatusParMqtt() - Envoi %s sur %s\n", this->etat ? "ONR" : "OFFR", this->mqttSubTopicCommand.c_str());
  onMqttPublish(this->mqttSubTopicCommand.c_str(), this->etat ? "ONR" : "OFFR");
  delay(200);
  return ret;
} 

//
// Etat réel de la chaudière
// Vient de l'observatoion de la DEL rouge
//  Eteint : 0
//  Allulmée : 1
//
void CChaudiere::setEtatReelOnOff(bool state) {
  // On prévient les appareils distants
  //Serial.printf("void CChaudiere::setEtatReelOnOff() - state=%d - etat=%d\n", state, etat);
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

}

void CChaudiere::handleMqttCommand(const String& payload) {
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
    /*else if (bForceDisable) {
      setActive(false);
      String s = nomEquipement + " : désactivee (force OFF)";
      if (mEcran != nullptr) mEcran->updateStatus(s, true);
      Serial.println(s);
    }*/
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
      if (bForceDisable /*&& !mbOldActif*/) {
        mbForcerDisable=true;
      }
      // Si on a éteint de manière forcée, on remet l'équipement dans sont état précédent actif / inactif
      /*if (bForceDisable) {
        if (active != mbOldActif) {
          setActive(mbOldActif);
          if (onMqttPublish != nullptr)
            onMqttPublish(this->mqttSubTopicCommand.c_str(), active ? "ENABLER" : "DISABLER");
          String s = nomEquipement + " : désactivee (force OFF)";
          if (mEcran != nullptr) mEcran->updateStatus(s, true);
          Serial.println(s);
        }
      }*/
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

/*void CChaudiere::handleMqttCommand(const String& payload) {
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
   

void CChaudiere::loadFromWebServer (WebServer& server) {
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
}

String CChaudiere::getHTML() {
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
}


