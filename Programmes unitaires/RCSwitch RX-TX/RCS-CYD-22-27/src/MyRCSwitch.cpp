#include "MyRCSwitch.h"


  void CMyRCSwitch::setup() {
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

    mySwitch.enableReceive(CC1101_GDO0);

    Serial.println(F("CMyRCSwitch::setup() terminé"));
  }

  void CMyRCSwitch::send(unsigned long code, int protocol, int pulseLength, int repeatCount) {
    mySwitch.disableReceive();
    ELECHOUSE_cc1101.SetTx();
    delay(10);

    mySwitch.enableTransmit(CC1101_GDO0);
    mySwitch.setProtocol(protocol);
    mySwitch.setPulseLength(pulseLength);
    mySwitch.setRepeatTransmit(repeatCount);

    mySwitch.send(code, BIT_LENGTH);

    mySwitch.disableTransmit();
    pinMode(CC1101_GDO0, INPUT);
    ELECHOUSE_cc1101.SetRx();
    mySwitch.enableReceive(CC1101_GDO0);
  }

  bool CMyRCSwitch::available() { return mySwitch.available(); }
  unsigned long CMyRCSwitch::getReceivedValue() { return mySwitch.getReceivedValue(); }
  int CMyRCSwitch::getReceivedBitlength() { return mySwitch.getReceivedBitlength(); }
  int CMyRCSwitch::getReceivedProtocol() { return mySwitch.getReceivedProtocol(); }
  int CMyRCSwitch::getReceivedDelay() { return mySwitch.getReceivedDelay(); }
  void CMyRCSwitch::resetAvailable() { mySwitch.resetAvailable(); }
  int CMyRCSwitch::getRssi() { return ELECHOUSE_cc1101.getRssi(); }
