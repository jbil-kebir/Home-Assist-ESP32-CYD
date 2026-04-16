#include "MyRCDevice.h"
#include "MyConfig.h"
#include "MyEcran.h"


void CRCDevice::setup(const String pref) {
  nomEquipement = "Equipement 433 MHz";
  mqttSubTopic = "Appareil";
  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
}

void CRCDevice::loop() {
    
}

int CRCDevice::activeEquipement() {
  int ret = 0;
  setActive(!active);
  ret = onMqttPublish(mqttSubTopicCommand.c_str(), active ? "ENABLER" : "DISABLER");

  return ret;
}

void CRCDevice::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  code = prefs.getULong((mPrefixNVS+"code").c_str(), 0x3D3B72);
  protocol = prefs.getInt((mPrefixNVS+"proto").c_str(), 1);
  pulseLength = prefs.getInt((mPrefixNVS+"pulse").c_str(), 349);
  repeat = prefs.getInt((mPrefixNVS+"repeat").c_str(), 5);
  frequency = prefs.getFloat((mPrefixNVS+"freq").c_str(), 433.92);
  buttonName = prefs.getString((mPrefixNVS+"name").c_str(), "Projecteur");
//  etat = prefs.getBool((mPrefixNVS+"st").c_str(), false);
  
  prefs.end();
   
  CEquipementBase::loadFromNVS();
  etatStr = etat ? "ON" : "OFF"; // etat est récupéré depuis CEquipementBase

}

void CRCDevice::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  prefs.putULong((mPrefixNVS+"code").c_str(), code);
  prefs.putInt((mPrefixNVS+"proto").c_str(), protocol);
  prefs.putInt((mPrefixNVS+"pulse").c_str(), pulseLength);
  prefs.putInt((mPrefixNVS+"repeat").c_str(), repeat);
  prefs.putFloat((mPrefixNVS+"freq").c_str(), frequency);
  prefs.putString((mPrefixNVS+"name").c_str(), buttonName);
//  prefs.putBool((mPrefixNVS+"st").c_str(), etat);
  prefs.end();

  CEquipementBase::saveToNVS();
}

void CRCDevice::print() const {

  DBG(DBG_ACTIONNEURS, "     Nom            : %s\n", nomEquipement.c_str());
  DBG(DBG_ACTIONNEURS, "     Actif          : %s\n", active ? "OUI" : "NON");
  DBG(DBG_ACTIONNEURS, "     Etat           : %s\n", etatStr);
  DBG(DBG_ACTIONNEURS, "     ButtonName     : %s\n", buttonName.c_str());
  DBG(DBG_ACTIONNEURS, "     Code RCSwitch  : 0x%x\n", code);
  DBG(DBG_ACTIONNEURS, "     Protocole      : %d\n", protocol);
  DBG(DBG_ACTIONNEURS, "     PulseLength    : %d\n", pulseLength);
  DBG(DBG_ACTIONNEURS, "     Repeat         : %d\n", repeat);
  DBG(DBG_ACTIONNEURS, "     Frequency      : %f\n", frequency);
  DBG(DBG_ACTIONNEURS, "     MQTTSubTopic   : %s\n", mqttSubTopic.c_str());
  DBG(DBG_ACTIONNEURS, "     MQTTCmd        : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_ACTIONNEURS, "     MQTTState      : %s\n", mqttSubTopicState.c_str());
}

void CRCDevice::saveState(bool state) {
  etat = state;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"st").c_str(), state);
  prefs.end();

  etatStr = state ? "ON" : "OFF";
}

//----------------------------------------------------------------------------------
// CRCDevice::envoiOnOff()
//
// Envoi trame ONOFF
// Retour :
// 0 : RAS
// -1 : Equipement inactif
//----------------------------------------------------------------------------------
int CRCDevice::envoiOnOff() {
  int ret = 0;
  if (active) {
    toggleDevice();
    etat = !etat;
    saveState(etat);
    ret =0;
  }
  else {
    String s = "L'equipement " + nomEquipement + " n'est pas actif";
    if (mEcran != nullptr)
      mEcran->updateStatus(s);
    DBGLN(DBG_ACTIONNEURS, s);
    return -1;
  }
  if (mEcran != nullptr)
    mEcran->updateAllStates();

  if (onMqttPublish != nullptr)
    ret = onMqttPublish(mqttSubTopicCommand.c_str(), etat ? "ONR" : "OFFR"); // Mise à jour de l'IHM des CYD auxiliaires

  return ret;

}
 
void CRCDevice::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }

void CRCDevice::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
      
  //Serial.println("CRCDevice::handleMqttCommand() - Bouton Chauffage - Cmd : " + cmd);

  if (cmd == "ON" || cmd == "OFF") {
    if (this->active) {
      this->toggleDevice();
      this->etat = !this->etat;
      DBG(DBG_ACTIONNEURS, "CRCDevice::handleMqttCommand() - Envoi %s sur %s\n", this->etat ? "ONR" : "OFFR", this->mqttSubTopicCommand.c_str());
      onMqttPublish(this->mqttSubTopicCommand.c_str(), this->etat ? "ONR" : "OFFR");
      this->saveState(this->etat);
      if (mEcran != nullptr) mEcran->updateAllStates();
    }
    else {
      String s = "L'equipement " + String(this->nomEquipement) + " n'est pas actif";
      if (mEcran != nullptr) mEcran->updateStatus(s);
      DBGLN(DBG_ACTIONNEURS, s);
    }
 }
 else if (cmd == "ENABLE") {
    setActive(true);
    onMqttPublish(this->mqttSubTopicCommand.c_str(), "ENABLER");
    String s = nomEquipement + " : activé";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    DBGLN(DBG_ACTIONNEURS, s);
  }
 else if (cmd == "DISABLE") {
    setActive(false);
    onMqttPublish(this->mqttSubTopicCommand.c_str(), "DISABLER");
    String s = nomEquipement + " : désactivé";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    DBGLN(DBG_ACTIONNEURS, s);
  }
  else {
    DBG(DBG_ACTIONNEURS, "CRCDevice::handleMqttCommand() message non traité ***%s*** : \n", cmd.c_str());
  }

}
// Envoie l'état actif/inactif ainsi que ON/OFF
int CRCDevice::remonteStatusParMqtt() {
  int ret = 0;
  DBG(DBG_ACTIONNEURS, "CRCDevice::remonteStatusParMqtt() - Envoi %s sur %s\n", this->active ? "ENABLER" : "DISABLER", this->mqttSubTopicCommand.c_str());
  onMqttPublish(this->mqttSubTopicCommand.c_str(), this->active ? "ENABLER" : "DISABLER");
  delay(200);
  DBG(DBG_ACTIONNEURS, "CRCDevice::remonteStatusParMqtt() - Envoi %s sur %s\n", this->etat ? "ONR" : "OFFR", this->mqttSubTopicCommand.c_str());
  onMqttPublish(this->mqttSubTopicCommand.c_str(), this->etat ? "ONR" : "OFFR");
  delay(200);
  return ret;
} 

void CRCDevice::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"name").c_str())) buttonName = server.arg((mPrefixNVS+"name").c_str());
  if (server.hasArg((mPrefixNVS+"code").c_str())) code = strtoul(server.arg((mPrefixNVS+"code").c_str()).c_str(), NULL, 16);
  if (server.hasArg((mPrefixNVS+"proto").c_str())) protocol = server.arg((mPrefixNVS+"proto").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"pulse").c_str())) pulseLength = server.arg((mPrefixNVS+"pulse").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"repeat").c_str())) repeat = server.arg((mPrefixNVS+"repeat").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"freq").c_str())) frequency = server.arg((mPrefixNVS+"freq").c_str()).toFloat();

  CEquipementBase::loadFromWebServer (server);

}

String CRCDevice::getHTML() {
  String html = "";
  html =  "<h3>Equipement</h3>"
          "<div class=\"row\"><div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+String("nom")) + " value=\"" + nomEquipement + "\"></div>"
          "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active").c_str() + " value=\"1\"" + String(active ? " checked" : "") + "></div></div>"
          "<div class=\"row\">"
            "<div><label>Nom bouton</label><input type=\"text\" name=" + (mPrefixNVS+"name").c_str() + " value=\"" + buttonName + "\"></div>"
            "<div><label>MQTT subtopic</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic").c_str() + " value=\"" + mqttSubTopic + "\"></div>"
          "</div>"
          "<div class=\"row\">"
            "<div><label>Code (hex)</label><input type=\"text\" name=" + (mPrefixNVS+"code").c_str() + " value=\"" + String(code, HEX) + "\"></div>"
            "<div><label>Protocole</label><input type=\"number\" name=" + (mPrefixNVS+"proto").c_str() + " value=\"" + String(protocol) + "\"></div>"
          "</div>"
          "<div class=\"row\">"
            "<div><label>Largeur impulsion (µs)</label><input type=\"number\" name=" + (mPrefixNVS+"pulse").c_str() + " value=\"" + String(pulseLength) + "\"></div>"
            "<div><label>Répétitions</label><input type=\"number\" name=" + (mPrefixNVS+"repeat").c_str() + " value=\"" + String(repeat) + "\"></div>"
          "</div>"
          "<label>Fréquence (MHz)</label><input type=\"number\" step=\"0.01\" name=" + (mPrefixNVS+"freq").c_str() + " value=\"" + String(frequency, 3) + "\">"
        "</div>";

  return html;
}