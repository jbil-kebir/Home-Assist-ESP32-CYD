#include "RemoteTor.h"
#include "ParsedMqttMessage.h"


int CRemoteTor::setup(const String pref) {
  nomEquipement = "Equipement_Tor";
  mqttSubTopic = "tor";
  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  mulWatchDog = millis();
  return 0;
}

//--------------------------------------------------------------------------
//  CRemoteTor::loop()
//
// Retour
// -10 : BatterieAAmètre KO
// -99 : RAS
//--------------------------------------------------------------------------
int CRemoteTor::loop() {
  int ret = -99;
  if (!active) return -2;
  if ((millis() - mulWatchDog) >= mulWatchdogIntervalle*1000) {
    // Ca fait trop longtemps que rien n'a été reçu
    return -10;
  }

  return ret;
}

//
// Pour un appareil distant, cela consiste à envoyer
// une commande MQTT : NomEquipement MESURE
// 
bool CRemoteTor::readMesure() {
  String sCmd = nomEquipement + " " + sMqttCommandMesure;
  onMqttPublish(mqttSubTopicCommand.c_str(), sCmd.c_str());
  return true;
}

float CRemoteTor::getLastMesure() const { 
  return lastMesure; 
}

void CRemoteTor::printMesure() const {
  if (lastMesure != -1.0) {
    DBG(DBG_ACTIONNEURS, "Mesure : %d\n", lastMesure);
  } else {
    DBG(DBG_ACTIONNEURS, "Aucune mesure valide\n");
  }
}

void CRemoteTor::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "bat/");
  mulWatchdogIntervalle = prefs.getLong((mPrefixNVS+"wdog").c_str(), mulWatchdogDefaultIntervalle);
  prefs.end();
  CEquipementBase::loadFromNVS();
   
}
void CRemoteTor::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putLong((mPrefixNVS+"wdog").c_str(), mulWatchdogIntervalle);
  prefs.end();
  CEquipementBase::saveToNVS();
}

void CRemoteTor::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"wdog").c_str())) mulWatchdogIntervalle = server.arg((mPrefixNVS+"wdog")).toInt();
  CEquipementBase::loadFromWebServer (server);
}

void CRemoteTor::print() const {

  DBG(DBG_ACTIONNEURS, "==========================================================\n");
  DBG(DBG_ACTIONNEURS, "     Nom              : %s\n", nomEquipement.c_str());
  DBG(DBG_ACTIONNEURS, "     Actif            : %s\n", active ? "OUI" : "NON");
  DBG(DBG_ACTIONNEURS, "     Watchdog         : %ld s\n", mulWatchdogIntervalle);
  DBG(DBG_ACTIONNEURS, "     MQTTSubTopic     : %s\n", mqttSubTopic.c_str());
  DBG(DBG_ACTIONNEURS, "     MQTTCmd          : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_ACTIONNEURS, "     MQTTState        : %s\n", mqttSubTopicState.c_str());
}

/*void CRemoteTor::mqttMessageToStruct_split(MQTTMessage_5& st, const String& s) {
    int idx = 0;
    st.sExpediteur = s.substring(0, s.indexOf(' ', idx));
    idx = s.indexOf(' ', idx) + 1;
    st.sAction = s.substring(idx, s.indexOf(' ', idx));
    idx = s.indexOf(' ', idx) + 1;
//    st.sArg = s.substring(idx, s.indexOf(' ', idx));
//    idx = s.indexOf(' ', idx) + 1;
    st.sDate       = s.substring(idx, s.indexOf(' ', idx));
    idx = s.indexOf(' ', idx) + 1;
    st.sTime       = s.substring(idx, s.indexOf(' ', idx));
    idx = s.indexOf(' ', idx) + 1;
    st.fVal        = s.substring(idx).toInt();
}*/

void CRemoteTor::setDisplayCallback(std::function<void(const String&, int)> cb) {
        onMesureChanged = cb;
    }
void CRemoteTor::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }
        
void CRemoteTor::handleMqttState(const String& payload) {
  static bool inactifAffiche = false;
  if (!active) {
    if (!inactifAffiche) {
      DBG(DBG_ACTIONNEURS, "void CRemoteTor::handleMqttState (); - %s inactif\n", nomEquipement.c_str());
      inactifAffiche = true;
    }
    return;
  }
  inactifAffiche = false;

  ParsedMqttMessage msg;
  msg.parse(payload);
  int n = msg.miTailleMesure;
  //Serial.printf("void CRemoteTor::handleMqttState(const String& payload) - paylod = %s - Nb de champs = %d\n", payload.c_str(), n);
  
  if (n > 0) {
    msg.printDebug();
    if (onEquipement != nullptr) 
      onEquipement(msg.msExpediteur, msg.msIp); // On ajoute à la liste d'équuipements si ça n'est pas déjà fait
    else DBG(DBG_ACTIONNEURS, "void CRemoteTor::handleMqttState() - Pas de callback onEquipement() - %s\n", nomEquipement.c_str());

    if (msg.msExpediteur != getNomEquipement()) {
      DBG(DBG_ACTIONNEURS, "void CRemoteTor::handleMqttState() - Message pour %s, pas pour nous (%s). On sort.\n", msg.msExpediteur.c_str(), getNomEquipement().c_str());
      return; // pas pour nous
    }

    if (msg.mvsMesure.empty()) {
      DBG(DBG_ACTIONNEURS, "void CRemoteTor::handleMqttState() Equipement %s - Mesure vide. On sort.\n", getNomEquipement().c_str());
      return;
    }
    // On réinitiallise le WatchDog
    mulWatchDog = millis();

    if (!msg.msIp.isEmpty()) mIP = msg.msIp.isEmpty();

    String premiereMesure = msg.mvsMesure[0];
    premiereMesure.toUpperCase();

    if (premiereMesure == "ONOFF") {
      if (onMesureChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        lastMesure = valStr.toInt();
        onMesureChanged(msg.msExpediteur, lastMesure);
      }
      else {
        DBGLN(DBG_ACTIONNEURS, "Aucun callback défini pour " + nomEquipement);
      }
    }
    else if (premiereMesure == "ETAT") {
      setActive(true);
      if (onMesureChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        lastMesure = valStr.toInt();
        onMesureChanged(msg.msExpediteur, lastMesure);
      }
      else {
        DBGLN(DBG_ACTIONNEURS, "Aucun callback défini pour " + nomEquipement);
      }
    }
    #ifndef __LOCAL_MODE__
    else if (premiereMesure == "ONOFFR") {
      // Lorsqu'on reçoit une mesure, cela indique que le capteur est actif
      setActive(true);
      
      if (onMesureChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        lastMesure = valStr.toInt();
        onMesureChanged(msg.msExpediteur, lastMesure);
      }
    } 
    else if (premiereMesure == "INACTIFR") {
      setActive(false);
    } 
    else {
      DBGLN(DBG_ACTIONNEURS, "Aucun callback défini pour " + nomEquipement);
    }
    #endif


  }
  

}

// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale
bool CRemoteTor::remonteStatusParMqtt() {
  if (onMqttPublish == nullptr) return false;
  int temp = getLastMesure(); 
  CMyDateTime mDateTime;
  String sVal;
  if (active) {
    sVal = nomEquipement + " ONOFFR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp);
    sVal += " FORCE";
  }
  else {
    sVal = nomEquipement + " INACTIFR " + mDateTime.getDate() + " " + mDateTime.getTime();
  }
  DBGLN(DBG_ACTIONNEURS, "CRemoteTor::publieSurMqtt() : " + sVal);
  bool ret = (onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str()) == 0);
  return ret;
} 

void CRemoteTor::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

  // On réinitiallise le WatchDog
  mulWatchDog = millis();

  if (cmd == "ON" || cmd == "OFF") {
  }
 else if (cmd == "ENABLE") {
  }
 else if (cmd == "DISABLE") {
  }
}


String CRemoteTor::getHTML() {
  String html = "";
  html =  "<h2>Configuration de la batterie AA " + nomEquipement + "</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active").c_str() + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Intervalle Watchdog (s)</label><input type=\"text\" name=" + (mPrefixNVS + "wdog").c_str() + " value=\"" + mulWatchdogIntervalle + "\"></div>"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS + "subtopic").c_str() + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}