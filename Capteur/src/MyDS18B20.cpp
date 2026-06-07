#include "MyDS18B20.h"
#include "MyDateTime.h"


int MyDS18B20::begin(const String pref) {
  mPrefixNVS = pref;
  mbMesureRemontee = false;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  sensors.begin();
  Serial.print("DS18B20 trouvé : ");
  uint8_t nbDevices = sensors.getDeviceCount();
  if (nbDevices == 0) {
    return -1;
  } 
  Serial.println();
  // Premier relevé
  if (readTemperature()) {
    printTemperature();
  } else {
    Serial.println("DS18B20 non détecté !");
    return -2;
  }
  return 0;
}

//--------------------------------------------------------------------------
//  MyDS18B20::loop()
//
// Retour
// 1 : Température lue et changée 
// 0 : Température lue mais inchangée
// -1 : Echec de lecture de température
// -9 : Thermomètre inactif
//--------------------------------------------------------------------------
int MyDS18B20::loop() {
  int ret = -99;
  if (!active) return -9;
  // Exemple : lecture toutes les 10 secondes
  static unsigned long lastRead = 0;
  static unsigned long lastForcageRemontee = 0;
  bool bForce=false;
  if (millis() - lastForcageRemontee > mulIntervalleForcageRemonteeMesure*1000 /**60 */) { // mulIntervalleForcageRemonteeMesure en secondes pour les tests
    lastForcageRemontee = millis();
    bForce = true;
  }
  //static float lastTemperature=0;
  if ((millis() - lastRead > mulIntervalleMesure * 1000 /** 60 */) || (lastRead == 0) || bForce){ // mulIntervalleMesure en secondes pour les tests
    lastRead = millis();
    if (!active) return -9;
    ret = readAndPublish(bForce); // Lit et publie sur MQTT
  }  
  return ret;
}

//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message MQTT provient d'un forçage de mesure.
// A supprimer à terme
//
bool MyDS18B20::publieSurMqtt(bool force/*=false*/) {
    if (onMqttPublish == nullptr) return false;

    float temp = getLastTemperature(); //round(10*ds18b20.getLastTemperature())/10.0; // On arrondit à une décimale
    String sDate = (mDateTime != nullptr) ? mDateTime->getDate() : "DATE";
    String sTime = (mDateTime != nullptr) ? mDateTime->getTime() : "TIME";
    String sVal = nomEquipement + " Temp " + sDate + " " + sTime + " " + String(temp, 1);
    if (force) sVal += " FORCE";
    Serial.println("MyDS18B20::publieSurMqtt() : " + sVal);
    bool ret = (onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str()) == 0);
    return ret;
}

//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message provient d'un forçage de mesure.
//
bool MyDS18B20::publieParLoraP2P(bool force/*=false*/) {
    if (onLoraP2PPublish == nullptr) return false;

    float temp = getLastTemperature(); //round(10*ds18b20.getLastTemperature())/10.0; // On arrondit à une décimale
    String sDate = (mDateTime != nullptr) ? mDateTime->getDate() : "DATE";
    String sTime = (mDateTime != nullptr) ? mDateTime->getTime() : "TIME";
    String sVal = mqttSubTopicState + " " +  nomEquipement + " Temp " + sDate + " " + sTime + " " + String(temp, 1);
    if (force) sVal += " FORCE";
    Serial.println("MyDS18B20::publieParLoraP2P() : " + sVal);
    bool ret = (onLoraP2PPublish(sVal.c_str()) == 0);
    return ret;
}

bool MyDS18B20::readTemperature() {
  sensors.requestTemperatures();
  delay(20);  // Petit délai pour conversion
  float temp = sensors.getTempCByIndex(0);

  if ( temp == DEVICE_DISCONNECTED_C || temp == -127.0) {
    Serial.println("Erreur : DS18B20 déconnecté ou erreur");
    return false;
  }
  newTempC = temp;
  return true;
}

//--------------------------------------------------------------------------
//  MyDS18B20::readAndPublish()
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
int MyDS18B20::readAndPublish(bool force/*=false*/) {
  int ret = 0;
  if (!active) return -9;
  if (readTemperature()) {
    if ( (round(10*newTempC) != round(10*lastTempC)) || force) { // On compare à la décimale près
      lastTempC = newTempC;
      printTemperature(); // On affiche sur le moniteur série
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
    //Serial.println("ds18b20.readTemperature() - échec");
    ret = -1;
  }
  return ret;
}

float MyDS18B20::getLastTemperature() const { 
  return lastTempC; 
}

void MyDS18B20::printTemperature() const {
  if (lastTempC != -127.0) {
    Serial.printf("Température : %.2f °C\n", lastTempC);
  } else {
    Serial.println("Aucune température valide");
  }
}

void MyDS18B20::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Thermomètre");
#ifdef CAPTEUR_DS18B20
  mucPin = prefs.getUShort((mPrefixNVS+"pin").c_str(), DS18B20_PIN);
#else
  mucPin = prefs.getUShort((mPrefixNVS+"pin").c_str(), 0);
#endif
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "thermometre/");
  active = prefs.getBool((mPrefixNVS+"active").c_str(), true);
  mulIntervalleMesure = prefs.getLong((mPrefixNVS+"inter").c_str(), mulDefaultIntervalleMesure);
  mulIntervalleForcageRemonteeMesure = prefs.getLong((mPrefixNVS+"force").c_str(), mulDefaultIntervalleForcageRemonteeMesure);
  
  // On forme les subtopic MQTT
  domotique_prefix = prefs.getString("domo_prefix", default_domotique_topic_prefix);
  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";

  prefs.end();
   
}
void MyDS18B20::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  // Thermomètre
  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putUShort((mPrefixNVS+"pin").c_str(), mucPin);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putBool((mPrefixNVS+"active").c_str(), active);
  prefs.putLong((mPrefixNVS+"inter").c_str(), mulIntervalleMesure);
  prefs.putLong((mPrefixNVS+"force").c_str(), mulIntervalleForcageRemonteeMesure);

  prefs.end();
}

void MyDS18B20::setActive(bool state) {
  active = state;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"active").c_str(), state);
  prefs.end();

  active = state;
}

void MyDS18B20::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"pin").c_str())) mucPin = server.arg((mPrefixNVS+"pin").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"inter").c_str())) mulIntervalleMesure = server.arg((mPrefixNVS+"inter")).toInt();
  if (server.hasArg((mPrefixNVS+"force").c_str())) mulIntervalleForcageRemonteeMesure = server.arg((mPrefixNVS+"force")).toInt();
}

void MyDS18B20::print() const {

  Serial.printf("     Nom                : %s\n", nomEquipement.c_str());
  Serial.printf("     GPIO               : %d\n", mucPin);
  Serial.printf("     Actif              : %s\n", active ? "OUI" : "NON");
  Serial.printf("     Intervalle mesures : %ld min\n", mulIntervalleMesure);
  Serial.printf("     Forcage remontée   : %ld min\n", mulIntervalleForcageRemonteeMesure);
  //Serial.printf("     Equipement local : %s\n", bLocal ? "OUI" : "NON");
  Serial.printf("     MQTTSubTopic       : %s\n", mqttSubTopic.c_str());
  Serial.printf("     MQTTCmd            : %s\n", mqttSubTopicCommand.c_str());
  Serial.printf("     MQTTState          : %s\n", mqttSubTopicState.c_str());
}

String MyDS18B20::getHTML() {
  String html = "";
  html =  "<h2>Configuration de " + nomEquipement + "</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div><label>GPIO</label><input type=\"text\" name=" + (mPrefixNVS+"pin") + " value=\"" + mucPin + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active") + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Intervalle mesure</label><input type=\"text\" name=" + (mPrefixNVS+"inter") + " value=\"" + mulIntervalleMesure + "\"></div>"
        "<div><label>Intervalle forçage remontée</label><input type=\"text\" name=" + (mPrefixNVS+"force") + " value=\"" + mulIntervalleForcageRemonteeMesure + "\"></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic") + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}

void MyDS18B20::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }
void MyDS18B20::setLoraP2PPublishCallback(std::function<int(const char*)> cb) {
    onLoraP2PPublish = cb;
}

//
//
// Retour
// 0 : RAS
// -1 : Mauvais nom d'équipement
// -2 : Commande incomplète
// 
int MyDS18B20::handleMqttCommand(const String& payload) {
  int ret = 0;
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

    MQTT_COMMAND st;
    int idx = 0;
    Serial.println("=== MyDS18B20::handleMqttCommand ===");
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