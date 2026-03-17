#include <Arduino.h>
#include "DetecteurRGB_TCS34725.h"

bool CDetecteurRGB_TCS34725::begin(const String pref) {
  mPrefixNVS = pref;
  mbMesureRemontee = false;
  mbForce=true; // On force une remontée à l'initialisation

  domotique_prefix = "home/";

  // On charge les infos de config depuis le NVS
  loadFromNVS();
  while(!Wire.begin(mucSdaPin, mucSclPin)) {
      static unsigned long ulLastTime = millis();
      Serial.println("Wire.begin() échec");
      if (millis() - ulLastTime > TIMEOUT_WIRE)
          return false;
      ulLastTime = millis();
      delay(500);
  };               // Pins par défaut SDA/SCL de ta board ESP32-C3
  while(!tcs.begin()) {
      static unsigned long ulLastTime = millis();
      Serial.println("tcs.begin() échec");
      if (millis() - ulLastTime > TIMEOUT_DHT)
          return false;
      ulLastTime = millis();
      delay(500);
  };               
  
  tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_614MS);  // 614 ms = max sensibilité
  tcs.setGain(TCS34725_GAIN_60X);                          // gain x60  

  Serial.println("TCS34725 trouvé.");
  _initialized = true;

  #ifdef LED_CAPTEUR_RGB
  pinMode(mucLedCapteurRGBPin, OUTPUT);
  digitalWrite(mucLedCapteurRGBPin, mucLedRGBIsOFF);
  #endif

  //print();

  return true;

  // Test rapide optionnel
  //int status = dht.read();
  //return (status == DHT20_OK || status == DHT20_ERROR_CHECKSUM); // tolérant au premier read parfois
}

//--------------------------------------------------------------------------
//  MyDS18B20::loop()
//
// Retour
// 1 : Couleur lue et changée 
// 0 : Couleur lue mais inchangée
// -1 : Echec de lecture de température
// -9 : Capteur inactif
//--------------------------------------------------------------------------
int CDetecteurRGB_TCS34725::loop() {
  int ret = -99;
  if (!active) return -9;
  static bool bEnvoiEnCours = false; // Falg utile s'il faut envoyer plusieurs fois (si mucNbEnvois > 1)
  static unsigned char nbEnvoisAFaire = mucNbEnvois;
  bool bMesureNormale = false;
  static unsigned long ulCompteurTempsEnvoi = 0UL;

  // Exemple : lecture toutes les 10 secondes
  static unsigned long lastRead = 0L;
  static unsigned long lastForcageRemontee = 0UL;
  if (millis() - lastForcageRemontee > mulIntervalleForcageRemonteeMesure*1000 /**60 */) { // mulIntervalleForcageRemonteeMesure en secondes pour les tests
    lastForcageRemontee = millis();
    mbForce = true;
  //  nbEnvoisAFaire = mucNbEnvois;
  //  bEnvoiEnCours = true;
  }
  //static float lastCouleur=0;
  if (millis() - lastRead > mulIntervalleMesure * 1000 /** 60 */){ // mulIntervalleMesure en secondes pour les tests
    lastRead = millis();
    bMesureNormale = true;
  }  
  if (bMesureNormale || (lastRead == 0UL) || mbForce /*|| bEnvoiEnCours*/){ // mulIntervalleMesure en secondes pour les tests
    if (!bEnvoiEnCours) {           // Si l'envoi n'a pas encore été effectué, ...
      nbEnvoisAFaire = mucNbEnvois; // ... on initialise le nb d'envois 
      bEnvoiEnCours = true;         // ... et le flag
    }
  }  
  if (bEnvoiEnCours) {
    if (nbEnvoisAFaire > 0) {
      if (millis() - ulCompteurTempsEnvoi > mulIntervalleEnvoi*1000 || ulCompteurTempsEnvoi == 0L) {
        Serial.printf("CDetecteurRGB_TCS34725::loop() - Nb d'envois restants : %d\n", nbEnvoisAFaire); Serial.flush();
        bool bMesurer = false;
        if (nbEnvoisAFaire == mucNbEnvois) bMesurer = true;
        ret = readAndPublish(mbForce, bMesurer); // Lit et publie sur MQTT
        nbEnvoisAFaire--;             // On décrémente le nb d'envois
        ulCompteurTempsEnvoi = millis();
      }
    }
    else {
      mbForce = false; lastForcageRemontee = millis(); 
      bMesureNormale = false; lastRead = millis();
      bEnvoiEnCours = false;
    }
  } // if (bEnvoiEnCours) {

  return ret;
}

//--------------------------------------------------------------------------
//  CDetecteurRGB_TCS34725::readAndPublish()
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
int CDetecteurRGB_TCS34725::readAndPublish(bool force/*=false*/, bool mesurer/*=false*/) {
  //read();
  //publieSurMqtt(true);
  //return 1;
  int ret = 0;
  if (!active) return -9;
  bool readStatus = false;
  if (mesurer) readStatus = read();
  else readStatus = true;

  if (readStatus) {
    Serial.printf("muiNewR : 0x%x muiLastR : 0x%x\n", muiNewR, muiLastR);
    Serial.printf("muiNewG : 0x%x muiLastG : 0x%x\n", muiNewR, muiLastR);
    Serial.printf("muiNewB : 0x%x muiLastB : 0x%x\n", muiNewR, muiLastR);
    Serial.printf("newLux : 0x%x lastLux : 0x%x\n", muiNewLux, muiLastLux);
    if ( (muiNewR != muiLastR) || (muiNewBrutLux != muiLastBrutLux) || force) { 
      muiLastR = muiNewR;
      muiLastG = muiNewG;
      muiLastB = muiNewB;
      muiLastLux = muiNewLux;
      muiLastBrutLux = muiNewBrutLux;
      mbLastEtatOnOff = mbNewEtatOnOff;
      //printCouleur(); // On affiche sur le moniteur série
      // On publie sur Mqtt
      // Si échec de remontée MQTT, mbMesureRemontee sera à false
      // ce qui empêchera le deep sleep
      #ifdef _RCSWITCH_MODE_
      mbMesureRemontee = publieParCC1101(force); 
      //if(publieParCC1101(force)) mucNbMesuresRemontees++; 
      #endif
      #ifdef _LORA_P2P_MODE_
      mbMesureRemontee = publieParLoraP2POnOff(force); 
      if (remonterMesuresBrutes)
        mbMesureRemontee = publieParLoraP2PBrut(force); 
      //if(publieParCC1101(force)) mucNbMesuresRemontees++; 
      #endif
      #ifdef _WIFI_MODE_
      #ifndef __DESACTIVE_ENVOI_MQTT__
      mbMesureRemontee = publieSurMqttOnOff(force); 
      if (remonterMesuresBrutes)
        mbMesureRemontee = publieSurMqttBrut(force); 
      #endif
      //if(publieParCC1101(force)) mucNbMesuresRemontees++; 
      #endif
      ret = 1;
    } 
    else
      ret = 0; // température inchangée
  }
  else {
    //Serial.println("ds18b20.readCouleur() - échec");
    ret = -1;
  }
  return ret;
}

//--------------------------------------------------------------------------
//  CDetecteurRGB_TCS34725::readAndPublishTEST()
//
// Pour les tests : envoie plusieurs mesures d'affilé
//
// Retour
// 1 : Température lue et changée 
// 0 : Température lue mais inchangée
// -1 : Echec de lecture de température
// -9 : Thermomètre inactif
//--------------------------------------------------------------------------
int CDetecteurRGB_TCS34725::readAndPublishTEST(float t, float h) {
#ifdef _RCSWITCH_MODE_

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
  Serial.printf("int CDetecteurRGB_TCS34725::readAndPublishTEST - température : %.1f Humidité %.1f\n", t, h);
  Serial.printf("CRC Calculé : 0x%lx = %d\n", crc, crc); 
  Serial.printf("Header : 0x%lx\n", codes[0]);
  Serial.printf("Temp MSB : 0x%lx\n", codes[1]);
  Serial.printf("Temp LSB : 0x%lx\n", codes[2]);
  Serial.printf("Humi MSB : 0x%lx\n", codes[3]);
  Serial.printf("Humi LSB : 0x%lx\n", codes[4]);
  Serial.printf("Footer : 0x%lx\n", codes[5]);
  Serial.flush();

  mRCSwitch->envoieCode(codes, 6);


  /*unsigned long codes[6] = {  0x014FFFUL, // ID 1, 4 codes de mesure, 0xFFF
                              0x011001UL, // ID 1, N° Mesure (1), Type Mesure (0), MSB 18.5 °C (1)
                              0x011081UL, // ID 1, N° Mesure (1), Type Mesure (0), MSB 18.5 °C (0x81)
                              0x012100UL, // ID 1, N° Mesure (2), Type Mesure (1), MSB 56 % (0)
                              0x012138UL, // ID 1, N° Mesure (2), Type Mesure (1), MSB 56 % (0x38)
                              0x010000UL  // ID 1, CRC (), 0x00
                            };
  uint8_t crc = computeCRC(codes, 5); // Calcul de CRC sur les 5 premiers éléments du tableau
  codes[5] = (codes[5] | (unsigned long)crc << 8);
  
  Serial.printf("CRC Calculé : 0x%lx = %d\n", crc, crc); 
  Serial.printf("Header : 0x%lx\n", codes[0]);
  Serial.printf("Temp MSB : 0x%lx\n", codes[1]);
  Serial.printf("Temp LSB : 0x%lx\n", codes[2]);
  Serial.printf("Humi MSB : 0x%lx\n", codes[3]);
  Serial.printf("Humi LSB : 0x%lx\n", codes[4]);
  Serial.printf("Footer : 0x%lx\n", codes[5]);
  Serial.flush();

  mRCSwitch->envoieCode(codes, 6);*/

#endif
  
  
  return 0;


}

//
//
// Envoi par MQTT les trames correspondant à la couleur et à la luminosité
// Exemple : 
//  "NomEquipement Coul 12/03/2025 11:52:46 210 45 74 133 1206 FORCE"
//  "NomEquipement Lux 12/03/2025 11:52:46 97 FORCE" (cet envoi est commenté pour le moment)
//  
// 210 : R, 45 : G, 74 : B
// 133 : luminosité
// 1206 : luminosité brute
//
// Retour
//  true : OK
//  false : KO
//
bool CDetecteurRGB_TCS34725::publieSurMqttBrut(bool force/*=false*/) {
    if (onMqttPublish == nullptr) return false;
    bool ret = false;
//    unsigned long rgb = lastRgb; 
    //unsigned int lux = muiLastLux; 
    //unsigned int luxBrut = muiLastBrutLux; 
    String sValTemp = nomEquipement + " Coul " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(muiLastR) + " " + String(muiLastG) + " " + String(muiLastB) + " " + String(muiLastLux) +  + " " + String(muiLastBrutLux);
    if (force) sValTemp += " FORCE";
    //lux = 0xFFFFFF00 | lux; // Pour permettre le contrôle à l'arrivée
    String sValLux = nomEquipement + " Lux " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(muiLastLux);
    //if (force) sValLux += " FORCE";
    Serial.println("CDetecteurRGB_TCS34725::publieSurMqttBrut() Coul : " + sValTemp);
    //Serial.println("CDetecteurRGB_TCS34725::publieSurMqttBrut() Lux  : " + sValLux);
    if (onMqttPublish != nullptr) {
      ret = (onMqttPublish(mqttSubTopicState.c_str(), sValTemp.c_str()) == 0);
      //ret = (onMqttPublish(mqttSubTopicState.c_str(), sValLux.c_str()) == 0);
    }
    else {
      Serial.printf("CDetecteurRGB_TCS34725::publieSurMqttBrut() - callback onMqttPublish() non définie\n");
    }
    return ret;
}

//
//
// Envoi par Lora P2P les trames correspondant à la couleur et à la luminosité
// Exemple : 
//  "NomEquipement Coul DATE TIME 210 45 74 133 1206 FORCE"
//  "NomEquipement Lux DATE TIME 97 FORCE" (cet envoi est commenté pour le moment)
//  
// DATE TIME : Envoyés tels quels sous format texte car on ne dispose pas encore 
// d'horodatage pour envoyer la date et l'heure. A prévoir ultérieurement
//
// 210 : R, 45 : G, 74 : B
// 133 : luminosité
// 1206 : luminosité brute
//
// Retour
//  true : OK
//  false : KO
//
bool CDetecteurRGB_TCS34725::publieParLoraP2PBrut(bool force/*=false*/) {
    if (onLoraP2PPublish == nullptr) return false;
    bool ret = false;
//    unsigned long rgb = lastRgb; 
    //unsigned long lux = lastLux; 
    //unsigned long luxBrut = lastBrutLux; 
    String sValTemp = nomEquipement + " Coul " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(muiLastR) + " " + String(muiLastG) + " " + String(muiLastB) + " " + String(muiLastLux) +  + " " + String(muiLastBrutLux);
    if (force) sValTemp += " FORCE";
    //lux = 0xFFFFFF00 | lux; // Pour permettre le contrôle à l'arrivée
    String sValLux = nomEquipement + " Lux " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(muiLastLux);
    //if (force) sValLux += " FORCE";
    Serial.println("CDetecteurRGB_TCS34725::publieParLoraP2PBrut() Coul : " + sValTemp);
    //Serial.println("CDetecteurRGB_TCS34725::publieParLoraP2PBrut() Lux  : " + sValLux);
    if (onLoraP2PPublish != nullptr) {
      ret = (onLoraP2PPublish(sValTemp.c_str()) == 0);
      //delay(3000);
      //ret = (onLoraP2PPublish(sValHum.c_str()) == 0);
    }
    else {
      Serial.printf("CDetecteurRGB_TCS34725::publieParLoraP2PBrut() - callback onMqttPublish() non définie\n");
    }
    return ret;
}

//
//
// Envoi par MQTT la trame correspondant à l'état : ON si led allumée, Off sinon
// Exemple : 
//  "NomEquipement Del 12/03/2025 11:52:46 125469875 1 FORCE"
//    ou
//  "NomEquipement Del 12/03/2025 11:52:46 125469875 0 FORCE"
//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message provient d'un forçage de mesure.
//
// Retour
//  true : OK
//  false : KO
//
bool CDetecteurRGB_TCS34725::publieSurMqttOnOff(bool force/*=false*/) {
    if (onMqttPublish == nullptr) return false;
    bool ret = false;
    String sValTemp = nomEquipement + " Del " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(mbLastEtatOnOff);
    if (force) sValTemp += " FORCE";
    Serial.println("CDetecteurRGB_TCS34725::publieSurMqttOnOff() Del : " + sValTemp);
    if (onMqttPublish != nullptr) {
      ret = (onMqttPublish(mqttSubTopicState.c_str(), sValTemp.c_str()) == 0);
    }
    else {
      Serial.printf("CDetecteurRGB_TCS34725::publieSurMqttOnOff() - callback onMqttPublish() non définie\n");
    }
    return ret;
}

//
//
// Envoi par Lora P2P la trame correspondant à l'état : ON si led allumée, Off sinon
// Exemple : 
//  "NomEquipement Del DATE TIME 125469875 1 FORCE"
//    ou
//  "NomEquipement Del DATE TIME 125469875 0 FORCE"
//  
// DATE TIME : Envoyés tels quels sous format texte car on ne dispose pas encore 
// d'horodatage pour envoyer la date et l'heure. A prévoir ultérieurement
//
// La couleur 125469875 est sous format 0xFFRRGGBB
//
// L'argument force n'est utile que pour le debug. Ca permet de voir
// quand le message provient d'un forçage de mesure.
//
// Retour
//  true : OK
//  false : KO
//
bool CDetecteurRGB_TCS34725::publieParLoraP2POnOff(bool force/*=false*/) {
    if (onLoraP2PPublish == nullptr) return false;
    bool ret = false;
    String sValTemp = nomEquipement + " Del " + mDateTime->getDate() + " " + mDateTime->getTime() + " " + String(mbLastEtatOnOff);
    if (force) sValTemp += " FORCE";
    Serial.println("CDetecteurRGB_TCS34725::publieParLoraP2POnOff() Coul : " + sValTemp);
    if (onLoraP2PPublish != nullptr) {
      ret = (onLoraP2PPublish(sValTemp.c_str()) == 0);
    }
    else {
      Serial.printf("CDetecteurRGB_TCS34725::publieParLoraP2POnOff() - callback onMqttPublish() non définie\n");
    }
    return ret;
}

uint8_t CDetecteurRGB_TCS34725::computeCRC(const unsigned long* codes, int count) {
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
bool CDetecteurRGB_TCS34725::publieParCC1101(bool force/*=false*/) {
  //  unsigned long rgb = lastRgb; 
  //  unsigned long lux = lastLux; 
    int ret = false;
    unsigned long codes[4];
    int idx=0;
/*

    // Codage
    #ifdef TEST_EMISSION_RCS
    static int t=-20, h=0;
    temp = (float)t; hum = (float)h;
    t++; h++; if (t == 60) t=-20; if (h==100) h=0;
    // temp = 18.1; hum = 58.1;
    #endif // TEST_EMISSION_RCS
    
    // Affichage pour comparer avec la réception
    Serial.println("--- Valeurs à envoyer ---");
    Serial.printf("Couleur : 0x%lx\n", rgb);
    Serial.printf("Luminosité    : 0x%lx\n", lux);
    Serial.println("---------------------------------");

    uint16_t rgb_raw = constrain((int)((temp + 20.0f) * 10.0), 0, 799); // de -20°C à 60 °C à 0.1°C près ==> 81*10 = 810 valeurs. On mappe de 0 à 800
    uint16_t lux_raw  = constrain(hum, 0, 100); // de 0 à 63 % mappé de 0 à 100 %

    //temp_raw = 0xABCD; hum_raw = 0x2D;
    
    unsigned char temp_raw_LSB = temp_raw & 0x00FF;
    unsigned char temp_raw_MSB = (temp_raw >> 8);
    unsigned char hum_raw_LSB = hum_raw & 0x00FF;
    unsigned char hum_raw_MSB = (hum_raw >> 8);
    
    // 2 mesures → 6 codes au total (header + 2 x (mesure MSB + Mesure LSB) + fin)
    // Code 1 : Header (ID + Nb=2 + réservé=0)
//    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | 0UL;
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (4UL << 12) | 0x0FFF;
    // Code 2 : Température (type 0)
    //codes[idx++] = ((unsigned long)mucCapteurID << 16) | (1UL << 12) | (0UL << 8) | temp_raw;
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (1UL << 12) | (0UL << 8) | temp_raw_MSB;
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (1UL << 12) | (0UL << 8) | temp_raw_LSB;
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | (1UL << 8) | hum_raw_MSB;
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | (1UL << 8) | hum_raw_LSB;
    // Code 3 : Humidité (type 1)
    //codes[idx++] = ((unsigned long)mucCapteurID << 16) | (2UL << 12) | (1UL << 8) | hum_raw;
    // Code 4 : Fin (ID + CRC + réservé=0)
    uint8_t crc = computeCRC(codes, idx);
    Serial.printf("CRC Calculé : %d\n", crc);
    codes[idx++] = ((unsigned long)mucCapteurID << 16) | ((unsigned long)crc << 8);

    Serial.printf("Code %d : 0x%06lX\n", 1, codes[0]);
  //  mRCSwitch->envoieCode(codes[0]);  // Code 24 bits, protocol 1 (très courant)
  //  delay(mDelai_Inter_Envoi);
    Serial.printf("Code %d : 0x%06lX\n", 2, codes[1]);
  //  mRCSwitch->envoieCode(codes[1]);  // Code 24 bits, protocol 1 (très courant)
  //  delay(mDelai_Inter_Envoi);
    Serial.printf("Code %d : 0x%06lX\n", 3, codes[2]);
  //  mRCSwitch->envoieCode(codes[2]);  // Code 24 bits, protocol 1 (très courant)
  //  delay(mDelai_Inter_Envoi);
    Serial.printf("Code %d : 0x%06lX\n", 4, codes[3]);
  //  mRCSwitch->envoieCode(codes[3]);  // Code 24 bits, protocol 1 (très courant)
  //  delay(mDelai_Inter_Envoi);
    Serial.printf("Code %d : 0x%06lX\n", 5, codes[4]);
  //  mRCSwitch->envoieCode(codes[3]);  // Code 24 bits, protocol 1 (très courant)
  //  delay(mDelai_Inter_Envoi);
    Serial.printf("Code %d : 0x%06lX\n", 6, codes[5]);
  //  mRCSwitch->envoieCode(codes[3]);  // Code 24 bits, protocol 1 (très courant)
  //  delay(mDelai_Inter_Envoi);
    Serial.println("---------------------------------");
    #ifdef _RCSWITCH_MODE_
    mRCSwitch->envoieCode(codes, 6);
    #endif
    */
    ret = true;
    return ret;
}

bool CDetecteurRGB_TCS34725::read() {
    if (!_initialized) {
        Serial.println("CDetecteurRGB_TCS34725::read() - DHT20 non initialized"); 
        return false;
    }
    if (!active) {
        Serial.println("CDetecteurRGB_TCS34725::read() - DHT20 non actif"); 
        return false;
    }

    //------------------------------------------------------------------------
  // Contrôle d'erreur à faire
  //------------------------------------------------------------------------
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  // Luminosité approximative (clear channel)
  uint16_t lux = tcs.calculateLux(r, g, b);

  //------------------------------------------------------------------------
  // Pour une couleur soit valide, elle doit être remontée sous la forme :
  // 0xFFRRGGBB Si l'octet de poids fort n'est pas 0xFF ==> Erreur
  //
  // Pour qu'une luminosité soit valide, elle doit être remontée sous la forme : 
  // 0xFFFFFFXX, où XX = luminosité. 
  //------------------------------------------------------------------------
  //newRgb = 0xFF<<24 | (unsigned long)r<<16 | (unsigned long)g<<8 | (unsigned long)b;
  
  muiNewR = r;
  muiNewG = g;
  muiNewB = b;
  muiNewLux = lux; // Les bits de contrôle 0xFFFFFF00 seront ajoutés au moment de l'envoi (newLux = 0xFFFFFF00 | lux;)
  muiNewBrutLux = c;

  // On calcul le On / Off
  unsigned int seuilRGB = (muiRougeOn + muiRougeOff) / 2; // On fait la moyenne mais on peut envisager une autre formule
  unsigned int seuilLux = (muiLuxOn + muiLuxOff) / 2; // On fait la moyenne mais on peut envisager une autre formule
  if (muiNewR >= seuilRGB) {
    if (muiNewBrutLux >= seuilLux) {
      mbNewEtatOnOff = true;
    }
    else
      mbNewEtatOnOff = false;
  }
  else
    mbNewEtatOnOff = false;
  // On allume ou éteint la del 
  #ifdef LED_CAPTEUR_RGB
  unsigned char delStatus = mbNewEtatOnOff ? !mucLedRGBIsOFF : mucLedRGBIsOFF;
  digitalWrite(mucLedCapteurRGBPin, mbNewEtatOnOff);
  #endif
  
  Serial.printf("bool CDetecteurRGB_TCS34725::read() - R:0x%x=%d G:0x%x=%d B:0x%x=%d C:0x%x=%d → Lux ≈ 0x%x=%d\n", r, r, g, g, b, b, c, c, lux, lux);

  return true;
}

// Variante avec retry (utile en low-power, car parfois le premier read échoue)
bool CDetecteurRGB_TCS34725::readWithRetry(uint8_t retries/* = 3*/) {
    for (uint8_t i = 0; i < retries; i++) {
        if (read()) {
            return true;
        }
        delay(50);  // Petit délai entre retries
    }
    return false;
}

void CDetecteurRGB_TCS34725::setLoraP2PPublishCallback(std::function<int(const char*)> cb) {
    onLoraP2PPublish = cb;
}
void CDetecteurRGB_TCS34725::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
    onMqttPublish = cb;
}

void CDetecteurRGB_TCS34725::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Thermomètre");
  mucCapteurID = prefs.getUShort((mPrefixNVS+"id").c_str(), DEFAULT_CAPTEUR_RGB_TCS34725_ID);
  mucSdaPin = prefs.getUShort((mPrefixNVS+"sda").c_str(), DEFAULT_SDA_PIN);
  mucSclPin = prefs.getUShort((mPrefixNVS+"scl").c_str(), DEFAULT_SCL_PIN);
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "thermometre/");
  active = prefs.getBool((mPrefixNVS+"active").c_str(), true);
  remonterMesuresBrutes = prefs.getBool((mPrefixNVS+"brut").c_str(), false);
  mulIntervalleMesure = prefs.getLong((mPrefixNVS+"inter").c_str(), mulDefaultIntervalleMesure);
  mulIntervalleForcageRemonteeMesure = prefs.getLong((mPrefixNVS+"force").c_str(), mulDefaultIntervalleForcageRemonteeMesure);
  mucNbEnvois = prefs.getUShort((mPrefixNVS+"renvois").c_str(), 1);
  mulIntervalleEnvoi = prefs.getLong((mPrefixNVS+"interr").c_str(), mulDefaultIntervalleEnvoi);
  mucIntegrationTime = prefs.getUChar((mPrefixNVS+"integr").c_str(), TCS34725_INTEGRATIONTIME_614MS);
  mucGain = prefs.getUChar((mPrefixNVS+"gain").c_str(), TCS34725_GAIN_60X);
  muiRougeOn = prefs.getUInt((mPrefixNVS+"ron").c_str(), VAL_LED_ROUGE_ON);
  muiRougeOff = prefs.getUInt((mPrefixNVS+"roff").c_str(), VAL_LED_ROUGE_OFF);
  muiLuxOn = prefs.getUInt((mPrefixNVS+"luxon").c_str(), VAL_LUX_ON);
  muiLuxOff = prefs.getUInt((mPrefixNVS+"luxoff").c_str(), VAL_LUX_OFF);
  mucLedCapteurRGBPin = prefs.getUShort((mPrefixNVS+"rgbpin").c_str(), LED_CAPTEUR_RGB_PIN);
  mucLedRGBIsOFF = prefs.getUChar((mPrefixNVS+"rgboff").c_str(), LOW);

  
  // On forme les subtopic MQTT
  //domotique_prefix = prefs.getString((mPrefixNVS+"domo_pref").c_str(), default_domotique_topic_prefix);

  prefs.end();

  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";
  Serial.printf("void CDetecteurRGB_TCS34725::loadFromNVS() - mqttSubTopicCommand : '%s' mqttSubTopicState '%s'\n", mqttSubTopicCommand.c_str(), mqttSubTopicState.c_str());
  
}
void CDetecteurRGB_TCS34725::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  // Thermomètre
  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putUShort((mPrefixNVS+"id").c_str(), mucCapteurID);
  prefs.putUShort((mPrefixNVS+"sda").c_str(), mucSdaPin);
  prefs.putUShort((mPrefixNVS+"scl").c_str(), mucSclPin);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putBool((mPrefixNVS+"active").c_str(), active);
  prefs.putBool((mPrefixNVS+"brut").c_str(), remonterMesuresBrutes);
  prefs.putLong((mPrefixNVS+"inter").c_str(), mulIntervalleMesure);
  prefs.putLong((mPrefixNVS+"force").c_str(), mulIntervalleForcageRemonteeMesure);
  prefs.putUShort((mPrefixNVS+"renvois").c_str(), mucNbEnvois);
  prefs.putLong((mPrefixNVS+"interr").c_str(), mulIntervalleEnvoi);
  prefs.putUChar((mPrefixNVS+"integr").c_str(), mucIntegrationTime);
  prefs.putUChar((mPrefixNVS+"gain").c_str(), mucGain);
  prefs.putUInt((mPrefixNVS+"ron").c_str(), muiRougeOn);
  prefs.putUInt((mPrefixNVS+"roff").c_str(), muiRougeOff);
  prefs.putUInt((mPrefixNVS+"luxon").c_str(), muiLuxOn);
  prefs.putUInt((mPrefixNVS+"luxoff").c_str(), muiLuxOff);
//  prefs.putString((mPrefixNVS+"domo_pref").c_str(), domotique_prefix);
  prefs.putUShort((mPrefixNVS+"rgbpin").c_str(), mucLedCapteurRGBPin);
  prefs.putUChar((mPrefixNVS+"rgboff").c_str(), mucLedRGBIsOFF);

  prefs.end();
}

void CDetecteurRGB_TCS34725::setActive(bool state) {
  active = state;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"active").c_str(), state);
  prefs.end();

  active = state;
}

void CDetecteurRGB_TCS34725::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"id").c_str())) mucCapteurID = server.arg((mPrefixNVS+"id")).toInt();
  if (server.hasArg((mPrefixNVS+"sda").c_str())) mucSdaPin = server.arg((mPrefixNVS+"sda")).toInt();
  if (server.hasArg((mPrefixNVS+"scl").c_str())) mucSclPin = server.arg((mPrefixNVS+"scl")).toInt();
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"brut").c_str())) remonterMesuresBrutes = true; else remonterMesuresBrutes = false;
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"inter").c_str())) mulIntervalleMesure = server.arg((mPrefixNVS+"inter")).toInt();
  if (server.hasArg((mPrefixNVS+"force").c_str())) mulIntervalleForcageRemonteeMesure = server.arg((mPrefixNVS+"force")).toInt();
  if (server.hasArg((mPrefixNVS+"renvois").c_str())) mucNbEnvois = server.arg((mPrefixNVS+"renvois")).toInt();
  if (server.hasArg((mPrefixNVS+"interr").c_str())) mulIntervalleEnvoi = server.arg((mPrefixNVS+"interr")).toInt();
  if (server.hasArg((mPrefixNVS+"integr").c_str())) mucIntegrationTime = server.arg((mPrefixNVS+"integr")).toInt();
  if (server.hasArg((mPrefixNVS+"gain").c_str())) mucGain = server.arg((mPrefixNVS+"gain")).toInt();
  if (server.hasArg((mPrefixNVS+"ron").c_str())) muiRougeOn = server.arg((mPrefixNVS+"ron")).toInt();
  if (server.hasArg((mPrefixNVS+"roff").c_str())) muiRougeOff = server.arg((mPrefixNVS+"roff")).toInt();
  if (server.hasArg((mPrefixNVS+"luxon").c_str())) muiLuxOn = server.arg((mPrefixNVS+"luxon")).toInt();
  if (server.hasArg((mPrefixNVS+"luxoff").c_str())) muiLuxOff = server.arg((mPrefixNVS+"luxoff")).toInt();
  if (server.hasArg((mPrefixNVS+"rgbpin").c_str())) mucLedCapteurRGBPin = server.arg((mPrefixNVS+"rgbpin")).toInt();
  if (server.hasArg((mPrefixNVS+"rgboff").c_str())) mucLedRGBIsOFF = server.arg((mPrefixNVS+"rgboff")).toInt();
}

String CDetecteurRGB_TCS34725::getHTML() {
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
        "<div><label>GPIO SDA</label><input type=\"text\" name=" + (mPrefixNVS+"sda") + " value=\"" + mucSdaPin + "\"></div>"
        "<div><label>GPIO SCL</label><input type=\"text\" name=" + (mPrefixNVS+"scl") + " value=\"" + mucSclPin + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Temps d'intégration</label><input type=\"text\" name=" + (mPrefixNVS+"integr") + " value=\"" + mucIntegrationTime + "\"></div>"
        "<div><label>Gain</label><input type=\"text\" name=" + (mPrefixNVS+"gain") + " value=\"" + mucGain + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Valeur Led ON (0-255)</label><input type=\"text\" name=" + (mPrefixNVS+"ron") + " value=\"" + muiRougeOn + "\"></div>"
        "<div><label>Valeur Led OFF (0-255)</label><input type=\"text\" name=" + (mPrefixNVS+"roff") + " value=\"" + muiRougeOff + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>DEL RGB Pin</label><input type=\"text\" name=" + (mPrefixNVS+"rgbpin") + " value=\"" + mucLedCapteurRGBPin + "\"></div>"
        "<div><label>DEL OFF (0 ou 1)</label><input type=\"text\" name=" + (mPrefixNVS+"rgboff") + " value=\"" + mucLedRGBIsOFF + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Luminosité ON (0-255)</label><input type=\"text\" name=" + (mPrefixNVS+"luxon") + " value=\"" + muiLuxOn + "\"></div>"
        "<div><label>Luminosité OFF (0-255)</label><input type=\"text\" name=" + (mPrefixNVS+"luxoff") + " value=\"" + muiLuxOff + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div class=\"checkbox-row\"><label>Mesures brutes</label><input type=\"checkbox\" name=" + (mPrefixNVS+"brut") + " value=\"0\"" + String(remonterMesuresBrutes ? " checked" : "") + "></div>"
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

void CDetecteurRGB_TCS34725::print() const {

  Serial.printf("     Nom                  : %s\n", nomEquipement);
  Serial.printf("     ID                   : %d\n", mucCapteurID);
  Serial.printf("     Actif                : %s\n", active ? "OUI" : "NON");
  Serial.printf("     GPIO SDA             : %d\n", mucSdaPin);
  Serial.printf("     GPIO SCL             : %d\n", mucSclPin);
  Serial.printf("     DEL RGB PIN          : %d\n", mucLedCapteurRGBPin);
  Serial.printf("     DEL RGB OFF (0 ou 1) : %d\n", mucLedRGBIsOFF);
  Serial.printf("     Temps d'intégration  : 0x%x\n", mucIntegrationTime);
  Serial.printf("     Gain                 : 0x%x\n", mucGain);
  Serial.printf("     Valeur Led ON        : %d\n", muiRougeOn);
  Serial.printf("     Valeur Led OFF       : %d\n", muiRougeOff);
  Serial.printf("     Luminosité ON        : %d\n", muiLuxOn);
  Serial.printf("     Luminosité OFF       : %d\n", muiLuxOff);
  Serial.printf("     Remontée brute       : %s\n", remonterMesuresBrutes ? "OUI" : "NON");
  Serial.printf("     Intervalle mesures   : %ld s\n", mulIntervalleMesure);
  Serial.printf("     Forcage remontée     : %ld s\n", mulIntervalleForcageRemonteeMesure);
  Serial.printf("     Nombre de renvois    : %d\n", mucNbEnvois);
  Serial.printf("     Intervalle de renvoi : %ld s\n", mulIntervalleEnvoi);
  Serial.printf("     MQTTSubTopic         : %s\n", mqttSubTopic.c_str());

  Serial.printf("     Domo prefix          : %s\n", domotique_prefix.c_str());
  Serial.printf("     MQTTCmd              : %s\n", mqttSubTopicCommand.c_str());
  Serial.printf("     MQTTState            : %s\n", mqttSubTopicState.c_str());
}

//
// Retour
// 0 : RAS
// -1 : Mauvais nom d'équipement
// -2 : Commande incomplète
// 
int CDetecteurRGB_TCS34725::handleMqttCommand(const String& payload) {
  int ret = 0;
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

    MQTT_COMMAND_33 st;
    int idx = 0;
    Serial.println("=== CDetecteurRGB_TCS34725::handleMqttCommand ===");
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