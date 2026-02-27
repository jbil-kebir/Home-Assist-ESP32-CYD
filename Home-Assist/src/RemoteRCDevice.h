#ifndef __REMOTERCDEVICE_H__
#define __REMOTERCDEVICE_H__

#include "global.h"
#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include "MyRCSwitch.h"
#include "EquipementBase.h"

class CEcran;  // Forward declaration

class CRemoteRCDevice : public CEquipementBase {
private:
  CEcran* mEcran = nullptr;

public:
  CRemoteRCDevice() = default;

  CRemoteRCDevice(const String& nomEqu, CEcran& ecran,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(&ecran), onMqttPublish(cbonMqttPublish)
  {
    nomEquipement = nomEqu;
    mPrefixNVS = nomEquipement.substring(0, 2) + "_";  // Ex : "Pr_" pour Projecteur
    #ifndef __CYD__
    mEcran = nullptr;
    #endif
  }

  // Publication MQTT
  std::function<int(const char*, const char*)> onMqttPublish; 

  String etatStr = "OFF";
  String buttonName = "Appareil";

  int toggleDevice();
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  void setup(const String pref);
  void loop();
  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void saveState(bool state);
  void handleMqttCommand(const String& payload);
  void print() const;
  String getHTML();
  int envoiOnOff();
  int activeEquipement();
};

#endif // __REMOTERCDEVICE_H__