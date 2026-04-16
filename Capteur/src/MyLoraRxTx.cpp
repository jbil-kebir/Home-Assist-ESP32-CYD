#include "global.h"
#include "MyConfig.h"
// Test : on initialise Lora pour l'endormir si pas utilisé
//#ifdef _LORA_P2P_MODE_

#include "MyLoraRxTx.h"

// this file contains binary patch for the SX1262
//#include <modules/SX126x/patches/SX126x_patch_scan.h>

enum RadioMode { MODE_RX, MODE_TX };
RadioMode currentMode = MODE_RX;


volatile bool operationDone = false;
// this file contains binary patch for the SX1262
//#include <modules/SX126x/patches/SX126x_patch_scan.h>

SX1262 radio = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RESET_PIN, LORA_BUSY_PIN);

void startRx() {
  radio.startReceive();
  operationDone = false;   // efface l'interruption parasite de la transition d'état
  currentMode = MODE_RX;
}


#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  operationDone = true;
}

CMyLoraRxTx::CMyLoraRxTx(CConfig& cfg/*, SX1262& radio*/) : mConfig(&cfg)/*, mRadio(radio)*/ {
  //mRadio = &
  };

void CMyLoraRxTx::sleep() {
  if (!mbInitialized) {
    Serial.println("[LoRa] sleep() ignoré : module non initialisé");
    Serial.flush();
    return;
  }
  int state = radio.sleep(false); // false = cold sleep (~0.5 µA), le module sera réinitialisé au réveil
  Serial.printf("[LoRa] sleep(false) => %d (%s)\n", state, state == 0 ? "OK" : "ERREUR");
  Serial.flush();
}


//
// Retour
// 0 : RAS
// -7 < Erreur < -1 : Paramètres Lora invalides
// -8 : Echec du begin()
// -9 : Echec au démarrage de la réception
//
int CMyLoraRxTx::setup() {
  int ret = 0;

  ret = validateParameters(); // Si < 0 => Params invalides
  if (ret < 0) return ret;

  Serial.print(F("[SX1262] Initializing ... "));
  // Force SPI init
  SPI.begin(mucSckPin, mucMisoPin, mucMosiPin, mucCsPin);  
  
  int nbTentatives = 5; // 5 tentatives
  while (true) {
    int state = radio.begin(mfFrequency, mfBandwidth, mucSpreadingFactor, mucCodingRate, mucSyncWord, mfOutputPower, muiPreambleLength, LORA_TCXO_VOLTAGE, false);

    if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));
      // Active la callback sur DIO1 (paquet reçu)
      //radio.setDio1Action(setFlag);    
      // Démarre la réception continue
      /*state = radio.startReceive();
      if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Réception démarrée");
      } 
      else {
        Serial.print("startReceive failed, code ");
        Serial.println(state);
        return -9;
      }*/
      ret = 0; 
      mbInitialized = true;
      // Un seul callback pour TX done et RX done
      radio.setPacketSentAction(setFlag);
      radio.setPacketReceivedAction(setFlag);

      startRx();

    } 
    else {
      Serial.print(F("failed, code "));
      Serial.println(state);
      mbInitialized = false;
      ret = -8;
    } // if (state == RADIOLIB_ERR_NONE)
    nbTentatives--;
    
  if ( (ret == 0) || (nbTentatives == 0) ) break;
  delay(5000);
  }
  
  return ret;
}

// Retour :
// > 0  = nombre total de champs extraits (topic + fields)
// -1   = présence d'un '=' interdit à l'intérieur
// -2   = ne commence pas par '='
// -3   = ne finit pas par '='
// -4   = pas assez de champs (moins de 2 : topic + au moins 1 champ)
int CMyLoraRxTx::parseMessage(ParsedLoraP2PMessage& result, const String& input) {
    result.isValid = false;
    result.fields.clear();

    String s = input;
    s.trim();

    // 1. Vérifier qu'il commence et finit par au moins un '='
    if (!s.startsWith("=") || !s.endsWith("=")) {
        return -2;  // pas de bordure '='
    }

    // Trouver la fin des '=' de début
    int startContent = 0;
    while (startContent < s.length() && s[startContent] == '=') startContent++;

    // Trouver le début des '=' de fin
    int endContent = s.length() - 1;
    while (endContent >= 0 && s[endContent] == '=') endContent--;

    if (startContent >= endContent) return -3;  // contenu vide

    // 2. Vérifier qu'il n'y a AUCUN '=' dans la partie centrale
    String middle = s.substring(startContent, endContent + 1);
    if (middle.indexOf('=') >= 0) return -1;

    // 3. Découpage en tokens (espaces multiples ignorés)
    std::vector<String> tokens;
    int pos = 0;
    while (pos < middle.length()) {
        while (pos < middle.length() && middle[pos] == ' ') pos++;
        if (pos >= middle.length()) break;

        int next = middle.indexOf(' ', pos);
        if (next == -1) next = middle.length();

        String token = middle.substring(pos, next);
        token.trim();
        if (token.length() > 0) tokens.push_back(token);

        pos = next;
    }

    // 4. Vérification minimale : au moins topic
    if (tokens.size() < 1) return -4;

    // 5. Remplissage
    result.topic  = tokens[0];
    //result.device = tokens[1];

    // Tous les champs suivants vont dans le vecteur fields
    for (size_t i = 1; i < tokens.size(); i++) {
        result.fields.push_back(tokens[i]);
    }

    result.isValid = true;

    // Retourne le nombre total de champs utiles (topic + device + fields)
    return 1 + result.fields.size();
}

void printParsed(const ParsedLoraP2PMessage& msg, int nbChamps) {
if (!msg.isValid || nbChamps < 0) {
        Serial.println("Parsing invalide");
        return;
    }

    Serial.print("Succès - ");
    Serial.print(nbChamps);
    Serial.println(" champs extraits");

    Serial.print("  Topic  : "); Serial.println(msg.topic);
    //Serial.print("  Device : "); Serial.println(msg.device);

    Serial.println("  Champs suivants :");
    for (size_t i = 0; i < msg.fields.size(); i++) {
        Serial.print("    [");
        Serial.print(i + 1);
        Serial.print("]  : ");
        Serial.println(msg.fields[i]);
    }
    Serial.println();
}

//
// Récupère la chaîne de caractères reçue par Lora, la parse, et si elle est valide, ajoute : 
// - la date et l'heure (en remplaçant les tokens "DATE" et "TIME" s'ils sont présents)
// - 0.0.0.0 (à la fin du message, pour l'instant, à remplacer par l'IP réelle si besoin)
// Exemple d'entrée :
// =home/thermometre/state=ThNomade Temp DATE TIME 23.0
// Exemple de sortie MQTT :
// Topic : home/thermometre/state 
// Payload : ThNomade Temp 2024-06-01 14:30:00 23.0
// Retour :
// > 0  = nombre total de champs extraits (topic + fields)  
// 0 = message non parsé (ex: message vide ou ne respectant pas le format de base)
//
int CMyLoraRxTx::handleIncommingLoraP2PMessage(const String& inMessage) {
  Serial.println("int CMyLoraRxTx::handleIncommingLoraP2PMessage() - Reçu : " + inMessage);

  ParsedLoraP2PMessage msg;
  int nb = parseMessage(msg, inMessage);

  if (nb <= 0 || !msg.isValid) {
    Serial.printf("handleIncommingLoraP2PMessage() - parsing invalide (%d)\n", nb);
    return nb;
  }

  // printParsed(msg, nb);

  String topic = msg.topic; // topic = 1er champ
  // Construction payload
  String payload = "";
  for (size_t i = 0; i < msg.fields.size(); i++) {
    if (i > 0) payload += " ";
    payload += msg.fields[i];
  }
  String topicUpper = topic; topicUpper.toUpperCase();
  String payloadUpper = payload; payloadUpper.toUpperCase();  

  #ifdef CAPTEUR_DS18B20
  String sDs18B20Equipement = mConfig->ds18b20->nomEquipement; sDs18B20Equipement.toUpperCase();
  #endif
  #ifdef CAPTEUR_DHT20  
  String sDht20Equipement = mConfig->dht20->nomEquipement; sDht20Equipement.toUpperCase();
  #endif
  #ifdef CAPTEUR_RGB_TCS34725
  String sRgbEquipement = mConfig->mCapteurRGB->nomEquipement; sRgbEquipement.toUpperCase();
  #endif
  #ifdef FLOTTEUR_VERTICAL
  String sFlotteurEquipement = mConfig->mFlotteurVertical->nomEquipement; sFlotteurEquipement.toUpperCase();
  #endif

    // Commandes de configuration
  if (topic == mConfig->topic_config_command) {
    Serial.println("int CMyLoraRxTx::handleIncommingLoraP2PMessage() - Commande configuration reçue : " + payload);
    mConfig->handleMqttCommand(payload);
    if (payloadUpper == "STATUS") {
      String statusMsg = "=== ÉTAT "+ String(mConfig->nomEquipement) +" ===\n";
      statusMsg += String(mConfig->mDateTime->getDate()) + " " + String(mConfig->mDateTime->getTime()) + "\n";
      if (WiFi.status() == WL_CONNECTED) { // On peut être en Lora et avoir une adresse IP (voir commutateur _WIFI_MODE_ dans global.h)
        statusMsg += "IP     : " + WiFi.localIP().toString() + "\n";
      }
      statusMsg += "Uptime : " + String(millis() / 1000) + "s";

      sendPacket((mConfig->topic_config_state + " " + statusMsg).c_str());
      Serial.println("STATUS publié sur " + mConfig->topic_config_state);
      Serial.println(statusMsg); Serial.flush();
    }
    else if (payloadUpper == "REBOOT") {
      ESP.restart();
    }
  } // if (String(topic) == mConfig.topic_config_command) { 
  else if (topic == "home/thermometre/command") {
    if (false) {
      // Traiter les commandes de configuration
    }
    #ifdef CAPTEUR_DS18B20
    else if (payloadUpper.startsWith(sDs18B20Equipement)) {
      mConfig->ds18b20->handleMqttCommand(payload);
    }
    #endif
    #ifdef CAPTEUR_DHT20
    else if (payloadUpper.startsWith(sDht20Equipement)) {
      mConfig->dht20->handleMqttCommand(payload );
    }
    #endif
    #ifdef CAPTEUR_RGB_TCS34725
    else if (payloadUpper.startsWith(sRgbEquipement)) {
      mConfig->mCapteurRGB->handleMqttCommand(payload);
    }
    #endif
    #ifdef FLOTTEUR_VERTICAL
    else if (payloadUpper.startsWith(sFlotteurEquipement)) {
      mConfig->mFlotteurVertical->handleMqttCommand(payload);
    } 
    #endif
    else {
      Serial.println("void CMyLoraRxTx::handleIncommingLoraP2PMessage() - Non traité " + topic + " : " + payload);
      // Traiter les autres commandes MQTT spécifiques à d'autres capteurs ou fonctions
    }

  } // if (String(topic) == "home/thermometre/command") {
  else {
    Serial.printf("void CMyLoraRxTx::handleIncommingLoraP2PMessage() - Topic non pris en charge - %s : %s\n", topic.c_str(), payload.c_str());
  }


  return nb;
}


//
// Retour
// 0 : RAS
// -1 : Erreur de lecture des données reçues
// -9 : Non initialisé
// > 0  = nombre total de champs extraits (topic + fields)
// -1   = présence d'un '=' interdit à l'intérieur
// -2   = ne commence pas par '='
// -3   = ne finit pas par '='
// -4   = pas assez de champs (moins de 2 : topic + au moins 1 champ)
int CMyLoraRxTx::loop() {
  int ret = 0;
  if (!mbInitialized) return -9;
  if (operationDone) {
    operationDone = false;

    if (currentMode == MODE_TX) {
      // C'était une émission
      radio.finishTransmit();
      Serial.println(F("[LoRa] Envoi terminé, retour en écoute."));
      startRx();
  
    } else {
      // C'était une réception
      String received;
      int state = radio.readData(received);

      if (state == RADIOLIB_ERR_NONE) {
        Serial.print(F("[LoRa] Reçu : "));
        Serial.println(received);
        Serial.print(F("  RSSI : "));
        Serial.print(radio.getRSSI());
        Serial.print(F(" dBm  SNR : "));
        Serial.print(radio.getSNR());
        Serial.println(F(" dB"));
      } else {
        Serial.print(F("[LoRa] Erreur RX, code "));
        Serial.println(state);
      }
      Serial.printf("int CMyLoraRxTx::loop() - Reçu (RSSI: %.1f dBm) : %s\n", radio.getRSSI(), received.c_str());

      ret = handleIncommingLoraP2PMessage(received);

      startRx();
    }
  }
  /*if (operationDone) {
    operationDone = false;

    String str;
    int state = radio.readData(str);

    // Relancer la réception continue immédiatement
    radio.startReceive();

    if (state != RADIOLIB_ERR_NONE) {
      Serial.printf("loop() - readData failed, code %d\n", state);
      return -1;
    }

    Serial.printf("int CMyLoraRxTx::loop() - Reçu (RSSI: %.1f dBm) : %s\n", radio.getRSSI(), str.c_str());

    ret = handleIncommingLoraP2PMessage(str);
  }*/

  return ret;
}


// Function to validate parameters
int CMyLoraRxTx::validateParameters() { //float mfFrequency, float mfBandwidth, uint8_t mucSpreadingFactor, uint8_t mucCodingRate, uint8_t mucSyncWord, float mfOutputPower, uint16_t muiPreambleLength) {
  if (mfFrequency < 150.0 || mfFrequency > 960.0) {
    Serial.println(F("Error: Frequency must be between 150.0 MHz and 960.0 MHz."));
    return -1;
  }
  if (mfBandwidth != 7.8 && mfBandwidth != 10.4 && mfBandwidth != 15.6 && mfBandwidth != 20.8 && mfBandwidth != 31.25 &&
      mfBandwidth != 41.7 && mfBandwidth != 62.5 && mfBandwidth != 125.0 && mfBandwidth != 250.0 && mfBandwidth != 500.0) {
    Serial.println(F("Error: Invalid mfBandwidth value."));
    return -2;
  }
  if (mucSpreadingFactor < 6 || mucSpreadingFactor > 12) {
    Serial.println(F("Error: Spreading factor must be between 6 and 12."));
    return -3;
  }
  if (mucCodingRate < 5 || mucCodingRate > 8) {
    Serial.println(F("Error: Coding rate must be between 5 and 8."));
    return -4;
  }
  if (mfOutputPower < -17.0 || mfOutputPower > 22.0) {
    Serial.println(F("Error: Output power must be between -17 dBm and 22 dBm."));
    return -5;
  }
  if (muiPreambleLength < 6 || muiPreambleLength > 65535) {
    Serial.println(F("Error: Preamble length must be between 6 and 65535 symbols."));
    return -6;
  }
  if (mucSyncWord > 0xFF) {
    Serial.println(F("Error: Sync word must be a valid 1-byte value."));
    return -7;
  }
  return 0;
}


// Send packet
int CMyLoraRxTx::sendPacket(const char* message) {
  int ret = 0;//RADIOLIB_ERR_NONE;
  if (!mbInitialized) return -9;
  String sToSend = START_STOP + String(message);
  if (WiFi.status() == WL_CONNECTED) { // On peut être en Lora et avoir une adresse IP (voir commutateur _WIFI_MODE_ dans global.h)
    sToSend += String(" ") + WiFi.localIP().toString();
  } 
  sToSend += START_STOP; // pour marquer la fin du message (utile pour le parsing côté réception)
  Serial.printf("[SX1262] Sending packet <%s> ... \n", sToSend.c_str());
  currentMode = MODE_TX;
  operationDone = false;   // efface l'interruption parasite de la transition d'état
  /*transmissionState = */ret = radio.startTransmit(sToSend.c_str());
  //transmitFlag = true;
  if (ret == RADIOLIB_ERR_NONE) ret = 0;
  return ret;
}

// Receive packet
void CMyLoraRxTx::receivePacket() {
  String str;
  int state = radio.readData(str);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("[SX1262] Received packet!"));
    Serial.print(F("[SX1262] Data: "));
    Serial.println(str);
    Serial.print(F("[SX1262] RSSI: "));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));
    Serial.print(F("[SX1262] SNR: "));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));
  } else {
    Serial.print(F("readData failed, code "));
    Serial.println(state);
  }
}

//#endif // #ifdef _LORA_P2P_MODE_
