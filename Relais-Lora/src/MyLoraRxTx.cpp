#include "global.h"
#include <WiFi.h>
#include "MyConfig.h"
#include "MyLoraRxTx.h"

// this file contains binary patch for the SX1262
//#include <modules/SX126x/patches/SX126x_patch_scan.h>

extern CMyDateTime mDateTime;

volatile bool operationDone = false;


#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  operationDone = true;
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
  
  int state = mRadio.begin(mfFrequency, mfBandwidth, mucSpreadingFactor, mucCodingRate, mucSyncWord, mfOutputPower, muiPreambleLength, 1.6, false);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    // Active la callback sur DIO1 (paquet reçu)
    mRadio.setDio1Action(setFlag);    
    // Démarre la réception continue
    state = mRadio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("Réception démarrée");
    } 
    else {
      Serial.print("startReceive failed, code ");
      Serial.println(state);
      return -9;
    }
    ret = 0;
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    ret = -8;
  } // if (state == RADIOLIB_ERR_NONE)
  
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
void CMyLoraRxTx::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
    onMqttPublish = cb;
}

int CMyLoraRxTx::handleIncommingLoraP2PMessage(const String& inMessage) {
  int ret = 0;
  ParsedLoraP2PMessage msg;
  int nb = parseMessage(msg, inMessage);

  Serial.println("int CMyLoraRxTx::handleIncommingLoraP2PMessage() - Chaîne : " + inMessage);
  //printParsed(msg, nb);

  //Serial.println("---------------------");

  // On met en place la date et l'heure
  for (size_t i = 0; i < msg.fields.size(); i++) {
    //msg.fields[i].toUpperCase();
    if (msg.fields[i] == "DATE") msg.fields[i] = mDateTime.getDate();
    else if (msg.fields[i] == "TIME") msg.fields[i] = mDateTime.getTime();
  }
  //Serial.println("++++++++++++++++++++++++++++");
  //printParsed(msg, nb);
  //Serial.println("++++++++++++++++++++++++++++");
  if (msg.fields.size() > 0) {
    // On forme le message MQTT à envoyer
    String mqttMsg="";
    for (size_t i = 0; i < msg.fields.size(); i++) {
      mqttMsg += msg.fields[i] + " ";
    }
    // On ajoute l'IP
    mqttMsg += "0.0.0.0"; //WiFi.localIP().toString();
    // Envoi
    if (onMqttPublish != nullptr) {
      String sTopic = msg.topic;
      onMqttPublish(sTopic.c_str(), mqttMsg.c_str());
      //Serial.printf("int CMyLoraRxTx::handleIncommingLoraP2PMessage() - Envoi MQTT sur %s : %s\n", sTopic.c_str(), mqttMsg.c_str());
    }
  }
  ret = nb;
  return ret;
}
//
// Retour
// 0 : RAS
// -1 : Erreur de lecture des données reçues
//
int CMyLoraRxTx::loop() {
  int ret = 0;

  // check if the flag is set
  if(operationDone) {
    // reset flag
    operationDone = false;

    // you can read received data as an Arduino String
    String str;
    int state = mRadio.readData(str);

    handleIncommingLoraP2PMessage(str);

    /*if (state == RADIOLIB_ERR_NONE) {
      // packet was successfully received
      Serial.println(F("[SX1262] Received packet!"));

      // print data of the packet
      Serial.print(F("[SX1262] Data:\t\t"));
      Serial.println(str);

      // print RSSI (Received Signal Strength Indicator)
      Serial.print(F("[SX1262] RSSI:\t\t"));
      Serial.print(mRadio.getRSSI());
      Serial.println(F(" dBm"));

      // print SNR (Signal-to-Noise Ratio)
      Serial.print(F("[SX1262] SNR:\t\t"));
      Serial.print(mRadio.getSNR());
      Serial.println(F(" dB"));

      // print mfFrequency error
      Serial.print(F("[SX1262] Frequency error:\t"));
      Serial.print(mRadio.getFrequencyError());
      Serial.println(F(" Hz"));

    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial.println(F("CRC error!"));

    } else {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);

    }*/
  }
  
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
void CMyLoraRxTx::sendPacket(const char* message) {
  Serial.print(F("[SX1262] Sending packet ... "));
  transmissionState = mRadio.startTransmit(message);
  transmitFlag = true;
}

// Receive packet
void CMyLoraRxTx::receivePacket() {
  String str;
  int state = mRadio.readData(str);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("[SX1262] Received packet!"));
    Serial.print(F("[SX1262] Data: "));
    Serial.println(str);
    Serial.print(F("[SX1262] RSSI: "));
    Serial.print(mRadio.getRSSI());
    Serial.println(F(" dBm"));
    Serial.print(F("[SX1262] SNR: "));
    Serial.print(mRadio.getSNR());
    Serial.println(F(" dB"));
  } else {
    Serial.print(F("readData failed, code "));
    Serial.println(state);
  }
}