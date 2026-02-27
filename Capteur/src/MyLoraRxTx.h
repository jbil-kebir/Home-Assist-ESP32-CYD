#ifndef __MY_LORA_RX_TX_H__
#define __MY_LORA_RX_TX_H__

#ifdef _LORA_P2P_MODE_
#include <RadioLib.h>
class CConfig;



class CMyLoraRxTx {
private:
  CConfig* mConfig;

  SX1262* mRadio=nullptr; // C'est un pointeur

  bool mbInitialized = false; // Mettre à true si tout le processus d'init s'est bien déroulé 
  // Flag pour savoir si une réception est terminée (set par la callback)
  volatile bool mbReceptionDone = false;
  // Buffer pour stocker le dernier message reçu
  String msLastReceivedMessage;
  //---------------------- PINS ----------------------
  unsigned char mucMisoPin  = LORA_MISO_PIN;
  unsigned char mucSckPin  = LORA_SCK_PIN;
  unsigned char mucMosiPin  = LORA_MOSI_PIN;
  unsigned char mucCsPin  = LORA_CS_PIN;
  unsigned char mucDio2Pin  = LORA_DIO2_PIN;
  unsigned char mucDio1Pin  = LORA_DIO1_PIN;
  unsigned char mucResetPin  = LORA_RESET_PIN;
  unsigned char mucBusyPin  = LORA_BUSY_PIN;
  //---------------------- PROTOCOLE ----------------------
  float mfFrequency = LORA_FREQUENCE;
  float mfBandwidth = LORA_BANDWIDTH;
  uint8_t mucSpreadingFactor = LORA_SPREADING_FACTOR;
  uint8_t mucCodingRate = LORA_CODING_RATE;
  uint8_t mucSyncWord = LORA_SYNC_WORD;
  float mfOutputPower = LORA_OUTPUT_POWER;
  uint16_t muiPreambleLength = LORA_PREAMBLE_LENGTH;

public:
  CMyLoraRxTx(CConfig& cfg/*, SX1262& radio*/);
  int setup();
  int loop();
  // Méthode pour récupérer le dernier message reçu
  String getLastMessage() const { return msLastReceivedMessage; }

  //=========================================================================================
  //bool transmitFlag = false;
  //int transmissionState = RADIOLIB_ERR_NONE;
  
  void receivePacket();
  int sendPacket(const char* message);
  int validateParameters();


};
#endif // _LORA_P2P_MODE_

#endif // __MY_LORA_RX_TX_H__