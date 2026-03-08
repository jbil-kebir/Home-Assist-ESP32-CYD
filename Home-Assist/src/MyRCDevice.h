#ifndef MYRCDEVICE_H
#define MYRCDEVICE_H

#include "global.h"
#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include "MyRCSwitch.h"
#include "EquipementBase.h"

class CEcran;  // Forward declaration

class CRCDevice : public CMyRCSwitch, public CEquipementBase  {
private:
  CEcran* mEcran = nullptr;

public:
  CRCDevice() = default;

  CRCDevice(const String& nomEqu, CEcran& ecran,
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

  //bool etat = false;
  String etatStr = "OFF";
  String buttonName = "Appareil";

  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  void setup(const String pref);
  void loop();
  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void saveState(bool state);
  void handleMqttCommand(const String& payload);
  int remonteStatusParMqtt(); // Envoie l'état actif/inactif ainsi que ON/OFF
  void print() const;
  String getHTML();
  int envoiOnOff();
  int activeEquipement();
};

#endif