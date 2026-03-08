#include "global.h"
#include "MyDS18B20.h"
#include "MyDateTime.h"
#ifdef __LOCAL_MODE__
#ifdef __LOCAL_DS18B20__

int MyDS18B20::begin(const String pref) {
  // Dû à CEquipementBase 2 lignes
  nomEquipement = "ThChRdc";
  mqttSubTopic = "thermometre";
  
  mPrefixNVS = pref;
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
    ret = readAndPublish(bForce); // Lit et publie sur MQTT
  }  
  return ret;
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
      publieSurMqtt(force); // On publie sur Mqtt
      ret = 1;
    } 
    else
      ret = 0; // température inchangée
  }
  else {
    ret = -1;
  }
  return ret;
}
//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message MQTT provient d'un forçage de mesure.
// A supprimer à terme
//
bool MyDS18B20::publieSurMqtt(bool force/*=false*/) {
    float temp = getLastTemperature(); //round(10*ds18b20.getLastTemperature())/10.0; // On arrondit à une décimale
    CMyDateTime mDateTime;
    String sVal = nomEquipement + " Temp " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, 1);
    if (force) sVal += " FORCE";
    Serial.println("MyDS18B20::publieSurMqtt() : " + sVal);
    bool ret = onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str());
    return ret;
}
void MyDS18B20::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }

// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale
bool MyDS18B20::remonteStatusParMqtt() {
  float temp = getLastTemperature(); //round(10*ds18b20.getLastTemperature())/10.0; // On arrondit à une décimale
  CMyDateTime mDateTime;
  String sVal;
  if (active) {
    sVal = nomEquipement + " TEMPR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, 1);
    sVal += " FORCE";
  }
  else {
    sVal = nomEquipement + " INACTIFR " + mDateTime.getDate() + " " + mDateTime.getTime();
  }
  Serial.println("MyDS18B20::remonteStatusParMqtt() : " + sVal);
  bool ret = onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str());
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

float MyDS18B20::getLastTemperature() const { 
  return lastTempC; 
}

void MyDS18B20::printTemperature() const {
  if (lastTempC != -127.0) {
    Serial.printf("void MyDS18B20::printTemperature() - Température : %.2f °C\n", lastTempC);
  } else {
    Serial.println("Aucune température valide");
  }
}

void MyDS18B20::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  mucPin = prefs.getUShort((mPrefixNVS+"pin").c_str(), DEFAULT_DS18B20_PIN);
  mulIntervalleMesure = prefs.getLong((mPrefixNVS+"inter").c_str(), mulDefaultIntervalleMesure);
  mulIntervalleForcageRemonteeMesure = prefs.getLong((mPrefixNVS+"force").c_str(), mulDefaultIntervalleForcageRemonteeMesure);
  
  prefs.end();

  CEquipementBase::loadFromNVS();
   
}
void MyDS18B20::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  // Thermomètre
  prefs.putUShort((mPrefixNVS+"pin").c_str(), mucPin);
  prefs.putLong((mPrefixNVS+"inter").c_str(), mulIntervalleMesure);
  prefs.putLong((mPrefixNVS+"force").c_str(), mulIntervalleForcageRemonteeMesure);

  prefs.end();

  CEquipementBase::saveToNVS();
}

void MyDS18B20::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"pin").c_str())) mucPin = server.arg((mPrefixNVS+"pin").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"inter").c_str())) mulIntervalleMesure = server.arg((mPrefixNVS+"inter")).toInt();
  if (server.hasArg((mPrefixNVS+"force").c_str())) mulIntervalleForcageRemonteeMesure = server.arg((mPrefixNVS+"force")).toInt();
  CEquipementBase::loadFromWebServer (server);
}

void MyDS18B20::print() const {

  Serial.printf("     Nom                : %s\n", nomEquipement);
  Serial.printf("     GPIO               : %d\n", mucPin);
  Serial.printf("     Actif              : %s\n", active ? "OUI" : "NON");
  Serial.printf("     Intervalle mesures : %ld s\n", mulIntervalleMesure);
  Serial.printf("     Forcage remontée   : %ld s\n", mulIntervalleForcageRemonteeMesure);
  Serial.printf("     MQTTSubTopic       : %s\n", mqttSubTopic.c_str());
  Serial.printf("     MQTTCmd            : %s\n", mqttSubTopicCommand.c_str());
  Serial.printf("     MQTTState          : %s\n", mqttSubTopicState.c_str());
}

String MyDS18B20::getHTML() {
  String html = "";
  html =  "<h2>Configuration du thermomètre " + nomEquipement + "</h2>"
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
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=\"th_subtopic\" value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}
#endif
#endif