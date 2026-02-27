#include "global.h"
#include <Arduino.h>
#ifdef _RCSWITCH_MODE_
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#endif
#include "MyCC1101.h"

  void CCC1101::setup() {
#ifdef _RCSWITCH_MODE_
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
    ELECHOUSE_cc1101.Init();
//    ELECHOUSE_cc1101.setMHZ(FREQUENCE);
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.setPA(10);
    ELECHOUSE_cc1101.SetRx();

    
    pinMode(CC1101_GDO0, INPUT);
#endif

  }

  void CCC1101::loop() {

  }


//#endif // _LOCAL_MODE