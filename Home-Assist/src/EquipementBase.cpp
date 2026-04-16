#include "EquipementBase.h"

void CEquipementBase::saveState(bool state) {
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"st").c_str(), state);
  prefs.end();

  etat = state;
}

void CEquipementBase::setActive(bool state) {
  active = state;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"active").c_str(), active);
  prefs.end();
}

void CEquipementBase::setonEquipementCallback(std::function<int(const String& nom, const String& ip)> onEqu) {
  onEquipement = onEqu;
}      

void CEquipementBase::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Thermomètre");
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "thermometre/");
  active = prefs.getBool((mPrefixNVS+"active").c_str(), true);
  etat = prefs.getBool((mPrefixNVS+"st").c_str(), false);
  
  // On forme les subtopic MQTT
  domotique_prefix = prefs.getString("domo_prefix", default_domotique_topic_prefix);
  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";

  prefs.end();

  
}

void CEquipementBase::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  // Thermomètre
  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putBool((mPrefixNVS+"active").c_str(), active);
  prefs.putBool((mPrefixNVS+"st").c_str(), etat);

  prefs.end();
}

void CEquipementBase::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
}

void CEquipementBase::print() const {

  DBG(DBG_CONFIG, "     Nom                : %s\n", nomEquipement.c_str());
  DBG(DBG_CONFIG, "     Actif              : %s\n", active ? "OUI" : "NON");
  DBG(DBG_CONFIG, "     MQTTSubTopic       : %s\n", mqttSubTopic.c_str());
  DBG(DBG_CONFIG, "     MQTTCmd            : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_CONFIG, "     MQTTState          : %s\n", mqttSubTopicState.c_str());
}
