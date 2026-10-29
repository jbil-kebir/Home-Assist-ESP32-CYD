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
protected:
  CEcran* mEcran = nullptr;
  // Valeurs par défaut quand le NVS est vide (setDefauts()) ; valeurs historiques pour projecteur et guirlande
  unsigned long mulCodeDefaut = 0x3D3B72;
  int miProtocoleDefaut = 1;
  int miImpulsionDefaut = 349;
  String msNomDefaut = "";       // vide : celui de CEquipementBase::loadFromNVS()
  String msLibelleDefaut = "Projecteur";
  String msSubTopicDefaut = "";  // vide : celui de CEquipementBase::loadFromNVS()

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

  void setDefauts(unsigned long codeDef, int protocoleDef, int impulsionDef, const String& nomDef,
                  const String& libelleDef, const String& subTopicDef); // Avant setup()
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
  virtual int envoiOnOff();
  int activeEquipement();
};

#endif