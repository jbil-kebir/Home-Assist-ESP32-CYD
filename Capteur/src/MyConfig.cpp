
#include "MyConfig.h"
#ifdef _WIFI_MODE_
#include <WiFi.h> // RSSI dans les ALIVE
#endif
#ifdef _RCSWITCH_MODE_
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#endif
#include "driver/adc.h"
#if IS_ESP32_S3
#include "soc/usb_serial_jtag_struct.h"  // USB_SERIAL_JTAG struct
#include "soc/usb_serial_jtag_reg.h"     // USB_SERIAL_JTAG_CONF0_REG
#endif

//
// Les opérations de cette fonction ne peuvent pas être dans le setup
// car le setup est déclenché avant que le mqtt ne soit valide.
// Par conséquent, cette fonction est appelée plus tard que le setup, 
// lorsque le wifi et le mqtt 
//
/*
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
*/
void CConfig::traiteReveil() {
 String sMqttMsg;
 String sDate = (mDateTime != nullptr) ? mDateTime->getDate() : "DATE";
 String sTime = (mDateTime != nullptr) ? mDateTime->getTime() : "TIME";
 // Si réveil après deep sleep
 if (mbWakeFromDeepSleep) {
  mbWakeFromDeepSleep = false;
  if (mDateTime != nullptr) {
   mulDateReveil = mDateTime->getAbsoluteSecondes();
   mulNbSecondesDeSommeil = mulDateReveil - mulDateMiseEnSommeil;
  }
  String sDuree = (mDateTime != nullptr) ? String(mulNbSecondesDeSommeil) : "DUREE INCONNUE";
  sMqttMsg = nomEquipement + " REVEIL DEEPSLEEP " + sDate + " " + sTime + " " + sDuree;
  saveToNVS();
  Serial.println(sMqttMsg+String(" seconde. Réveil avec Deep Sleep préalable ")); Serial.flush();
 }
 else {
  sMqttMsg = nomEquipement + " REVEIL OFF " + sDate + " " + sTime;
  Serial.println(sMqttMsg+String(" Réveil sans Deep Sleep préalable ")); Serial.flush();
 }

  #ifdef CAPTEUR_DS18B20
  if (onMqttPublish != nullptr && ds18b20 != nullptr)
    int ret = onMqttPublish(ds18b20->mqttSubTopicState.c_str(), sMqttMsg.c_str());
  #endif
  #ifdef CAPTEUR_DHT20
  if (onMqttPublish != nullptr && dht20 != nullptr)
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

 // Puce non configurée : jamais de deep sleep (page Web joignable, annonces NONCONFIGURE, moniteur série).
 // Non enregistré dans le NVS : l'enregistrement de la page Web fixe le réglage voulu.
 if (!mbConfiguree) mbDeepSleepActive = false;

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
#ifdef CAPTEUR_BATTERIE
if (mBatterieAA != nullptr)
  mBatterieAA->domotique_prefix = domotique_prefix;
#endif

 Serial.printf("CConfig::setup() - Deep sleep : %d - Durée avant : %ld\n", mbDeepSleepActive, mulWakeDuration); Serial.flush();
 Serial.printf("CConfig::setup() - Puce %s - Topic de configuration : %s\n",
               mbConfiguree ? "CONFIGUREE (pas d'annonce NONCONFIGURE)" : "NON CONFIGUREE (annonce NONCONFIGURE)",
               topic_config_command.c_str()); Serial.flush();
}

void CConfig::loop() {

  // ==================================== Watchdog ALIVE ====================================
  loopWatchdog();

  // ==================================== Puce non configurée ====================================
  annonceNonConfiguree();

  // ==================================== Deep sleep ====================================
  // S'il est actif, on n'entre en deep sleep que si le délai avant deep sleep est écoulé et qu'une mesure a été remontée (pour tous les capteurs)
  // Pour les capteurs qui ont besoin d'un ACK, on n'entre en deep-sleep que si l'ACK a été reçu
  //
  if (mbDeepSleepActive) {
    if (millis() < mulWakeDuration*1000) {
      // Pas encore le temps de dormir
      return;
    }
    
    Serial.printf("[Deep Sleep] CConfig::loop() - : délai avant deep sleep écoulé (%ld secondes). ", mulWakeDuration); Serial.flush();
    bool bDeepSleepPossible = true;
    bool bMesureRemontee = false; // Permet d'empêcher le deep sleep tant qu'une mesure n'a pas été remontée
    // Si LoRa non initialisé, on ne peut pas attendre une remontée LoRa → on autorise le deep sleep directement
    bool bLoraOk = false;//
    #ifdef _LORA_P2P_MODE_
    bLoraOk = (mLoraRxTx != nullptr) && mLoraRxTx->isInitialized();; // Dans le mode P2P, on considère que LoRa est toujours disponible
    #endif

    // Deep Sleep. On n'y entre que si une mesure a été envoyée (ou si LoRa non dispo)
    #ifdef CAPTEUR_DS18B20
    if (ds18b20 != nullptr)
      if (ds18b20->active && bLoraOk) {
        bMesureRemontee = ds18b20->mbMesureRemontee;
        if (!bMesureRemontee) bDeepSleepPossible = false;
      }
    #endif
    #ifdef CAPTEUR_DHT20
    if (dht20 != nullptr)
      if (dht20->active && bLoraOk) {
        bMesureRemontee = dht20->mbMesureRemontee;
        if (!bMesureRemontee) bDeepSleepPossible = false;
      }
    #endif

    #ifdef FLOTTEUR_VERTICAL
    if (mFlotteurVertical != nullptr) {
      if (mFlotteurVertical->active && bLoraOk) {
        bMesureRemontee = mFlotteurVertical->mbMesureRemontee;
        if (!bMesureRemontee) bDeepSleepPossible = false;
      }
      if (mFlotteurVertical->mbAckNeeded && bLoraOk) {
        bMesureRemontee = mFlotteurVertical->mbAckReceived;
        if (!bMesureRemontee) bDeepSleepPossible = false;
      }
    }
    #endif

    #ifdef CAPTEUR_RGB_TCS34725
    if (mCapteurRGB != nullptr) {
      if (mCapteurRGB->active && bLoraOk) {
        bMesureRemontee = mCapteurRGB->mbMesureRemontee;
        if (!bMesureRemontee) bDeepSleepPossible = false;
      }
    }   
    #endif
    
    // Micro : l'écoute des bips est continue, pas de deep sleep tant qu'il est actif
    #ifdef CAPTEUR_MICRO_MAX4466
    if (mMicro != nullptr && mMicro->active) bDeepSleepPossible = false;
    #endif

    //Serial.printf("[Deep Sleep] CConfig::loop() - : mesure remontée : %d. ACK reçu (pour les capteurs qui en ont besoin) : %d. Deep sleep possible : %d\n", bMesureRemontee, mFlotteurVertical != nullptr ? mFlotteurVertical->mbAckReceived : 1, bDeepSleepPossible); Serial.flush();  
    if (bDeepSleepPossible) {
      enterDeepSleep();
    }
    else {
      //Serial.println("Deep sleep différé : pas encore de mesure remontée ou ACK reçu pour les capteurs qui en ont besoin");
      //Serial.flush();
    }
  } // if (mbDeepSleepActive)
}

/*
void CConfig::enterDeepSleep() {
  #ifdef DEBUG_NO_DEEP_SLEEP
   Serial.println("Entrée en deep sleep... Mode Deep Sleep Inactif (commutateur). On sort."); Serial.flush();
   return;
  #endif
  Serial.printf("Deep sleep pour %lu secondes...\n", mulSleepDuration);
  Serial.flush();           // Attend que le serial soit vide
  delay(100);               // Sécurité

  String sDate = "DATE";
  String sHeure = "HEURE";
  #ifdef __WIFI_MODE__
  if (mDateTime != nullptr) {
    sDate = mDateTime->getDate();
    sHeure = mDateTime->getTime();
  }
  #endif

  unsigned int dureeEveil = 0; // A initialiser correctement par synchro avec le contrôleur principal
  #ifdef __WIFI_MODE__
  dureeEveil = mDateTime->getAbsoluteSecondes() - mulDateReveil; // Durée de réveil
  #endif

  String sMqttMsg = nomEquipement + " DEEPSLEEP ON " + sDate + " " + sHeure + " " + String(mulSleepDuration) + " Durée éveil : " + String (dureeEveil);
  Serial.println(sMqttMsg + String(". Deep sleep in few seconds ..."));
  Serial.flush();           // Attend que le serial soit vide
  mbWakeFromDeepSleep = true;
  mulDateMiseEnSommeil = 0; // A initialiser correctement par synchro avec le contrôleur principal
  #ifdef __WIFI_MODE__
  mulDateMiseEnSommeil = mDateTime->getAbsoluteSecondes();
  #endif
*/
void CConfig::enterDeepSleep() {
  #ifdef DEBUG_NO_DEEP_SLEEP
   Serial.println("Entrée en deep sleep... Mode Deep Sleep Inactif (commutateur). On sort."); Serial.flush();
   return;
  #endif
  Serial.printf("Deep sleep pour %lu secondes...\n", mulSleepDuration);
  Serial.flush();           // Attend que le serial soit vide
  delay(100);               // Sécurité

  String sDate = "DATE";
  String sHeure = "HEURE";
  #ifdef __WIFI_MODE__
  if (mDateTime != nullptr) {
    sDate = mDateTime->getDate();
    sHeure = mDateTime->getTime();
  }
  #endif

  unsigned int dureeEveil = 0;
  String sDureeEveil = "Durée inconnue";
  #ifdef __WIFI_MODE__
  if (mDateTime != nullptr) {
    dureeEveil = mDateTime->getAbsoluteSecondes() - mulDateReveil;
    sDureeEveil = String(dureeEveil);
  }
  #endif

  String sMqttMsg = nomEquipement + " DEEPSLEEP ON " + sDate + " " + sHeure + " " + String(mulSleepDuration) + " Durée éveil : " + sDureeEveil;
  Serial.println(sMqttMsg + String(". Deep sleep in few seconds ..."));
  Serial.flush();           // Attend que le serial soit vide
  mbWakeFromDeepSleep = true;
  mulDateMiseEnSommeil = 0;
  #ifdef __WIFI_MODE__
  if (mDateTime != nullptr)
    mulDateMiseEnSommeil = mDateTime->getAbsoluteSecondes();
  #endif
  saveToNVS();
  // Envoie par MQTT de la durée de deep-sleep à venir
  #ifdef CAPTEUR_DS18B20
  #ifdef _WIFI_MODE_
  if (onMqttPublish != nullptr && ds18b20 != nullptr)
    int ret = onMqttPublish(ds18b20->mqttSubTopicState.c_str(), sMqttMsg.c_str());
  #endif
  #endif
  #ifdef CAPTEUR_DHT20
  #ifdef _WIFI_MODE_
  if (onMqttPublish != nullptr && dht20 != nullptr)
    int ret = onMqttPublish(dht20->mqttSubTopicState.c_str(), sMqttMsg.c_str());
  #endif
  #endif

  #ifdef FLOTTEUR_VERTICAL
  #ifdef _WIFI_MODE_
  if (onMqttPublish != nullptr && mFlotteurVertical != nullptr)
    int ret = onMqttPublish(mFlotteurVertical->mqttSubTopicState.c_str(), sMqttMsg.c_str());
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
  // ---- Sleep du module SX1262 (module externe, reste alimenté pendant le deep sleep ESP32) ----
  // Sans sleep explicite : reste en RX (~4.6 mA) ou Standby RC (~600 µA)
  // cold sleep (false) : aucune retention de config → ~0.5 µA; le module sera réinitialisé au réveil
  #ifdef _LORA_P2P_MODE_
  if (mLoraRxTx != nullptr) {
    mLoraRxTx->sleep();
  }
  #endif

  // ---- Gestion GPIO : minimise les fuites de courant pendant le deep sleep ----

  // SPI (SX1262) : force les lignes en INPUT_PULLDOWN pour éviter fuites via diodes de protection internes
  #if IS_ESP32_S3
  #ifdef _LORA_P2P_MODE_
  pinMode(LORA_MOSI_PIN,  INPUT_PULLDOWN);
  pinMode(LORA_SCK_PIN,   INPUT_PULLDOWN);
  pinMode(LORA_MISO_PIN,  INPUT_PULLDOWN);
  pinMode(LORA_CS_PIN,    OUTPUT); digitalWrite(LORA_CS_PIN, HIGH); // CS haut = SX1262 non sélectionné sur bus SPI
  pinMode(LORA_DIO1_PIN,  INPUT_PULLDOWN);  // Pin d'interruption, plus nécessaire en sleep
  pinMode(LORA_BUSY_PIN,  INPUT_PULLDOWN);
  pinMode(LORA_RESET_PIN, INPUT);           // Actif LOW → INPUT pour éviter reset involontaire
  #endif
  #endif

  // I2C (DHT20...) : libère le bus et supprime fuites via pull-ups externes (~100 µA si actif)
  #if defined(CAPTEUR_DHT20) || defined(CAPTEUR_RGB_TCS34725)
  Wire.end();
  pinMode(DEFAULT_SDA_PIN, INPUT_PULLDOWN);
  pinMode(DEFAULT_SCL_PIN, INPUT_PULLDOWN);
  #endif

  // Flotteur vertical : supprime le pull-up interne (économise ~70 µA si pin connecté à GND)
  #ifdef FLOTTEUR_VERTICAL
  pinMode(DEFAULT_TOR_PIN, INPUT_PULLDOWN);
  #endif

  // Micro MAX4466 : libère l'entrée ADC et coupe la LED bleue
  #ifdef CAPTEUR_MICRO_MAX4466
  pinMode(CAPTEUR_MICRO_MAX4466_PIN, INPUT_PULLDOWN);
  pinMode(CAPTEUR_MICRO_MAX4466_LED_PIN, INPUT_PULLDOWN);
  #endif

  // Actionneur IR : coupe les LED
  #ifdef ACTIONNEUR_IR
  pinMode(ACTIONNEUR_IR_PIN,         INPUT_PULLDOWN);
  pinMode(ACTIONNEUR_IR_LED_PIN,     INPUT_PULLDOWN);
  #endif

  // ADC : libère le power domain (~20 µA)
  // adc_power_acquire/release utilisent un compteur de références interne.
  // Si aucun périphérique n'a appelé acquire (ex. sans DHT20/I2C), release() panique (abort).
  // On acquire avant release pour garantir que le compteur est ≥ 1.
  adc_power_acquire();
  adc_power_release();

  // RTC periph : inutile si uniquement timer wakeup
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);

  // ---- ESP32-S3 : désactive le PHY USB Serial/JTAG avant deep-sleep ----
  // BIT(14) = USB_PAD_ENABLE dans USB_SERIAL_JTAG_CONF0_REG (évite la dépendance au nom de constante)
  #if IS_ESP32_S3
  USB_SERIAL_JTAG.conf0.dp_pullup = 0;
  CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, BIT(14));
  delay(10);
  #endif

  esp_sleep_enable_timer_wakeup(mulSleepDuration * 1000000ULL);
  #ifdef __ESP32_C3__
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  #endif
  esp_deep_sleep_start();   // Bonne nuit !
}

void CConfig::loadFromNVS() {
  prefs.begin(nvs_namespace, true);


  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Thermomètre");
  mbDeepSleepActive = prefs.getBool((mPrefixNVS+"sleepA").c_str(), true);
  mulSleepDuration = prefs.getLong((mPrefixNVS+"sleepI").c_str(), DEFAULT_SLEEP_DURATION_SEC);
  mulSleepDurationS = prefs.getLong((mPrefixNVS+"sleepS").c_str(), DEFAULT_SLEEP_DURATIONS_SEC);
  mulSleepDurationM = prefs.getLong((mPrefixNVS+"sleepM").c_str(), DEFAULT_SLEEP_DURATIONM_SEC);
  mulSleepDurationL = prefs.getLong((mPrefixNVS+"sleepL").c_str(), DEFAULT_SLEEP_DURATIONL_SEC);
  mulWakeDuration = prefs.getLong((mPrefixNVS+"wakeI").c_str(), DEFAULT_WAKE_DURATION_SEC);
  mbWakeFromDeepSleep = prefs.getBool((mPrefixNVS+"wkFDS").c_str(), false);
  mulDateMiseEnSommeil = prefs.getLong((mPrefixNVS+"dtSom").c_str(), 0UL);
  mulDateReveil = prefs.getLong((mPrefixNVS+"dtRev").c_str(), 0UL);
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), CONFIG_SUB_TOPIC);
  // Topics domotique
  domotique_prefix = prefs.getString((mPrefixNVS+"domo_pref").c_str(), default_domotique_topic_prefix);
  mbWatchdogActive = prefs.getBool((mPrefixNVS+"wdogA").c_str(), false);
  mulWatchdogPeriod = prefs.getLong((mPrefixNVS+"wdogP").c_str(), DEFAULT_WATCHDOG_PERIOD_SEC);
  mbJournalSauvegarde = prefs.getBool((mPrefixNVS+"jrnSv").c_str(), true);
  gJournal.setSauvegardeActive(mbJournalSauvegarde);
  mbConfiguree = prefs.getBool((mPrefixNVS+"conf").c_str(), prefs.isKey((mPrefixNVS+"nom").c_str()));

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
  prefs.putBool((mPrefixNVS+"conf").c_str(), mbConfiguree);
  prefs.putString((mPrefixNVS+"domo_pref").c_str(), domotique_prefix);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putBool((mPrefixNVS+"sleepA").c_str(), mbDeepSleepActive);
  prefs.putLong((mPrefixNVS+"sleepI").c_str(), mulSleepDuration);
  prefs.putLong((mPrefixNVS+"sleepS").c_str(), mulSleepDurationS);
  prefs.putLong((mPrefixNVS+"sleepM").c_str(), mulSleepDurationM);
  prefs.putLong((mPrefixNVS+"sleepL").c_str(), mulSleepDurationL);
  prefs.putLong((mPrefixNVS+"wakeI").c_str(), mulWakeDuration);
  prefs.putBool((mPrefixNVS+"wkFDS").c_str(), mbWakeFromDeepSleep);
  prefs.putLong((mPrefixNVS+"dtSom").c_str(), mulDateMiseEnSommeil);
  prefs.putLong((mPrefixNVS+"dtRev").c_str(), mulDateReveil);
  prefs.putBool((mPrefixNVS+"wdogA").c_str(), mbWatchdogActive);
  prefs.putLong((mPrefixNVS+"wdogP").c_str(), mulWatchdogPeriod);
  prefs.putBool((mPrefixNVS+"jrnSv").c_str(), mbJournalSauvegarde);

  prefs.end();
}

void CConfig::loadFromWebServer (WebServer& server) {
  mbConfiguree = true; // Enregistrement depuis la page Web : fin des annonces NONCONFIGURE
  // === PARAMÈTRES CLASSIQUES ===
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"domo_pref").c_str())) domotique_prefix = server.arg((mPrefixNVS+"domo_pref").c_str());
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"sleepA").c_str())) mbDeepSleepActive = true; else mbDeepSleepActive = false;
  if (server.hasArg((mPrefixNVS+"sleepI").c_str())) mulSleepDuration = server.arg((mPrefixNVS+"sleepI")).toInt();
  if (server.hasArg((mPrefixNVS+"sleepI").c_str())) mulSleepDuration = server.arg((mPrefixNVS+"sleepI")).toInt();
  if (server.hasArg((mPrefixNVS+"sleepI").c_str())) mulSleepDuration = server.arg((mPrefixNVS+"sleepI")).toInt();
  if (server.hasArg((mPrefixNVS+"sleepS").c_str())) mulSleepDurationS = server.arg((mPrefixNVS+"sleepS")).toInt();
  if (server.hasArg((mPrefixNVS+"sleepM").c_str())) mulSleepDurationM = server.arg((mPrefixNVS+"sleepM")).toInt();
  if (server.hasArg((mPrefixNVS+"sleepL").c_str())) mulSleepDurationL = server.arg((mPrefixNVS+"sleepL")).toInt();
  if (server.hasArg((mPrefixNVS+"wakeI").c_str())) mulWakeDuration = server.arg((mPrefixNVS+"wakeI")).toInt();
  if (server.hasArg((mPrefixNVS+"wdogA").c_str())) mbWatchdogActive = true; else mbWatchdogActive = false;
  if (server.hasArg((mPrefixNVS+"wdogP").c_str())) mulWatchdogPeriod = server.arg((mPrefixNVS+"wdogP")).toInt();
  mbJournalSauvegarde = server.hasArg((mPrefixNVS+"jrnSv").c_str());
  gJournal.setSauvegardeActive(mbJournalSauvegarde);
}

void CConfig::print() const {
  Serial.println("[LOGICIEL]");
  Serial.printf("  VERSION                 : %s\n", String(VERSION).c_str());
  Serial.printf("  Deep Sleep              : %s\n", mbDeepSleepActive ? "ACTIF" : "INACTIF");
  Serial.printf("  Intervalle Deep Sleep   : %ld s\n", mulSleepDuration);
  Serial.printf("  Intervalle Deep SleepS  : %ld s\n", mulSleepDurationS);
  Serial.printf("  Intervalle Deep SleepM  : %ld s\n", mulSleepDurationM);
  Serial.printf("  Intervalle Deep SleepL  : %ld s\n", mulSleepDurationL);
  Serial.printf("  Intervalle Entre Sleep  : %ld s\n", mulWakeDuration);
  Serial.printf("  Watchdog ALIVE          : %s\n", mbWatchdogActive ? "ACTIF" : "INACTIF");
  Serial.printf("  Période Watchdog ALIVE  : %ld s\n", mulWatchdogPeriod);
  Serial.printf("  Sauvegarde journal      : %s\n", mbJournalSauvegarde ? "ACTIVE" : "INACTIVE");
  Serial.printf("  Préfixe domotique       : %s\n", domotique_prefix.c_str());
  Serial.printf("  Préfixe configuration   : %s\n", mqttSubTopic.c_str());

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
        "<div><label>Intervalle Deep Sleep Small</label><input type=\"text\" name=" + (mPrefixNVS+"sleepS") + " value=\"" + mulSleepDurationS + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Intervalle Deep Sleep Medium</label><input type=\"text\" name=" + (mPrefixNVS+"sleepM") + " value=\"" + mulSleepDurationM + "\"></div>"
        "<div><label>Intervalle Deep Sleep Large</label><input type=\"text\" name=" + (mPrefixNVS+"sleepL") + " value=\"" + mulSleepDurationL + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Intervalle entre Sleep</label><input type=\"text\" name=" + (mPrefixNVS+"wakeI") + " value=\"" + mulWakeDuration + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div class=\"checkbox-row\"><label>Watchdog ALIVE</label><input type=\"checkbox\" name=" + (mPrefixNVS+"wdogA") + " value=\"1\"" + String(mbWatchdogActive ? " checked" : "") + "></div>"
        "<div><label>Période Watchdog ALIVE (s)</label><input type=\"text\" name=" + (mPrefixNVS+"wdogP") + " value=\"" + mulWatchdogPeriod + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div class=\"checkbox-row\"><label>Sauvegarde du journal avant coupure (OnOff 0, IRKO)</label><input type=\"checkbox\" name=" + (mPrefixNVS+"jrnSv") + " value=\"1\"" + String(mbJournalSauvegarde ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Préfixe domotique</label><input type=\"text\" name=" + (mPrefixNVS+"domo_pref") + " value=\"" + domotique_prefix + "\"></div>"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic") + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";

  return html;
}


//
// Retour :
// -1 : pas une commande sleep
// -2 : commande sleep mal formée (ni ENABLE, ni DISABLE, ni un nombre valide)
//  0 : commande sleep traitée avec succès
int CConfig::parseSleepCommandSML(const String& msg) {
  int result = -1;
  String s = msg;
  s.trim();
  s.toUpperCase();

  if (!s.startsWith("SLEEPS") && !s.startsWith("SLEEPM") && !s.startsWith("SLEEPL")) 
  {
    Serial.println("CConfig::parseSleepCommandSML() - Commande ne commence pas par SLEEPS/M/L : "+s);
    return result;
  }

  String reste = s.substring(6);
  reste.trim();
  if (reste.length() == 0) {
    Serial.println("CConfig::parseSleepCommandSML() - Pas d'argument après SLEEPS/M/L, on applique la durée correspondante");
    switch (s.charAt(5)) {
      case 'S':
        Serial.println("CConfig::parseSleepCommandSML() - SLEEP SMALL");
        setSleepIntervalle(mulSleepDurationS);
        setDeepSleep(true); // Active le deep sleep
        result = 0;
        break;
      case 'M':
        Serial.println("CConfig::parseSleepCommandSML() - SLEEP MEDIUM");
        setSleepIntervalle(mulSleepDurationM);
        setDeepSleep(true); // Active le deep sleep
        result = 0;
        break;
      case 'L':
        Serial.println("CConfig::parseSleepCommandSML() - SLEEP LARGE");
        setSleepIntervalle(mulSleepDurationL);
        setDeepSleep(true); // Active le deep sleep
        result = 0;
        break;
      default:
        Serial.println("CConfig::parseSleepCommandSML() - Commande inconnue");
        result = -2; // Commande inconnue
    }
    //Serial.println("CConfig::parseSleepCommandSML() - Configuration");
    return result; 
  }

  Serial.println("CConfig::parseSleepCommandSML() - Argument après SLEEPS/M/L est-il un nombre ? : "+reste); Serial.flush();
  // Cas SLEEP S/M/L suivi d'un argument : on vérifie que c'est un nombre et on l'applique 
  bool isNumber = true;
  for (char c : reste) {
    if (!isdigit(c)) {
      isNumber = false;
      break;
    }
  }

  if (isNumber && reste.length() > 0) {
    Serial.println("CConfig::parseSleepCommandSML() - Argument est un nombre : "+reste); Serial.flush();
    unsigned long duration_ms = reste.toInt();  // ou toLong() si besoin
    Serial.println("CConfig::parseSleepCommandSML() - NUMBER : "+String(duration_ms)); Serial.flush();
    switch (s.charAt(5)) {
      case 'S':
        Serial.println("CConfig::parseSleepCommandSML() - SLEEP SMALL");
        mulSleepDurationS = duration_ms;
        result = 0;
        break;
      case 'M':
        Serial.println("CConfig::parseSleepCommandSML() - SLEEP MEDIUM");
        mulSleepDurationM = duration_ms;
        result = 0;
        break;
      case 'L':
        Serial.println("CConfig::parseSleepCommandSML() - SLEEP LARGE");
        mulSleepDurationL = duration_ms;
        result = 0;
        break;
      default:
        Serial.println("CConfig::parseSleepCommandSML() - Commande inconnue");
        result = -2; // Commande inconnue
    }
  }
  else
    result = -2;

  return result;
}
//
// Retour
// 0 : commande sleep traitée avec succès
// -1 : pas une commande sleep
// -2 : commande sleep mal formée (ni ENABLE, ni DISABLE, ni un
//
int CConfig::parseSleepCommand(const String& msg) {
  int result = -1;
  String s = msg;
  s.trim();
  s.toUpperCase();

  if (!s.startsWith("SLEEP")) {
    Serial.println("CConfig::parseSleepCommand() - Commande ne commence pas par SLEEP : "+s);
    return result;
  }

  String reste = s.substring(6);
  reste.trim();

  String cmd = reste;
  cmd.toUpperCase();

  if (cmd.length() == 0) {
    Serial.println("CConfig::parseSleepCommand() - Pas d'argument après SLEEP, on sort");
    result = -1; // Pas une commande sleep valide
    return result; 
  }

  if (cmd.startsWith("SLEEPS") || cmd.startsWith("SLEEPM") || cmd.startsWith("SLEEPL")) {
    Serial.println("CConfig::handleMqttCommand - Commande SML détectée : " + cmd); Serial.flush();
    parseSleepCommandSML(cmd);
  }
  else if (cmd == "ENABLE") {
    Serial.println("CConfig::parseSleepCommand - ENABLE");
    setDeepSleep(true);
    result = 0;
  }
  else if (cmd == "DISABLE") {
    Serial.println("CConfig::parseSleepCommand - DISABLE");
    setDeepSleep(false);
    result = 0;
  }
  else if(cmd.startsWith("-1")) {
    Serial.println("CConfig::parseSleepCommand - -1");
    setDeepSleep(false); // Désactive le deep sleep
    result = 0;
  }
  else {
    // Vérification stricte : tout doit être un nombre
    bool isNumber = true;
    for (char c : cmd) {
      if (!isdigit(c)) {
        isNumber = false;
        break;
      }
    }

    if (isNumber && cmd.length() > 0) {
      unsigned long duration_ms = cmd.toInt();  // ou toLong() si besoin
      Serial.println("CConfig::parseSleepCommand - NUMBER : "+String(duration_ms)); Serial.flush();
      setSleepIntervalle(duration_ms);
      setDeepSleep(true); // Active le deep sleep
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

  if (cmd.startsWith("SLEEP")) {
    Serial.println("CConfig::handleMqttCommand - Commande SLEEP détectée : " + cmd); Serial.flush();
    parseSleepCommand(cmd);
  }
  else if (cmd.startsWith("WDOG")) {
    Serial.println("CConfig::handleMqttCommand - Commande WDOG détectée : " + cmd); Serial.flush();
    parseWdogCommand(cmd);
  }
  else {
    Serial.println("CConfig::handleMqttCommand - Commande inconnue : " + cmd); Serial.flush();
    ret = -1; // Commande inconnue
  }

  return ret;
}

//
// WDOG ENABLE  : active le watchdog ALIVE
// WDOG DISABLE : désactive le watchdog ALIVE
// WDOG <s>     : fixe la période (en secondes) et active le watchdog
//
// Retour
// 0 : commande traitée avec succès
// -2 : commande mal formée
//
int CConfig::parseWdogCommand(const String& msg) {
  String reste = msg.substring(4);
  reste.trim();
  reste.toUpperCase();

  if (reste == "ENABLE") {
    setWatchdog(true);
    return 0;
  }
  if (reste == "DISABLE") {
    setWatchdog(false);
    return 0;
  }

  // Vérification stricte : tout doit être un nombre
  bool isNumber = reste.length() > 0;
  for (char c : reste) {
    if (!isdigit(c)) {
      isNumber = false;
      break;
    }
  }
  if (!isNumber || reste.toInt() <= 0) {
    Serial.println("CConfig::parseWdogCommand() - Commande mal formée : " + msg);
    return -2;
  }
  setWatchdogPeriod(reste.toInt());
  setWatchdog(true);
  return 0;
}

void CConfig::setWatchdog(bool active) {
  mbWatchdogActive = active;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"wdogA").c_str(), mbWatchdogActive);
  prefs.end();
  Serial.println(String("Watchdog ALIVE ") + (mbWatchdogActive ? "activé" : "désactivé"));
}

void CConfig::setWatchdogPeriod(unsigned long st) {
  mulWatchdogPeriod = st;
  prefs.begin(nvs_namespace, false);
  prefs.putLong((mPrefixNVS+"wdogP").c_str(), mulWatchdogPeriod);
  prefs.end();
  Serial.println("Watchdog ALIVE - Nouvelle période : " + String(mulWatchdogPeriod) + " s");
}

// Ligne ajoutée aux messages STATUS (MQTT et LoRa)
String CConfig::getWatchdogStatus() const {
  return "Wdog   : " + String(mbWatchdogActive ? "ACTIF " : "INACTIF ") + String(mulWatchdogPeriod) + "s" + String(mbDeepSleepActive ? " (ignoré : deep sleep)" : "") + "\n";
}

//
// Appelée à chaque publication (MQTT ou LoRa) : remet à zéro le décompte ALIVE de l'équipement expéditeur.
// message : "<nomEquipement> ..." en MQTT, "<topic> <nomEquipement> ..." en LoRa (bAvecTopic = true)
//
void CConfig::noteEmission(const String& message, bool bAvecTopic/*=false*/) {
  String s = message;
  s.trim();
  if (bAvecTopic) {
    int i = s.indexOf(' ');
    if (i < 0) return;
    s = s.substring(i + 1);
    s.trim();
  }
  int i = s.indexOf(' ');
  String nom = (i < 0) ? s : s.substring(0, i);
  auto it = mEtatsWatchdog.find(nom);
  if (it == mEtatsWatchdog.end()) return;
  it->second.ulDerniereEmission = millis();
  if (i >= 0 && s.substring(i + 1).startsWith("ALIVE"))
    it->second.bAliveDemarrageEnvoye = true;
}

//
// Retour : true si un ALIVE a été envoyé (un seul par appel, pour étaler les émissions LoRa)
//
bool CConfig::loopWatchdog() {
  if (!mbWatchdogActive || mbDeepSleepActive || mulWatchdogPeriod == 0) return false;

  #ifdef CAPTEUR_DS18B20
  if (ds18b20 != nullptr && ds18b20->active)
    if (verifieWatchdog(ds18b20->nomEquipement, ds18b20->mqttSubTopicState)) return true;
  #endif
  #ifdef CAPTEUR_DHT20
  if (dht20 != nullptr && dht20->active)
    if (verifieWatchdog(dht20->nomEquipement, dht20->mqttSubTopicState)) return true;
  #endif
  #ifdef FLOTTEUR_VERTICAL
  if (mFlotteurVertical != nullptr && mFlotteurVertical->active)
    if (verifieWatchdog(mFlotteurVertical->nomEquipement, mFlotteurVertical->mqttSubTopicState)) return true;
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  if (mCapteurRGB != nullptr && mCapteurRGB->active)
    if (verifieWatchdog(mCapteurRGB->nomEquipement, mCapteurRGB->mqttSubTopicState)) return true;
  #endif
  #ifdef CAPTEUR_BATTERIE
  if (mBatterieAA != nullptr && mBatterieAA->active)
    if (verifieWatchdog(mBatterieAA->nomEquipement, mBatterieAA->mqttSubTopicState)) return true;
  #endif
  #ifdef CAPTEUR_MICRO_MAX4466
  if (mMicro != nullptr && mMicro->active)
    if (verifieWatchdog(mMicro->nomEquipement, mMicro->mqttSubTopicState)) return true;
  #endif

  return false;
}

//
// Retour : true si un ALIVE a été envoyé pour cet équipement
//
bool CConfig::verifieWatchdog(const String& nom, const String& topic) {
  unsigned long now = millis();
  EtatWatchdog& etat = mEtatsWatchdog[nom]; // Créé au premier passage

  // ALIVE de démarrage (ou d'activation du watchdog) : envoyé tout de suite, puis retenté toutes les 2 s
  // tant qu'il n'a pas été publié avec succès (WiFi / MQTT pas encore connectés). Voir noteEmission().
  if (!etat.bAliveDemarrageEnvoye) {
    if (etat.bAliveDemarrageTente && now - etat.ulDerniereTentative < 2000UL) return false;
    etat.bAliveDemarrageTente = true;
    etat.ulDerniereTentative = now;
    envoieAlive(nom, topic);
    return true;
  }

  if (now - etat.ulDerniereEmission < mulWatchdogPeriod * 1000UL) return false;

  envoieAlive(nom, topic);
  etat.ulDerniereEmission = now; // Au cas où aucune publication n'a abouti (noteEmission() non appelée)
  return true;
}

void CConfig::envoieAlive(const String& nom, const String& topic) {
  String sHorodatage;
  if (mDateTime != nullptr && mDateTime->isTimeValid())
    sHorodatage = mDateTime->getDate() + " " + mDateTime->getTime();
  else
    sHorodatage = "UPTIME " + String(millis() / 1000);
  String sMsg = nom + " ALIVE " + sHorodatage;
  #ifdef _WIFI_MODE_
  // Niveau de réception de la box (dBm), pour juger de la portée sans liaison série.
  // Placé en fin de message : les récepteurs (CYD) ne lisent que le premier champ après le nom.
  if (WiFi.status() == WL_CONNECTED)
    sMsg += " RSSI " + String(WiFi.RSSI());
  #endif
  Serial.println("CConfig::envoieAlive() - " + sMsg);

  if (onMqttPublish != nullptr)
    onMqttPublish(topic.c_str(), sMsg.c_str());

  #ifdef _LORA_P2P_MODE_
  if (mLoraRxTx != nullptr && mLoraRxTx->isInitialized())
    mLoraRxTx->sendPacket((topic + " " + sMsg).c_str());
  #endif
}

//
// Puce non configurée : annonce sur le topic de configuration, pour que le CYD affiche son IP
// (l'IP est ajoutée en fin de message par CMqtt::publish()).
// Tentative toutes les 2 s tant qu'aucune annonce n'a abouti (WiFi / MQTT pas encore connectés), puis toutes les 60 s.
//
void CConfig::annonceNonConfiguree() {
  #ifdef _WIFI_MODE_
  if (mbConfiguree || onMqttPublish == nullptr) {
    static bool bTraceFaite = false;
    if (!bTraceFaite) {
      Serial.printf("CConfig::annonceNonConfiguree() - Pas d'annonce : %s\n",
                    mbConfiguree ? "puce configuree (NVS cfg_conf, ou cfg_nom present)" : "callback MQTT absente");
      bTraceFaite = true;
    }
    return;
  }
  unsigned long now = millis();
  unsigned long ulAttente = mbAnnonceNonConfigureeEnvoyee ? 60000UL : 2000UL;
  if (mulDerniereAnnonceNonConfiguree != 0 && now - mulDerniereAnnonceNonConfiguree < ulAttente) return;
  mulDerniereAnnonceNonConfiguree = (now == 0) ? 1 : now;

  // Identifiant : modèle de puce et fin de l'adresse MAC (ex. ESP32-C3-A1B2C3)
  String sMac = WiFi.macAddress();
  sMac.replace(":", "");
  String sMsg = "NONCONFIGURE " + String(ESP.getChipModel()) + "-" + sMac.substring(6);
  if (mDateTime != nullptr && mDateTime->isTimeValid())
    sMsg += " " + mDateTime->getDate() + " " + mDateTime->getTime();

  int ret = onMqttPublish(topic_config_command.c_str(), sMsg.c_str());
  // 0 : publié, -1 : MQTT inactif, -2 : échec de publication, -3 : MQTT non connecté
  Serial.printf("CConfig::annonceNonConfiguree() - %s sur %s -> %d\n", sMsg.c_str(), topic_config_command.c_str(), ret);
  if (ret == 0)
    mbAnnonceNonConfigureeEnvoyee = true;
  #endif
}


/*int CConfig::handleMqttCommand(const String& payload) {
  int ret = 0;
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

  if (cmd.startsWith("SLEEPS") || cmd.startsWith("SLEEPM") || cmd.startsWith("SLEEPL")) {
    Serial.println("CConfig::handleMqttCommand - Commande SML détectée : " + cmd); Serial.flush();
    parseSleepCommandSML(cmd);
  }
  else if (cmd.startsWith("SLEEP")) {
    parseSleepCommand(cmd);
  }
  else {
    Serial.println("CConfig::handleMqttCommand - Commande inconnue : " + cmd); Serial.flush();
    ret = -1; // Commande inconnue
  }

  return ret;
}*/

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
    