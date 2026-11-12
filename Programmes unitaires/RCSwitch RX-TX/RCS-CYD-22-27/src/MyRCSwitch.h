#ifndef __MYRCSWITCH_H__
#define __MYRCSWITCH_H__

#include <SPI.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>

// CC1101 VSPI
#define CC1101_GDO0   22 // 2
#define CC1101_CS     27 // 5
#define CC1101_SCK    18
#define CC1101_MISO   19
#define CC1101_MOSI   23

#define BIT_LENGTH    24

//===================================================================================
//                    CLASSE CMyRCSwitch 
//===================================================================================
class CMyRCSwitch {
private:
  RCSwitch mySwitch;

public:
  void setup();

  void send(unsigned long code, int protocol, int pulseLength, int repeatCount);

  bool available();
  unsigned long getReceivedValue();
  int getReceivedBitlength();
  int getReceivedProtocol();
  int getReceivedDelay();
  void resetAvailable();
  int getRssi();
};
#endif // __MYRCSWITCH_H__
