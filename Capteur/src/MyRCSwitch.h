#ifndef __MYRCSWITCH_H__
#define __MYRCSWITCH_H__

#ifdef _RCSWITCH_MODE_
#include <RCSwitch.h>
//#include "RCSwitchConfig.h"
class CConfig;

#define DEFAULT_RCS_CODE 0xFEDCBA

class CMyRCSwitch {
private:
  RCSwitch mySwitch;
  CConfig* config;

public:
  CMyRCSwitch() = default;
  CMyRCSwitch(CConfig& cfg) : config(&cfg) {};
  unsigned long code = 0L;
  int protocol = 1;
  int pulseLength = 320;
  int repeat = 3;
  float frequency = 433.92;

  void setup();
  int envoieCode(unsigned long *cde, int nb);
};
#endif // _RCSWITCH_MODE_
#endif