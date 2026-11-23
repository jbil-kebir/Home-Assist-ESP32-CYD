#include "MyRCDevice.h"
#include "MyConfig.h"
#include "MyEcran.h"


void CRCDevice::setup(const String pref) {
  nomEquipement = "Equipement 433 MHz";
  mqttSubTopic = "Appareil";
  mPrefixNVS = pref;
  // On charge les infos de config depuis le NVS
  loadFromNVS();
  // Nœud de présence : clés NVS <préfixe>p... (ex. guir_pnoeud)
  mPresence.setup(pref + "p");
  branchePresence();
  // Mode maintenance : topic du nœud de présence (home/conf<nœud>/state → home/conf<nœud>/maintenance)
  if (mPresence.estConfiguree() && mPresence.mqttSubTopicState.endsWith("state"))
    setTopicMaintenance(mPresence.mqttSubTopicState.substring(0, mPresence.mqttSubTopicState.length() - 5) + "maintenance");
}

void CRCDevice::loop() {
  mPresence.loop();
  loopMaintenance();
  // Sortie de maintenance (4.5) : état réel vérifié, état d'avant remis (code seulement s'il diffère)
  if (mbSortieMaintenanceEnAttente && !mPresence.verificationEnCours()) {
    mbSortieMaintenanceEnAttente = false;
    commandeDirecte(mbEtatAvantMaintenance);
  }
}

// Réactions de l'appareil aux conclusions du nœud de présence (messages de la barre d'état : courts, sans accents)
void CRCDevice::branchePresence() {
  mPresence.setMqttPublishCallback(onMqttPublish);
  mPresence.onEtatChange = [this](int etatMesure) {
    bool bAllume = (etatMesure == PRESENCE_PRESENT);
    DBGLN(DBG_ACTIONNEURS, "CRCDevice - " + nomEquipement + (bAllume ? " allume (mesure)" : " eteint (mesure)"));
    if (bAllume == etat) return;
    saveState(bAllume);
    if (onMqttPublish != nullptr) onMqttPublish(mqttSubTopicCommand.c_str(), etat ? "ONR" : "OFFR");
    if (mEcran != nullptr) mEcran->updateAllStates();
  };
  mPresence.onRenvoi = [this](int essai, int nbEssais) {
    toggleDevice();
    String s = buttonName + " : renvoi " + String(essai) + "/" + String(nbEssais);
    DBGLN(DBG_ACTIONNEURS, "CRCDevice - " + s);
    if (mEcran != nullptr) mEcran->updateStatus(s);
  };
  mPresence.onEchec = [this](int voulu, int essais) {
    String sVoulu = (voulu == PRESENCE_PRESENT) ? "ON" : "OFF";
    String s = buttonName + " : ECHEC " + sVoulu + " (" + String(essais) + " essais)";
    DBGLN(DBG_ACTIONNEURS, "CRCDevice - " + s);
    if (mEcran != nullptr) mEcran->updateStatus(s, false, true);
    // Alerte pour Home Assistant : « <nom> ECHEC ON|OFF <essais> » sur le topic d'état de l'appareil
    if (onMqttPublish != nullptr)
      onMqttPublish(mqttSubTopicState.c_str(), (nomEquipement + " ECHEC " + sVoulu + " " + String(essais)).c_str());
  };
  // Début (CRCDevice::commande) et fin de la vérification d'une commande : « <nom> VERIFICATION EN_COURS|FIN » sur le
  // topic d'état de l'appareil ; HA remplace le bouton par un sablier entre les deux
  mPresence.onFinCommande = [this]() {
    if (onMqttPublish != nullptr)
      onMqttPublish(mqttSubTopicState.c_str(), (nomEquipement + " VERIFICATION FIN").c_str());
    finForcerDisable();
  };
  mPresence.onNonVerifieChange = [this](bool nonVerifie) {
    String s = buttonName + (nonVerifie ? " : etat non verifie" : " : etat verifie");
    if (mEcran != nullptr) mEcran->updateStatus(s);
    if (onMqttPublish != nullptr)
      onMqttPublish(mqttSubTopicState.c_str(), (nomEquipement + (nonVerifie ? " NONVERIFIE" : " VERIFIE")).c_str());
  };
}

// OFF_FORCE (4.8) : désactivation une fois la commande terminée, seulement si l'appareil est bien éteint (échec de
// l'extinction : il reste actif, le thermostat de HA peut réessayer)
void CRCDevice::finForcerDisable() {
  if (!mbForcerDisable) return;
  mbForcerDisable = false;
  if (etat || !active) return;
  setActive(false);
  if (onMqttPublish != nullptr) onMqttPublish(mqttSubTopicCommand.c_str(), "DISABLER");
  String s = nomEquipement + " : desactive (force OFF)";
  if (mEcran != nullptr) mEcran->updateStatus(s, true);
  DBGLN(DBG_ACTIONNEURS, s);
}

int CRCDevice::activeEquipement() {
  int ret = 0;
  setActive(!active);
  ret = onMqttPublish(mqttSubTopicCommand.c_str(), active ? "ENABLER" : "DISABLER");

  return ret;
}

// Valeurs par défaut d'un appareil neuf (NVS vide), à appeler avant setup().
// Sans appel : valeurs historiques (code du projecteur, sous-topic « thermometre/ » de CEquipementBase).
void CRCDevice::setDefauts(unsigned long codeDef, int protocoleDef, int impulsionDef, const String& nomDef,
                           const String& libelleDef, const String& subTopicDef) {
  mulCodeDefaut = codeDef;
  miProtocoleDefaut = protocoleDef;
  miImpulsionDefaut = impulsionDef;
  msNomDefaut = nomDef;
  msLibelleDefaut = libelleDef;
  msSubTopicDefaut = subTopicDef;
}

void CRCDevice::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  code = prefs.getULong((mPrefixNVS+"code").c_str(), mulCodeDefaut);
  protocol = prefs.getInt((mPrefixNVS+"proto").c_str(), miProtocoleDefaut);
  pulseLength = prefs.getInt((mPrefixNVS+"pulse").c_str(), miImpulsionDefaut);
  repeat = prefs.getInt((mPrefixNVS+"repeat").c_str(), 5);
  frequency = prefs.getFloat((mPrefixNVS+"freq").c_str(), 433.92);
  buttonName = prefs.getString((mPrefixNVS+"name").c_str(), msLibelleDefaut);
//  etat = prefs.getBool((mPrefixNVS+"st").c_str(), false);
  bool bNomEnregistre = prefs.isKey((mPrefixNVS+"nom").c_str());
  bool bSubTopicEnregistre = prefs.isKey((mPrefixNVS+"subtopic").c_str());

  prefs.end();

  CEquipementBase::loadFromNVS();
  // Appareil neuf : nom et sous-topic par défaut de l'appareil (sinon « Thermomètre » / « thermometre/ »)
  if (!bNomEnregistre && msNomDefaut.length() > 0) nomEquipement = msNomDefaut;
  if (!bSubTopicEnregistre && msSubTopicDefaut.length() > 0) {
    mqttSubTopic = msSubTopicDefaut;
    mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
    mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";
  }
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
  mPresence.saveToNVS();
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
  mPresence.print();
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
  return commande(!etat);
}

//----------------------------------------------------------------------------------
// CRCDevice::commande()
//
// État voulu (bouton de l'écran : inverse de l'état affiché ; MQTT : ON / OFF).
// Sans nœud de présence, ou nœud en deep sleep (non vérifié) : fonctionnement historique, le code bascule
// et l'état affiché suit la commande.
// Avec nœud de présence : le code (qui bascule) n'est envoyé que si l'état mesuré diffère de l'état voulu ;
// le nœud vérifie ensuite (CRemotePresenceReseauElectrique) et l'état affiché suit la mesure.
// Retour :
// 0 : RAS
// -1 : Equipement inactif
// -2 : Vérification en cours, commande ignorée
// -4 : Mode maintenance, commande refusée
//----------------------------------------------------------------------------------
int CRCDevice::commande(bool voulu) {
  if (refuseEnMaintenance()) return -4;
  return commandeDirecte(voulu);
}

int CRCDevice::commandeDirecte(bool voulu) {
  int ret = 0;
  if (!active) {
    String s = "L'equipement " + nomEquipement + " n'est pas actif";
    if (mEcran != nullptr) mEcran->updateStatus(s);
    DBGLN(DBG_ACTIONNEURS, s);
    return -1;
  }
  if (!mPresence.estConfiguree() || mPresence.estNonVerifie()) {
    toggleDevice();
    etat = !etat;
    saveState(etat);
    if (mEcran != nullptr) mEcran->updateAllStates();
    if (onMqttPublish != nullptr)
      ret = onMqttPublish(mqttSubTopicCommand.c_str(), etat ? "ONR" : "OFFR"); // Mise à jour de l'IHM des CYD auxiliaires
    return ret;
  }
  if (mPresence.verificationEnCours()) {
    String s = buttonName + " : verification en cours";
    if (mEcran != nullptr) mEcran->updateStatus(s);
    DBGLN(DBG_ACTIONNEURS, "CRCDevice - " + s + ", commande ignoree");
    return -2;
  }
  int iMesure = mPresence.getEtat();
  if (iMesure != PRESENCE_INCONNUE && (iMesure == PRESENCE_PRESENT) == voulu) {
    // Déjà dans l'état voulu : surtout ne pas renvoyer le code (il basculerait l'appareil)
    String s = buttonName + (voulu ? " : deja allume" : " : deja eteint");
    if (mEcran != nullptr) mEcran->updateStatus(s);
    DBGLN(DBG_ACTIONNEURS, "CRCDevice - " + s);
    if (onMqttPublish != nullptr) ret = onMqttPublish(mqttSubTopicCommand.c_str(), etat ? "ONR" : "OFFR");
    return ret;
  }
  toggleDevice();
  mPresence.demandeEtat(voulu ? PRESENCE_PRESENT : PRESENCE_ABSENT);
  if (onMqttPublish != nullptr)
    onMqttPublish(mqttSubTopicState.c_str(), (nomEquipement + " VERIFICATION EN_COURS").c_str());
  String s = buttonName + (voulu ? " : ON envoye, verification" : " : OFF envoye, verification");
  if (mEcran != nullptr) mEcran->updateStatus(s);
  DBGLN(DBG_ACTIONNEURS, "CRCDevice - " + s);
  return ret;
}
 
void CRCDevice::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
        mPresence.setMqttPublishCallback(cb);
    }

void CRCDevice::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
      
  //Serial.println("CRCDevice::handleMqttCommand() - Bouton Chauffage - Cmd : " + cmd);

  if (cmd == "ON" || cmd == "OFF" || cmd == "ON_FORCE" || cmd == "OFF_FORCE") {
    // Sans nœud de présence, ON comme OFF basculent l'appareil (fonctionnement historique, voir commande())
    bool bVoulu = (cmd == "ON" || cmd == "ON_FORCE");
    // 4.8 : ON_FORCE / OFF_FORCE (thermostat de HA, comme la chaudière) : commande exécutée même appareil désactivé,
    // qui est réactivé ; après OFF_FORCE, il est désactivé de nouveau une fois éteint (finForcerDisable())
    if (cmd.endsWith("_FORCE")) {
      if (refuseEnMaintenance()) return;
      setActive(true);
      if (onMqttPublish != nullptr) onMqttPublish(mqttSubTopicCommand.c_str(), "ENABLER");
      String s = nomEquipement + (bVoulu ? " : active (force ON)" : " : active (force OFF)");
      if (mEcran != nullptr) mEcran->updateStatus(s, true);
      DBGLN(DBG_ACTIONNEURS, s);
      mbForcerDisable = !bVoulu;
    }
    commande(bVoulu);
    // Commande déjà terminée (sans nœud de présence, ou appareil déjà dans l'état voulu) ; sinon : fin de la vérification
    if (!mPresence.verificationEnCours()) finForcerDisable();
 }
 else if (cmd.startsWith("MAINTENANCER")) {
    // Notre propre annonce aux CYD auxiliaires (publieMaintenanceAux()), reçue sur le topic de commande : rien à faire
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
  publieMaintenanceAux();
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
  mPresence.loadFromWebServer(server);
  // Réglages à chaud (4.6) : nœud de présence changé → topic de maintenance recalculé (abonné à la reconnexion MQTT)
  if (mPresence.estConfiguree() && mPresence.mqttSubTopicState.endsWith("state"))
    setTopicMaintenance(mPresence.mqttSubTopicState.substring(0, mPresence.mqttSubTopicState.length() - 5) + "maintenance");

}

String CRCDevice::getHTML() {
  String html = "";
  html =  "<h3>Equipement</h3>"
          "<div class=\"row\"><div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+String("nom")) + " value=\"" + nomEquipement + "\"></div>"
          "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active").c_str() + " value=\"1\"" + String(active ? " checked" : "") + "></div></div>"
          "<div class=\"row\">"
            "<div><label>Nom bouton</label><input type=\"text\" name=" + (mPrefixNVS+"name").c_str() + " value=\"" + buttonName + "\"></div>"
            "<div><label>MQTT subtopic" MARQUE_REDEMARRAGE "</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic").c_str() + " value=\"" + mqttSubTopic + "\"></div>"
          "</div>"
          "<div class=\"row\">"
            "<div><label>Code (hex)</label><input type=\"text\" name=" + (mPrefixNVS+"code").c_str() + " value=\"" + String(code, HEX) + "\"></div>"
            "<div><label>Protocole</label><input type=\"number\" name=" + (mPrefixNVS+"proto").c_str() + " value=\"" + String(protocol) + "\"></div>"
          "</div>"
          "<div class=\"row\">"
            "<div><label>Largeur impulsion (µs)</label><input type=\"number\" name=" + (mPrefixNVS+"pulse").c_str() + " value=\"" + String(pulseLength) + "\"></div>"
            "<div><label>Répétitions</label><input type=\"number\" name=" + (mPrefixNVS+"repeat").c_str() + " value=\"" + String(repeat) + "\"></div>"
          "</div>"
          "<label>Fréquence (MHz)</label><input type=\"number\" step=\"0.01\" name=" + (mPrefixNVS+"freq").c_str() + " value=\"" + String(frequency, 3) + "\">";
  html += mPresence.getHTML();
  html += "</div>";

  return html;
}

//==================================================================================
//      Mode maintenance (Home-Assist 4.4)
//==================================================================================
void CRCDevice::setTopicMaintenance(const String& topic) {
  msTopicMaintenance = topic;
  msTopicMaintenanceEtat = topic.length() > 0 ? topic + "/etat" : "";
}

// Commande refusée pendant la maintenance (écran, CYD auxiliaires, Home Assistant)
// 4.7 : état réel republié (ONR / OFFR : l'interrupteur de HA, qui avait basculé, revient) et refus annoncé sur le
// topic d'état (« <nom> REFUSEE en maintenance » : notification de HA)
bool CRCDevice::refuseEnMaintenance() {
  if (!maintenance) return false;
  String s = buttonName + " : en maintenance";
  if (mEcran != nullptr) mEcran->updateStatus(s);
  DBGLN(DBG_ACTIONNEURS, "CRCDevice - " + s + ", commande refusee");
  if (onMqttPublish != nullptr) {
    onMqttPublish(mqttSubTopicCommand.c_str(), etat ? "ONR" : "OFFR");
    onMqttPublish(mqttSubTopicState.c_str(), (nomEquipement + " REFUSEE en maintenance").c_str());
  }
  return true;
}

// 4.7 : rien avant la lecture de l'ordre retenu (au démarrage, un « MAINTENANCER OFF » précédait le « ON » de la reprise)
void CRCDevice::publieMaintenanceAux() {
  if (!mbOrdreMaintenanceLu) return;
  if (msTopicMaintenance.length() == 0 || onMqttPublish == nullptr) return;
  onMqttPublish(mqttSubTopicCommand.c_str(), maintenance ? "MAINTENANCER ON" : "MAINTENANCER OFF");
}

void CRCDevice::publieEtatMaintenance(const String& sEtat) {
  msEtatMaintenancePublie = sEtat;
  DBGLN(DBG_ACTIONNEURS, "CRCDevice - " + nomEquipement + " maintenance : " + sEtat);
  if (onMqttPublishRetenu != nullptr && msTopicMaintenanceEtat.length() > 0)
    onMqttPublishRetenu(msTopicMaintenanceEtat.c_str(), sEtat.c_str(), true);
}

//----------------------------------------------------------------------------------
// Ordre retenu de Home Assistant : « ON » ou « OFF » (4.7 : plus d'heure de fin, la maintenance dure jusqu'à l'OFF
// demandé par l'utilisateur ; une heure après « ON », envoyée par HA avant la 4.7, est ignorée).
// Relu à chaque connexion MQTT (redémarrage du CYD : reprise, marque « mnton » en NVS).
//----------------------------------------------------------------------------------
void CRCDevice::handleMaintenance(const String& payload) {
  String s = payload; s.trim();
  String sMaj = s; sMaj.toUpperCase();
  bool bPremiereLecture = !mbOrdreMaintenanceLu;
  mbOrdreMaintenanceLu = true;
  if (sMaj.startsWith("OFF") || sMaj.length() == 0) {
    if (maintenance) quitteMaintenance("OFF recu");
    else {
      if (msEtatMaintenancePublie.length() > 0 && !msEtatMaintenancePublie.startsWith("INACTIVE")) publieEtatMaintenance("INACTIVE");
      if (bPremiereLecture) publieMaintenanceAux();
    }
    return;
  }
  if (!sMaj.startsWith("ON")) return;
  if (maintenance) return;   // Ordre relu (reconnexion MQTT)
  String sRefus = raisonRefusMaintenance();
  if (sRefus.length() > 0) {
    publieEtatMaintenance("REFUSEE " + sRefus);
    return;
  }
  // Reprise (CYD redémarré pendant la maintenance) : marque « mnton » restée en NVS, état d'avant gardé
  prefs.begin(nvs_namespace, false);
  bool bReprise = prefs.getBool((mPrefixNVS + "mnton").c_str(), false);
  if (!bReprise) {
    prefs.putBool((mPrefixNVS + "mnton").c_str(), true);
    prefs.putBool((mPrefixNVS + "mntav").c_str(), etat);
  }
  mbEtatAvantMaintenance = prefs.getBool((mPrefixNVS + "mntav").c_str(), etat);
  prefs.remove((mPrefixNVS + "mntfin").c_str());   // Heure de fin des versions 4.4 à 4.6
  prefs.end();
  maintenance = true;
  msEtatMaintenancePublie = "";
  entreMaintenance(bReprise);
  publieMaintenanceAux();
  if (mEcran != nullptr) { mEcran->updateAllStates(); mEcran->updateStatus(buttonName + " : maintenance"); }
}

void CRCDevice::quitteMaintenance(const String& sRaison) {
  DBGLN(DBG_ACTIONNEURS, "CRCDevice - " + nomEquipement + " : fin de maintenance (" + sRaison + ")");
  maintenance = false;
  mbEntreeMaintenanceEnAttente = false;
  prefs.begin(nvs_namespace, false);
  prefs.remove((mPrefixNVS + "mnton").c_str());
  prefs.remove((mPrefixNVS + "mntfin").c_str());
  prefs.end();
  sortMaintenance();
  publieEtatMaintenance("INACTIVE");
  publieMaintenanceAux();
  if (mEcran != nullptr) { mEcran->updateAllStates(); mEcran->updateStatus(buttonName + " : fin maintenance"); }
}

// Appelée par loop() : entrée différée, état publié (« ACTIVE, <détail> » quand il change ; 4.7 : plus d'heure de fin)
void CRCDevice::loopMaintenance() {
  if (!maintenance) return;
  if (mbEntreeMaintenanceEnAttente && !mPresence.verificationEnCours() && mPresence.getEtat() != PRESENCE_INCONNUE) {
    mbEntreeMaintenanceEnAttente = false;
    if (mPresence.getEtat() != PRESENCE_PRESENT) commandeDirecte(true);
  }
  String sEtat = "ACTIVE, " + detailMaintenance();
  if (sEtat != msEtatMaintenancePublie) publieEtatMaintenance(sEtat);
}

// Appareil à nœud de présence (guirlande) : pas de maintenance sans nœud associé
String CRCDevice::raisonRefusMaintenance() {
  if (!active) return "appareil inactif";
  if (!mPresence.estConfiguree()) return "pas de noeud de presence associe";
  return "";
}

// Guirlande : prise fermée (guirlande allumée) ; état réel inconnu (démarrage du CYD) : attendu avant d'envoyer le code
void CRCDevice::entreMaintenance(bool /*bReprise*/) {
  if (mPresence.verificationEnCours() || mPresence.getEtat() == PRESENCE_INCONNUE) {
    mbEntreeMaintenanceEnAttente = true;
    return;
  }
  if (mPresence.getEtat() != PRESENCE_PRESENT) commandeDirecte(true);
}

// Guirlande : remise dans l'état d'avant la maintenance. 4.5 : l'état supposé n'est pas fiable (nœud téléversé,
// redémarré, testament « offline » d'une coupure Wi-Fi) : vérification par PING d'abord, puis code seulement s'il le
// faut (voir loop()) ; sans quoi « déjà éteint » laissait la prise fermée (essai du 22/11/2026)
void CRCDevice::sortMaintenance() {
  if (!mPresence.estConfiguree()) {
    commandeDirecte(mbEtatAvantMaintenance);
    return;
  }
  mbSortieMaintenanceEnAttente = true;
  if (!mPresence.verificationEnCours()) {
    mPresence.demandeEtat(PRESENCE_VERIF_SANS_COMMANDE);
    if (onMqttPublish != nullptr)
      onMqttPublish(mqttSubTopicState.c_str(), (nomEquipement + " VERIFICATION EN_COURS").c_str());
  }
}

String CRCDevice::detailMaintenance() {
  if (mPresence.getEtat() == PRESENCE_PRESENT)
    return mPresence.mIP.length() > 0 ? "http://" + mPresence.mIP + "/update" : String("nœud présent");
  if (mbEntreeMaintenanceEnAttente || mPresence.verificationEnCours()) return "nœud en démarrage";
  return "nœud absent";
}
