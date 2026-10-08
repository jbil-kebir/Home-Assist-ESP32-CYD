// MyActionneurIR.cpp
#include "MyActionneurIR.h"

#ifdef ACTIONNEUR_IR
// Émission seule, mais le code de réception d'IRremote reste compilé : avec
// DISABLE_CODE_FOR_RECEIVER, sans -D DEBUG (PRODUCTION), l'édition de liens échoue
// (décodeurs ir_*.hpp conservés, matchMark()/matchSpace() de IRReceive.hpp absents).
// Coût : quelques Ko de Flash, rien à l'exécution tant que IrReceiver.begin() n'est pas appelé.
//#define DISABLE_CODE_FOR_RECEIVER
// IRremote.hpp contient des définitions : à n'inclure que dans ce fichier (sinon définitions multiples au link)
#include <IRremote.hpp>

bool CActionneurIR::setup(const String pref, int ledPin/*=-1*/) {
  mPrefixNVS = pref;

  // On charge les infos de config depuis le NVS
  loadFromNVS();

  // Initialisé même inactif : l'activation par la page Web ne nécessite pas de redémarrage
  if (ledPin >= 0)
    IrSender.begin(mucPin, ENABLE_LED_FEEDBACK, ledPin); // LED allumée pendant l'émission
  else
    IrSender.begin(mucPin, DISABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
  _initialized = true;
  return true;
}

//--------------------------------------------------------------------------
//  CActionneurIR::loop()
//
// Retour
// 0 : RAS
// -1 : Non initialisé
// -9 : Inactif
//--------------------------------------------------------------------------
int CActionneurIR::loop() {
  if (!active) return -9;
  if (!_initialized) return -1;

  // Pas de bip dans le délai : réémission, ou échec si toutes les tentatives sont épuisées
  if (mEtatSequence == SEQ_ATTENTE_BIP && millis() - mulDebutAttente >= muiAttenteBip) {
    Serial.printf("CActionneurIR::loop() - Pas de bip après l'émission %u/%u\n", muiTentative, muiNbTentatives);
    if (muiTentative < muiNbTentatives) emet();
    else finSequence(-1);
  }

  gerePublications();
  return 0;
}

//--------------------------------------------------------------------------
//  CActionneurIR::envoie()
//
// Emission brute du code IR. Bloquante (~70 ms par trame NEC + ~110 ms par répétition).
//--------------------------------------------------------------------------
void CActionneurIR::envoie() {
  if (!_initialized) return;
  Serial.printf("[IR] Emission (addr:0x%X, cmd:0x%02X, proto:%u, rep:%u)\n", muiAdresse, mucCommande, mucProtocole, mucRepetitions);
  switch (mucProtocole) {
    case 1: // NEC
      IrSender.sendNEC(muiAdresse, mucCommande, mucRepetitions);
      break;
    case 2: // SONY
      IrSender.sendSony(muiAdresse, mucCommande, mucRepetitions);
      break;
    case 3: // RC5
      IrSender.sendRC5(muiAdresse, mucCommande, mucRepetitions);
      break;
    case 4: // RC6
      IrSender.sendRC6(muiAdresse, mucCommande, mucRepetitions);
      break;
    default:
      Serial.printf("[IR] Protocole inconnu : %u\n", mucProtocole);
      break;
  }
}

//--------------------------------------------------------------------------
//  CActionneurIR::demandeEtat()
//
// Lance la séquence : émission puis vérification par le bip (voir MyActionneurIR.h).
// Une nouvelle demande remplace la séquence en cours.
//
// Retour
// 0 : séquence lancée
// -1 : consigne invalide
// -9 : Inactif
//--------------------------------------------------------------------------
int CActionneurIR::demandeEtat(int consigne) {
  if (!active) return -9;
  if (consigne != 0 && consigne != 1) return -1;
  Serial.printf("CActionneurIR::demandeEtat() - Consigne : %s\n", consigne ? "ON" : "OFF");
  miConsigne = consigne;
  muiTentative = 0;
  emet();
  return 0;
}

//--------------------------------------------------------------------------
//  CActionneurIR::handleMqttCommand()
//
// "<nomEquipement du micro> OFF|ON" reçu sur home/<subtopic>command :
//  - OFF : extinction du chauffage. Le CYD coupe ensuite le relais à réception de "OnOff 0"
//          (ou de IRKO / après son délai).
//  - ON  : allumage, quand le noeud est déjà alimenté (après un IRKO, ou chauffage éteint à la main).
//          Sinon, la mise en marche est la mise sous tension par le relais (demandeEtat(1) dans setup()).
//
// Retour
// 0 : séquence lancée
// -1 : commande non adressée à l'actionneur IR
// -9 : Inactif
//--------------------------------------------------------------------------
int CActionneurIR::handleMqttCommand(const String& payload) {
  #ifdef CAPTEUR_MICRO_MAX4466
  if (mMicro == nullptr) return -1;
  String cmd = payload;
  cmd.trim();
  cmd.toUpperCase();
  int sp = cmd.indexOf(' ');
  if (sp < 0) return -1;
  String sExpediteur = cmd.substring(0, sp);
  String sNom = mMicro->nomEquipement; sNom.toUpperCase();
  if (sExpediteur != sNom) return -1;
  String sCommande = cmd.substring(sp + 1);
  sp = sCommande.indexOf(' ');
  if (sp >= 0) sCommande = sCommande.substring(0, sp); // Ignore la suite (IP...)
  if (sCommande != "OFF" && sCommande != "ON") return -1;

  Serial.println("CActionneurIR::handleMqttCommand() - Commande " + sCommande + " reçue");
  return demandeEtat(sCommande == "ON" ? 1 : 0);
  #else
  return -1;
  #endif
}

void CActionneurIR::emet() {
  muiTentative++;
  Serial.printf("CActionneurIR::emet() - Emission %u/%u (consigne %s)\n", muiTentative, muiNbTentatives, miConsigne ? "ON" : "OFF");
  envoie();
  mulDebutAttente = millis();
  mEtatSequence = SEQ_ATTENTE_BIP;
  #ifdef CAPTEUR_MICRO_MAX4466
  // Sans micro actif, on ne peut pas vérifier : la séquence s'arrête à l'émission
  if (mMicro == nullptr || !mMicro->active) {
    Serial.println("CActionneurIR::emet() - Micro absent ou inactif : pas de vérification par le bip");
    mEtatSequence = SEQ_REPOS;
  }
  #else
  mEtatSequence = SEQ_REPOS;
  #endif
}

//--------------------------------------------------------------------------
//  CActionneurIR::onBip()
//
// Appelée par le micro à chaque bip valide (avant son filtre anti-répétition).
// val : 1 = bip court (ON), 0 = bip long (OFF)
//
// Retour
// true : bip pris en charge par la séquence (le micro ne le publie pas lui-même)
// false : pas de séquence en cours (bip dû à un autre usage du chauffage : le micro le traite)
//--------------------------------------------------------------------------
bool CActionneurIR::onBip(int val) {
  if (mEtatSequence != SEQ_ATTENTE_BIP) return false;

  if (val == miConsigne) {
    Serial.printf("CActionneurIR::onBip() - Bip %s conforme à la consigne\n", val ? "ON" : "OFF");
    finSequence(val);
  }
  else if (muiTentative < muiNbTentatives) {
    // Le chauffage était déjà dans l'état demandé : la bascule l'a inversé, on réémet
    Serial.printf("CActionneurIR::onBip() - Bip %s contraire à la consigne : réémission\n", val ? "ON" : "OFF");
    emet();
  }
  else {
    Serial.printf("CActionneurIR::onBip() - Bip %s contraire à la consigne, tentatives épuisées\n", val ? "ON" : "OFF");
    finSequence(-1);
  }
  return true;
}

void CActionneurIR::finSequence(int etatObtenu) {
  mEtatSequence = SEQ_REPOS;
  if (etatObtenu >= 0) {
    miEtatAPublier = etatObtenu;
    mbIrkoAPublier = false;
  }
  else {
    Serial.println("CActionneurIR::finSequence() - ECHEC");
    miEtatAPublier = -1;
    mbIrkoAPublier = true;
  }
  mulDerniereTentativePublication = millis() - IR_INTERVALLE_REPUBLICATION; // Publication immédiate
}

//--------------------------------------------------------------------------
// Publie le résultat de la séquence, en retentant tant que MQTT n'est pas connecté
//--------------------------------------------------------------------------
void CActionneurIR::gerePublications() {
  if (miEtatAPublier < 0 && !mbIrkoAPublier) return;
  if (millis() - mulDerniereTentativePublication < IR_INTERVALLE_REPUBLICATION) return;
  mulDerniereTentativePublication = millis();

  #ifdef CAPTEUR_MICRO_MAX4466
  if (miEtatAPublier >= 0) {
    if (mMicro == nullptr || mMicro->onMqttPublish == nullptr) miEtatAPublier = -1; // Pas de MQTT : abandon
    else if (mMicro->publieEtat(miEtatAPublier)) miEtatAPublier = -1;
  }
  #else
  miEtatAPublier = -1;
  #endif

  if (mbIrkoAPublier) {
    if (onMqttPublish == nullptr) mbIrkoAPublier = false; // Pas de MQTT : abandon
    else if (publieIrko()) mbIrkoAPublier = false;
  }
}

bool CActionneurIR::publieIrko() {
  #ifdef CAPTEUR_MICRO_MAX4466
  if (mMicro == nullptr) return true; // Ni nom ni topic : rien à publier
  String sDate = (mDateTime != nullptr) ? mDateTime->getDate() : "DATE";
  String sTime = (mDateTime != nullptr) ? mDateTime->getTime() : "TIME";
  String sVal = mMicro->nomEquipement + " IRKO " + sDate + " " + sTime;
  Serial.println("CActionneurIR::publieIrko() : " + sVal);
  gJournal.sauvegarde("IRKO"); // Le CYD peut couper le relais (coupure forcée) : on garde la trace de l'échec
  return (onMqttPublish(mMicro->mqttSubTopicState.c_str(), sVal.c_str()) == 0);
  #else
  return true;
  #endif
}

void CActionneurIR::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
    onMqttPublish = cb;
}

void CActionneurIR::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  active = prefs.getBool((mPrefixNVS+"active").c_str(), false);
  mucPin = prefs.getUShort((mPrefixNVS+"pin").c_str(), ACTIONNEUR_IR_PIN);
  mucProtocole = prefs.getUChar((mPrefixNVS+"proto").c_str(), IR_DEFAULT_PROTOCOLE);
  muiAdresse = prefs.getUShort((mPrefixNVS+"addr").c_str(), IR_DEFAULT_ADRESSE);
  mucCommande = prefs.getUChar((mPrefixNVS+"cmd").c_str(), IR_DEFAULT_COMMANDE);
  mucRepetitions = prefs.getUChar((mPrefixNVS+"rep").c_str(), IR_DEFAULT_REPETITIONS);
  muiAttenteBip = prefs.getUShort((mPrefixNVS+"attente").c_str(), IR_DEFAULT_ATTENTE_BIP);
  muiNbTentatives = prefs.getUChar((mPrefixNVS+"tentat").c_str(), IR_DEFAULT_NB_TENTATIVES);

  prefs.end();
  bornesParametres();
}

void CActionneurIR::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  prefs.putBool((mPrefixNVS+"active").c_str(), active);
  prefs.putUShort((mPrefixNVS+"pin").c_str(), mucPin);
  prefs.putUChar((mPrefixNVS+"proto").c_str(), mucProtocole);
  prefs.putUShort((mPrefixNVS+"addr").c_str(), muiAdresse);
  prefs.putUChar((mPrefixNVS+"cmd").c_str(), mucCommande);
  prefs.putUChar((mPrefixNVS+"rep").c_str(), mucRepetitions);
  prefs.putUShort((mPrefixNVS+"attente").c_str(), muiAttenteBip);
  prefs.putUChar((mPrefixNVS+"tentat").c_str(), muiNbTentatives);

  prefs.end();
}

void CActionneurIR::setActive(bool state) {
  active = state;
  prefs.begin(nvs_namespace, false);
  prefs.putBool((mPrefixNVS+"active").c_str(), state);
  prefs.end();
}

// Adresse et commande : saisies en hexadécimal (0xCDAB) ou en décimal
void CActionneurIR::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"active").c_str())) active = true; else active = false;
  if (server.hasArg((mPrefixNVS+"pin").c_str())) mucPin = server.arg((mPrefixNVS+"pin")).toInt();
  if (server.hasArg((mPrefixNVS+"proto").c_str())) mucProtocole = server.arg((mPrefixNVS+"proto")).toInt();
  if (server.hasArg((mPrefixNVS+"addr").c_str())) muiAdresse = strtoul(server.arg((mPrefixNVS+"addr")).c_str(), nullptr, 0);
  if (server.hasArg((mPrefixNVS+"cmd").c_str())) mucCommande = strtoul(server.arg((mPrefixNVS+"cmd")).c_str(), nullptr, 0);
  if (server.hasArg((mPrefixNVS+"rep").c_str())) mucRepetitions = server.arg((mPrefixNVS+"rep")).toInt();
  // Lus en long : une saisie négative est ramenée au minimum par bornesParametres(), pas à une valeur énorme
  if (server.hasArg((mPrefixNVS+"attente").c_str())) muiAttenteBip = max(0L, server.arg((mPrefixNVS+"attente")).toInt());
  if (server.hasArg((mPrefixNVS+"tentat").c_str())) muiNbTentatives = max(0L, server.arg((mPrefixNVS+"tentat")).toInt());
  bornesParametres();
}

//--------------------------------------------------------------------------
// Ramène l'attente du bip et le nombre de tentatives dans leurs bornes (voir MyActionneurIR.h).
// Aussi au chargement du NVS, pour les valeurs enregistrées avant l'ajout des bornes.
//--------------------------------------------------------------------------
void CActionneurIR::bornesParametres() {
  unsigned int uiAttente = constrain(muiAttenteBip, (unsigned int)IR_MIN_ATTENTE_BIP, (unsigned int)IR_MAX_ATTENTE_BIP);
  unsigned int uiTentatives = constrain(muiNbTentatives, (unsigned int)IR_MIN_NB_TENTATIVES, (unsigned int)IR_MAX_NB_TENTATIVES);
  if (uiAttente != muiAttenteBip || uiTentatives != muiNbTentatives)
    Serial.printf("CActionneurIR::bornesParametres() - Attente du bip %u -> %u ms, tentatives %u -> %u\n", muiAttenteBip, uiAttente, muiNbTentatives, uiTentatives);
  muiAttenteBip = uiAttente;
  muiNbTentatives = uiTentatives;
}

String CActionneurIR::getHTML() {
  char sAdresse[8], sCommande[8];
  snprintf(sAdresse, sizeof(sAdresse), "0x%X", muiAdresse);
  snprintf(sCommande, sizeof(sCommande), "0x%X", mucCommande);
  String html = "";
  html =  "<h2>Configuration de l'actionneur IR</h2>"
      "<div class=\"row\">"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=" + (mPrefixNVS+"active") + " value=\"1\"" + String(active ? " checked" : "") + "></div>"
        "<div><label>Pin</label><input type=\"text\" name=" + (mPrefixNVS+"pin") + " value=\"" + mucPin + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Protocole (1 NEC, 2 SONY, 3 RC5, 4 RC6)</label><input type=\"text\" name=" + (mPrefixNVS+"proto") + " value=\"" + mucProtocole + "\"></div>"
        "<div><label>Adresse</label><input type=\"text\" name=" + (mPrefixNVS+"addr") + " value=\"" + sAdresse + "\"></div>"
        "<div><label>Commande</label><input type=\"text\" name=" + (mPrefixNVS+"cmd") + " value=\"" + sCommande + "\"></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Répétitions</label><input type=\"text\" name=" + (mPrefixNVS+"rep") + " value=\"" + mucRepetitions + "\"></div>"
        "<div><label>Attente du bip (ms, " + String(IR_MIN_ATTENTE_BIP) + "-" + String(IR_MAX_ATTENTE_BIP) + ")</label><input type=\"text\" name=" + (mPrefixNVS+"attente") + " value=\"" + muiAttenteBip + "\"></div>"
        "<div><label>Nb de tentatives (" + String(IR_MIN_NB_TENTATIVES) + "-" + String(IR_MAX_NB_TENTATIVES) + ")</label><input type=\"text\" name=" + (mPrefixNVS+"tentat") + " value=\"" + muiNbTentatives + "\"></div>"
      "</div>";

  return html;
}

void CActionneurIR::print() const {
  Serial.printf("     Actionneur IR        : %s\n", active ? "ACTIF" : "INACTIF");
  Serial.printf("     Pin                  : %d\n", mucPin);
  Serial.printf("     Protocole            : %u (1 NEC, 2 SONY, 3 RC5, 4 RC6)\n", mucProtocole);
  Serial.printf("     Adresse / Commande   : 0x%X / 0x%02X\n", muiAdresse, mucCommande);
  Serial.printf("     Répétitions          : %u\n", mucRepetitions);
  Serial.printf("     Attente du bip       : %u ms\n", muiAttenteBip);
  Serial.printf("     Nb de tentatives     : %u\n", muiNbTentatives);
}

#endif // ACTIONNEUR_IR
