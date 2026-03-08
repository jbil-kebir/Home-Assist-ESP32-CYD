
#include "MyConfig.h"
#ifdef _RCSWITCH_MODE_
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#endif
#include "driver/adc.h"

//
// Les opérations de cette fonction ne peuvent pas être dans le setup
// car le setup est déclenché avant que le mqtt ne soit valide.
// Par conséquent, cette fonction est appelée plus tard que le setup, 
// lorsque le wifi et le mqtt 
//
void CConfig::traiteReveil() {
 String sMqttMsg;
 // Si réveil après deep sleep
 if (mbWakeFromDeepSleep) {
  mbWakeFromDeepSleep = false;
  mulDateReveil = mDateTime->getAbsoluteSecondes();
  mulNbSecondesDeSommeil = mulDateReveil - mulDateMiseEnSommeil;
  sMqttMsg = nomEquipement + " REVEIL DEEPSLEEP " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(mulNbSecondesDeSommeil); 
  saveToNVS();
  Serial.println(sMqttMsg+String(" seconde. Réveil avec Deep Sleep préalable ")); Serial.flush();
 }
 else {
  sMqttMsg = nomEquipement + " REVEIL OFF " + mDateTime->getDate() + " " + mDateTime->getTime(); 
  Serial.println(sMqttMsg+String(" Réveil sans Deep Sleep préalable ")); Serial.flush();
 }

  #ifdef CAPTEUR_DS18B20
  if (onMqttPublish != nullptr)
    int ret = onMqttPublish(ds18b20->mqttSubTopicState.c_str(), sMqttMsg.c_str());
  #endif  
  #ifdef CAPTEUR_DHT20
  if (onMqttPublish != nullptr)
    int retdht20 = onMqttPublish(dht20->mqttSubTopicState.c_str(), sMqttMsg.c_str());
  #endif  
  
}
void CConfig::setup() {

 loadFromNVS();

 #ifdef DEBUG_NO_DEEP_SLEEP
 mbDeepSleepActive = false;
 //mulSleepDuration = 30;
 //mulWakeDuration = 30;
 //saveToNVS();
 #endif

#ifdef CAPTEUR_DS18B20
if (ds18b20 != nullptr)
  ds18b20->domotique_prefix = domotique_prefix;
#endif
#ifdef CAPTEUR_DHT20
if (dht20 != nullptr)
  dht20->domotique_prefix = domotique_prefix;
#endif
#ifdef FLOTTEUR_VERTICAL
if (mFlotteurVertical != nullptr)
  mFlotteurVertical->domotique_prefix = domotique_prefix;
#endif  
if (mBatterieAA != nullptr)
  mBatterieAA->domotique_prefix = domotique_prefix;

 Serial.printf("CConfig::setup() - Deep sleep : %d - Durée avant : %ld\n", mbDeepSleepActive, mulWakeDuration); Serial.flush();
}

void CConfig::loop() {

  if (mbDeepSleepActive) {
    if (millis() < mulWakeDuration*1000) {
      // Pas encore le temps de dormir
      return;
    }
    // Deep Sleep. On n'y entre que si une mesure a été envoyée
    #ifdef CAPTEUR_DS18B20
    if (ds18b20->mbMesureRemontee)
      enterDeepSleep();
    #endif
    #ifdef CAPTEUR_DHT20
    if (dht20->mbMesureRemontee) {//dht20->mucNbMesuresRemontees >= dht20->mucNbMesuresRemonteesAvantDeepSleep) {
      //Serial.printf("void CConfig::loop() - Nb de mesures remontées : %d\n", dht20->mucNbMesuresRemontees); Serial.flush();
      enterDeepSleep();
      //dht20->mucNbMesuresRemontees = 0; // Juste pour la rigueur
    }
    #endif
  }
}

void CConfig::enterDeepSleep() {
  Serial.printf("Deep sleep pour %lu secondes...\n", mulSleepDuration);
  Serial.flush();           // Attend que le serial soit vide
  delay(100);               // Sécurité

  unsigned int dureeEveil = mDateTime->getAbsoluteSecondes() - mulDateReveil; // Durée de réveil
  String sMqttMsg = nomEquipement + " DEEPSLEEP ON " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(mulSleepDuration) + " Durée éveil : " + String (dureeEveil); 
  Serial.println(sMqttMsg + String(". Deep sleep in few seconds ..."));
  Serial.flush();           // Attend que le serial soit vide
  mbWakeFromDeepSleep = true;
  mulDateMiseEnSommeil = mDateTime->getAbsoluteSecondes();
  saveToNVS();
  // Envoie par MQTT de la durée de deep-sleep à venir
  #ifdef CAPTEUR_DS18B20
  #ifdef _WIFI_MODE_
  if (onMqttPublish != nullptr)
    int ret = onMqttPublish(ds18b20->mqttSubTopicState.c_str(), sMqttMsg.c_str());
  #endif
  #endif
  #ifdef CAPTEUR_DHT20
  #ifdef _WIFI_MODE_
  if (onMqttPublish != nullptr)
    int ret = onMqttPublish(dht20->mqttSubTopicState.c_str(), sMqttMsg.c_str());
  #endif
  #endif
  delay(1000);               // Sécurité

  // Si MQTT
  // Ligne commentée car ça fait grimper la consommation à 85 mA en Deep-Sleep ???
  //onMqttPowerDown();

  #ifdef _WIFI_MODE_
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  #endif

  #ifdef _RCSWITCH_MODE_
  // Optionnel : termine proprement toute opération RF en cours
  ELECHOUSE_cc1101.setSidle();     // Passe en idle d'abord si besoin (souvent pas obligatoire, mais sécurise)
  delay(5);                        // Petit délai pour que le chip accepte la commande suivante

  // Mise en power down du CC1101
  ELECHOUSE_cc1101.goSleep();      // ← C'EST LA LIGNE CLÉ ICI


  // Optionnel mais recommandé : CSN high pour isoler le SPI (évite fuites ou glitches)
  // Force CSn high (très important)
  pinMode(CC1101_CS, OUTPUT);
  digitalWrite(CC1101_CS, HIGH);  

  digitalWrite(CC1101_POWER_GND_GPIO, CC1101_OFF);   // Coupe physiquement l'alim → 0 µA espéré
  //digitalWrite(CC1101_POWER_33V_GPIO, CC1101_OFF);   // Coupe physiquement l'alim → 0 µA espéré
  #endif
    // Force tous les GPIOs inutiles en INPUT_PULLDOWN pour minimiser fuites
    /*for (int i = 0; i < 22; i++) {       // Pins 0 à 21 sur C3
        if (i != CC1101_CS && i != DEFAULT_SDA_PIN && i != DEFAULT_SCL_PIN && i != 9 && i != CC1101_POWER_GPIO) {  // Garde tes I2C
            pinMode(i, INPUT_PULLDOWN);
        }
    }*/

    // Option avancée : désactive power domain RTC peripherals si pas needed
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);

  // Coupe Serial si debug
  //Serial.end();  
  esp_sleep_enable_timer_wakeup(mulSleepDuration * 1000000ULL);
  #ifdef __ESP32_C3__
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);  
  #endif
  // Option : désactive ADC si pas needed
  //adc_power_release();
  esp_deep_sleep_start();   // Bonne nuit !
}

void CConfig::loadFromNVS() {
  prefs.begin(nvs_namespace, true);


  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Thermomètre");
  mbDeepSleepActive = prefs.getBool((mPrefixNVS+"sleepA").c_str(), true);
  mulSleepDuration = prefs.getLong((mPrefixNVS+"sleepI").c_str(), DEFAULT_SLEEP_DURATION_SEC);
  mulWakeDuration = prefs.getLong((mPrefixNVS+"wakeI").c_str(), DEFAULT_WAKE_DURATION_SEC);
  mbWakeFromDeepSleep = prefs.getBool((mPrefixNVS+"wkFDS").c_str(), false);
  mulDateMiseEnSommeil = prefs.getLong((mPrefixNVS+"dtSom").c_str(), 0UL);
  mulDateReveil = prefs.getLong((mPrefixNVS+"dtRev").c_str(), 0UL);
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), CONFIG_SUB_TOPIC);
  // Topics domotique
  domotique_prefix = prefs.getString((mPrefixNVS+"domo_pref").c_str(), default_domotique_topic_prefix);

  prefs.end();

  // topic command de configuration
  // subtopic config
  //this->mqttSubTopic = CONFIG_SUB_TOPIC;
  topic_config_command = domotique_prefix + this->mqttSubTopic + /*mqqtInfo.subtopic_command; //*/"command";
  topic_config_state   = domotique_prefix + this->mqttSubTopic + /*mqqtInfo.subtopic_state; //*/"state";

}

void CConfig::saveToNVS() {
  prefs.begin(nvs_namespace, false);
 
  // Autres paramètres. 

  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putString((mPrefixNVS+"domo_pref").c_str(), domotique_prefix);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putBool((mPrefixNVS+"sleepA").c_str(), mbDeepSleepActive);
  prefs.putLong((mPrefixNVS+"sleepI").c_str(), mulSleepDuration);
  prefs.putLong((mPrefixNVS+"wakeI").c_str(), mulWakeDuration);
  prefs.putBool((mPrefixNVS+"wkFDS").c_str(), mbWakeFromDeepSleep);
  prefs.putLong((mPrefixNVS+"dtSom").c_str(), mulDateMiseEnSommeil);
  prefs.putLong((mPrefixNVS+"dtRev").c_str(), mulDateReveil);

  prefs.end();
}

void CConfig::loadFromWebServer (WebServer& server) {
  // === PARAMÈTRES CLASSIQUES ===
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"domo_pref").c_str())) domotique_prefix = server.arg((mPrefixNVS+"domo_pref").c_str());
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"sleepA").c_str())) mbDeepSleepActive = true; else mbDeepSleepActive = false;
  if (server.hasArg((mPrefixNVS+"sleepI").c_str())) mulSleepDuration = server.arg((mPrefixNVS+"sleepI")).toInt();
  if (server.hasArg((mPrefixNVS+"wakeI").c_str())) mulWakeDuration = server.arg((mPrefixNVS+"wakeI")).toInt();
}

void CConfig::print() const {
  Serial.println("[LOGICIEL]");
  Serial.printf("  VERSION                : %s\n", String(VERSION).c_str());
  Serial.printf("  Deep Sleep             : %s\n", mbDeepSleepActive ? "ACTIF" : "INACTIF");
  Serial.printf("  Intervalle Deep Sleep  : %ld s\n", mulSleepDuration);
  Serial.printf("  Intervalle Entre Sleep : %ld s\n", mulWakeDuration);
  Serial.printf("  Préfixe domotique      : %s\n", domotique_prefix.c_str());
  Serial.printf("  Préfixe configuration  : %s\n", mqttSubTopic.c_str());

  Serial.printf("  Config cmd  : %s\n", topic_config_command.c_str());
  Serial.printf("  Config state  : %s\n", topic_config_state.c_str());

  Serial.flush();
}

String CConfig::getHTML() {
  String html = "";
  html =  "<h2>Configuration générale</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div class=\"checkbox-row\"><label>Deep Sleep</label><input type=\"checkbox\" name=" + (mPrefixNVS+"sleepA") + " value=\"1\"" + String(mbDeepSleepActive ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Intervalle Deep Sleep</label><input type=\"text\" name=" + (mPrefixNVS+"sleepI") + " value=\"" + mulSleepDuration + "\"></div>"
        "<div><label>Intervalle entre Sleep</label><input type=\"text\" name=" + (mPrefixNVS+"wakeI") + " value=\"" + mulWakeDuration + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Préfixe domotique</label><input type=\"text\" name=" + (mPrefixNVS+"domo_pref") + " value=\"" + domotique_prefix + "\"></div>"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic") + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}

int CConfig::parseSleepCommand(const String& msg) {
  int result = -1;
  String s = msg;
  s.trim();
  s.toUpperCase();

  if (!s.startsWith("SLEEP ")) return result;

  String reste = s.substring(6);
  reste.trim();

  if (reste == "ENABLE") {
    Serial.println("CConfig::parseSleepCommand - ENABLE");
    setDeepSleep(true);
    result = 0;
  }
  else if (reste == "DISABLE") {
    Serial.println("CConfig::parseSleepCommand - DISABLE");
    setDeepSleep(false);
    result = 0;
  }
  else {
    // Vérification stricte : tout doit être un nombre
    bool isNumber = true;
    for (char c : reste) {
      if (!isdigit(c)) {
        isNumber = false;
        break;
      }
    }

    if (isNumber && reste.length() > 0) {
      unsigned long duration_ms = reste.toInt();  // ou toLong() si besoin
      Serial.println("CConfig::parseSleepCommand - NUMBER : "+String(duration_ms)); Serial.flush();
      setSleepIntervalle(duration_ms);
    }
    else
      result = -2;
  }


  return result;
}

int CConfig::handleMqttCommand(const String& payload) {
  int ret = 0;
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

  if (cmd == "REBOOT") {
    ESP.restart();
  }
  else if (cmd == "STATUS") {
    String statusMsg = "=== ÉTAT Thermomètre ===\n";
    statusMsg += "IP     : " + WiFi.localIP().toString() + "\n";
    statusMsg += "Uptime : " + String(millis() / 1000) + "s";
    if (onMqttPublish != nullptr) {
      onMqttPublish(topic_config_state.c_str(), statusMsg.c_str());
      //client.publish(mConfig.topic_config_state.c_str(), statusMsg.c_str(), true);
      Serial.println("STATUS publié sur " + topic_config_state);
    }

  }
  else if (cmd.startsWith("SLEEP")) {
    parseSleepCommand(cmd);
    /*// On regarde s'il y a un argujment
    int idx = 0;
    String sCmd = cmd.substring(0, cmd.indexOf(' ', idx));
    idx = cmd.indexOf(' ', idx) + 1;
    Serial.println("Long : " + String(cmd.length()) + " Index : "+String(idx));
    String sArg = cmd.substring(idx, cmd.indexOf(' ', idx));

    Serial.println("============================================");
    Serial.println("cmd : "+sCmd+" Arg : " + sArg) + " index : " + String(idx);*/
    
  }

  return ret;
}

  void CConfig::setSleepIntervalle(unsigned long st) {
    mulSleepDuration = st;
    prefs.begin(nvs_namespace, false);
    prefs.putLong((mPrefixNVS+"sleepI").c_str(), mulSleepDuration);
    prefs.end();
    Serial.println("Deep Sleep - Nouvelle durée : " + String(mulSleepDuration));
  }

  void CConfig::setWakeIntervalle(unsigned long st) {
    mulWakeDuration = st;
    prefs.begin(nvs_namespace, false);
    prefs.putLong((mPrefixNVS+"wakeI").c_str(), mulWakeDuration);
    prefs.end();
}

  void CConfig::setDeepSleep(bool active) {
    mbDeepSleepActive = active;
    prefs.begin(nvs_namespace, false);
    prefs.putBool((mPrefixNVS+"sleepA").c_str(), mbDeepSleepActive);
    prefs.end();
    Serial.println("Deep Sleep Désactivé");
  }


  
  
  void CConfig::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }

  /*void CConfig::setMqttPowerDownCallback(std::function<void()> cb) {
        onMqttPowerDown = cb;
    }*/
    