#include "RemoteTh.h"
#include "ParsedMqttMessage.h"

int CRemoteThermo::begin(const String pref) {
  nomEquipement = "Equipement";
  mqttSubTopic = "thermometre";
  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  mulWatchDog = millis();
  return 0;
}

//--------------------------------------------------------------------------
//  CRemoteThermo::loop()
//
// Retour
// -2  : Inactif
// -10 : Thermomètre KO (WatchDog)
// -99 : RAS
//--------------------------------------------------------------------------
int CRemoteThermo::loop() {

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
bool CRemoteThermo::readTemperature() {
  String sCmd = nomEquipement + " " + sMqttCommandMesure;
  onMqttPublish(mqttSubTopicCommand.c_str(), sCmd.c_str());
  return true;
}

float CRemoteThermo::getLastTemperature() const { 
  return lastTempC; 
}

void CRemoteThermo::printTemperature() const {
  if (lastTempC != -127.0) {
    DBG(DBG_CAPTEURS, "Temperature : %.2f °C\n", lastTempC);
  } else {
    DBG(DBG_CAPTEURS, "Aucune Temperature valide\n");
  }
}

void CRemoteThermo::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Thermomètre");
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "thermometre/");
  active = prefs.getBool((mPrefixNVS+"active").c_str(), true);
  mulWatchdogIntervalle = prefs.getLong((mPrefixNVS+"wdog").c_str(), mulWatchdogDefaultIntervalle);
  
  // On forme les subtopic MQTT
  domotique_prefix = prefs.getString("domo_prefix", default_domotique_topic_prefix);
  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";

  prefs.end();
   
}
void CRemoteThermo::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putLong((mPrefixNVS+"wdog").c_str(), mulWatchdogIntervalle);
  prefs.end();
  CEquipementBase::saveToNVS();
}

void CRemoteThermo::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"wdog").c_str())) mulWatchdogIntervalle = server.arg((mPrefixNVS+"wdog")).toInt();

  CEquipementBase::loadFromWebServer (server);
}

void CRemoteThermo::print() const {

  DBG(DBG_CAPTEURS, "==========================================================\n");
  DBG(DBG_CAPTEURS, "     Nom              : %s\n", nomEquipement.c_str());
  DBG(DBG_CAPTEURS, "     Actif            : %s\n", active ? "OUI" : "NON");
  DBG(DBG_CAPTEURS, "     Watchdog         : %ld s\n", mulWatchdogIntervalle);
  DBG(DBG_CAPTEURS, "     MQTTSubTopic     : %s\n", mqttSubTopic.c_str());
  DBG(DBG_CAPTEURS, "     MQTTCmd          : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_CAPTEURS, "     MQTTState        : %s\n", mqttSubTopicState.c_str());
}

void CRemoteThermo::setDisplayCallback(std::function<void(const String&, float)> cb) {
  onTemperatureChanged = cb;
}
void CRemoteThermo::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
  onMqttPublish = cb;
}
  
void CRemoteThermo::handleMqttState(const String& payload) {
  //String cmd = payload;
  //cmd.trim();
  static bool inactifAffiche = false;
  if (!active) {
    if (!inactifAffiche) {
      DBG(DBG_CAPTEURS, "void CRemoteThermo::handleMqttState (); - %s inactif\n", nomEquipement.c_str());
      inactifAffiche = true;
    }
    return;
  }
  inactifAffiche = false;

  ParsedMqttMessage msg;
  msg.parse(payload);
  int n = msg.miTailleMesure;
  //Serial.printf("void CRemoteThermo::handleMqttState(const String& payload) - paylod = %s - Nb de champs = %d\n", payload.c_str(), n);
  
  if (n > 0) {
    //msg.printDebug();
    if (onEquipement != nullptr) 
      onEquipement(msg.msExpediteur, msg.msIp); // On ajoute à la liste d'équuipements si ça n'est pas déjà fait
    else DBG(DBG_CAPTEURS, "void CRemoteThermo::handleMqttState() - Pas de callback onEquipement() - %s\n", nomEquipement.c_str());
    
    if (msg.msExpediteur != getNomEquipement()) {
      DBG(DBG_CAPTEURS, "void CRemoteThermo::handleMqttState() - Message pour %s, pas pour nous (%s). On sort.\n", msg.msExpediteur.c_str(), getNomEquipement().c_str());
      return; // pas pour nous
    }

    if (msg.mvsMesure.empty()) {
      DBG(DBG_CAPTEURS, "void CRemoteThermo::handleMqttState() Equipement %s - Mesure vide. On sort.\n", getNomEquipement().c_str());
      return;
    }
    // On réinitiallise le WatchDog
    mulWatchDog = millis();

    if (!msg.msIp.isEmpty()) mIP = msg.msIp.isEmpty();

    String premiereMesure = msg.mvsMesure[0];
    premiereMesure.toUpperCase();

    if (premiereMesure == "TEMP") {
      if (onTemperatureChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        lastTempC = valStr.toFloat();
        onTemperatureChanged(msg.msExpediteur, lastTempC);
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
      }
    } 
    #ifndef __LOCAL_MODE__ // Les CYD auxiliaires peuvent recevoir des infos température à leur demande
    else if (premiereMesure == "TEMPR") {
      // Lorsqu'on reçoit une mesure, cela indique que le capteur est actif
      setActive(true);

      if (onTemperatureChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        lastTempC = valStr.toFloat();
        onTemperatureChanged(msg.msExpediteur, lastTempC);
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
      }
    } 
    else if (premiereMesure == "INACTIFR") {
      setActive(false);
    } 
    #endif    
  }


}

// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale
bool CRemoteThermo::remonteStatusParMqtt() {
  float temp = getLastTemperature(); //round(10*ds18b20.getLastTemperature())/10.0; // On arrondit à une décimale
  CMyDateTime mDateTime;
  String sVal="";
  if (active) {
    sVal = nomEquipement + " TEMPR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, 1);
    sVal += " FORCE";
  }
  else {
    sVal = nomEquipement + " INACTIFR " + mDateTime.getDate() + " " + mDateTime.getTime();
  }
  DBGLN(DBG_CAPTEURS, "CRemoteThermo::remonteStatusParMqtt() : " + sVal);
  bool ret = onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str());
  return ret;
} 

void CRemoteThermo::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

  //Serial.println("void CRemoteThermo::handleMqttCommand() reçu : " + cmd);

  // On réinitiallise le WatchDog
  mulWatchDog = millis();

  if (cmd == "ON" || cmd == "OFF") {
 }
 else if (cmd == "ENABLE") {
  }
 else if (cmd == "DISABLE") {
  }
}


String CRemoteThermo::getHTML() {
  String html = "";
  html =  "<h2>Configuration du thermomètre " + nomEquipement + "</h2>"
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