#ifndef __MYWIFI_H__
#define __MYWIFI_H__
class CWifi {
  private:
    CConfig &mConfig;
public:
  CWifi(CConfig& config) : mConfig(config) {}
  void setup();
};
#endif // __MYWIFI_H__
