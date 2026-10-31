#include "RemotePresenceReseauElectrique.h"

int CRemotePresenceReseauElectrique::setup(const String pref) {
  mPrefixNVS = pref;
  loadFromNVS();
  calculeTopics();
  mPhase = PHASE_REPOS;
  mulDebutPhase = millis();
  return 0;
}

// Vérification utilisée = nom non vide ET case « Vérification active » cochée (sinon fonctionnement historique de
// l'appareil, nom et réglages gardés) ; topics de configuration du nœud : home/conf<nom en minuscules>/command|state
void CRemotePresenceReseauElectrique::calculeTopics() {
  nomEquipement.trim();
  active = nomEquipement.length() > 0 && mbVerifActive;
  String s = nomEquipement;
  s.toLowerCase();
  domotique_prefix = default_domotique_topic_prefix;
  mqttSubTopic = "conf" + s + "/";
  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";
}

void CRemotePresenceReseauElectrique::loop() {
  if (!active) return;
  unsigned long now = millis();

  // Démarrage du CYD : état réel inconnu, on interroge le nœud (vérification seule, sans renvoi du code)
  if (!mbVerifInitialeFaite && now >= PRESENCE_DELAI_DEMARRAGE_S * 1000UL) {
    mbVerifInitialeFaite = true;
    if (mPhase == PHASE_REPOS && miEtat == PRESENCE_INCONNUE && !mbNonVerifie) demandeEtat(PRESENCE_VERIF_SANS_COMMANDE);
  }

  switch (mPhase) {
    case PHASE_REPOS:
      // Nœud muet plus de 2 périodes ALIVE : appareil éteint hors du CYD (télécommande, coupure secteur...)
      if (!mbNonVerifie && miEtat == PRESENCE_PRESENT && mulDernierSignal != 0 &&
          now - mulDernierSignal > (2 * mulAliveS + PRESENCE_MARGE_ALIVE_S) * 1000UL) {
        DBGLN(DBG_ACTIONNEURS, "CRemotePresenceReseauElectrique - " + nomEquipement + " muet depuis 2 périodes ALIVE : absent");
        changeEtat(PRESENCE_ABSENT);
      }
      break;
    case PHASE_ATTENTE:
      if (now - mulDebutPhase >= mulDelaiS * 1000UL) {
        debutePhase(PHASE_QUESTION);
        pose();
      }
      break;
    case PHASE_QUESTION:
      if (now - mulDebutPhase >= mulAttenteS * 1000UL) {
        if (miNbQuestions >= miNbPing) conclut(PRESENCE_ABSENT);
        else pose();
      }
      break;
  }
}

//
// Commande de l'appareil : voulu = PRESENCE_ABSENT / PRESENCE_PRESENT, juste après l'envoi du code (1er essai) ;
// PRESENCE_VERIF_SANS_COMMANDE : interroge le nœud sans rien renvoyer (état réel au démarrage du CYD)
//
void CRemotePresenceReseauElectrique::demandeEtat(int voulu) {
  if (!active) return;
  miEtatVoulu = voulu;
  if (voulu == PRESENCE_VERIF_SANS_COMMANDE) {
    miEssai = 0;
    debutePhase(PHASE_QUESTION);
    pose();
    return;
  }
  miEssai = 1;
  debutePhase(PHASE_ATTENTE);
}

void CRemotePresenceReseauElectrique::debutePhase(EPhase phase) {
  mPhase = phase;
  mulDebutPhase = millis();
  mbSignalPhase = false;
  if (phase == PHASE_QUESTION) miNbQuestions = 0;
}

void CRemotePresenceReseauElectrique::pose() {
  miNbQuestions++;
  mulDebutPhase = millis();
  mbSignalPhase = false;
  DBG(DBG_ACTIONNEURS, "CRemotePresenceReseauElectrique - PING %d/%d sur %s\n", miNbQuestions, miNbPing, mqttSubTopicCommand.c_str());
  if (onMqttPublish != nullptr) onMqttPublish(mqttSubTopicCommand.c_str(), "PING");
}

void CRemotePresenceReseauElectrique::conclut(int etat) {
  mPhase = PHASE_REPOS;
  DBGLN(DBG_ACTIONNEURS, "CRemotePresenceReseauElectrique - " + nomEquipement + (etat == PRESENCE_PRESENT ? " présent" : " absent"));
  changeEtat(etat);
  if (miEtatVoulu == PRESENCE_VERIF_SANS_COMMANDE) return;
  if (etat == miEtatVoulu) { finCommande(); return; }
  if (miEssai < miNbEssais) {
    // Le code bascule : on ne le renvoie qu'une fois l'écart avec l'état voulu établi
    miEssai++;
    if (onRenvoi != nullptr) onRenvoi(miEssai, miNbEssais);
    debutePhase(PHASE_ATTENTE);
  }
  else {
    if (onEchec != nullptr) onEchec(miEtatVoulu, miEssai);
    finCommande();
  }
}

// Fin de la vérification d'une commande (pas d'une vérification seule) : HA déverrouille le bouton de l'appareil
void CRemotePresenceReseauElectrique::finCommande() {
  miEtatVoulu = PRESENCE_VERIF_SANS_COMMANDE;
  if (onFinCommande != nullptr) onFinCommande();
}

void CRemotePresenceReseauElectrique::changeEtat(int etat) {
  if (etat == miEtat) return;
  miEtat = etat;
  if (onEtatChange != nullptr) onEtatChange(etat);
}

void CRemotePresenceReseauElectrique::changeNonVerifie(bool nonVerifie) {
  if (nonVerifie == mbNonVerifie) return;
  mbNonVerifie = nonVerifie;
  if (nonVerifie && mPhase != PHASE_REPOS) {
    mPhase = PHASE_REPOS; // Plus de conclusion par le silence : vérification abandonnée
    if (miEtatVoulu != PRESENCE_VERIF_SANS_COMMANDE) finCommande();
  }
  DBGLN(DBG_ACTIONNEURS, "CRemotePresenceReseauElectrique - " + nomEquipement + (nonVerifie ? " : non vérifié (deep sleep)" : " : vérifié"));
  if (onNonVerifieChange != nullptr) onNonVerifieChange(nonVerifie);
}

// Trame du nœud (autre que son testament) : il est alimenté
void CRemotePresenceReseauElectrique::signalRecu() {
  mulDernierSignal = millis();
  if (mulDernierSignal == 0) mulDernierSignal = 1;
  mbSignalPhase = true;
  if (mPhase == PHASE_QUESTION) { conclut(PRESENCE_PRESENT); return; }
  if (mPhase == PHASE_ATTENTE) {
    // Allumage : le nœud s'annonce à son démarrage, inutile d'attendre. Extinction : on attend les questions.
    if (miEtatVoulu == PRESENCE_PRESENT) conclut(PRESENCE_PRESENT);
    return;
  }
  changeEtat(PRESENCE_PRESENT);
}

void CRemotePresenceReseauElectrique::handleMqttState(const String& payload) {
  if (!active) return;
  String s = payload;
  s.trim();
  int i = s.indexOf(' ');
  String sNom = (i < 0) ? s : s.substring(0, i);
  if (!sNom.equalsIgnoreCase(nomEquipement)) return;
  String sReste = (i < 0) ? "" : s.substring(i + 1);
  sReste.toUpperCase();

  if (sReste.startsWith("OFFLINE")) {
    // Testament (Wi-Fi), publié par le broker ~3 à 8 s après la coupure (keepalive 5 s du nœud ; identifiant fixe :
    // le testament d'une ancienne session arrive avant la nouvelle annonce PRESENT). Ignoré en deep sleep.
    if (mbNonVerifie) return;
    if (mPhase == PHASE_REPOS) { changeEtat(PRESENCE_ABSENT); return; }
    // Vérification en cours : extinction (ou vérification seule) conclue aussitôt, sans attendre les PING ;
    // allumage : on laisse les questions conclure (un testament tardif ne doit pas faire renvoyer le code qui bascule)
    if (miEtatVoulu != PRESENCE_PRESENT) conclut(PRESENCE_ABSENT);
    return;
  }
  signalRecu();
  if (sReste.startsWith("DEEPSLEEP ON")) changeNonVerifie(true);
  else if (sReste.startsWith("ALIVE")) changeNonVerifie(false); // ALIVE jamais émis en deep sleep
  else if (sReste.startsWith("CONFIG")) {
    if (sReste.indexOf("SLEEPA:1") >= 0) changeNonVerifie(true);
    else if (sReste.indexOf("SLEEPA:0") >= 0) changeNonVerifie(false);
  }
}

// Clés écrites seulement si un nœud est associé (NVS du CYD principal presque pleine le 30/10/2026)
void CRemotePresenceReseauElectrique::loadFromNVS() {
  prefs.begin(nvs_namespace, true);
  nomEquipement = prefs.getString((mPrefixNVS+"noeud").c_str(), "");
  mbVerifActive = prefs.getBool((mPrefixNVS+"actif").c_str(), true);
  mulDelaiS = prefs.getULong((mPrefixNVS+"delai").c_str(), PRESENCE_DELAI_DEFAUT_S);
  mulAttenteS = prefs.getULong((mPrefixNVS+"attente").c_str(), PRESENCE_ATTENTE_DEFAUT_S);
  miNbPing = prefs.getInt((mPrefixNVS+"nbping").c_str(), PRESENCE_NB_PING_DEFAUT);
  miNbEssais = prefs.getInt((mPrefixNVS+"nbess").c_str(), PRESENCE_NB_ESSAIS_DEFAUT);
  mulAliveS = prefs.getULong((mPrefixNVS+"alive").c_str(), PRESENCE_ALIVE_DEFAUT_S);
  prefs.end();
}

void CRemotePresenceReseauElectrique::saveToNVS() {
  const char* cles[] = {"noeud", "actif", "delai", "attente", "nbping", "nbess", "alive"};
  prefs.begin(nvs_namespace, false);
  if (nomEquipement.length() == 0) {
    for (const char* c : cles)
      if (prefs.isKey((mPrefixNVS+c).c_str())) prefs.remove((mPrefixNVS+c).c_str());
  }
  else {
    prefs.putString((mPrefixNVS+"noeud").c_str(), nomEquipement);
    prefs.putBool((mPrefixNVS+"actif").c_str(), mbVerifActive);
    prefs.putULong((mPrefixNVS+"delai").c_str(), mulDelaiS);
    prefs.putULong((mPrefixNVS+"attente").c_str(), mulAttenteS);
    prefs.putInt((mPrefixNVS+"nbping").c_str(), miNbPing);
    prefs.putInt((mPrefixNVS+"nbess").c_str(), miNbEssais);
    prefs.putULong((mPrefixNVS+"alive").c_str(), mulAliveS);
  }
  prefs.end();
}

void CRemotePresenceReseauElectrique::loadFromWebServer(WebServer& server) {
  if (server.hasArg((mPrefixNVS+"noeud").c_str())) nomEquipement = server.arg((mPrefixNVS+"noeud").c_str());
  mbVerifActive = server.hasArg((mPrefixNVS+"actif").c_str()); // Case à cocher : absente du formulaire si décochée
  if (server.hasArg((mPrefixNVS+"delai").c_str()) && server.arg((mPrefixNVS+"delai")).toInt() > 0)
    mulDelaiS = server.arg((mPrefixNVS+"delai")).toInt();
  if (server.hasArg((mPrefixNVS+"attente").c_str()) && server.arg((mPrefixNVS+"attente")).toInt() > 0)
    mulAttenteS = server.arg((mPrefixNVS+"attente")).toInt();
  if (server.hasArg((mPrefixNVS+"nbping").c_str()) && server.arg((mPrefixNVS+"nbping")).toInt() > 0)
    miNbPing = server.arg((mPrefixNVS+"nbping")).toInt();
  if (server.hasArg((mPrefixNVS+"nbess").c_str()) && server.arg((mPrefixNVS+"nbess")).toInt() > 0)
    miNbEssais = server.arg((mPrefixNVS+"nbess")).toInt();
  if (server.hasArg((mPrefixNVS+"alive").c_str()) && server.arg((mPrefixNVS+"alive")).toInt() > 0)
    mulAliveS = server.arg((mPrefixNVS+"alive")).toInt();
  calculeTopics();
}

String CRemotePresenceReseauElectrique::getHTML() {
  nomEquipement.trim();
  bool bNom = nomEquipement.length() > 0;
  String sEtat = (miEtat == PRESENCE_PRESENT) ? "allumé" : (miEtat == PRESENCE_ABSENT) ? "éteint" : "inconnu";
  if (mbNonVerifie) sEtat += " (non vérifié : nœud en deep sleep)";
  if (mPhase != PHASE_REPOS) sEtat += " (vérification en cours)";
  String html =
      "<h3>Vérification par nœud de présence</h3>"
      "<div class=\"row\">"
        "<div><label>Nœud de présence (vide = pas de vérification)</label><input type=\"text\" name=" + (mPrefixNVS+"noeud") + " value=\"" + nomEquipement + "\" placeholder=\"PrGuirlande\"></div>"
        "<div><label>État mesuré</label><input type=\"text\" value=\"" + (active ? sEtat : String(bNom ? "— (vérification désactivée)" : "—")) + "\" disabled></div>"
      "</div>"
      "<div class=\"checkbox-row\"><label>Vérification active (décochée : nom et réglages gardés, fonctionnement sans vérification)</label><input type=\"checkbox\" name=" + (mPrefixNVS+"actif") + " value=\"1\"" + String(mbVerifActive ? " checked" : "") + "></div>"
      "<div class=\"row\">"
        "<div><label>Délai avant la 1re question (s)</label><input type=\"number\" name=" + (mPrefixNVS+"delai") + " value=\"" + String(mulDelaiS) + "\"></div>"
        "<div><label>Attente de la réponse (s)</label><input type=\"number\" name=" + (mPrefixNVS+"attente") + " value=\"" + String(mulAttenteS) + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Questions (PING) avant « éteint »</label><input type=\"number\" name=" + (mPrefixNVS+"nbping") + " value=\"" + String(miNbPing) + "\"></div>"
        "<div><label>Essais (envois du code) avant l'alerte</label><input type=\"number\" name=" + (mPrefixNVS+"nbess") + " value=\"" + String(miNbEssais) + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Période ALIVE du nœud (s)</label><input type=\"number\" name=" + (mPrefixNVS+"alive") + " value=\"" + String(mulAliveS) + "\"></div>"
        "<div><label>Topic du nœud</label><input type=\"text\" value=\"" + (active ? mqttSubTopicState : String("—")) + "\" disabled></div>"
      "</div>";
  return html;
}

void CRemotePresenceReseauElectrique::print() const {
  DBG(DBG_ACTIONNEURS, "     Nœud de présence   : %s%s\n", nomEquipement.length() > 0 ? nomEquipement.c_str() : "(aucun)",
      (nomEquipement.length() > 0 && !mbVerifActive) ? " (vérification désactivée)" : "");
  if (!active) return;
  DBG(DBG_ACTIONNEURS, "     État mesuré        : %d%s\n", miEtat, mbNonVerifie ? " (non vérifié)" : "");
  DBG(DBG_ACTIONNEURS, "     Délai / attente    : %lu s / %lu s\n", mulDelaiS, mulAttenteS);
  DBG(DBG_ACTIONNEURS, "     PING / essais      : %d / %d\n", miNbPing, miNbEssais);
  DBG(DBG_ACTIONNEURS, "     Période ALIVE      : %lu s\n", mulAliveS);
  DBG(DBG_ACTIONNEURS, "     MQTTCmd / State    : %s / %s\n", mqttSubTopicCommand.c_str(), mqttSubTopicState.c_str());
}
