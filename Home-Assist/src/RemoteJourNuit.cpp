#include "RemoteJourNuit.h"
#include "ParsedMqttMessage.h"


int CRemoteJourNuit::setup(const String pref) {
  mqttSubTopic = "thermometre/";
  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  mulWatchDog = millis();
  return 0;
}

//--------------------------------------------------------------------------
//  CRemoteJourNuit::loop()
//
// Retour
// -2 : Inactif
// -10 : Rien reçu depuis l'intervalle du watchdog
// -99 : RAS
//--------------------------------------------------------------------------
int CRemoteJourNuit::loop() {
  if (!active) return -2;
  if ((millis() - mulWatchDog) >= mulWatchdogIntervalle*1000) return -10;
  return -99;
}

//
// Pour un appareil distant, cela consiste à envoyer
// une commande MQTT : NomEquipement MESURE
//
bool CRemoteJourNuit::readMesure() {
  if (onMqttPublish == nullptr) return false;
  String sCmd = nomEquipement + " " + sMqttCommandMesure;
  onMqttPublish(mqttSubTopicCommand.c_str(), sCmd.c_str());
  return true;
}

void CRemoteJourNuit::loadFromNVS() {
  String sNomDefaut = nomEquipement; // Nom passé au constructeur (ex : JnCave)
  CEquipementBase::loadFromNVS();    // Sans clé « nom » en NVS, il le remplace par « Thermomètre »
  prefs.begin(nvs_namespace, true);
  if (!prefs.isKey((mPrefixNVS+"nom").c_str())) nomEquipement = sNomDefaut;
  mulWatchdogIntervalle = prefs.getLong((mPrefixNVS+"wdog").c_str(), mulWatchdogDefaultIntervalle);
  prefs.end();
}

void CRemoteJourNuit::saveToNVS() {
  prefs.begin(nvs_namespace, false);
  prefs.putLong((mPrefixNVS+"wdog").c_str(), mulWatchdogIntervalle);
  prefs.end();
  CEquipementBase::saveToNVS();
}

void CRemoteJourNuit::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"wdog").c_str())) mulWatchdogIntervalle = server.arg((mPrefixNVS+"wdog")).toInt();
  CEquipementBase::loadFromWebServer (server);
}

void CRemoteJourNuit::print() const {
  DBG(DBG_CAPTEURS, "==========================================================\n");
  DBG(DBG_CAPTEURS, "     Nom              : %s\n", nomEquipement.c_str());
  DBG(DBG_CAPTEURS, "     Actif            : %s\n", active ? "OUI" : "NON");
  DBG(DBG_CAPTEURS, "     Éclairement      : %d %%\n", miLastEclairement);
  DBG(DBG_CAPTEURS, "     État             : %d (0 nuit, 1 jour, -1 inconnu)\n", miLastEtat);
  DBG(DBG_CAPTEURS, "     Watchdog         : %ld s\n", mulWatchdogIntervalle);
  DBG(DBG_CAPTEURS, "     MQTTSubTopic     : %s\n", mqttSubTopic.c_str());
  DBG(DBG_CAPTEURS, "     MQTTCmd          : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_CAPTEURS, "     MQTTState        : %s\n", mqttSubTopicState.c_str());
}

//
// « <nom> Jour ... 1|0 » ou « <nom> Eclairement ... <pct> » (ALIVE : watchdog seulement)
//
void CRemoteJourNuit::handleMqttState(const String& payload) {
  static bool inactifAffiche = false;
  if (!active) {
    if (!inactifAffiche) {
      DBG(DBG_CAPTEURS, "void CRemoteJourNuit::handleMqttState() - %s inactif\n", nomEquipement.c_str());
      inactifAffiche = true;
    }
    return;
  }
  inactifAffiche = false;

  ParsedMqttMessage msg;
  msg.parse(payload);
  if (msg.miTailleMesure <= 0) return;
  if (!msg.msExpediteur.equalsIgnoreCase(getNomEquipement())) return; // pas pour nous

  if (onEquipement != nullptr) onEquipement(msg.msExpediteur, msg.msIp);
  // On réinitialise le watchdog
  mulWatchDog = millis();
  mbMessageRecu = true;
  if (!msg.msIp.isEmpty()) mIP = msg.msIp;

  String sMesure = msg.mvsMesure[0];
  sMesure.toUpperCase();
  if (sMesure == "ALIVE") return;
  if (msg.miTailleMesure < 2) {
    DBG(DBG_CAPTEURS, "void CRemoteJourNuit::handleMqttState() - %s : valeur absente\n", nomEquipement.c_str());
    return;
  }
  int iVal = msg.mvsMesure.back().toInt();

  if (sMesure == "JOUR") miLastEtat = (iVal != 0) ? 1 : 0;
  else if (sMesure == "ECLAIREMENT") miLastEclairement = constrain(iVal, 0, 100);
  else {
    DBG(DBG_CAPTEURS, "void CRemoteJourNuit::handleMqttState() - %s : mesure inconnue %s\n", nomEquipement.c_str(), sMesure.c_str());
    return;
  }
  if (onMesureChanged != nullptr) onMesureChanged(msg.msExpediteur, miLastEclairement, miLastEtat);
}

// Envoie la dernière mesure.
// Méthode appelée par main lorsqu'un nouveau CYD se signale
bool CRemoteJourNuit::remonteStatusParMqtt() {
  if (onMqttPublish == nullptr || !active) return false;
  if (miLastEtat == JOUR_NUIT_INCONNU) return false; // Aucune mesure reçue depuis le démarrage
  CMyDateTime mDateTime;
  String sVal = nomEquipement + " Jour " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(miLastEtat) + " FORCE";
  bool ret = (onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str()) == 0);
  if (miLastEclairement != JOUR_NUIT_INCONNU) {
    sVal = nomEquipement + " Eclairement " + mDateTime.getDate() + " " + mDateTime.getTime() + " " + String(miLastEclairement) + " FORCE";
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str()) == 0) && ret;
  }
  return ret;
}

String CRemoteJourNuit::getHTML() {
  String html =  "<h2>Configuration jour / nuit " + nomEquipement + "</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active").c_str() + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Intervalle Watchdog (s)</label><input type=\"text\" name=" + (mPrefixNVS + "wdog").c_str() + " value=\"" + mulWatchdogIntervalle + "\"></div>"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS + "subtopic").c_str() + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";
  return html;
}
