#include "global.h"
#ifdef __LOCAL_MODE__
#include <Arduino.h>
#include <XPT2046_Touchscreen.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "MyCC1101.h"

  void CCC1101::setup() {

    ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.setPA(12);
    ELECHOUSE_cc1101.SetRx();

// Test Grok
ELECHOUSE_cc1101.setMHZ(433.92);
ELECHOUSE_cc1101.setRxBW(812.50);
ELECHOUSE_cc1101.setModulation(2); // 0 : 2-FSK, 2 : ASK/OOK
ELECHOUSE_cc1101.setDeviation(0);
ELECHOUSE_cc1101.setPktFormat(3);
ELECHOUSE_cc1101.setSyncMode(0);
ELECHOUSE_cc1101.setCrc(0);
//ELECHOUSE_cc1101.setModulation(2);     
//ELECHOUSE_cc1101.setDeviation(47.607); // Deviation standard pour bonne portée
//ELECHOUSE_cc1101.setDeviation(0);
ELECHOUSE_cc1101.setDRate(2.4);//4.8);        // Débit bas = meilleure sensibilité
//ELECHOUSE_cc1101.setPA(12);            // Max puissance

    pinMode(CC1101_GDO0, INPUT);
    pinMode(LED_RED, OUTPUT);
    digitalWrite(LED_RED, LED_OFF);


  }

  void CCC1101::loop() {

  }

#endif // __LOCAL_MODE__