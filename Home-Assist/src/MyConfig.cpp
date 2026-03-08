
#include "MyConfig.h"
#include "ParsedMqttMessage.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>


void CConfig::setup(const String pref, std::vector<CIPModule> *ctrl, std::vector<CIPModule> *escl) {
  mPrefixNVS = pref;
  mvsControleurs = ctrl;
  mvsEsclaves = escl;

  loadFromNVS();
  // topic command de configuration. Par exemple home/configaux/command Permet de différenbcier les config des CYD
  topic_config_command = domotique_prefix + mqttConfigSubTopic + "command";
  topic_config_state   = domotique_prefix + mqttConfigSubTopic + "state";
  Serial.printf("void CConfig::setup() - topic_config_state : %s\n", topic_config_state.c_str());

  // topic command de configuration général. Typiquement, au démarrage, un CYD demande au maître l'état des appareils
  // C'est home/configuration/command. 
  #ifndef __LOCAL_MODE__
  topic_config_maitre_command = domotique_prefix + mqttConfigMaitreSubTopic + "command";
  #endif
}

void CConfig::loop() {
  // Deep Sleep pour le moment uniquement si on n'est pas contrôleur principal
  #ifndef __LOCAL_MODE__ 
  if (mbDeepSleepActive) {
    if (millis() < mulWakeDuration*1000) {
      // Pas encore le temps de dormir
      return;
    }
    else
      // Deep Sleep. 
      enterDeepSleep();
  }
  #endif
}

void CConfig::enterDeepSleep() {
  Serial.printf("Deep sleep pour %lu secondes...\n", mulSleepDuration);
  Serial.flush();           // Attend que le serial soit vide
  delay(100);               // Sécurité

  unsigned int dureeEveil = mDateTime->getAbsoluteSecondes() - mulDateReveil; // Durée de réveil
  String sMqttMsg = nomEquipement + " DEEPSLEEP ON le " + mDateTime->getDate() + " à " + mDateTime->getTime() + " " + String(mulSleepDuration) + " Durée éveil : " + String (dureeEveil); 
  Serial.println(sMqttMsg + String(". Deep sleep in few seconds ..."));
  Serial.flush();           // Attend que le serial soit vide
  mbWakeFromDeepSleep = true;
  mulDateMiseEnSommeil = mDateTime->getAbsoluteSecondes();
  saveToNVS();
  // Envoie par MQTT de la durée de deep-sleep à venir
  Serial.printf("void CConfig::enterDeepSleep() - topic_config_state : %s\n", topic_config_state.c_str());
  if (onMqttPublish != nullptr)
    int ret = onMqttPublish(topic_config_state.c_str(), sMqttMsg.c_str());
  delay(1000);               // Sécurité


  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  #ifdef __LOCAL_MODE__
  // Optionnel : termine proprement toute opération RF en cours
  ELECHOUSE_cc1101.setSidle();     // Passe en idle d'abord si besoin (souvent pas obligatoire, mais sécurise)
  delay(5);                        // Petit délai pour que le chip accepte la commande suivante

  // Mise en power down du CC1101
  ELECHOUSE_cc1101.goSleep();      // ← C'EST LA LIGNE CLÉ ICI


  // Optionnel mais recommandé : CSN high pour isoler le SPI (évite fuites ou glitches)
  // Force CSn high (très important)
  pinMode(CC1101_CS, OUTPUT);
  digitalWrite(CC1101_CS, HIGH);  
  #endif

  // A faire plus tard sur les Contrôleurs munis de CC1101
  //digitalWrite(CC1101_POWER_GPIO, HIGH);   // Coupe physiquement l'alim → 0 µA espéré
  
  // Si la valeur du Deep-Sleep est nulle, on entre en deep-sleep infini
  if (mulSleepDuration > 0)
    esp_sleep_enable_timer_wakeup(mulSleepDuration * 1000000ULL);
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);  
  #define TOUCH_IRQ_PIN XPT2046_IRQ
  // Réveil par touche tactile (EXT0 sur front descendant)
  // On configure le GPIO IRQ du tactile comme source wake-up
  // Niveau bas = touche pressée (la plupart des XPT2046 font IRQ low quand touché)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)TOUCH_IRQ_PIN, LOW);  // 0 = LOW  

  esp_deep_sleep_start();   // Bonne nuit !
}

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

  #ifndef __LOCAL_MODE__
  if (onMqttPublish != nullptr)
    int ret = onMqttPublish(topic_config_state.c_str(), sMqttMsg.c_str());
  #endif  
  
}
void CConfig::setonEquipementCallback(std::function<int(const String& nom, const String& ip)> onEqu) {
  onEquipement = onEqu;
}      

int CConfig::remonteCYDInfoToEcran(const String & message) {
  ParsedMqttMessage msg;
  msg.parse(message);
  int n = msg.miTailleMesure;
  
  String nom="";
  if (n > 0) {
    Serial.printf("void CConfig::remonteCYDInfoToEcran()\n");
    for(int i=0; i<msg.mvsMesure.size(); i++) {
      nom += msg.mvsMesure[i] + " ";
    }
    msg.printDebug();
    if (onEquipement == nullptr) {
      Serial.printf("void CConfig::remonteCYDInfoToEcran() - Pas de callback pour %s()\n", nom.c_str());
      return -1;
    }
    onEquipement(nom, msg.msIp);
    
  }
  return 0;
}
void CConfig::loadFromNVS() {
  prefs.begin(nvs_namespace, true);


   // Topics domotique
  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), NOM_EQUIPEMENT);
  mqttConfigSubTopic = prefs.getString((mPrefixNVS+"cfg_pre").c_str(), CONFIG_SUB_TOPIC);
  #ifndef __LOCAL_MODE__
  mqttConfigMaitreSubTopic = prefs.getString((mPrefixNVS+"cfg_mai").c_str(), CONFIG_SUB_TOPIC_MAITRE);
  #endif
  domotique_prefix = prefs.getString((mPrefixNVS+"dom_pre").c_str(), default_domotique_topic_prefix);
  mbDeepSleepActive = prefs.getBool((mPrefixNVS+"sleepA").c_str(), false);
  mulSleepDuration = prefs.getLong((mPrefixNVS+"sleepI").c_str(), DEFAULT_SLEEP_DURATION_SEC);
  mulWakeDuration = prefs.getLong((mPrefixNVS+"wakeI").c_str(), DEFAULT_WAKE_DURATION_SEC);
  mbWakeFromDeepSleep = prefs.getBool((mPrefixNVS+"wkFDS").c_str(), false);
  mulDateMiseEnSommeil = prefs.getLong((mPrefixNVS+"dtSom").c_str(), 0UL);
  mulDateReveil = prefs.getLong((mPrefixNVS+"dtRev").c_str(), 0UL);

  prefs.end();
   
}
void CConfig::saveToNVS() {
  prefs.begin(nvs_namespace, false);
 
  // Autres paramètres. 
  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putString((mPrefixNVS+"cfg_pre").c_str(), mqttConfigSubTopic);
  #ifndef __LOCAL_MODE__
  prefs.putString((mPrefixNVS+"cfg_mai").c_str(), mqttConfigMaitreSubTopic);
  #endif
  prefs.putString((mPrefixNVS+"dom_pre").c_str(), domotique_prefix);
  prefs.putBool((mPrefixNVS+"sleepA").c_str(), mbDeepSleepActive);
  prefs.putLong((mPrefixNVS+"sleepI").c_str(), mulSleepDuration);
  prefs.putLong((mPrefixNVS+"wakeI").c_str(), mulWakeDuration);
  prefs.putBool((mPrefixNVS+"wkFDS").c_str(), mbWakeFromDeepSleep);
  prefs.putLong((mPrefixNVS+"dtSom").c_str(), mulDateMiseEnSommeil);
  prefs.putLong((mPrefixNVS+"dtRev").c_str(), mulDateReveil);

  prefs.end();
}

void CConfig::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
      
}

void CConfig::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"cfg_pre").c_str())) mqttConfigSubTopic = server.arg((mPrefixNVS+"cfg_pre").c_str());
  #ifndef __LOCAL_MODE__
  if (server.hasArg((mPrefixNVS+"cfg_mai").c_str())) mqttConfigMaitreSubTopic = server.arg((mPrefixNVS+"cfg_mai").c_str());
  #endif
  if (server.hasArg((mPrefixNVS+"dom_pre").c_str())) domotique_prefix = server.arg((mPrefixNVS+"dom_pre").c_str());
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"sleepA").c_str())) mbDeepSleepActive = true; else mbDeepSleepActive = false;
  if (server.hasArg((mPrefixNVS+"sleepI").c_str())) mulSleepDuration = server.arg((mPrefixNVS+"sleepI")).toInt();
  if (server.hasArg((mPrefixNVS+"wakeI").c_str())) mulWakeDuration = server.arg((mPrefixNVS+"wakeI")).toInt();

  Serial.printf("CConfig::loadFromWebServer () - Nom equ : %s\n", nomEquipement.c_str());
}

String CConfig::getHTML() {
  String html = "<h1>Configuration " + nomEquipement + "</h1>";
      html += "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div class=\"checkbox-row\"><label>Deep Sleep</label><input type=\"checkbox\" name=" + (mPrefixNVS+"sleepA") + " value=\"1\"" + String(mbDeepSleepActive ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Durée Deep Sleep (0 = infini)</label><input type=\"text\" name=" + (mPrefixNVS+"sleepI") + " value=\"" + mulSleepDuration + "\"></div>"
        "<div><label>Intervalle entre Sleep</label><input type=\"text\" name=" + (mPrefixNVS+"wakeI") + " value=\"" + mulWakeDuration + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Préfixe domotique</label><input type=\"text\" name=" + (mPrefixNVS+"dom_pre") + " value=\"" + domotique_prefix + "\"></div>"
        "<div><label>Préfixe Configuration</label><input type=\"text\" name=" + (mPrefixNVS+"cfg_pre") + " value=\"" + mqttConfigSubTopic + "\"></div>"
      "</div>";
    #ifndef __LOCAL_MODE__
    html +=  "<div class=\"row\">"
        "<div><label>Préfixe Configuration du Maître</label><input type=\"text\" name=" + (mPrefixNVS+"cfg_mai") + " value=\"" + mqttConfigMaitreSubTopic + "\"></div>"
      "</div>";
    #endif
  return html;
}

void CConfig::print() const {
  Serial.println("[LOGICIEL]");
  Serial.printf("  VERSION      : %s\n", String(VERSION).c_str());
  Serial.printf("  Nom équipement         : %s\n", nomEquipement.c_str());
  Serial.printf("  Deep Sleep             : %s\n", mbDeepSleepActive ? "ACTIF" : "INACTIF");
  Serial.printf("  Intervalle Deep Sleep  : %ld min\n", mulSleepDuration);
  Serial.printf("  Intervalle Entre Sleep : %ld min\n", mulWakeDuration);
  Serial.printf("  Préfixe domotique      : %s\n", domotique_prefix.c_str());
  Serial.printf("  Préfixe configuration  : %s\n", mqttConfigSubTopic.c_str());
  #ifndef __LOCAL_MODE__
  Serial.printf("  Préfixe du Maître      : %s\n", mqttConfigMaitreSubTopic.c_str());
  #endif
}