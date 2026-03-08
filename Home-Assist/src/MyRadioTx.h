#ifndef __MYRADIOTX_H__
#define __MYRADIOTX_H__

class CRadioTX {
private:

public:

  void setup();
  void loop();
  void transmitPulses(const uint16_t* pulses, int nbPulses, const char* action);
};
#endif // __MYRADIOTX_H__
