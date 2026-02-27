#include "global.h"
#include "MyConfig.h"
#ifdef _LORA_P2P_MODE_

#include "MyLoraRxTx.h"

// this file contains binary patch for the SX1262
//#include <modules/SX126x/patches/SX126x_patch_scan.h>


volatile bool operationDone = false;
// this file contains binary patch for the SX1262
//#include <modules/SX126x/patches/SX126x_patch_scan.h>

SX1262 radio = new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RESET_PIN, LORA_BUSY_PIN);

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  operationDone = true;
}

CMyLoraRxTx::CMyLoraRxTx(CConfig& cfg/*, SX1262& radio*/) : mConfig(&cfg)/*, mRadio(radio)*/ {
  //mRadio = &
  };

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
  
  while (true) { 
    int nbTentatives = 5; // 5 tentatives
    int state = radio.begin(mfFrequency, mfBandwidth, mucSpreadingFactor, mucCodingRate, mucSyncWord, mfOutputPower, muiPreambleLength, 1.6, false);

    if (state == RADIOLIB_ERR_NONE) {
      Serial.println(F("success!"));
      // Active la callback sur DIO1 (paquet reçu)
      radio.setDio1Action(setFlag);    
      // Démarre la réception continue
      state = radio.startReceive();
      if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Réception démarrée");
      } 
      else {
        Serial.print("startReceive failed, code ");
        Serial.println(state);
        return -9;
      }
      ret = 0; 
      mbInitialized = true;
    } 
    else {
      Serial.print(F("failed, code "));
      Serial.println(state);
      ret = -8;
    } // if (state == RADIOLIB_ERR_NONE)
    nbTentatives--;
    
  if ( (ret == 0) || (nbTentatives == 0) ) break;
  delay(5000);
  }
  
  return ret;
}

//
// Retour
// 0 : RAS
// -1 : Erreur de lecture des données reçues
// -9 : Non initialisé
//
int CMyLoraRxTx::loop() {
  int ret = 0;
  if (!mbInitialized) return -9;
  // check if the flag is set
  if(operationDone) {
    // reset flag
    operationDone = false;

    // you can read received data as an Arduino String
    String str;
    int state = radio.readData(str);


    /*if (state == RADIOLIB_ERR_NONE) {
      // packet was successfully received
      Serial.println(F("[SX1262] Received packet!"));

      // print data of the packet
      Serial.print(F("[SX1262] Data:\t\t"));
      Serial.println(str);

      // print RSSI (Received Signal Strength Indicator)
      Serial.print(F("[SX1262] RSSI:\t\t"));
      Serial.print(radio.getRSSI());
      Serial.println(F(" dBm"));

      // print SNR (Signal-to-Noise Ratio)
      Serial.print(F("[SX1262] SNR:\t\t"));
      Serial.print(radio.getSNR());
      Serial.println(F(" dB"));

      // print mfFrequency error
      Serial.print(F("[SX1262] Frequency error:\t"));
      Serial.print(radio.getFrequencyError());
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
int CMyLoraRxTx::sendPacket(const char* message) {
  int ret = 0;//RADIOLIB_ERR_NONE;
  if (!mbInitialized) return -9;
  String sToSend = START_STOP + String(message) + START_STOP;
  Serial.printf("[SX1262] Sending packet <%s> ... \n", sToSend.c_str());
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

#endif // #ifdef _LORA_P2P_MODE_
