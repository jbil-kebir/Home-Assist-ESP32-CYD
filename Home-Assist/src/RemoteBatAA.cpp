#include "RemoteBatAA.h"
#include "ParsedMqttMessage.h"

int CRemoteBatterieAA::begin(const String pref) {
  nomEquipement = "Equipement";
  mqttSubTopic = "bat";
  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  mulWatchDog = millis();
  return 0;
}

//--------------------------------------------------------------------------
//  CRemoteBatterieAA::loop()
//
// Retour
// -10 : BatterieAAmètre KO
// -99 : RAS
//--------------------------------------------------------------------------
int CRemoteBatterieAA::loop() {
  int ret = -99;
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
bool CRemoteBatterieAA::readTension() {
  String sCmd = nomEquipement + " " + sMqttCommandMesure;
  onMqttPublish(mqttSubTopicCommand.c_str(), sCmd.c_str());
  return true;
}

float CRemoteBatterieAA::getLastTension() const { 
  return lastTension; 
}

void CRemoteBatterieAA::printTension() const {
  if (lastTension != -1.0) {
    Serial.printf("Tension : %.2f °C\n", lastTension);
  } else {
    Serial.println("Aucune tension valide");
  }
}

void CRemoteBatterieAA::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "bat/");
  mulWatchdogIntervalle = prefs.getLong((mPrefixNVS+"wdog").c_str(), mulWatchdogDefaultIntervalle);
  prefs.end();
  CEquipementBase::loadFromNVS();
   
}
void CRemoteBatterieAA::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putLong((mPrefixNVS+"wdog").c_str(), mulWatchdogIntervalle);
  prefs.end();
  CEquipementBase::saveToNVS();
}

void CRemoteBatterieAA::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"wdog").c_str())) mulWatchdogIntervalle = server.arg((mPrefixNVS+"wdog")).toInt();
  CEquipementBase::loadFromWebServer (server);
}

void CRemoteBatterieAA::print() const {

  Serial.printf("==========================================================\n");
  Serial.printf("     Nom              : %s\n", nomEquipement);
  Serial.printf("     Actif            : %s\n", active ? "OUI" : "NON");
  Serial.printf("     Watchdog         : %ld s\n", mulWatchdogIntervalle);
  Serial.printf("     MQTTSubTopic     : %s\n", mqttSubTopic.c_str());
  Serial.printf("     MQTTCmd          : %s\n", mqttSubTopicCommand.c_str());
  Serial.printf("     MQTTState        : %s\n", mqttSubTopicState.c_str());
}

void CRemoteBatterieAA::mqttMessageToStruct_split(MQTTMessage_2& st, const String& s) {
    int idx = 0;
    st.sExpediteur = s.substring(0, s.indexOf(' ', idx));
    idx = s.indexOf(' ', idx) + 1;
    st.sAction = s.substring(idx, s.indexOf(' ', idx));
    idx = s.indexOf(' ', idx) + 1;
    st.sArg = s.substring(idx, s.indexOf(' ', idx));
    idx = s.indexOf(' ', idx) + 1;
    st.sDate       = s.substring(idx, s.indexOf(' ', idx));
    idx = s.indexOf(' ', idx) + 1;
    st.sTime       = s.substring(idx, s.indexOf(' ', idx));
    idx = s.indexOf(' ', idx) + 1;
    st.fVal        = s.substring(idx).toFloat();
}

void CRemoteBatterieAA::setDisplayCallback(std::function<void(const String&, bool, float)> cb) {
        onTensionChanged = cb;
    }
void CRemoteBatterieAA::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }
        
/*void CRemoteBatterieAA::handleMqttState(const String& payload) {
    static bool inactifAffiche = false;
    if (!active) {
      if (!inactifAffiche) {
        Serial.printf("void CRemoteBatterieAA::handleMqttState (); - %s inactif\n", nomEquipement.c_str());
        inactifAffiche = true;
      }
      return;
    }
    inactifAffiche = false;
    // On réinitiallise le WatchDog
    mulWatchDog = millis();

    ParsedMqttMessage msg;
    msg.parse(payload);
    int n = msg.miTailleMesure;
    Serial.printf("void CRemoteBatterieAA::handleMqttState(const String& payload) - paylod = %s - Nb de champs = %d\n", payload.c_str(), n);
    if (n) { //msg.parse(payload)) {
        msg.printDebug();
        if (msg.msExpediteur != getNomEquipement()) {
            return; // pas pour nous
        }

        if (msg.mvsMesure.empty()) return;

        if (!msg.msIp.isEmpty()) mIP = msg.msIp.isEmpty();
        
        String premiereMesure = msg.mvsMesure[0];
        premiereMesure.toUpperCase();

        if (premiereMesure == "TENSION") {
            // La valeur est généralement le dernier champ
            if (!msg.mvsMesure.empty()) {
              String sEtatBatterie = msg.mvsMesure[1];
              String sTension = msg.mvsMesure.back();
              int etatBatterie;
              if (sEtatBatterie == "CHARGEE") etatBatterie = 1;
              else if (sEtatBatterie == "DECHARGEE") etatBatterie = 0;
              float val = sTension.toFloat();
              // Traiter val selon premiereMesure
                if (onTensionChanged != nullptr) {
                  onTensionChanged(msg.msExpediteur, etatBatterie, val);
                }
                else {
                  Serial.printf("void CRemoteBatterieAA::handleMqttState - Aucun callback défini pour <%s %s %d %.2f>", getNomEquipement(), premiereMesure, etatBatterie, val);
                }
              } // if (!msg.mvsMesure.empty())
            } // if (premiereMesure == "TENSION")
  #ifndef __LOCAL_MODE__
        if (premiereMesure == "TENSIONR") {
            // La valeur est généralement le dernier champ
            if (!msg.mvsMesure.empty()) {
              String sEtatBatterie = msg.mvsMesure[1];
              String sTension = msg.mvsMesure.back();
              int etatBatterie;
              if (sEtatBatterie == "CHARGEE") etatBatterie = 1;
              else if (sEtatBatterie == "DECHARGEE") etatBatterie = 0;
              float val = sTension.toFloat();
              // Traiter val selon premiereMesure
                if (onTensionChanged != nullptr) {
                  onTensionChanged(msg.msExpediteur, etatBatterie, val);
                }
                else {
                  Serial.printf("void CRemoteBatterieAA::handleMqttState - Aucun callback défini pour <%s %s %d %.2f>", getNomEquipement(), premiereMesure, etatBatterie, val);
                }
              } // if (!msg.mvsMesure.empty())
            } // if (premiereMesure == "TENSIONR")
        else if (st.sAction == "INACTIFR") {
          setActive(false);
        } 
        else {
          Serial.println("Aucun callback défini pour " + nomEquipement);
        }

  #endif
        } // if (n)
}*/


void CRemoteBatterieAA::handleMqttState(const String& payload) {
  String cmd = payload, cmdOrg;
  cmd.trim();
  cmdOrg = cmd;
  cmd.toUpperCase();
  static bool inactifAffiche = false;
  if (!active) {
    if (!inactifAffiche) {
      Serial.printf("void CRemoteBatterieAA::handleMqttState (); - %s inactif\n", nomEquipement.c_str());
      inactifAffiche = true;
    }
    return;
  }
  inactifAffiche = false;

  MQTTMessage_2 st, stOrg;
  mqttMessageToStruct_split(st, cmd);
  mqttMessageToStruct_split(stOrg, cmdOrg);
  // On réinitiallise le WatchDog
  mulWatchDog = millis();
  if (st.sAction == "TENSION") {
    if (onTensionChanged != nullptr) {
      lastTension = st.fVal;
      if (st.sArg == "CHARGEE") miLastEtatBatterie = 1;
      else if (st.sArg == "DECHARGEE") miLastEtatBatterie = 0;
      else miLastEtatBatterie = -1;
      onTensionChanged(stOrg.sExpediteur, miLastEtatBatterie, st.fVal);
    }
    else {
      Serial.println("Aucun callback défini pour " + nomEquipement);
    }
  } 
  #ifndef __LOCAL_MODE__
  else if (st.sAction == "TENSIONR") {
    // Lorsqu'on reçoit une mesure, cela indique que le capteur est actif
    setActive(true);
    
    if (onTensionChanged != nullptr) {
      lastTension = st.fVal;
      if (st.sArg == "CHARGEE") miLastEtatBatterie = 1;
      else if (st.sArg == "DECHARGEE") miLastEtatBatterie = 0;
      else miLastEtatBatterie = -1;
      onTensionChanged(stOrg.sExpediteur, miLastEtatBatterie, st.fVal);
    }
  } 
  else if (st.sAction == "INACTIFR") {
    setActive(false);
  } 
  else {
    Serial.println("Aucun callback défini pour " + nomEquipement);
  }
  #endif

}


// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale
bool CRemoteBatterieAA::remonteStatusParMqtt() {
  if (onMqttPublish == nullptr) return false;
  float temp = getLastTension(); //round(10*ds18b20.getLastTension())/10.0; // On arrondit à une décimale
  CMyDateTime mDateTime;
  String sEtatBatterie;
  if (miLastEtatBatterie) sEtatBatterie = "CHARGEE " ;
  else sEtatBatterie = "DECHARGEE ";
  String sVal;
  if (active) {
    sVal = nomEquipement + " TENSIONR " + sEtatBatterie + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, 1);
    sVal += " FORCE";
  }
  else {
    sVal = nomEquipement + " INACTIFR " + mDateTime.getDate() + " " + mDateTime.getTime();
  }
  Serial.println("CBatterieAA::publieSurMqtt() : " + sVal);
  bool ret = (onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str()) == 0);
  return ret;
} 

void CRemoteBatterieAA::handleMqttCommand(const String& payload) {
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


String CRemoteBatterieAA::getHTML() {
  String html = "";
  html =  "<h2>Configuration de la batterie AA " + nomEquipement + "</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active").c_str() + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Intervalle Watchdog (min)</label><input type=\"text\" name=" + (mPrefixNVS + "wdog").c_str() + " value=\"" + mulWatchdogIntervalle + "\"></div>"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS + "subtopic").c_str() + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}