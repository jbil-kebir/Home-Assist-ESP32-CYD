#include "global.h"
#ifdef _RCSWITCH_MODE_
#include "MyRCSwitch.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "MyConfig.h"

void CMyRCSwitch::setup() {
  Serial.println("\n=== RCSwitch ÉMETTEUR - Multi-codes + CRC (test aléatoire) ===\n");

  ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(433.92);
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setRxBW(812.50);
  ELECHOUSE_cc1101.setDeviation(0);
  ELECHOUSE_cc1101.setPktFormat(3);
  ELECHOUSE_cc1101.setSyncMode(0);
  ELECHOUSE_cc1101.setCrc(0);
  ELECHOUSE_cc1101.setPA(12);
  ELECHOUSE_cc1101.SetRx();

// Test Grok   
/*ELECHOUSE_cc1101.setMHZ(433.92);
ELECHOUSE_cc1101.setModulation(0);     // 2-FSK (plus robuste que ASK)
ELECHOUSE_cc1101.setDeviation(47.607); // Deviation standard pour bonne portée
ELECHOUSE_cc1101.setDRate(4.8);        // Débit bas = meilleure sensibilité
ELECHOUSE_cc1101.setPA(12); */           // Max puissance

  mySwitch.enableReceive(CC1101_GDO0);

}


int CMyRCSwitch::envoieCode(unsigned long *cde, int nb) {
  int ret = 0;

  mySwitch.disableReceive(); 
  ELECHOUSE_cc1101.SetTx();
  delay(100);

  mySwitch.enableTransmit(CC1101_GDO0);
  // test KJ
  /*float freq = 433.90;
  static unsigned long code = 0L; //0x3D3B72UL;
  int protocole = 1;
  int pulseLen = 349;
  int rep = 5;
  ELECHOUSE_cc1101.setMHZ(freq);
  mySwitch.setProtocol(protocole);
  mySwitch.setRepeatTransmit(rep);
  mySwitch.setPulseLength(pulseLen);
  String s = String(freq) + " MHz, " + String("0x") + String(code, HEX) + ", " + String(protocole) + ", " + String(pulseLen) + ", " + String(rep);
  Serial.printf("CMyRCSwitch::envoieCode() - Envoi %s\n", s.c_str());
  mySwitch.send(code, 24);
  code++;*/

      // Test Grok
      //mySwitch.setProtocol(2);
      frequency = 433.92;
      protocol = 1;
      pulseLength = 349;
      repeat = 3;
      unsigned long delaiEntreEnvois = 250; //350;
      ELECHOUSE_cc1101.setMHZ(frequency);
      ELECHOUSE_cc1101.setPA(12);
      ELECHOUSE_cc1101.setModulation(2);
      //ELECHOUSE_cc1101.setDeviation(47.607);
      ELECHOUSE_cc1101.setDRate(2.4);//4.8);
      mySwitch.setProtocol(protocol);
      mySwitch.setPulseLength(pulseLength);
      mySwitch.setRepeatTransmit(repeat);//4);//3);

  for (int i = 0; i< nb; i++) {
    unsigned long code = cde[i];
    String s = String(frequency) + " MHz, " + String(code, HEX) + ", " + String(protocol) + ", " + String(pulseLength) + ", " + String(repeat);
    //Serial.printf("CMyRCSwitch::envoieCode() - Envoi %s\n", s.c_str());
    ELECHOUSE_cc1101.SpiStrobe(0x3D); // Flush RX Fifo 
    ELECHOUSE_cc1101.SpiStrobe(0x3B); // Flush TX Fifo
    mySwitch.send(code, 24);
    delay(delaiEntreEnvois); //150); //350
  }

  mySwitch.disableTransmit();
  pinMode(CC1101_GDO0, INPUT);
  ELECHOUSE_cc1101.SetRx();
  mySwitch.enableReceive(CC1101_GDO0);

  return ret;
}
#endif // _RCSWITCH_MODE_