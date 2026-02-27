#ifndef __EQUIPEMENTBASE_H__
#define __EQUIPEMENTBASE_H__
#include <functional>
#include <Preferences.h>
#include <WebServer.h>
#include "global.h"


class CEquipementBase {
private:


public:
  Preferences prefs;
  const char* default_domotique_topic_prefix = "home/";
  String domotique_prefix;
  String mqttSubTopic = "equ";
  String mqttSubTopicCommand;
  String mqttSubTopicStatus;
  String mqttSubTopicState;

  const char* nvs_namespace = NVS_NAME_SPACE;  // 
  String mPrefixNVS = "th_";  // Préfixe par défaut
  String nomEquipement = "Equ";
  String mIP;
  bool active = true;
  bool etat = false;

  String &getNomEquipement() {return nomEquipement;}
  void saveState(bool state);
  void setActive(bool state);
  void loadFromNVS();
  void saveToNVS();
  void setPrefixNVS(const char* pr) {mPrefixNVS = pr;}
  void print() const;
  void loadFromWebServer (WebServer& server);

};

#endif // __EQUIPEMENTBASE_H__