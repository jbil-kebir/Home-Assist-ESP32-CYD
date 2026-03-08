#ifndef __MYRCSWITCH_H__
#define __MYRCSWITCH_H__

#include <RCSwitch.h>
#include "RCSwitchConfig.h"
class CConfig;

class CMyRCSwitch {
private:
  RCSwitch mySwitch;
  CConfig* config;

public:
  CMyRCSwitch() = default;
  CMyRCSwitch(CConfig& cfg) : config(&cfg) {};
  unsigned long code = 0L;
  int protocol = 1;
  int pulseLength = 350;
  int repeat = 5;
  float frequency = 433.92;

  void setup();
  int loop();
  int toggleDevice();
  void afficheDetailRCS();
  unsigned char getIDFromRSCCode(unsigned long code);
};

#endif