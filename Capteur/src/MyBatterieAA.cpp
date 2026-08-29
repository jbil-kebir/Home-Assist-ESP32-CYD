#include "MyBatterieAA.h"
//#include "MyDateTime.h"


int CBatterieAA::setup(const String pref) {
  mPrefixNVS = pref;
  mbMesureRemontee = false;

  // On charge les infos de config depuis le NVS
  loadFromNVS();
  analogReadResolution(12);  // 12 bits (0–4095) sur ESP32
  // Premier relevé
  if (readTension()) {
    printTension();
  } 
  else {
    Serial.println("Batterie non détectée !");
    return -2;
  }
  return 0;
}

//--------------------------------------------------------------------------
//  CBatterieAA::loop()
//
// Retour
// 1 : Température lue et changée 
// 0 : Température lue mais inchangée
// -1 : Echec de lecture de température
// -9 : Thermomètre inactif
//--------------------------------------------------------------------------
int CBatterieAA::loop() {
  int ret = -99;
  if (!active) return -9;
  // Exemple : lecture toutes les 10 secondes
  static unsigned long lastRead = 0UL;
  static unsigned long lastForcageRemontee = 0UL;
  bool bForce=false;
  if ((millis() - lastForcageRemontee) > (mulIntervalleForcageRemonteeMesure*1000UL)/**60 */) { // mulIntervalleForcageRemonteeMesure en secondes pour les tests
    lastForcageRemontee = millis();
    lastRead = millis(); // On n'a pas besoin de remontée normale.
    bForce = true;
  }
  //static float lastTension=0;
  if ((millis() - lastRead > mulIntervalleMesure * 1000UL /** 60 */) || (lastRead == 0UL) || bForce){ // mulIntervalleMesure en secondes pour les tests
    lastRead = millis();
    //lastForcageRemontee = millis(); 
    if (!active) return -9;
    ret = readAndPublish(bForce); // Lit et publie sur MQTT
    lastForcageRemontee = millis(); 
    bForce = false; // On remet aussi à 0 le compteur de forçage
  }  
  return ret;
}

//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message MQTT provient d'un forçage de mesure.
// A supprimer à terme
//
bool CBatterieAA::publieSurMqtt(bool force/*=false*/) {
    if (onMqttPublish == nullptr) return false;
    float temp = getLastTension(); //round(10*ds18b20.getLastTension())/10.0; // On arrondit à une décimale
    String sEtatBatterie;
    if (temp < mfTensionMin) sEtatBatterie = "DECHARGEE " ;
    else sEtatBatterie = "CHARGEE ";
    String sDate = (mDateTime != nullptr) ? mDateTime->getDate() : "DATE";
    String sTime = (mDateTime != nullptr) ? mDateTime->getTime() : "TIME";
    String sVal = nomEquipement + " Tension " + sEtatBatterie + sDate + " " + sTime + " " + String(temp, 1);
    if (force) sVal += " FORCE";
    Serial.println("CBatterieAA::publieSurMqtt() : " + sVal);
    bool ret = (onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str()) == 0);
    return ret;
}

//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message provient d'un forçage de mesure.
//
bool CBatterieAA::publieParLoraP2P(bool force/*=false*/) {
    if (onLoraP2PPublish == nullptr) return false;
    float temp = getLastTension(); //round(10*ds18b20.getLastTension())/10.0; // On arrondit à une décimale
    String sEtatBatterie;
    if (temp < mfTensionMin) sEtatBatterie = "DECHARGEE " ;
    else sEtatBatterie = "CHARGEE ";
    String sDate = (mDateTime != nullptr) ? mDateTime->getDate() : "DATE";
    String sTime = (mDateTime != nullptr) ? mDateTime->getTime() : "TIME";
    String sVal = mqttSubTopicState + " " +  nomEquipement + " Tension " + sEtatBatterie + sDate + " " + sTime + " " + String(temp, 1);
    if (force) sVal += " FORCE";
    Serial.println("CBatterieAA::publieParLoraP2P() : " + sVal);
    bool ret = (onLoraP2PPublish(sVal.c_str()) == 0);
    return ret;
}


bool CBatterieAA::readTension() {
  float raw = analogRead(mucPin);
  float v_gpio = (raw / 4095.0f) * 3.3f;
  newTension = v_gpio * mCalibre;

  return true;
}

//--------------------------------------------------------------------------
//  CBatterieAA::readAndPublish()
//
// L'argument force permet de remonter la température même si elle est
// inchangée.
//
// Retour
// 1 : Température lue et changée 
// 0 : Température lue mais inchangée
// -1 : Echec de lecture de température
// -9 : Thermomètre inactif
//--------------------------------------------------------------------------
int CBatterieAA::readAndPublish(bool force/*=false*/) {
  int ret = 0;
  if (!active) return -9;
  if (readTension()) {
    if ( (round(100*newTension) != round(100*mLastTension)) || force) { // On compare au 100ème près
      mLastTension = newTension;
      printTension(); // On affiche sur le moniteur série
      // On publie 
      // Si échec de remontée MQTT, mbMesureRemontee sera à false
      // ce qui empêchera le deep sleep
      #ifdef _RCSWITCH_MODE_
      mbMesureRemontee = publieParCC1101(force); 
      //if(publieParCC1101(force)) mucNbMesuresRemontees++; 
      #endif
      #ifdef _LORA_P2P_MODE_
      mbMesureRemontee = publieParLoraP2P(force); 
      //if(publieParCC1101(force)) mucNbMesuresRemontees++; 
      #endif
      #ifdef _WIFI_MODE_
      #ifndef __DESACTIVE_ENVOI_MQTT__
      mbMesureRemontee = publieSurMqtt(force); 
      #endif
      //if(publieParCC1101(force)) mucNbMesuresRemontees++; 
      #endif
      ret = 1;
    } 
    else
      ret = 0; // température inchangée
  }
  else {
    //Serial.println("ds18b20.readTension() - échec");
    ret = -1;
  }
  return ret;
}

float CBatterieAA::getLastTension() const { 
  return mLastTension; 
}

void CBatterieAA::printTension() const {
  if (mLastTension != -.0) {
    Serial.printf("Tension : %.2f V\n", mLastTension);
  } else {
    Serial.println("Aucune tension valide");
  }
}

void CBatterieAA::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Batterie");
  mucPin = prefs.getUShort((mPrefixNVS+"pin").c_str(), BATTERIE_PIN);
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "batt/");
  active = prefs.getBool((mPrefixNVS+"active").c_str(), true);
  mLastTension = prefs.getFloat((mPrefixNVS + "u").c_str(), 0.0);
  mfTensionMin = prefs.getFloat((mPrefixNVS + "umin").c_str(), 0.0);
  mCalibre = prefs.getFloat((mPrefixNVS + "cal").c_str(), mDefaultCalibre);
  mulIntervalleMesure = prefs.getLong((mPrefixNVS+"inter").c_str(), mulDefaultIntervalleMesure);
  mulIntervalleForcageRemonteeMesure = (unsigned long)prefs.getLong((mPrefixNVS+"force").c_str(), mulDefaultIntervalleForcageRemonteeMesure);
  
  // On forme les subtopic MQTT
//  domotique_prefix = prefs.getString("domo_prefix", default_domotique_topic_prefix);
  domotique_prefix = "home/";

  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";

  prefs.end();
   
}
void CBatterieAA::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  // Thermomètre
  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putUShort((mPrefixNVS+"pin").c_str(), mucPin);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putBool((mPrefixNVS+"active").c_str(), active);
  prefs.putFloat((mPrefixNVS + "u").c_str(), mLastTension);
  prefs.putFloat((mPrefixNVS + "umin").c_str(), mfTensionMin);
  prefs.putFloat((mPrefixNVS + "cal").c_str(), mCalibre);
  prefs.putLong((mPrefixNVS+"inter").c_str(), mulIntervalleMesure);
  prefs.putLong((mPrefixNVS+"force").c_str(), mulIntervalleForcageRemonteeMesure);

  prefs.end();
}

void CBatterieAA::setActive(bool state) {
  active = state;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"active").c_str(), state);
  prefs.end();

  active = state;
}

void CBatterieAA::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"pin").c_str())) mucPin = server.arg((mPrefixNVS+"pin").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"u").c_str())) mLastTension = server.arg((mPrefixNVS+"u").c_str()).toFloat();
  if (server.hasArg((mPrefixNVS+"umin").c_str())) mfTensionMin = server.arg((mPrefixNVS+"umin").c_str()).toFloat();
  if (server.hasArg((mPrefixNVS+"cal").c_str())) mCalibre = server.arg((mPrefixNVS+"cal").c_str()).toFloat();
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"inter").c_str())) mulIntervalleMesure = server.arg((mPrefixNVS+"inter")).toInt();
  if (server.hasArg((mPrefixNVS+"force").c_str())) mulIntervalleForcageRemonteeMesure = server.arg((mPrefixNVS+"force")).toInt();
}

void CBatterieAA::print() const {

  Serial.printf("     Nom                : %s\n", nomEquipement);
  Serial.printf("     GPIO               : %d\n", mucPin);
  Serial.printf("     Actif              : %s\n", active ? "OUI" : "NON");
  Serial.printf("     Tension actuelle   : %.0f\n", mLastTension);
  Serial.printf("     Tension min        : %.2f\n", mfTensionMin);
  Serial.printf("     Calibre            : %.2f\n", mCalibre);
  Serial.printf("     Intervalle mesures : %ld s\n", mulIntervalleMesure);
  Serial.printf("     Forcage remontée   : %ld s\n", mulIntervalleForcageRemonteeMesure);
  Serial.printf("     MQTTSubTopic       : %s\n", mqttSubTopic.c_str());
  Serial.printf("     MQTTCmd            : %s\n", mqttSubTopicCommand.c_str());
  Serial.printf("     MQTTState          : %s\n", mqttSubTopicState.c_str());
}

String CBatterieAA::getHTML() {
  String html = "";
  html =  
      "<h2>Configuration de " + nomEquipement + "</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div><label>GPIO</label><input type=\"text\" name=" + (mPrefixNVS+"pin") + " value=\"" + mucPin + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active") + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>";
      "<div class=\"row\">"
        "<div><label>Dernière tension</label><input type=\"text\" name=" + (mPrefixNVS+"u") + " value=\"" + mLastTension + "\"></div>"
        "<div><label>Tension min.</label><input type=\"text\" name=" + (mPrefixNVS+"umin") + " value=\"" + mfTensionMin + "\"></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Intervalle mesure</label><input type=\"text\" name=" + (mPrefixNVS+"inter") + " value=\"" + mulIntervalleMesure + "\"></div>"
        "<div><label>Intervalle forçage remontée</label><input type=\"text\" name=" + (mPrefixNVS+"force") + " value=\"" + mulIntervalleForcageRemonteeMesure + "\"></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Calibre</label><input type=\"text\" name=" + (mPrefixNVS+"cal") + " value=\"" + mCalibre + "\"></div>"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic") + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}

void CBatterieAA::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }
void CBatterieAA::setLoraP2PPublishCallback(std::function<int(const char*)> cb) {
    onLoraP2PPublish = cb;
}

//
//
// Retour
// 0 : RAS
// -1 : Mauvais nom d'équipement
// -2 : Commande incomplète
// 
int CBatterieAA::handleMqttCommand(const String& payload) {
  int ret = 0;
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

    MQTT_COMMAND_2 st;
    int idx = 0;
    Serial.println("=== CBatterieAA::handleMqttCommand ===");
    Serial.println("cmd : " + cmd);
    st.sExpediteur = cmd.substring(0, cmd.indexOf(' ', idx));
    String s = nomEquipement; s.toUpperCase();
    if (st.sExpediteur != s) {
      Serial.println("Mavais nom d'équipement (" + nomEquipement + ") : " + st.sExpediteur);
      return -1; // Mavais nom d'équipement
    }
    idx = cmd.indexOf(' ', idx) + 1;
    if (idx >= cmd.length()) {
      Serial.println("Commande incomplète longueur : " + String(cmd.length()) + " index : " + String(idx));
      return -2; // Commande incomplète
    }
    st.sCommand = cmd.substring(idx, cmd.indexOf(' ', idx));
    idx = cmd.indexOf(' ', idx) + 1;
    if (idx >= cmd.length()) {
      Serial.println("Commande sans argument. RAS");
      ret = 0; // Commande sans argument. RAS
    }
    else {
      if (st.sCommand == "SLEEP") {
        st.sArg = cmd.substring(idx, cmd.indexOf(' ', idx));
        /*if (st.sArg == "ENABLE") {
          Serial.println("SLEEP ENABLE");

        }
        else if (st.sArg == "DISABLE") {
          Serial.println("SLEEP DISABLE");
        }*/
        unsigned long sleep = (unsigned int)st.sArg.substring(0).toDouble();
        Serial.println("Sleep : "+String(sleep));
        if (sleep!=0L) {
          
        }
      }
//      
//      Serial.println("Argument : " + st.sArg);
      ret = 0; // Argument récupéré. RAS
    }
    // Traitement de la commande
    if (st.sCommand == "MESURE") { // Demande de remontée de mesure

      // Appel de la méthode pour lire la température et la remonter par Mqtt
      ret = readAndPublish(true);
    }
    return ret;
}