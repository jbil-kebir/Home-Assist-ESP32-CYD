#include "RemoteDHT20.h"
#include "ParsedMqttMessage.h"

int CRemoteDHT20::begin(const String pref) {
  nomEquipement = "Equipement";
  mqttSubTopic = "thermometre";
  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  mulWatchDog = millis();
  return 0;
}

//--------------------------------------------------------------------------
//  CRemoteDHT20::loop()
//
// Retour
// -10 : Thermomètre KO
// -99 : RAS
//--------------------------------------------------------------------------
int CRemoteDHT20::loop() {
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
bool CRemoteDHT20::readMesures() {
  String sCmd = nomEquipement + " " + sMqttCommandMesure;
  onMqttPublish(mqttSubTopicCommand.c_str(), sCmd.c_str());
  return true;
}

float CRemoteDHT20::getLastTemperature() const { 
  return lastTempC; 
}
float CRemoteDHT20::getLastHumidite() const { 
  return lastHum; 
}

void CRemoteDHT20::printMesures() const {
  if (lastTempC != -127.0 && lastHum != -1) {
    DBG(DBG_CAPTEURS, "void CRemoteDHT20::printMesures() - Température : %.2f °C Humidité : %.2f\n", lastTempC, lastHum);
  } else {
    DBG(DBG_CAPTEURS, "Aucune température valide\n");
  }
}

void CRemoteDHT20::loadFromNVS() {
  prefs.begin(nvs_namespace, true);
  mucCapteurID = prefs.getUShort((mPrefixNVS+"id").c_str(), 0);
  mulWatchdogIntervalle = prefs.getLong((mPrefixNVS+"wdog").c_str(), mulWatchdogDefaultIntervalle);
  prefs.end();

  CEquipementBase::loadFromNVS();
   
}
void CRemoteDHT20::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putUShort((mPrefixNVS+"id").c_str(), mucCapteurID);
  prefs.putLong((mPrefixNVS+"wdog").c_str(), mulWatchdogIntervalle);
  prefs.end();

  CEquipementBase::saveToNVS();
}


void CRemoteDHT20::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"id").c_str())) mucCapteurID = server.arg((mPrefixNVS+"id")).toInt();
  if (server.hasArg((mPrefixNVS+"wdog").c_str())) mulWatchdogIntervalle = server.arg((mPrefixNVS+"wdog")).toInt();

  CEquipementBase::loadFromWebServer (server);
}

void CRemoteDHT20::print() const {

  DBG(DBG_CAPTEURS, "==========================================================\n");
  DBG(DBG_CAPTEURS, "     Nom              : %s\n", nomEquipement);
  DBG(DBG_CAPTEURS, "     ID               : %d\n", mucCapteurID);
  DBG(DBG_CAPTEURS, "     Actif            : %s\n", active ? "OUI" : "NON");
  DBG(DBG_CAPTEURS, "     Watchdog         : %ld s\n", mulWatchdogIntervalle);
  DBG(DBG_CAPTEURS, "     MQTTSubTopic     : %s\n", mqttSubTopic.c_str());
  DBG(DBG_CAPTEURS, "     MQTTCmd          : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_CAPTEURS, "     MQTTState        : %s\n", mqttSubTopicState.c_str());
}

void CRemoteDHT20::setDisplayCallbackTemperature(std::function<void(const String&, float)> cb) {
        onTemperatureChanged = cb;
    }
void CRemoteDHT20::setDisplayCallbackHumidite(std::function<void(const String&, float)> cb) {
        onHumiditeChanged = cb;
    }
void CRemoteDHT20::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }

void CRemoteDHT20::handleMqttState(const String& payload) {
  //String cmd = payload;
  //cmd.trim();
  static bool inactifAffiche = false;
  if (!active) {
    if (!inactifAffiche) {
      DBG(DBG_CAPTEURS, "void CRemoteDHT20::handleMqttState (); - %s inactif\n", nomEquipement.c_str());
      inactifAffiche = true;
    }
    return;
  }
  inactifAffiche = false;

  ParsedMqttMessage msg;
  msg.parse(payload);
  int n = msg.miTailleMesure;
  //Serial.printf("void CRemoteDHT20::handleMqttState(const String& payload) - paylod = %s - Nb de champs = %d\n", payload.c_str(), n);
  
  if (n > 0) {
    //msg.printDebug();
    if (onEquipement != nullptr) 
      onEquipement(msg.msExpediteur, msg.msIp); // On ajoute à la liste d'équuipements si ça n'est pas déjà fait
    else DBG(DBG_CAPTEURS, "void CRemoteDHT20::handleMqttState() - Pas de callback onEquipement() - %s\n", nomEquipement.c_str());

    if (msg.msExpediteur != getNomEquipement()) {
      DBG(DBG_CAPTEURS, "void CRemoteDHT20::handleMqttState() - Message pour %s, pas pour nous (%s). On sort.\n", msg.msExpediteur.c_str(), getNomEquipement().c_str());
      return; // pas pour nous
    }

    if (msg.mvsMesure.empty()) {
      DBG(DBG_CAPTEURS, "void CRemoteDHT20::handleMqttState() Equipement %s - Mesure vide. On sort.\n", getNomEquipement().c_str());
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
        //Serial.printf("void CRemoteDHT20::handleMqttState() - Appel à remonteTemperatureParMqtt()\n");
        //remonteTemperatureParMqtt(); // Les CYD distants doivent être informés
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
      }
    } 
    else if (premiereMesure == "HUM") {
      if (onHumiditeChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        lastHum = valStr.toFloat();
        onHumiditeChanged(msg.msExpediteur, lastHum);
        //Serial.printf("void CRemoteDHT20::handleMqttState() - Appel à remonteHumiditeParMqtt()\n");
        //remonteHumiditeParMqtt(); // Les CYD distants doivent être informés
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
      }
    } 
    #ifndef __LOCAL_MODE__
    else if (premiereMesure == "TEMPR") {
      if (onTemperatureChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        lastTempC = valStr.toFloat();
        onTemperatureChanged(msg.msExpediteur, lastTempC);
        //Serial.printf("void CRemoteDHT20::handleMqttState() - Appel à remonteTemperatureParMqtt()\n");
        //remonteTemperatureParMqtt(); // Les CYD distants doivent être informés
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
      }
    } 
    else if (premiereMesure == "HUMR") {
      if (onHumiditeChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        lastHum = valStr.toFloat();
        onHumiditeChanged(msg.msExpediteur, lastHum);
        //Serial.printf("void CRemoteDHT20::handleMqttState() - Appel à remonteHumiditeParMqtt()\n");
        //remonteHumiditeParMqtt(); // Les CYD distants doivent être informés
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
      }
    } 
    #endif
  }

}

// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale ou lorsqu'une 
// mesure arrive par RF
bool CRemoteDHT20::remonteTemperatureParMqtt() {
    if (onMqttPublish == nullptr) return false;
    bool ret = false;
    CMyDateTime mDateTime;
    float temp = lastTempC; 
    String sValTemp = nomEquipement + " TEMPR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, 1);
    sValTemp += " FORCE";
    //Serial.println("CRemoteDHT20::publieSurMqtt() Temp : " + sValTemp);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sValTemp.c_str()) == 0);
    return ret;
} 

// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale ou lorsqu'une 
// mesure arrive par RF
bool CRemoteDHT20::remonteHumiditeParMqtt() {
    if (onMqttPublish == nullptr) return false;
    bool ret = false;
    CMyDateTime mDateTime;
    float hum = lastHum; 
    String sValHum = nomEquipement + " HUMR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(hum, 1);
    sValHum += " FORCE";
    //Serial.println("CRemoteDHT20::publieSurMqtt() Hum  : " + sValHum);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sValHum.c_str()) == 0);
    return ret;
} 


// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale
bool CRemoteDHT20::remonteStatusParMqtt() {
    if (onMqttPublish == nullptr) return false;
    bool ret = false;
    ret = remonteTemperatureParMqtt();
    if (ret) ret = remonteHumiditeParMqtt();
    /*CMyDateTime mDateTime;
    float temp = lastTempC; 
    float hum = lastHum; 
    String sValTemp = nomEquipement + " TEMPR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, 1);
    sValTemp += " FORCE";
    String sValHum = nomEquipement + " HUMR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(hum, 1);
    sValHum += " FORCE";
    Serial.println("CRemoteDHT20::publieSurMqtt() Temp : " + sValTemp);
    Serial.println("CRemoteDHT20::publieSurMqtt() Hum  : " + sValHum);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sValTemp.c_str()) == 0);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sValHum.c_str()) == 0);*/
    return ret;
} 

void CRemoteDHT20::handleMqttCommand(const String& payload) {
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





// Retour
// -1 : mauvais identifiant d'équipement
// -2 : mauvais nombre de mesures
// -3 : mauvais CRC
// -9 : inactif
int CRemoteDHT20::handleRCSwitchCode(unsigned long code) {
  int ret = 0;
  if (!active) return -9;
  static unsigned long tousLesCodes[NB_CODES_RCS+1];
  static bool receptionEnAttente = false;
  STRUCT_RCS_HEADER sth;
  decodeHeader(sth, code);

  if (sth.id == mucCapteurID && sth.reserved == 0xFFF) { // Si ID correct et champ 0xFFF OK, c'est un header
    // On réinitiallise le WatchDog. Il y aurait matière à discuter mais faisons simple
    mulWatchDog = millis();

    if (sth.nbMesures != NB_CODES_RCS) {
      DBG(DBG_CAPTEURS, "CRemoteDHT20::handleRCSwitchCode - mauvais nombre de mesures (%d). Attendues : %d\n", sth.nbMesures, NB_CODES_RCS);
      return -2;
    }
    receptionEnAttente = true; // On lance la réception des codes mesures
    mucNbCodesRecus = 0;
    tousLesCodes[mucNbCodesRecus] = code;
    DBG(DBG_CAPTEURS, "---------------------------------Réception début----------------------------------\n");
  }
  else if (receptionEnAttente) { // Sinon, si la réception de mesures est en cours
    if (mucNbCodesRecus < NB_CODES_RCS) { // Si tous les codes mesures n'ont pas encore été reçus
      STRUCT_RCS_MESURE stm;
      decodeMesure(stm, code);
      if (stm.id != mucCapteurID) {
        DBG(DBG_CAPTEURS, "CRemoteDHT20::handleRCSwitchCode - mauvais ID : %d vs capteurID : %d\n", sth.id, mucCapteurID);
        receptionEnAttente = false; mucNbCodesRecus=0; // On annule la remontée
        return -1;
      }
      DBG(DBG_CAPTEURS, "Mesure détectée N° %d === Type %d === Val 0x%x = %d\n", stm.numero, stm.type, stm.val, stm.val);
      mulCodes[mucNbCodesRecus] = code;
      mucNbCodesRecus++;
      tousLesCodes[mucNbCodesRecus] = code;
    }
    else if(mucNbCodesRecus == NB_CODES_RCS) {
      receptionEnAttente = false; mucNbCodesRecus=0; // On annule la remontée
      //Serial.println("---------------------------------Réception fin----------------------------------");
      // Le code reçu est le footer
      STRUCT_RCS_FOOTER stf;
      decodeFooter(stf, code); //printStructFooter(stf);
      
      unsigned char crc_calcule = computeCRC(tousLesCodes, NB_CODES_RCS+1);
      //Serial.printf("CRemoteDHT20::handleRCSwitchCode - CRC calculé : 0x%x = %d\n", crc_calcule, crc_calcule);
      if (crc_calcule == stf.crc8) {
        //Serial.printf("CRemoteDHT20::handleRCSwitchCode - CRC 0x%x = %d valide.\n", stf.crc8, stf.crc8);
        DBG(DBG_CAPTEURS, "================================================= MESURE VALIDEE =================================================\n");
      }
      else {
        DBG(DBG_CAPTEURS, "------------------------------------------------- CRC INVALIDE -------------------------------------------------\n");
        DBG(DBG_CAPTEURS, "CRemoteDHT20::handleRCSwitchCode - Mauvais CRC. CRC calculé : 0x%x = %d CRC reçu : 0x%x = %d\n", crc_calcule, crc_calcule, stf.crc8, stf.crc8);
        return -3; // A décommenter ultérieurement
      }

      decode(mStMesures, mulCodes, NB_MESURES);
      for (int i=0; i<NB_MESURES; i++) {
        // Envoi pour affichage des valeurs
        switch(mStMesures[i].type) {
          case MESURE_TEMPERATURE :
            if (onTemperatureChanged != nullptr) {
              lastTempC = mStMesures[i].val;
              onTemperatureChanged(nomEquipement, lastTempC);
              //Serial.printf("void CRemoteDHT20::handleRCSwitchCode() - Appel à remonteTemperatureParMqtt()\n");
              remonteTemperatureParMqtt(); // Les CYD distants doivent être informés
            }
            else {
              DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
            }

          break;
          case MESURE_HUMIDITE :
            if (onHumiditeChanged != nullptr) {
              lastHum = mStMesures[i].val;
              onHumiditeChanged(nomEquipement, lastHum);
              //Serial.printf("void CRemoteDHT20::handleRCSwitchCode() - Appel à remonteTemperatureParMqtt()\n");
              remonteHumiditeParMqtt(); // Les CYD distants doivent être informés
            }
            else {
              DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
            }
          break;
          case MESURE_TENSIOIN :
          break;
         default :
          break;
        }
      }


    }

  }




  return ret;
}


String CRemoteDHT20::getHTML() {
  String html = "";
  html =  "<h2>Configuration du thermomètre " + nomEquipement + "</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active").c_str() + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>ID</label><input type=\"text\" name=" + (mPrefixNVS+"id") + " value=\"" + mucCapteurID + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Intervalle Watchdog (min)</label><input type=\"text\" name=" + (mPrefixNVS + "wdog").c_str() + " value=\"" + mulWatchdogIntervalle + "\"></div>"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS + "subtopic").c_str() + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}