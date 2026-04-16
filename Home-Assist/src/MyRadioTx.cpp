#include "global.h"
#ifdef __LOCAL_MODE__
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "MyConfig.h"
#include "MyEcran.h"

#include "MyRadioTx.h"

// === CLASSES ===
  void CRadioTX::setup() {
  }

  void CRadioTX::loop() {

  }

  void CRadioTX::transmitPulses(const uint16_t* pulses, int nbPulses, const char* action) {
    #ifdef DISABLE_EFFECTEUR
    DBG(DBG_ACTIONNEURS, "void CRadioTX::transmitPulses() - Effecteur désactivé.\n");
    return;
    #endif
    DBG(DBG_ACTIONNEURS, "\n=== EMISSION %s : %d pulses ===\n", action, nbPulses);
    digitalWrite(LED_RED, LED_ON);

    ELECHOUSE_cc1101.setMHZ(FREQUENCE);

    ELECHOUSE_cc1101.SetTx();
    delay(20);
    pinMode(CC1101_GDO0, OUTPUT);
    digitalWrite(CC1101_GDO0, LOW);
    delay(50);

    bool state = HIGH;
    for (int i = 0; i < nbPulses; i++) {
      digitalWrite(CC1101_GDO0, state);
      delayMicroseconds(pulses[i]);
      state = !state;
    }

    digitalWrite(CC1101_GDO0, LOW);
    delay(50);

    pinMode(CC1101_GDO0, INPUT);
    ELECHOUSE_cc1101.SpiStrobe(0x36);
    delay(10);
    ELECHOUSE_cc1101.SetRx();

    digitalWrite(LED_RED, LED_OFF);
    DBG(DBG_ACTIONNEURS, "Emission terminee\n");
  }
#endif // __LOCAL_MODE__