#ifndef MYNOEUDCHAUFFAGEIRSOUND_H
#define MYNOEUDCHAUFFAGEIRSOUND_H

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "MyRCDevice.h"
#include "global.h"


class CEcran;
class CRadioTX;

class CNoeudChauffageIRSound : public CRCDevice {
public:
  using CRCDevice::CRCDevice;   // hérite des constructeurs de CRCDevice

  int envoiOnOff() override;
  void setEtatReelOnOff(bool state);
  /*CNoeudChauffageIRSound() = default;
  CNoeudChauffageIRSound(const String& nomEqu, CEcran& ecran,
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

#endif // MYNOEUDCHAUFFAGEIRSOUND_H