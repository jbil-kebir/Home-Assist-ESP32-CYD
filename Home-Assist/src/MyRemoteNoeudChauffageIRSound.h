#ifndef MYREMOTENOEUDCHAUFFAGEIRSOUND_H
#define MYREMOTENOEUDCHAUFFAGEIRSOUND_H

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "RemoteRCDevice.h"
#include "global.h"


class CEcran;
class CRadioTX;

class CRemoteNoeudChauffageIRSound : public CRemoteRCDevice {
public:
  using CRemoteRCDevice::CRemoteRCDevice;   // hérite des constructeurs de CRemoteRCDevice

  void setEtatReelOnOff(bool state);
  /*CRemoteNoeudChauffageIRSound() = default;
  CRemoteNoeudChauffageIRSound(const String& nomEqu, CEcran& ecran,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(&ecran), onMqttPublish(cbonMqttPublish) {
    nomEquipement = nomEqu;
    mPrefixNVS = nomEquipement.substring(0, 2);
    #ifndef __CYD__
    mEcran = nullptr;
    #endif
   
  }

  // Publication MQTT
  std::function<int(const char*, const char*)> onMqttPublish; 
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  //const bool default_boiler_state = false;
  //const char* default_topic_prefix = "chaudiere/";

  //String nomBoutonON = "CHAUD. ON";
  //String nomBoutonOFF = "CHAUD. OFF";

  //float frequency = 434.038;
  //String etatStr = "OFF";


  void setup();
  void loop();
  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void setEtatReelOnOff(bool state);
  void saveState(bool state);
  void setActive(bool state);
  int activeEquipement();
  String getHTML();

  ///void handleMqttCommand(const String& payload);
  void print() const;*/


};

#endif // MYREMOTENOEUDCHAUFFAGEIRSOUND_H