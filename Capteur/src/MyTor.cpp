#include <Arduino.h>
#include "MyTor.h"
#ifdef FLOTTEUR_VERTICAL
bool CTor::setup(const String pref) {
  mPrefixNVS = pref;
  mbMesureRemontee = false;
  //mucNbMesuresRemontees = 0;
  //mucNbMesuresRemonteesAvantDeepSleep = DEFAULT_NB_MESURES_REMONTEES_AVANT_DEEP_SLEEP;
  mbForce=true; // On force une remontée à l'initialisation

  // On charge les infos de config depuis le NVS
  loadFromNVS();

  pinMode(mucPin, mucPinMode);
  _initialized = true;

  return true;
}

//--------------------------------------------------------------------------
//  MyDS18B20::loop()
//
// Retour
// 1 : Mesure lue et changée 
// 0 : Mesure lue mais inchangée
// -1 : Echec de lecture de Mesure
// -9 : Capteur inactif
//--------------------------------------------------------------------------
int CTor::loop() {
  int ret = -99;
  if (!active) return -9;
  static bool bEnvoiEnCours = false; // Falg utile s'il faut envoyer plusieurs fois (si mucNbEnvois > 1)
  static unsigned char nbEnvoisAFaire = mucNbEnvois;
  bool bMesureNormale = false;
  static unsigned long ulCompteurTempsEnvoi = 0L;

  // Exemple : lecture toutes les 10 secondes
  static unsigned long lastRead = 0L;
  static unsigned long lastForcageRemontee = 0L;
  if (millis() - lastForcageRemontee > mulIntervalleForcageRemonteeMesure*1000 /**60 */) { // mulIntervalleForcageRemonteeMesure en secondes pour les tests
    lastForcageRemontee = millis();
    mbForce = true;
  //  nbEnvoisAFaire = mucNbEnvois;
  //  bEnvoiEnCours = true;
  }
  //static float lastTemperature=0;
  if (millis() - lastRead > mulIntervalleMesure * 1000 /** 60 */){ // mulIntervalleMesure en secondes pour les tests
    lastRead = millis();
    bMesureNormale = true;
  }  
  if (bMesureNormale || (lastRead == 0) || mbForce /*|| bEnvoiEnCours*/){ // mulIntervalleMesure en secondes pour les tests
    if (!bEnvoiEnCours) {           // Si l'envoi n'a pas encore été effectué, ...
      nbEnvoisAFaire = mucNbEnvois; // ... on initialise le nb d'envois 
      bEnvoiEnCours = true;         // ... et le flag
    }
  }  
  if (bEnvoiEnCours) {
    if (nbEnvoisAFaire > 0) {
      if (millis() - ulCompteurTempsEnvoi > mulIntervalleEnvoi*1000 || ulCompteurTempsEnvoi == 0L) {
        Serial.printf("CTor::loop() - Nb d'envois restants : %d\n", nbEnvoisAFaire); Serial.flush();
        bool bMesurer = false;
        if (nbEnvoisAFaire == mucNbEnvois) bMesurer = true;
        ret = readAndPublish(mbForce, bMesurer); // Lit et publie sur MQTT
        if (bMesurer) {
          bMesurer = false; 
          lastRead = millis();
          bMesureNormale = false;
        }
        if (mbForce) {
          mbForce = false; 
          lastForcageRemontee = millis(); 
          lastRead = millis();
          bMesureNormale = false;
          bMesureNormale = false;
        }
        nbEnvoisAFaire--;             // On décrémente le nb d'envois
        ulCompteurTempsEnvoi = millis();
      }
    }
    else {
      //mbForce = false;
      //bMesureNormale = false;
      bEnvoiEnCours = false;
    }
    //lastForcageRemontee = millis(); lastRead = millis();
  } // if (bEnvoiEnCours) {

  return ret;
}
int CTor::getLastMesure() const { 
  return miLastVal; 
}

//--------------------------------------------------------------------------
//  CTor::readAndPublish()
//
// L'argument force permet de remonter la Mesure même si elle est
// inchangée.
//
// Retour
// 1 : Mesure lue et changée 
// 0 : Mesure lue mais inchangée
// -1 : Echec de lecture de Mesure
// -9 : Capteur inactif
//--------------------------------------------------------------------------
int CTor::readAndPublish(bool force/*=false*/, bool mesurer/*=false*/) {
  //read();
  //publieSurMqtt(true);
  //return 1;
  int ret = 0;
  if (!active) return -9;
  bool readStatus = false;
  if (mesurer) readStatus = read();
  else readStatus = true;

  Serial.printf("int CTor::readAndPublish() - force = %d - mesurer = %d\n", force, mesurer);

  if (readStatus) {
    //Serial.printf("miNewVal : %d miLastVal : %d \n", miNewVal, miLastVal);
    if ( (miNewVal != miLastVal) || force) { // On compare 
      miLastVal = miNewVal;
      //lastHum = newHum;
      //printTMesure(); // On affiche sur le moniteur série
      // On publie sur Mqtt
      // Si échec de remontée MQTT, mbMesureRemontee sera à false
      // ce qui empêchera le deep sleep
      #ifdef _RCSWITCH_MODE_
      mbMesureRemontee = publieParCC1101(force); 
      //if(publieParCC1101(force)) mucNbMesuresRemontees++; 
      #endif
      #ifdef _LORA_P2P_MODE_
      mbMesureRemontee = publieParLoraP2P(force); 
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
      ret = 0; // mesure inchangée
  }
  else {
    //Serial.println("ds18b20.readTemperature() - échec");
    ret = -1;
  }
  return ret;
}

//--------------------------------------------------------------------------
//  CTor::readAndPublishTEST()
//
// Pour les tests : envoie plusieurs mesures d'affilé
//
// Retour
// 1 : Température lue et changée 
// 0 : Température lue mais inchangée
// -1 : Echec de lecture de mesure
// -9 : Thermomètre inactif
//--------------------------------------------------------------------------
int CTor::readAndPublishTEST(float t, float h) {

  unsigned long codes[6];
  int idx = 0;
  int idCapteur = 1;

  uint16_t temp_raw = constrain((int)((t + 20.0f) * 10.0), 0, 799); // de -20°C à 60 °C à 0.1°C près ==> 81*10 = 810 valeurs. On mappe de 0 à 800
  uint16_t hum_raw  = constrain(h, 0, 100); // de 0 à 63 % mappé de 0 à 100 %
  
  unsigned char temp_raw_LSB = temp_raw & 0x00FF;
  unsigned char temp_raw_MSB = (temp_raw >> 8);
  unsigned char hum_raw_LSB = hum_raw & 0x00FF;
  unsigned char hum_raw_MSB = (hum_raw >> 8);

  codes[idx++] = ((unsigned long)idCapteur << 16) | (4UL << 12) | 0x0FFF;
  // Code 2 : Température (type 0)
  //codes[idx++] = ((unsigned long)mucCapteurID << 16) | (1UL << 12) | (0UL << 8) | temp_raw;
  codes[idx++] = ((unsigned long)idCapteur << 16) | (1UL << 12) | (0UL << 8) | temp_raw_MSB;
  codes[idx++] = ((unsigned long)idCapteur << 16) | (1UL << 12) | (0UL << 8) | temp_raw_LSB;
  codes[idx++] = ((unsigned long)idCapteur << 16) | (2UL << 12) | (1UL << 8) | hum_raw_MSB;
  codes[idx++] = ((unsigned long)idCapteur << 16) | (2UL << 12) | (1UL << 8) | hum_raw_LSB;
  // Code 3 : Humidité (type 1)
  //codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | (1UL << 8) | hum_raw;
  // Code 4 : Fin (ID + CRC + réservé=0)
  uint8_t crc = computeCRC(codes, idx);
  //Serial.printf("CRC Calculé : %d\n", crc);
  codes[idx++] = ((unsigned long)idCapteur << 16) | ((unsigned long)crc << 8);
  
  Serial.printf("======================== MESURE A REMONTER ========================\n");
  Serial.printf("int CTor::readAndPublishTEST - mesure : %.1f Humidité %.1f\n", t, h);
  Serial.printf("CRC Calculé : 0x%lx = %d\n", crc, crc); 
  Serial.printf("Header : 0x%lx\n", codes[0]);
  Serial.printf("Temp MSB : 0x%lx\n", codes[1]);
  Serial.printf("Temp LSB : 0x%lx\n", codes[2]);
  Serial.printf("Humi MSB : 0x%lx\n", codes[3]);
  Serial.printf("Humi LSB : 0x%lx\n", codes[4]);
  Serial.printf("Footer : 0x%lx\n", codes[5]);
  Serial.flush();

  #ifdef _RCSWITCH_MODE_
  mRCSwitch->envoieCode(codes, 6);
  #endif

  
  return 0;


}

//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message MQTT provient d'un forçage de mesure.
// A supprimer à terme
//
bool CTor::publieSurMqtt(bool force/*=false*/) {
    if (onMqttPublish == nullptr) return false;
    bool ret = false;
    float temp = miLastVal; 
    //float hum = lastHum; 
    String sValTemp = nomEquipement + " OnOff " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(temp);
    if (force) sValTemp += " FORCE";
    //String sValHum = nomEquipement + " Hum " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(hum, 1);
    //if (force) sValHum += " FORCE";
    Serial.println("CTor::publieSurMqtt() Temp : " + sValTemp);
    //Serial.println("CTor::publieSurMqtt() Hum  : " + sValHum);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sValTemp.c_str()) == 0);
    //ret = (onMqttPublish(mqttSubTopicState.c_str(), sValHum.c_str()) == 0);
    return ret;
}

//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message provient d'un forçage de mesure.
//
bool CTor::publieParLoraP2P(bool force/*=false*/) {
    if (onLoraP2PPublish == nullptr) return false;
    bool ret = false;
    float temp = miLastVal; 
    //float hum = lastHum; 
//    String sValTemp = mqttSubTopicState + " " +  nomEquipement + " OnOff " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(temp);
    String sValTemp = mqttSubTopicState + " " +  nomEquipement + " OnOff " + "DATE TIME" + " " + String(temp);
    if (force) sValTemp += " FORCE";
    //String sValHum = nomEquipement + " Hum " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(hum, 1);
    //if (force) sValHum += " FORCE";
    Serial.println("CTor::publieParLoraP2P() Temp : " + sValTemp);
    //Serial.println("CTor::publieSurMqtt() Hum  : " + sValHum);
    ret = (onLoraP2PPublish(sValTemp.c_str()) == 0);
    //ret = (onMqttPublish(mqttSubTopicState.c_str(), sValHum.c_str()) == 0);
    return ret;
}

uint8_t CTor::computeCRC(const unsigned long* codes, int count) {
  uint8_t crc = 0;
  for (int i = 0; i < count; i++) {
    crc ^= (codes[i] >> 16) & 0xFF;
    crc ^= (codes[i] >> 8) & 0xFF;
    crc ^= codes[i] & 0xFF;
  }
  return crc;
}

//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message MQTT provient d'un forçage de mesure.
// A supprimer à terme
//
bool CTor::publieParCC1101(bool force/*=false*/) {
    int temp = miLastVal; 
    //float hum = lastHum; 
    int ret = false;
    unsigned long codes[4];
    int idx=0;


    // Codage
    #ifdef TEST_EMISSION_RCS
    static int t=-20;
    temp = (float)t; 
    t++; if (t == 60) t=-20; 
    #endif // TEST_EMISSION_RCS
    
    // Affichage pour comparer avec la réception
    Serial.println("--- Valeurs à envoyer ---");
    Serial.printf("Mesure : %d\n", temp);
    Serial.println("---------------------------------");

    uint16_t temp_raw = temp*0xFFFF; // 0 logic = 0x0000, 1 logic = 0xFFFF
    
    unsigned char temp_raw_LSB = temp_raw & 0x00FF;
    unsigned char temp_raw_MSB = (temp_raw >> 8);
    
    // 2 mesures → 6 codes au total (header + 2 x (mesure MSB + Mesure LSB) + fin)
    // Code 1 : Header (ID + Nb=2 + réservé=0)
//    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | 0UL;
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | 0x0FFF;
    // Code 2 : Température (type 0)
    //codes[idx++] = ((unsigned long)mucCapteurID << 16) | (1UL << 12) | (0UL << 8) | temp_raw;
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (1UL << 12) | (0UL << 8) | temp_raw_MSB;
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (1UL << 12) | (0UL << 8) | temp_raw_LSB;
    //codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | (1UL << 8) | hum_raw_MSB;
    //codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | (1UL << 8) | hum_raw_LSB;
    // Code 3 : Humidité (type 1)
    //codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | (1UL << 8) | hum_raw;
    // Code 4 : Fin (ID + CRC + réservé=0)
    uint8_t crc = computeCRC(codes, idx);
    Serial.printf("CRC Calculé : %d\n", crc);
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | ((unsigned long)crc << 8);

    Serial.printf("Code %d : 0x%06lX\n", 1, codes[0]);
    Serial.printf("Code %d : 0x%06lX\n", 2, codes[1]);
    Serial.printf("Code %d : 0x%06lX\n", 3, codes[2]);
    Serial.printf("Code %d : 0x%06lX\n", 4, codes[5]);
    Serial.println("---------------------------------");
    #ifdef _RCSWITCH_MODE_
    mRCSwitch->envoieCode(codes, 4);
    #endif
    
    ret = true;
    return ret;
}

bool CTor::read() {
    if (!_initialized) {
        Serial.println("CTor::read() - Capteur non initialisé"); 
        return false;
    }
    if (!active) {
        Serial.println("CTor::read() - Capteur inactif"); 
        return false;
    }
    pinMode(mucPin, mucPinMode);
    miNewVal = digitalRead(mucPin);
    // Si on est INPU_PULLUP, il faut inverser
    if (mucPinMode == INPUT_PULLUP) {
      if (miNewVal == LOW) miNewVal = HIGH;
      else miNewVal = LOW;
    }
    return true;
}

// Variante avec retry (utile en low-power, car parfois le premier read échoue)
bool CTor::readWithRetry(uint8_t retries/* = 3*/) {
    for (uint8_t i = 0; i < retries; i++) {
        if (read()) {
            return true;
        }
        delay(50);  // Petit délai entre retries
    }
    return false;
}

void CTor::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
    onMqttPublish = cb;
}

void CTor::setLoraP2PPublishCallback(std::function<int(const char*)> cb) {
    onLoraP2PPublish = cb;
}

void CTor::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Thermomètre");
  mucCapteurID = prefs.getUShort((mPrefixNVS+"id").c_str(), DEFAULT_CAPTEUR_ID);
  //mucSdaPin = prefs.getUShort((mPrefixNVS+"sda").c_str(), DEFAULT_SDA_PIN);
  //mucSclPin = prefs.getUShort((mPrefixNVS+"scl").c_str(), DEFAULT_SCL_PIN);
  mucPin = prefs.getUShort((mPrefixNVS+"pin").c_str(), DEFAULT_TOR_PIN);
  mucPinMode = prefs.getUShort((mPrefixNVS+"pmode").c_str(), DEFAULT_TOR_PULL_UP_DOWN_MODE);
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "thermometre/");
  active = prefs.getBool((mPrefixNVS+"active").c_str(), false);
  mulIntervalleMesure = prefs.getLong((mPrefixNVS+"inter").c_str(), mulDefaultIntervalleMesure);
  mulIntervalleForcageRemonteeMesure = prefs.getLong((mPrefixNVS+"force").c_str(), mulDefaultIntervalleForcageRemonteeMesure);
  mucNbEnvois = prefs.getUShort((mPrefixNVS+"renvois").c_str(), 1);
  mulIntervalleEnvoi = prefs.getLong((mPrefixNVS+"interr").c_str(), mulDefaultIntervalleEnvoi);

  
  // On forme les subtopic MQTT
//  domotique_prefix = prefs.getString("domo_prefix", default_domotique_topic_prefix);
  domotique_prefix = "home/";

  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";

  prefs.end();
   
}
void CTor::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  // Thermomètre
  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putUShort((mPrefixNVS+"id").c_str(), mucCapteurID);
  prefs.putUShort((mPrefixNVS+"pin").c_str(), mucPin);
  prefs.putUShort((mPrefixNVS+"pmode").c_str(), mucPinMode);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putBool((mPrefixNVS+"active").c_str(), active);
  prefs.putLong((mPrefixNVS+"inter").c_str(), mulIntervalleMesure);
  prefs.putLong((mPrefixNVS+"force").c_str(), mulIntervalleForcageRemonteeMesure);
  prefs.putUShort((mPrefixNVS+"renvois").c_str(), mucNbEnvois);
  prefs.putLong((mPrefixNVS+"interr").c_str(), mulIntervalleEnvoi);

  prefs.end();
}

void CTor::setActive(bool state) {
  active = state;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"active").c_str(), state);
  prefs.end();

  active = state;
}

void CTor::setpinMode(unsigned char mode/* = INPUT_PULLUP*/) {
  mucPinMode = mode;
  prefs.begin(nvs_namespace, false);
  prefs.putUShort((mPrefixNVS+"pmode").c_str(), mucPinMode);
  prefs.end();
}

void CTor::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"id").c_str())) mucCapteurID = server.arg((mPrefixNVS+"id")).toInt();
  //if (server.hasArg((mPrefixNVS+"sda").c_str())) mucSdaPin = server.arg((mPrefixNVS+"sda")).toInt();
  //if (server.hasArg((mPrefixNVS+"scl").c_str())) mucSclPin = server.arg((mPrefixNVS+"scl")).toInt();
  if (server.hasArg((mPrefixNVS+"pin").c_str())) mucPin = server.arg((mPrefixNVS+"pin")).toInt();
  if (server.hasArg((mPrefixNVS+"pmode").c_str())) mucPinMode = server.arg((mPrefixNVS+"pmode")).toInt();
  //Serial.printf("void CTor::loadFromWebServer () - mucPinMode  = %d\n", mucPinMode);
  if (mucPinMode != INPUT_PULLUP && mucPinMode != INPUT_PULLDOWN) setpinMode(INPUT_PULLUP);

  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"inter").c_str())) mulIntervalleMesure = server.arg((mPrefixNVS+"inter")).toInt();
  if (server.hasArg((mPrefixNVS+"force").c_str())) mulIntervalleForcageRemonteeMesure = server.arg((mPrefixNVS+"force")).toInt();
  if (server.hasArg((mPrefixNVS+"renvois").c_str())) mucNbEnvois = server.arg((mPrefixNVS+"renvois")).toInt();
  if (server.hasArg((mPrefixNVS+"interr").c_str())) mulIntervalleEnvoi = server.arg((mPrefixNVS+"interr")).toInt();
}

String CTor::getHTML() {
  String html = "";
  html =  "<h2>Configuration de " + nomEquipement + "</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active") + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>ID</label><input type=\"text\" name=" + (mPrefixNVS+"id") + " value=\"" + mucCapteurID + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Pin</label><input type=\"text\" name=" + (mPrefixNVS+"pin") + " value=\"" + mucPin + "\"></div>"
        "<div><label>Mode : " + String(INPUT_PULLUP) + " = PULLUP, " + String(INPUT_PULLDOWN) + " = PULLDOWN</label><input type=\"text\" name=" + (mPrefixNVS+"pmode") + " value=\"" + mucPinMode + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Intervalle mesure</label><input type=\"text\" name=" + (mPrefixNVS+"inter") + " value=\"" + mulIntervalleMesure + "\"></div>"
        "<div><label>Intervalle forçage remontée</label><input type=\"text\" name=" + (mPrefixNVS+"force") + " value=\"" + mulIntervalleForcageRemonteeMesure + "\"></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Nombre de renvois</label><input type=\"text\" name=" + (mPrefixNVS+"renvois") + " value=\"" + mucNbEnvois + "\"></div>"
        "<div><label>Intervalle de renvoi</label><input type=\"text\" name=" + (mPrefixNVS+"interr") + " value=\"" + mulIntervalleEnvoi + "\"></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic") + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}

void CTor::print() const {

  Serial.printf("     Nom                  : %s\n", nomEquipement.c_str());
  Serial.printf("     ID                   : %d\n", mucCapteurID);
  Serial.printf("     Actif                : %s\n", active ? "OUI" : "NON");
  Serial.printf("     Pin                  : %d\n", mucPin);
  Serial.printf("     Mode                 : %s\n", mucPinMode == INPUT_PULLUP ? "INPUT_PULLUP" : "INPUT_PULLDOWN");
  Serial.printf("     Intervalle mesures   : %ld s\n", mulIntervalleMesure);
  Serial.printf("     Forcage remontée     : %ld s\n", mulIntervalleForcageRemonteeMesure);
  Serial.printf("     Nombre de renvois    : %d\n", mucNbEnvois);
  Serial.printf("     Intervalle de renvoi : %ld s\n", mulIntervalleEnvoi);
  Serial.printf("     MQTTSubTopic         : %s\n", mqttSubTopic.c_str());
  Serial.printf("     MQTTCmd              : %s\n", mqttSubTopicCommand.c_str());
  Serial.printf("     MQTTState            : %s\n", mqttSubTopicState.c_str());
}

//
// Retour
// 0 : RAS
// -1 : Mauvais nom d'équipement
// -2 : Commande incomplète
// 
int CTor::handleMqttCommand(const String& payload) {
  int ret = 0;
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

    MQTT_COMMAND_4 st;
    int idx = 0;
    Serial.println("=== CTor::handleMqttCommand ===");
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

      // Appel de la méthode pour lire la mesure et la remonter par Mqtt
      ret = readAndPublish(true);
    }
    return ret;
}

#endif // FLOTTEUR_VERTICAL
