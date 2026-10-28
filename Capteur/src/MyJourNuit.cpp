#include "global.h" // Serial -> journal web (voir MyJournal.h)
#include "MyJourNuit.h"
#include <time.h>
#include <esp_system.h>

//------------------------------------------------------------------------------
// Mémoire RTC : conservée pendant le deep sleep, réinitialisée à la mise sous tension et au reset.
// Elle n'est relue qu'au réveil de deep sleep (esp_reset_reason() == ESP_RST_DEEPSLEEP) et si rtcbValide.
//------------------------------------------------------------------------------
RTC_DATA_ATTR static bool   rtcbValide = false;
RTC_DATA_ATTR static int8_t rtcEtat = JOUR_NUIT_ETAT_INCONNU;
RTC_DATA_ATTR static int    rtcEclairement = -1;
RTC_DATA_ATTR static time_t rtcDernierEnvoi = 0; // Heure système (continue à avancer pendant le deep sleep)


int CJourNuit::setup(const String pref) {
  mPrefixNVS = pref;
  mbMesureRemontee = false;

  // On charge les infos de config depuis le NVS (dont l'état jour / nuit : mcEtatNVS)
  loadFromNVS();
  if (esp_reset_reason() == ESP_RST_DEEPSLEEP && rtcbValide) {
    // Réveil de deep sleep : on reprend l'état et l'éclairement mémorisés avant le sommeil
    mcLastEtat = rtcEtat;
    miLastEclairement = rtcEclairement;
    Serial.printf("CJourNuit::setup() - Réveil : état %s, éclairement %d %% (mémoire RTC)\n", getLastEtatTexte().c_str(), miLastEclairement);
  }
  else {
    // Mise sous tension ou reset : état seul repris du NVS, éclairement inconnu (la 1re mesure sera envoyée)
    mcLastEtat = mcEtatNVS;
    miLastEclairement = -1;
    rtcDernierEnvoi = time(nullptr);
    Serial.printf("CJourNuit::setup() - Démarrage : état %s (NVS)\n", getLastEtatTexte().c_str());
  }
  coupeAlimentation();
  analogReadResolution(12);  // 12 bits (0–4095) sur ESP32
  return 0;
}

//--------------------------------------------------------------------------
//  CJourNuit::loop()
//
// Le forçage est compté en heure système (time()), qui continue d'avancer pendant le deep sleep :
// millis() repart de 0 à chaque réveil.
//
// Retour : voir readAndPublish() ; -99 : pas de mesure à ce tour
//--------------------------------------------------------------------------
int CJourNuit::loop() {
  if (!active) return -9;
  static bool bPremiereMesure = true;
  static unsigned long lastRead = 0UL;
  bool bForce = (unsigned long)(time(nullptr) - rtcDernierEnvoi) >= mulIntervalleForcageRemonteeMesure;
  if (bPremiereMesure || bForce || (millis() - lastRead > mulIntervalleMesure * 1000UL)) {
    bPremiereMesure = false;
    lastRead = millis();
    return readAndPublish(bForce);
  }
  return -99;
}

// Pont hors tension : aucun courant dans la photorésistance
void CJourNuit::coupeAlimentation() {
  pinMode(mucPinAlim, OUTPUT);
  digitalWrite(mucPinAlim, LOW);
}

bool CJourNuit::readEclairement() {
  pinMode(mucPinAlim, OUTPUT);
  digitalWrite(mucPinAlim, HIGH);
  delay(JOUR_NUIT_STABILISATION_MS);
  uint32_t ulSommeMv = 0;
  for (int i = 0; i < JOUR_NUIT_NB_ECHANTILLONS; i++)
    ulSommeMv += analogReadMilliVolts(mucPin); // Corrigée par l'étalonnage d'usine (eFuse), voir CBatterieAA
  coupeAlimentation();

  uint32_t ulMv = ulSommeMv / JOUR_NUIT_NB_ECHANTILLONS;
  miNewEclairement = (int)((ulMv * 100UL + JOUR_NUIT_TENSION_ALIM_MV / 2) / JOUR_NUIT_TENSION_ALIM_MV);
  if (miNewEclairement > 100) miNewEclairement = 100;
  mcNewEtat = calculeEtat(miNewEclairement);
  Serial.printf("CJourNuit::readEclairement() - %lu mV -> %d %% -> %s\n", (unsigned long)ulMv, miNewEclairement,
                mcNewEtat == JOUR_NUIT_ETAT_JOUR ? "JOUR" : "NUIT");
  return true;
}

//
// Hystérésis : entre les deux seuils, l'état précédent est conservé.
// État précédent inconnu (puce neuve) : on tranche sur le milieu de la fenêtre.
//
int8_t CJourNuit::calculeEtat(int eclairement) const {
  if (eclairement >= miSeuilHaut) return JOUR_NUIT_ETAT_JOUR;
  if (eclairement <= miSeuilBas) return JOUR_NUIT_ETAT_NUIT;
  if (mcLastEtat != JOUR_NUIT_ETAT_INCONNU) return mcLastEtat;
  return (eclairement >= (miSeuilBas + miSeuilHaut) / 2) ? JOUR_NUIT_ETAT_JOUR : JOUR_NUIT_ETAT_NUIT;
}

String CJourNuit::getLastEtatTexte() const {
  if (mcLastEtat == JOUR_NUIT_ETAT_JOUR) return "JOUR";
  if (mcLastEtat == JOUR_NUIT_ETAT_NUIT) return "NUIT";
  return "INCONNU";
}

// État écrit en NVS seulement s'il a changé (usure de la flash)
void CJourNuit::memoriseEtatNVS() {
  if (mcLastEtat == mcEtatNVS) return;
  prefs.begin(nvs_namespace, false);
  prefs.putChar((mPrefixNVS+"etat").c_str(), mcLastEtat);
  prefs.end();
  mcEtatNVS = mcLastEtat;
  Serial.println("CJourNuit::memoriseEtatNVS() - État " + getLastEtatTexte() + " enregistré en NVS");
}

//
// « <nom> <sMesure> <date> <heure> <sValeur> [FORCE] » par LoRa ou MQTT selon le mode
// Retour : true si la trame est partie
//
bool CJourNuit::publie(const String& sMesure, const String& sValeur, bool force) {
  String sDate = (mDateTime != nullptr) ? mDateTime->getDate() : "DATE";
  String sTime = (mDateTime != nullptr) ? mDateTime->getTime() : "TIME";
  String sVal = nomEquipement + " " + sMesure + " " + sDate + " " + sTime + " " + sValeur;
  if (force) sVal += " FORCE";
  bool ret = false;
  #ifdef _LORA_P2P_MODE_
  if (onLoraP2PPublish != nullptr) {
    Serial.println("CJourNuit::publie() LoRa : " + sVal);
    ret = (onLoraP2PPublish((mqttSubTopicState + " " + sVal).c_str()) == 0);
  }
  #endif
  #ifdef _WIFI_MODE_
  #ifndef __DESACTIVE_ENVOI_MQTT__
  if (onMqttPublish != nullptr) {
    Serial.println("CJourNuit::publie() MQTT : " + sVal);
    ret = (onMqttPublish(mqttSubTopicState.c_str(), sVal.c_str()) == 0);
  }
  #endif
  #endif
  return ret;
}

//--------------------------------------------------------------------------
//  CJourNuit::readAndPublish()
//
// Envoi si l'état change, si l'éclairement change (quand il est remonté) ou si force.
// Les dernières valeurs ne sont mises à jour qu'après un envoi réussi : un envoi raté sera refait
// à la mesure suivante.
// Rien à envoyer : mesure considérée comme remontée (le capteur peut s'endormir).
//
// Retour
// 1 : Envoi effectué (ou tenté : voir mbMesureRemontee)
// 0 : Mesure inchangée, rien envoyé
// -1 : Echec de lecture
// -9 : Capteur inactif
//--------------------------------------------------------------------------
int CJourNuit::readAndPublish(bool force/*=false*/) {
  if (!active) return -9;
  if (!readEclairement()) return -1;

  bool bChange = (mcNewEtat != mcLastEtat) || (mbRemonterEclairement && miNewEclairement != miLastEclairement);
  if (!bChange && !force) {
    mbMesureRemontee = true;
    return 0;
  }

  bool bOk = publie("Jour", String(mcNewEtat == JOUR_NUIT_ETAT_JOUR ? 1 : 0), force);
  if (mbRemonterEclairement)
    bOk = publie("Eclairement", String(miNewEclairement), force) && bOk;
  mbMesureRemontee = bOk; // Si échec, mbMesureRemontee à false empêche le deep sleep

  if (bOk) {
    mcLastEtat = mcNewEtat;
    miLastEclairement = miNewEclairement;
    rtcEtat = mcLastEtat;
    rtcEclairement = miLastEclairement;
    rtcDernierEnvoi = time(nullptr);
    rtcbValide = true;
    memoriseEtatNVS();
  }
  return 1;
}

void CJourNuit::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  // Puce déjà configurée (nœud en service) : capteur actif d'emblée, nommé d'après le nœud
  // (ThCave -> JnCave), pour qu'un nœud LoRa seul n'ait pas besoin de la page Web.
  // Puce neuve : désactivé, comme les autres capteurs (puce neuve muette).
  String sNomNoeud = prefs.getString("cfg_nom", "");
  if (sNomNoeud.startsWith("Th")) sNomNoeud = sNomNoeud.substring(2);
  bool bNoeudConfigure = prefs.getBool("cfg_conf", prefs.isKey("cfg_nom"));

  nomEquipement = prefs.getString((mPrefixNVS+"nom").c_str(), "Jn" + sNomNoeud);
  mqttSubTopic = prefs.getString((mPrefixNVS+"subtopic").c_str(), "thermometre/");
  active = prefs.getBool((mPrefixNVS+"active").c_str(), bNoeudConfigure);
  miSeuilBas = prefs.getInt((mPrefixNVS+"sbas").c_str(), JOUR_NUIT_SEUIL_BAS);
  miSeuilHaut = prefs.getInt((mPrefixNVS+"shaut").c_str(), JOUR_NUIT_SEUIL_HAUT);
  miSeuilBasValide = miSeuilBas;
  miSeuilHautValide = miSeuilHaut;
  mbRemonterEclairement = prefs.getBool((mPrefixNVS+"ecl").c_str(), true);
  mcEtatNVS = prefs.getChar((mPrefixNVS+"etat").c_str(), JOUR_NUIT_ETAT_INCONNU);
  mulIntervalleMesure = prefs.getLong((mPrefixNVS+"inter").c_str(), mulDefaultIntervalleMesure);
  mulIntervalleForcageRemonteeMesure = (unsigned long)prefs.getLong((mPrefixNVS+"force").c_str(), mulDefaultIntervalleForcageRemonteeMesure);

  domotique_prefix = "home/";
  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";

  prefs.end();
}

// L'état jour / nuit n'est pas enregistré ici : voir memoriseEtatNVS()
void CJourNuit::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  prefs.putString((mPrefixNVS+"nom").c_str(), nomEquipement);
  prefs.putString((mPrefixNVS+"subtopic").c_str(), mqttSubTopic);
  prefs.putBool((mPrefixNVS+"active").c_str(), active);
  prefs.putInt((mPrefixNVS+"sbas").c_str(), miSeuilBas);
  prefs.putInt((mPrefixNVS+"shaut").c_str(), miSeuilHaut);
  prefs.putBool((mPrefixNVS+"ecl").c_str(), mbRemonterEclairement);
  prefs.putLong((mPrefixNVS+"inter").c_str(), mulIntervalleMesure);
  prefs.putLong((mPrefixNVS+"force").c_str(), mulIntervalleForcageRemonteeMesure);

  prefs.end();
}

void CJourNuit::loadFromWebServer(WebServer& server) {
  if (server.hasArg((mPrefixNVS+"nom").c_str())) nomEquipement = server.arg((mPrefixNVS+"nom").c_str());
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"ecl").c_str())) mbRemonterEclairement = true; else mbRemonterEclairement = false;
  if (server.hasArg((mPrefixNVS+"subtopic").c_str())) mqttSubTopic = server.arg((mPrefixNVS+"subtopic").c_str());
  if (server.hasArg((mPrefixNVS+"inter").c_str())) mulIntervalleMesure = server.arg((mPrefixNVS+"inter")).toInt();
  if (server.hasArg((mPrefixNVS+"force").c_str())) mulIntervalleForcageRemonteeMesure = server.arg((mPrefixNVS+"force")).toInt();
  // Fenêtre d'hystérésis : 0 <= bas < haut <= 100, sinon les seuils précédents sont conservés
  if (server.hasArg((mPrefixNVS+"sbas").c_str()) && server.hasArg((mPrefixNVS+"shaut").c_str())) {
    int iBas = server.arg((mPrefixNVS+"sbas").c_str()).toInt();
    int iHaut = server.arg((mPrefixNVS+"shaut").c_str()).toInt();
    if (iBas >= 0 && iBas < iHaut && iHaut <= 100) {
      miSeuilBas = miSeuilBasValide = iBas;
      miSeuilHaut = miSeuilHautValide = iHaut;
    }
    else
      Serial.printf("CJourNuit::loadFromWebServer() - Seuils refusés (bas %d, haut %d) : il faut 0 <= bas < haut <= 100\n", iBas, iHaut);
  }
}

void CJourNuit::print() const {
  Serial.printf("     Nom                : %s\n", nomEquipement.c_str());
  Serial.printf("     GPIO mesure / alim : %d / %d\n", mucPin, mucPinAlim);
  Serial.printf("     Actif              : %s\n", active ? "OUI" : "NON");
  Serial.printf("     État               : %s\n", getLastEtatTexte().c_str());
  Serial.printf("     Éclairement        : %d %%\n", miLastEclairement);
  Serial.printf("     Seuils bas / haut  : %d / %d %%\n", miSeuilBas, miSeuilHaut);
  Serial.printf("     Remonter éclairement : %s\n", mbRemonterEclairement ? "OUI" : "NON");
  Serial.printf("     Intervalle mesures : %ld s\n", mulIntervalleMesure);
  Serial.printf("     Forcage remontée   : %ld s\n", mulIntervalleForcageRemonteeMesure);
  Serial.printf("     MQTTSubTopic       : %s\n", mqttSubTopic.c_str());
  Serial.printf("     MQTTCmd            : %s\n", mqttSubTopicCommand.c_str());
  Serial.printf("     MQTTState          : %s\n", mqttSubTopicState.c_str());
}

String CJourNuit::getHTML() {
  String html =
      "<h2>Configuration de " + nomEquipement + "</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=" + (mPrefixNVS+"nom") + " value=\"" + nomEquipement + "\"></div>"
        "<div><label>GPIO mesure / alim.</label><input type=\"text\" value=\"" + String(mucPin) + " / " + String(mucPinAlim) + "\" disabled></div>"
      "</div>"
      "<div class=\"row\">"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active") + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
        "<div class=\"checkbox-row\"><label>Remonter l'éclairement</label><input type=\"checkbox\" name=" + (mPrefixNVS+"ecl") + " value=\"1\"" + String(mbRemonterEclairement ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>État actuel</label><input type=\"text\" value=\"" + getLastEtatTexte() + "\" disabled></div>"
        "<div><label>Dernier éclairement (%)</label><input type=\"text\" value=\"" + String(miLastEclairement) + "\" disabled></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Seuil bas : jour &rarr; nuit (%)</label><input type=\"text\" name=" + (mPrefixNVS+"sbas") + " value=\"" + String(miSeuilBas) + "\"></div>"
        "<div><label>Seuil haut : nuit &rarr; jour (%)</label><input type=\"text\" name=" + (mPrefixNVS+"shaut") + " value=\"" + String(miSeuilHaut) + "\"></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Intervalle mesure</label><input type=\"text\" name=" + (mPrefixNVS+"inter") + " value=\"" + mulIntervalleMesure + "\"></div>"
        "<div><label>Intervalle forçage remontée</label><input type=\"text\" name=" + (mPrefixNVS+"force") + " value=\"" + mulIntervalleForcageRemonteeMesure + "\"></div>"
      "</div>";
  html += "<div class=\"row\">"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=" + (mPrefixNVS+"subtopic") + " value=\"" + mqttSubTopic + "\"></div>"
      "</div>";
  return html;
}

//
// « <nom> MESURE » : mesure et remontée forcée
//
// Retour
// 0 : RAS
// -1 : Mauvais nom d'équipement
// -2 : Commande incomplète
//
int CJourNuit::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
  Serial.println("=== CJourNuit::handleMqttCommand === cmd : " + cmd);

  int i = cmd.indexOf(' ');
  String s = nomEquipement; s.toUpperCase();
  if (i < 0) return -2; // Commande incomplète
  if (cmd.substring(0, i) != s) return -1; // Mauvais nom d'équipement
  String sCommande = cmd.substring(i + 1); sCommande.trim();
  if (sCommande.startsWith("MESURE")) readAndPublish(true);
  return 0;
}

//
// Paramètre modifiable à distance (SET cle valeur), appelé par CConfig::setParam() pour les clés « jn_... ».
// Nom de clé = nom NVS, casse indifférente. Les seuils ne sont contrôlés ici qu'entre 0 et 100 : la fenêtre
// (bas < haut) est vérifiée par valideSeuils() une fois toute la commande traitée, sinon « SET jn_sbas 35
// jn_shaut 45 » serait refusé quand le seuil haut vaut encore 30.
// Retour
//  0 : paramètre modifié (en RAM ; saveToNVS() à la charge de l'appelant)
// -1 : clé inconnue
// -2 : valeur invalide ou hors bornes
//
int CJourNuit::setParam(const String& cle, const String& valeur) {
  String c = cle; c.toUpperCase();
  String p = mPrefixNVS; p.toUpperCase();
  if (!c.startsWith(p)) return -1;
  c = c.substring(p.length());
  String v = valeur; v.trim();
  bool bNombre = v.length() > 0 && v.length() <= 6;
  for (char ch : v) if (!isdigit(ch)) bNombre = false;
  long l = bNombre ? v.toInt() : -1;

  if (c == "SBAS")       { if (!bNombre || l > 100) return -2; miSeuilBas = l; }
  else if (c == "SHAUT") { if (!bNombre || l > 100) return -2; miSeuilHaut = l; }
  else if (c == "ECL")   { if (v != "0" && v != "1") return -2; mbRemonterEclairement = (v == "1"); }
  else if (c == "INTER") {
    if (!bNombre || l < JOUR_NUIT_INTER_MIN_SEC || l > JOUR_NUIT_INTER_MAX_SEC) return -2;
    mulIntervalleMesure = l;
  }
  else if (c == "FORCE") {
    if (!bNombre || l < JOUR_NUIT_FORCE_MIN_SEC || l > JOUR_NUIT_FORCE_MAX_SEC) return -2;
    mulIntervalleForcageRemonteeMesure = l;
  }
  else return -1;
  Serial.println("CJourNuit::setParam() - " + cle + " = " + v);
  return 0;
}

//
// Fenêtre d'hystérésis après une commande SET : si bas >= haut, les derniers seuils valides sont restaurés.
// Retour : true si la fenêtre est valide
//
bool CJourNuit::valideSeuils() {
  if (miSeuilBas < miSeuilHaut) {
    miSeuilBasValide = miSeuilBas;
    miSeuilHautValide = miSeuilHaut;
    return true;
  }
  Serial.printf("CJourNuit::valideSeuils() - Seuils refusés (bas %d, haut %d) : il faut bas < haut. Seuils %d / %d conservés\n",
                miSeuilBas, miSeuilHaut, miSeuilBasValide, miSeuilHautValide);
  miSeuilBas = miSeuilBasValide;
  miSeuilHaut = miSeuilHautValide;
  return false;
}

// Ajouté à la réponse CONFIG (séparateur « : » : Relais-Lora refuse un '=' dans une trame)
String CJourNuit::getParams() const {
  return mPrefixNVS + "sbas:" + String(miSeuilBas) +
         " " + mPrefixNVS + "shaut:" + String(miSeuilHaut) +
         " " + mPrefixNVS + "ecl:" + String(mbRemonterEclairement ? 1 : 0) +
         " " + mPrefixNVS + "inter:" + String(mulIntervalleMesure) +
         " " + mPrefixNVS + "force:" + String(mulIntervalleForcageRemonteeMesure);
}
