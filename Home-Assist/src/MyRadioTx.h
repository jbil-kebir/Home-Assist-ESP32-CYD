#ifndef __MYRADIOTX_H__
#define __MYRADIOTX_H__

class CRadioTX {
private:

public:

  void setup();
  void loop();
  // freqMHz : fréquence de réglage du CC1101 ; 0 = FREQUENCE (global.h)
  void transmitPulses(const uint16_t* pulses, int nbPulses, const char* action, float freqMHz = 0);
};
#endif // __MYRADIOTX_H__
