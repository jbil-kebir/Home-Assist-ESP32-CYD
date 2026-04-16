#include "RemoteRCDevice.h"
#include "MyConfig.h"
#include "MyEcran.h"


void CRemoteRCDevice::setup(const String pref) {
  nomEquipement = "Equipement 433 MHz";
  mqttSubTopic = "Appareil/";
  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
}

void CRemoteRCDevice::loop() {
    
}

int CRemoteRCDevice::activeEquipement() {
  int ret = 0;
  setActive(!active);
  ret = onMqttPublish(mqttSubTopicCommand.c_str(), active ? "ENABLE" : "DISABLE");

  return ret;
}


void CRemoteRCDevice::loadFromNVS() {
  prefs.begin(nvs_namespace, true);
  buttonName = prefs.getString((mPrefixNVS+"name").c_str(), "Equipement");
  etat = prefs.getBool((mPrefixNVS+"st").c_str(), false);
  etatStr = etat ? "ON" : "OFF";

  prefs.end();
 
  CEquipementBase::loadFromNVS();
  
}

void CRemoteRCDevice::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putString((mPrefixNVS+"name").c_str(), buttonName);
  prefs.putBool((mPrefixNVS+"st").c_str(), etat);
  prefs.end();

  CEquipementBase::saveToNVS();
}

void CRemoteRCDevice::print() const {

  DBG(DBG_ACTIONNEURS, "     Nom            : %s\n", nomEquipement.c_str());
  DBG(DBG_ACTIONNEURS, "     Actif          : %s\n", active ? "OUI" : "NON");
  DBG(DBG_ACTIONNEURS, "     Etat           : %s\n", active ? "ON" : "OFF");
  DBG(DBG_ACTIONNEURS, "     ButtonName     : %s\n", buttonName.c_str());
  DBG(DBG_ACTIONNEURS, "     MQTTSubTopic   : %s\n", mqttSubTopic.c_str());
  DBG(DBG_ACTIONNEURS, "     MQTTCmd        : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_ACTIONNEURS, "     MQTTState      : %s\n", mqttSubTopicState.c_str());
}

void CRemoteRCDevice::saveState(bool state) {
  etat = state;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"st").c_str(), state);
  prefs.end();

  etatStr = state ? "ONR" : "OFFR";
}

void CRemoteRCDevice::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }

int CRemoteRCDevice::toggleDevice() {
  int ret = 0;
  DBG(DBG_ACTIONNEURS, "CRemoteRCDevice::toggleDevice()\n");
  String payload = (etat ? "ONR" : "OFFR");
  return ret;
}
//----------------------------------------------------------------------------------
// CRemoteRCDevice::envoiOnOff()
//
// Envoi trame ONOFF
// Retour :
// 0 : RAS
//----------------------------------------------------------------------------------
int CRemoteRCDevice::envoiOnOff() {
  int ret = 0;

  ret = onMqttPublish(mqttSubTopicCommand.c_str(), etat ? "ON" : "OFF");

  /*if (active) {
    toggleDevice();
    etat = !etat;
    saveState(etat);
    ret =0;
  }
  else {
    ret = -1;
  }*/

  return ret;

}

void CRemoteRCDevice::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();

  DBG(DBG_ACTIONNEURS, "CRemoteRCDevice::handleMqttCommand() - Equipement %s - Cmd : %s \n", nomEquipement.c_str(), cmd.c_str());

  if (cmd == "ONR") {
    if (this->active) {
      //this->toggleDevice();
      this->etat = true; //!this->etat;
      this->saveState(this->etat);
      if (mEcran != nullptr) mEcran->updateAllStates();
    }
    else {
      String s = "L'equipement " + String(this->nomEquipement) + " n'est pas actif";
      if (mEcran != nullptr) mEcran->updateStatus(s);
      DBGLN(DBG_ACTIONNEURS, s);
    }
 }
 else if (cmd == "OFFR") {
    if (this->active) {
      //this->toggleDevice();
      this->etat = false; //!this->etat;
      this->saveState(this->etat);
      if (mEcran != nullptr) mEcran->updateAllStates();
    }
    else {
      String s = "L'equipement " + nomEquipement + " n'est pas actif";
      if (mEcran != nullptr) mEcran->updateStatus(s);
      DBGLN(DBG_ACTIONNEURS, s);
    }
 }
 else if (cmd == "ENABLER") {
    setActive(true);
    String s = nomEquipement + " : active";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    //Serial.printf("CRemoteRCDevice::handleMqttCommand() - %s", s.c_str());
  }
 else if (cmd == "DISABLER") {
    setActive(false);
    String s = nomEquipement + " : desactive";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    //Serial.printf("CRemoteRCDevice::handleMqttCommand() - %s", s.c_str());
  }
  else {
    DBG(DBG_ACTIONNEURS, "CRemoteRCDevice::handleMqttCommand() message non traité ***%s*** : \n", cmd.c_str());
  }
}

void CRemoteRCDevice::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"name").c_str())) buttonName = server.arg((mPrefixNVS+"name").c_str());

  CEquipementBase::loadFromWebServer (server);
}

String CRemoteRCDevice::getHTML() {
  String html = "";
  html =  "<h3>Equipement</h3>"
          "<div class=\"row\"><div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+String("nom")) + " value=\"" + nomEquipement + "\"></div>"
          "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active").c_str() + " value=\"1\"" + String(active ? " checked" : "") + "></div></div>"
          "<div class=\"row\">"
            "<div><label>Nom bouton</label><input type=\"text\" name=" + (mPrefixNVS+"name").c_str() + " value=\"" + buttonName + "\"></div>"
            "<div><label>MQTT subtopic</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic").c_str() + " value=\"" + mqttSubTopic + "\"></div>"
          "</div>";

  return html;
}