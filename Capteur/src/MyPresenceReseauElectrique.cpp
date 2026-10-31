#include "MyPresenceReseauElectrique.h"

int CPresenceReseauElectrique::setup(const String pref) {
  mPrefixNVS = pref;
  mbMesureRemontee = false;
  mbAnnonceEnvoyee = false;
  mulDerniereTentative = 0;
  loadFromNVS();
  return 0;
}

//
// Annonce de démarrage, retentée toutes les PRESENCE_RESEAU_RETENTE_MS tant qu'elle n'a pas été publiée.
// Retour : -9 inactif, 1 annonce publiée à ce tour, 0 rien à faire
//
int CPresenceReseauElectrique::loop() {
  if (!active) return -9;
  if (mbAnnonceEnvoyee) return 0;
  if (mulDerniereTentative != 0 && millis() - mulDerniereTentative < PRESENCE_RESEAU_RETENTE_MS) return 0;
  mulDerniereTentative = millis();
  if (mulDerniereTentative == 0) mulDerniereTentative = 1;
  return (publiePresent() == 0) ? 1 : 0;
}

//
// « <nom> PRESENT <date> <heure> » sur le topic de configuration du nœud. Jamais muette (MUTE) : ce n'est pas une
// mesure mais la preuve que l'appareil est alimenté.
// Retour : 0 publiée, -1 échec (MQTT pas connecté...), -9 inactif
//
int CPresenceReseauElectrique::publiePresent() {
  if (!active) return -9;
  String sDate = (mDateTime != nullptr) ? mDateTime->getDate() : "DATE";
  String sTime = (mDateTime != nullptr) ? mDateTime->getTime() : "TIME";
  String sMsg = nomEquipement + " PRESENT " + sDate + " " + sTime;
  bool bOk = false;
  #ifdef _LORA_P2P_MODE_
  if (onLoraP2PPublish != nullptr) {
    Serial.println("CPresenceReseauElectrique::publiePresent() LoRa : " + sMsg);
    bOk = (onLoraP2PPublish((mqttSubTopicState + " " + sMsg).c_str()) == 0);
  }
  #endif
  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  if (onMqttPublish != nullptr) {
    Serial.println("CPresenceReseauElectrique::publiePresent() MQTT : " + sMsg);
    bOk = (onMqttPublish(mqttSubTopicState.c_str(), sMsg.c_str()) == 0);
  }
  #endif
  #endif
  if (!bOk) return -1;
  mbAnnonceEnvoyee = true;
  mbMesureRemontee = true;
  mulNbPresent++;
  return 0;
}

//
// Commandes reçues sur le topic de configuration du nœud (transmises par CConfig::handleMqttCommand()) :
// PING : répond « <nom> PRESENT ... »
// Retour : 0 traitée, -1 non concernée, -9 inactif
//
int CPresenceReseauElectrique::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
  if (cmd != "PING") return -1;
  if (!active) return -9;
  Serial.println("=== CPresenceReseauElectrique::handleMqttCommand === PING");
  publiePresent();
  return 0;
}

void CPresenceReseauElectrique::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  // Puce déjà configurée (nœud en service) : capteur actif d'emblée, nommé d'après le nœud (PrGuirlande) ;
  // puce neuve : désactivé, comme les autres capteurs (puce neuve muette)
  String sNomNoeud = prefs.getString("cfg_nom", "");
  bool bNoeudConfigure = prefs.getBool("cfg_conf", prefs.isKey("cfg_nom"));
  String sNomDefaut = sNomNoeud.startsWith("Pr") ? sNomNoeud
                    : "Pr" + (sNomNoeud.startsWith("Th") ? sNomNoeud.substring(2) : sNomNoeud);

  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), sNomDefaut);
  active = prefs.getBool((mPrefixNVS+"active").c_str(), bNoeudConfigure);

  prefs.end();
}

void CPresenceReseauElectrique::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putBool((mPrefixNVS+"active").c_str(), active);

  prefs.end();
}

void CPresenceReseauElectrique::loadFromWebServer(WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
}

String CPresenceReseauElectrique::getHTML() {
  String html =
      "<h2>Configuration de " + nomEquipement + " (présence sur le réseau électrique)</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active") + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Annonce de démarrage</label><input type=\"text\" value=\"" + String(mbAnnonceEnvoyee ? "publiée" : "en attente") + "\" disabled></div>"
        "<div><label>Trames PRESENT depuis le démarrage</label><input type=\"text\" value=\"" + String(mulNbPresent) + "\" disabled></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Topic des trames PRESENT</label><input type=\"text\" value=\"" + mqttSubTopicState + "\" disabled></div>"
      "</div>"
      "<p>Signe de vie : régler « Watchdog ALIVE » à 600 s dans la configuration générale.</p>";
  return html;
}

void CPresenceReseauElectrique::print() const {
  Serial.printf("     Nom                : %s\n", nomEquipement.c_str());
  Serial.printf("     Actif              : %s\n", active ? "OUI" : "NON");
  Serial.printf("     Annonce démarrage  : %s\n", mbAnnonceEnvoyee ? "publiée" : "en attente");
  Serial.printf("     Trames PRESENT     : %lu\n", mulNbPresent);
  Serial.printf("     MQTTState          : %s\n", mqttSubTopicState.c_str());
}
