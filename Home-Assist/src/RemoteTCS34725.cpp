#include "RemoteTCS34725.h"
#include "ParsedMqttMessage.h"

int CRemoteTCS34725::begin(const String pref) {
  nomEquipement = "Equipement";
  mqttSubTopic = "thermometre";
  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  mulWatchDog = millis();
  return 0;
}

//--------------------------------------------------------------------------
//  CRemoteTCS34725::loop()
//
// Retour
// -10 : Thermomètre KO
// -99 : RAS
//--------------------------------------------------------------------------
int CRemoteTCS34725::loop() {
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
bool CRemoteTCS34725::readMesures() {
  String sCmd = nomEquipement + " " + sMqttCommandMesure;
  onMqttPublish(mqttSubTopicCommand.c_str(), sCmd.c_str());
  return true;
}

unsigned long CRemoteTCS34725::getLastCouleur() const { 
  return lastRgb; 
}
unsigned long CRemoteTCS34725::getLastLuminosite() const { 
  return muiLastLux; 
}
unsigned long CRemoteTCS34725::getLastLuminositeBrut() const { 
  return muiLastLuxBrut; 
}
int CRemoteTCS34725::getLastLedStatus() const {
  return mbLastEtatOnOff;
}

//
// Couleur valide si de la forme 0xFFRRGGBB
//
bool CRemoteTCS34725::couleurValide(unsigned long coul) {
  unsigned short msb = (coul & 0xFF000000)>>24;

  return msb == 0xFF ? true : false;

}
//
// Luminosité valide si de la forme 0xFFFFFFXX
//
bool CRemoteTCS34725::lumValide(unsigned long lum) {
  unsigned long msb = (lum & 0xFFFFFF00)>>8;

  return msb == 0x00FFFFFF ? true : false;

}
void CRemoteTCS34725::printMesures() {
  /*if (couleurValide(lastRgb) && lumValide(muiLastLux)) {
    Serial.printf("void CRemoteTCS34725::printMesures() - Couleur : 0x%lx Luminosité : 0x%lx Luminosité Brute : 0x%lx\n", lastRgb, muiLastLux, muiLastLuxBrut);
  } else {
    Serial.println("Aucune température valide");
  }*/

  DBG(DBG_CAPTEURS, "R=0x%x=%d G=0x%x=%d B=0x%x=%d lux brut=0x%x=%d lux=0x%x=%d\n", muiLastR, muiLastR, muiLastG, muiLastG, muiLastB, muiLastB, muiLastLuxBrut, muiLastLuxBrut, muiLastLux, muiLastLux);

}

void CRemoteTCS34725::loadFromNVS() {
  prefs.begin(nvs_namespace, true);
  mucCapteurID = prefs.getUShort((mPrefixNVS+"id").c_str(), 0);
  mulWatchdogIntervalle = prefs.getLong((mPrefixNVS+"wdog").c_str(), mulWatchdogDefaultIntervalle);
  prefs.end();

  CEquipementBase::loadFromNVS();
   
}
void CRemoteTCS34725::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putUShort((mPrefixNVS+"id").c_str(), mucCapteurID);
  prefs.putLong((mPrefixNVS+"wdog").c_str(), mulWatchdogIntervalle);
  prefs.end();

  CEquipementBase::saveToNVS();
}


void CRemoteTCS34725::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"id").c_str())) mucCapteurID = server.arg((mPrefixNVS+"id")).toInt();
  if (server.hasArg((mPrefixNVS+"wdog").c_str())) mulWatchdogIntervalle = server.arg((mPrefixNVS+"wdog")).toInt();

  CEquipementBase::loadFromWebServer (server);
}

void CRemoteTCS34725::print() const {

  DBG(DBG_CAPTEURS, "==========================================================\n");
  DBG(DBG_CAPTEURS, "     Nom              : %s\n", nomEquipement.c_str());
  DBG(DBG_CAPTEURS, "     ID                 : %d\n", mucCapteurID);
  DBG(DBG_CAPTEURS, "     Actif            : %s\n", active ? "OUI" : "NON");
  DBG(DBG_CAPTEURS, "     Watchdog         : %ld s\n", mulWatchdogIntervalle);
  DBG(DBG_CAPTEURS, "     MQTTSubTopic     : %s\n", mqttSubTopic.c_str());
  DBG(DBG_CAPTEURS, "     MQTTCmd          : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_CAPTEURS, "     MQTTState        : %s\n", mqttSubTopicState.c_str());
}

void CRemoteTCS34725::setDisplayCallbackCouleur(std::function<void(const String&, unsigned long)> cb) {
        onCouleurChanged = cb;
    }
void CRemoteTCS34725::setDisplayCallbackLuminosite(std::function<void(const String&, unsigned long)> cb) {
        onLuminositeChanged = cb;
    }
void CRemoteTCS34725::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }

void CRemoteTCS34725::handleMqttState(const String& payload) {
  //String cmd = payload;
  //cmd.trim();
  static bool inactifAffiche = false;
  if (!active) {
    if (!inactifAffiche) {
      DBG(DBG_CAPTEURS, "void CRemoteTCS34725::handleMqttState (); - %s inactif\n", nomEquipement.c_str());
      inactifAffiche = true;
    }
    return;
  }
  inactifAffiche = false;

  ParsedMqttMessage msg;
  msg.parse(payload);
  int n = msg.miTailleMesure;
  //Serial.printf("void CRemoteTCS34725::handleMqttState(const String& payload) - paylod = %s - Nb de champs = %d\n", payload.c_str(), n);
  
  if (n > 0) {
    //msg.printDebug();

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

    if (premiereMesure == "COUL") {
      if (onCouleurChanged != nullptr) {
        //Serial.printf("------------------------------------------------------------------------------------------------------- Taille : %d\n", msg.mvsMesure.size());
        //for(int i=0; i<msg.mvsMesure.size(); i++) Serial.println(msg.mvsMesure[i]);
       // String luxBrutStr = msg.mvsMesure.back();                 // Dernier élément : Luminosité brute
       // String luxStr = msg.mvsMesure[msg.mvsMesure.size() - 2];  // Luminosité filtrée
        String coulRStr = msg.mvsMesure[1]; // Composante R de la couleur
        String coulGStr = msg.mvsMesure[2]; // Composante G de la couleur
        String coulBStr = msg.mvsMesure[3]; // Composante B de la couleur
        String coulLuxStr = msg.mvsMesure[4]; // Luminosité
        String coulLuxBruteStr = msg.mvsMesure[5]; // Luminosité brute

        muiLastR = coulRStr.toDouble();
        muiLastG = coulGStr.toDouble();
        muiLastB = coulBStr.toDouble();
        muiLastLux = coulLuxStr.toDouble();
        muiLastLuxBrut = coulLuxBruteStr.toDouble();
          
        //lastRgb = coulRStr.toDouble();
        //lastLuxBrut = luxBrutStr.toDouble();
        //lastLux = luxStr.toDouble();
        //unsigned char b = lastRgb & 0xFF;
        //unsigned char g = (lastRgb>>8) & 0xFF;
        //unsigned char r = (lastRgb>>16) & 0xFF;
        //unsigned char ctrl = (lastRgb>>24) & 0xFF;
        //Serial.println("=============================================================================================================");
        //Serial.printf("R=0x%x=%d G=0x%x=%d B=0x%x=%d lux brut=0x%x=%d lux=0x%x=%d\n", muiLastR, muiLastR, muiLastG, muiLastG, muiLastB, muiLastB, muiLastLuxBrut, muiLastLuxBrut, muiLastLux, muiLastLux);
//        Serial.printf("ctrl=0x%x=%d r=0x%x=%d g=0x%x=%d b=0x%x=%d lux brut=0x%lx=%ld lux=0x%lx=%ld\n", ctrl, ctrl, r, r, g, g, b, b, lastLuxBrut, lastLuxBrut, lastLux, lastLux);
        //Serial.println("=============================================================================================================");
        /*if (ctrl == 0xFF) {// Couleur valide
          //--------------------------------------------------------------------------------------------
          // Mettre ici le test pour valider l'allumage ou l'extincion de la led rouge
          // Ce test doit se faire normalement au niveau du capteur, mais c'est plus pratique
          // ici en phase de développement.
          // Dès que ça fonctionne, on transporte le code dans le capteur.
          // Il faudrait faire une calibration. Noter les valeurs de R et de Lux brut pour un ON et pour un OFF
          // Ou les mettre dans la page Web pour le moment
          //--------------------------------------------------------------------------------------------
          /*unsigned char R_ON = 210;
          unsigned char R_OFF = 99; 
          unsigned char R_Seuil = (R_ON + R_OFF)/2; 
          unsigned long luxBrut_ON = 1563;
          unsigned long luxBrut_OFF = 1206;
          unsigned long luxBrut_Seuil = (luxBrut_ON + luxBrut_OFF)/2;
          if ( lastLuxBrut > luxBrut_Seuil && r > R_Seuil) { // On est au-dessus des seuils ==> Led rouge allumée
            onCouleurChanged(msg.msExpediteur, TFT_RED); //lastRgb);
          }
            else onCouleurChanged(msg.msExpediteur, TFT_GREEN); //lastRgb);
          
        }*/
          //--------------------------------------------------------------------------------------------
        //Serial.printf("void CRemoteTCS34725::handleMqttState() - Appel à remonteCouleurParMqtt()\n");
        //remonteCouleurParMqtt(); // Les CYD distants doivent être informés
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement + " " + premiereMesure);
      }
    } 
    else if (premiereMesure == "LUX") {
      if (onLuminositeChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        muiLastLux = valStr.toDouble();
        //unsigned char lx = muiLastLux; // & 0xFF;
        //unsigned long ctrl = lastLux && 0xFFFFFF00;
      //  if (ctrl == 0xFFFFFF00) // Luminosité valide
          onLuminositeChanged(msg.msExpediteur, muiLastLux);
        //Serial.printf("void CRemoteTCS34725::handleMqttState() - Appel à remonteLuminositeParMqtt()\n");
        //remonteLuminositeParMqtt(); // Les CYD distants doivent être informés
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement + " " + premiereMesure);
      }
    } 
    else if (premiereMesure == "DEL") {
      if (onCouleurChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        mbLastEtatOnOff = valStr.toInt();
        onCouleurChanged(msg.msExpediteur, mbLastEtatOnOff);// ? TFT_RED : TFT_GREEN);
        //Serial.printf("void CRemoteTCS34725::handleMqttState() - mbLastEtatOnOff : %d\n", mbLastEtatOnOff);
        //remonteLuminositeParMqtt(); // Les CYD distants doivent être informés
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement + " " + premiereMesure);
      }
    } 
    #ifndef __LOCAL_MODE__
    else if (premiereMesure == "COULR") {
      if (onCouleurChanged != nullptr) {
        String coulRStr = msg.mvsMesure[1]; // Composante R de la couleur
        String coulGStr = msg.mvsMesure[2]; // Composante G de la couleur
        String coulBStr = msg.mvsMesure[3]; // Composante B de la couleur
        String coulLuxStr = msg.mvsMesure[4]; // Luminosité
        String coulLuxBruteStr = msg.mvsMesure[5]; // Luminosité brute

        muiLastR = coulRStr.toDouble();
        muiLastG = coulGStr.toDouble();
        muiLastB = coulBStr.toDouble();
        muiLastLux = coulLuxStr.toDouble();
        muiLastLuxBrut = coulLuxBruteStr.toDouble();
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
      }
    } 
    else if (premiereMesure == "LUXR") {
      if (onLuminositeChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        muiLastLux = valStr.toDouble();
        //unsigned char lx = muiLastLux; // & 0xFF;
        //unsigned long ctrl = lastLux && 0xFFFFFF00;
      //  if (ctrl == 0xFFFFFF00) // Luminosité valide
          onLuminositeChanged(msg.msExpediteur, muiLastLux);
        //Serial.printf("void CRemoteTCS34725::handleMqttState() - Appel à remonteLuminositeParMqtt()\n");
        //remonteLuminositeParMqtt(); // Les CYD distants doivent être informés
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
      }
    } 
    else if (premiereMesure == "DELR") {
      if (onCouleurChanged != nullptr) {
        String valStr = msg.mvsMesure.back();
        mbLastEtatOnOff = valStr.toInt();
        onCouleurChanged(msg.msExpediteur, mbLastEtatOnOff);// ? TFT_RED : TFT_GREEN);
        //Serial.printf("void CRemoteTCS34725::handleMqttState() - mbLastEtatOnOff : %d\n", mbLastEtatOnOff);
        //remonteLuminositeParMqtt(); // Les CYD distants doivent être informés
      }
      else {
        DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement + " " + premiereMesure);
      }
    } 
    #endif
  }

}

// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale ou lorsqu'une 
// mesure arrive par RF
bool CRemoteTCS34725::remonteCouleurParMqtt() {
    if (onMqttPublish == nullptr) return false;
    bool ret = false;
    CMyDateTime mDateTime;
    float temp = lastRgb; 
    String sValTemp = nomEquipement + " COULR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, 1);
    sValTemp += " FORCE";
    //Serial.println("CRemoteTCS34725::publieSurMqtt() Temp : " + sValTemp);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sValTemp.c_str()) == 0);
    return ret;
} 

// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale ou lorsqu'une 
// mesure arrive par RF
bool CRemoteTCS34725::remonteLuminositeParMqtt() {
    if (onMqttPublish == nullptr) return false;
    bool ret = false;
    CMyDateTime mDateTime;
    //unsigned int lux = lastLux; 
    String sValLux = nomEquipement + " LUXR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(muiLastLux);
    sValLux += " FORCE";
    //Serial.println("CRemoteTCS34725::publieSurMqtt() Lux  : " + sValLux);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sValLux.c_str()) == 0);
    return ret;
} 


// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale
bool CRemoteTCS34725::remonteStatusParMqtt() {
    if (onMqttPublish == nullptr) return false;
    bool ret = false;
    ret = remonteCouleurParMqtt();
    if (ret) ret = remonteLuminositeParMqtt();
    /*CMyDateTime mDateTime;
    float temp = lastRgb; 
    float hum = lastLux; 
    String sValTemp = nomEquipement + " COULR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, HEX);
    sValTemp += " FORCE";
    String sValLux = nomEquipement + " LUXR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(hum, HEX);
    sValLux += " FORCE";
    Serial.println("CRemoteTCS34725::publieSurMqtt() Coul : " + sValTemp);
    Serial.println("CRemoteTCS34725::publieSurMqtt() Lux  : " + sValLux);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sValTemp.c_str()) == 0);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sValLux.c_str()) == 0);*/
    return ret;
} 

void CRemoteTCS34725::handleMqttCommand(const String& payload) {
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
int CRemoteTCS34725::handleRCSwitchCode(unsigned long code) {
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
      DBG(DBG_CAPTEURS, "CRemoteTCS34725::handleRCSwitchCode - mauvais nombre de mesures (%d). Attendues : %d\n", sth.nbMesures, NB_CODES_RCS);
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
        DBG(DBG_CAPTEURS, "CRemoteTCS34725::handleRCSwitchCode - mauvais ID : %d vs capteurID : %d\n", sth.id, mucCapteurID);
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
      //Serial.printf("CRemoteTCS34725::handleRCSwitchCode - CRC calculé : 0x%x = %d\n", crc_calcule, crc_calcule);
      if (crc_calcule == stf.crc8) {
        //Serial.printf("CRemoteTCS34725::handleRCSwitchCode - CRC 0x%x = %d valide.\n", stf.crc8, stf.crc8);
        DBG(DBG_CAPTEURS, "================================================= MESURE VALIDEE =================================================\n");
      }
      else {
        DBG(DBG_CAPTEURS, "------------------------------------------------- CRC INVALIDE -------------------------------------------------\n");
        DBG(DBG_CAPTEURS, "CRemoteTCS34725::handleRCSwitchCode - Mauvais CRC. CRC calculé : 0x%x = %d CRC reçu : 0x%x = %d\n", crc_calcule, crc_calcule, stf.crc8, stf.crc8);
        return -3; // A décommenter ultérieurement
      }

      decode(mStMesures, mulCodes, NB_MESURES);
      for (int i=0; i<NB_MESURES; i++) {
        // Envoi pour affichage des valeurs
        switch(mStMesures[i].type) {
          case MESURE_TEMPERATURE :
            if (onCouleurChanged != nullptr) {
              lastRgb = mStMesures[i].val;
              onCouleurChanged(nomEquipement, lastRgb);
              //Serial.printf("void CRemoteTCS34725::handleRCSwitchCode() - Appel à remonteCouleurParMqtt()\n");
              remonteCouleurParMqtt(); // Les CYD distants doivent être informés
            }
            else {
              DBGLN(DBG_CAPTEURS, "Aucun callback défini pour " + nomEquipement);
            }

          break;
          case MESURE_HUMIDITE :
            if (onLuminositeChanged != nullptr) {
              muiLastLux = mStMesures[i].val;
              onLuminositeChanged(nomEquipement, muiLastLux);
              //Serial.printf("void CRemoteTCS34725::handleRCSwitchCode() - Appel à remonteCouleurParMqtt()\n");
              remonteLuminositeParMqtt(); // Les CYD distants doivent être informés
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


String CRemoteTCS34725::getHTML() {
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