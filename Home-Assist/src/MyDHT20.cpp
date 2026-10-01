#include "global.h"
#include "MyDHT20.h"
#include "MyDateTime.h"
#ifdef __LOCAL_MODE__
#ifdef __LOCAL_DHT20__

int MyDHT20::begin(const String pref) {
  // Dû à CEquipementBase 2 lignes
  nomEquipement = "ThCYD";
  mqttSubTopic = "thermometre";

  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  // Diagnostic : niveaux des lignes au repos (doivent être à 1 grâce aux pull-up)
  pinMode(mucSdaPin, INPUT);
  pinMode(mucSclPin, INPUT);
  int niveauSda = digitalRead(mucSdaPin);
  int niveauScl = digitalRead(mucSclPin);
  if (!Wire.begin(mucSdaPin, mucSclPin)) {
    DBG(DBG_CAPTEURS, "DHT20 : Wire.begin(%d, %d) échec\n", mucSdaPin, mucSclPin);
    return -1;
  }
  if (!dht.begin()) {
    DBG(DBG_CAPTEURS, "DHT20 non détecté ! SDA(%d)=%d SCL(%d)=%d au repos\n", mucSdaPin, niveauSda, mucSclPin, niveauScl);
    // Code retour I2C : 2 = pas d'acquittement de l'adresse, 4/5 = bus bloqué ou timeout
    Wire.beginTransmission(0x38);
    DBG(DBG_CAPTEURS, "DHT20 : endTransmission(0x38) = %d\n", Wire.endTransmission());
    DBG(DBG_CAPTEURS, "DHT20 : scan I2C :");
    int nb = 0;
    for (uint8_t adr = 1; adr < 127; adr++) {
      Wire.beginTransmission(adr);
      if (Wire.endTransmission() == 0) {
        DBG(DBG_CAPTEURS, " 0x%02X", adr);
        nb++;
      }
    }
    DBG(DBG_CAPTEURS, nb ? "\n" : " aucun périphérique\n");
    return -1;
  }
  DBG(DBG_CAPTEURS, "DHT20 trouvé\n");
  mbInitialise = true;
  // Premier relevé
  if (readTemperature()) {
    printTemperature();
  } else {
    DBG(DBG_CAPTEURS, "DHT20 : première lecture en échec !\n");
    return -2;
  }
  return 0;
}

//--------------------------------------------------------------------------
//  MyDHT20::loop()
//
// Retour
// 1 : Température lue et changée
// 0 : Température lue mais inchangée
// -1 : Echec de lecture de température
// -9 : Thermomètre inactif
//--------------------------------------------------------------------------
int MyDHT20::loop() {
  int ret = -99;
  if (!active) return -9;
  static unsigned long lastRead = 0;
  static unsigned long lastForcageRemontee = 0;
  bool bForce=false;
  if (millis() - lastForcageRemontee > mulIntervalleForcageRemonteeMesure*1000 /**60 */) { // mulIntervalleForcageRemonteeMesure en secondes pour les tests
    lastForcageRemontee = millis();
    bForce = true;
  }
  if ((millis() - lastRead > mulIntervalleMesure * 1000 /** 60 */) || (lastRead == 0) || bForce){ // mulIntervalleMesure en secondes pour les tests
    lastRead = millis();
    ret = readAndPublish(bForce); // Lit et publie sur MQTT
  }
  return ret;
}
//--------------------------------------------------------------------------
//  MyDHT20::readAndPublish()
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
int MyDHT20::readAndPublish(bool force/*=false*/) {
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
bool MyDHT20::publieSurMqtt(bool force/*=false*/) {
    float temp = getLastTemperature();
    CMyDateTime mDateTime;
    String sVal = nomEquipement + " Temp " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, 1);
    if (force) sVal += " FORCE";
    DBGLN(DBG_CAPTEURS, "MyDHT20::publieSurMqtt() : " + sVal);
    bool ret = onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str());
    return ret;
}
void MyDHT20::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }

// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un
// nouveau CYD se signale
bool MyDHT20::remonteStatusParMqtt() {
  // Aucune température lue depuis le démarrage : on n'envoie pas une valeur par défaut
  if (active && lastTempC == -127.0) return false;
  float temp = getLastTemperature();
  CMyDateTime mDateTime;
  String sVal;
  if (active) {
    sVal = nomEquipement + " TEMPR " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(temp, 1);
    sVal += " FORCE";
  }
  else {
    sVal = nomEquipement + " INACTIFR " + mDateTime.getDate() + " " + mDateTime.getTime();
  }
  DBGLN(DBG_CAPTEURS, "MyDHT20::remonteStatusParMqtt() : " + sVal);
  bool ret = onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str());
  return ret;
}

bool MyDHT20::readTemperature() {
  if (!mbInitialise) {
    DBG(DBG_CAPTEURS, "Erreur : DHT20 non initialisé\n");
    return false;
  }
  // Le DHT20 impose au moins 1 s entre deux lectures (sinon DHT20_ERROR_LASTREAD)
  unsigned long ecoule = millis() - dht.lastRead();
  if (dht.lastRead() != 0 && ecoule < 1000) delay(1000 - ecoule);

  int status = dht.read();
  if (status != DHT20_OK) {
    DBG(DBG_CAPTEURS, "Erreur : DHT20 lecture en échec (%d)\n", status);
    return false;
  }
  newTempC = dht.getTemperature();
  lastHum = dht.getHumidity();
  return true;
}

float MyDHT20::getLastTemperature() const {
  return lastTempC;
}

float MyDHT20::getLastHumidite() const {
  return lastHum;
}

void MyDHT20::printTemperature() const {
  if (lastTempC != -127.0) {
    DBG(DBG_CAPTEURS, "void MyDHT20::printTemperature() - Température : %.2f °C - Humidité : %.1f %%\n", lastTempC, lastHum);
  } else {
    DBG(DBG_CAPTEURS, "Aucune température valide\n");
  }
}

void MyDHT20::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  mucSdaPin = prefs.getUShort((mPrefixNVS+"sda").c_str(), DEFAULT_DHT20_SDA_PIN);
  mucSclPin = prefs.getUShort((mPrefixNVS+"scl").c_str(), DEFAULT_DHT20_SCL_PIN);
  mulIntervalleMesure = prefs.getLong((mPrefixNVS+"inter").c_str(), mulDefaultIntervalleMesure);
  mulIntervalleForcageRemonteeMesure = prefs.getLong((mPrefixNVS+"force").c_str(), mulDefaultIntervalleForcageRemonteeMesure);

  prefs.end();

  CEquipementBase::loadFromNVS();

}
void MyDHT20::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  // Thermomètre
  prefs.putUShort((mPrefixNVS+"sda").c_str(), mucSdaPin);
  prefs.putUShort((mPrefixNVS+"scl").c_str(), mucSclPin);
  prefs.putLong((mPrefixNVS+"inter").c_str(), mulIntervalleMesure);
  prefs.putLong((mPrefixNVS+"force").c_str(), mulIntervalleForcageRemonteeMesure);

  prefs.end();

  CEquipementBase::saveToNVS();
}

void MyDHT20::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"sda").c_str())) mucSdaPin = server.arg((mPrefixNVS+"sda").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"scl").c_str())) mucSclPin = server.arg((mPrefixNVS+"scl").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"inter").c_str())) mulIntervalleMesure = server.arg((mPrefixNVS+"inter")).toInt();
  if (server.hasArg((mPrefixNVS+"force").c_str())) mulIntervalleForcageRemonteeMesure = server.arg((mPrefixNVS+"force")).toInt();
  CEquipementBase::loadFromWebServer (server);
}

void MyDHT20::print() const {

  DBG(DBG_CAPTEURS, "     Nom                : %s\n", nomEquipement.c_str());
  DBG(DBG_CAPTEURS, "     GPIO SDA / SCL     : %d / %d\n", mucSdaPin, mucSclPin);
  DBG(DBG_CAPTEURS, "     Actif              : %s\n", active ? "OUI" : "NON");
  DBG(DBG_CAPTEURS, "     Intervalle mesures : %ld s\n", mulIntervalleMesure);
  DBG(DBG_CAPTEURS, "     Forcage remontée   : %ld s\n", mulIntervalleForcageRemonteeMesure);
  DBG(DBG_CAPTEURS, "     MQTTSubTopic       : %s\n", mqttSubTopic.c_str());
  DBG(DBG_CAPTEURS, "     MQTTCmd            : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_CAPTEURS, "     MQTTState          : %s\n", mqttSubTopicState.c_str());
}

String MyDHT20::getHTML() {
  String html = "";
  html =  "<h2>Configuration du thermomètre " + nomEquipement + " (DHT20)</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div><label>GPIO SDA</label><input type=\"text\" name=" + (mPrefixNVS+"sda") + " value=\"" + mucSdaPin + "\"></div>"
        "<div><label>GPIO SCL</label><input type=\"text\" name=" + (mPrefixNVS+"scl") + " value=\"" + mucSclPin + "\"></div>"
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
