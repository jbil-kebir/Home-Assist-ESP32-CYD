#include "MyRCDevice.h"
#include "MyConfig.h"
#include "MyEcran.h"
#include "MyNoeudChauffageIRSound.h"
#include "RemoteTor.h"
#include "MyDateTime.h"

extern CMyDateTime mDateTime; // Heure dans les traces DBG_NOEUD

//----------------------------------------------------------------------------------
// CNoeudChauffageIRSound::envoiOnOff()
//
// Bouton ON/OFF (écran, CYD auxiliaires, Home Assistant) :
//  - chauffage ON  : séquence d'arrêt (demandeOff())
//  - chauffage OFF : séquence de marche (demandeOn())
// L'état (etat) n'est pas basculé ici : c'est fait dans setEtatReelOnOff(), au retour de l'état réel.
// Retour :
// 0 : RAS
// -1 : Equipement inactif
// -2 : Séquence en cours
// -3 : Pas de noeud ou pas de MQTT
//----------------------------------------------------------------------------------
int CNoeudChauffageIRSound::envoiOnOff() {
  if (!active) {
    message(nomEquipement + ": inactif");
    return -1;
  }
  if (mEtatSequence != SEQ_REPOS) {
    message(nomEquipement + ": ignore: " + etapeSequence());
    return -2;
  }
  return etat ? demandeOff() : demandeOn();
}

//----------------------------------------------------------------------------------
// CNoeudChauffageIRSound::setEtatReelOnOff()
//
// Etat réel du chauffage, appelé depuis CEcran
//  Eteint : 0
//  Allumé : 1
//----------------------------------------------------------------------------------
void CNoeudChauffageIRSound::setEtatReelOnOff(bool state) {
  DBG(DBG_NOEUD, "[%s] %s : OnOff %d reçu (état affiché %s)\n", mDateTime.getTime().c_str(), nomEquipement.c_str(), state, etat ? "ON" : "OFF");
  // Chauffage désactivé (procédure : OFF puis DISABLE) : un ON est un bip parasite du micro.
  // Il ne doit ni annuler la coupure programmée, ni changer l'état affiché.
  // Les OnOff 0 restent traités (fin d'une séquence d'arrêt lancée juste avant le DISABLE).
  if (state && !active) {
    message(nomEquipement + ": ON ignore (inactif)");
    return;
  }
  // Arrêt en cours : le noeud a éteint le chauffage (OFF émis par IR et confirmé par le bip).
  //  - vérification avant coupure différée (voir loop()) : coupure immédiate du relais ;
  //  - sinon fin de la séquence, coupure différée (voir ci-dessous).
  bool bOffConfirme = false;
  if (!state && mEtatSequence == ARRET_ATTENTE_ETAT0) {
    if (mbCoupureImmediate) {
      mbCoupureImmediate = false;
      message(nomEquipement + ": OFF confirme, coupure prise");
      muiEnvoisCode = 0;
      coupeRelais();
    }
    else {
      mEtatSequence = SEQ_REPOS;
      mbCoupureDiffereeArmee = false; // Délai relancé à partir de maintenant
      bOffConfirme = true;
    }
  }

  // Chauffage éteint (fin d'arrêt, télécommande IR, republication périodique du noeud,
  // redémarrage du CYD...) : coupure du relais programmée dans mulDelaiCoupureApresOff min,
  // si elle ne l'est pas déjà (les republications périodiques ne la repoussent pas).
  // D'ici là, un ON rallume par IR, sans code relais ni redémarrage du C3.
  // Un OnOff 0 spontané (bip hors séquence) peut être un parasite alors que le chauffage est
  // allumé : il n'est pas confirmé, et la coupure sera précédée d'un arrêt par IR (voir loop()).
  if (!state && mEtatSequence == SEQ_REPOS && !mbCoupureDiffereeArmee) {
    mbCoupureDiffereeArmee = true;
    mbOffConfirme = bOffConfirme;
    mulDebutCoupureDifferee = millis();
    message(nomEquipement + ": eteint, coupure prise ds " + String(mulDelaiCoupureApresOff) + "min");
  }
  if (state) mbCoupureDiffereeArmee = false; // Chauffage rallumé : plus de coupure prévue

  // Attente de l'arrêt : le noeud signale le chauffage allumé (redémarrage du C3, qui allume
  // le chauffage à la mise sous tension, ou commande OFF perdue) : on renvoie la commande OFF
  if (state && mEtatSequence == ARRET_ATTENTE_ETAT0) {
    if (muiRenvoisOff < muiNbEnvoisCode) {
      muiRenvoisOff++;
      message(nomEquipement + ": ON recu, renvoi OFF");
      publieCommandeNoeud("OFF");
    }
  }

  // Coupure du relais en cours : un ON du noeud est un bip parasite (bruit, sonnette du banc de test).
  // Le chauffage a été éteint par IR, on garde OFF.
  if (state && mEtatSequence == ARRET_ATTENTE_DISPARITION) {
    message(nomEquipement + ": ON ignore (coupure relais)");
    return;
  }

  // Marche en cours : le noeud a allumé le chauffage (OnOff 1 peut précéder la détection de présence)
  if (state && (mEtatSequence == MARCHE_ATTENTE_APPARITION || mEtatSequence == MARCHE_ATTENTE_ETAT1)) {
    message(nomEquipement + ": ON confirme");
    mEtatSequence = SEQ_REPOS;
  }

  appliqueEtat(state);
}

// Mise à jour de l'état affiché (écran, CYD auxiliaires), sans effet sur les séquences
void CNoeudChauffageIRSound::appliqueEtat(bool state) {
  if (etat == state) return;

  saveState(state); // Bascule de etat et sauvegarde NVS
  if (mEcran != nullptr)
    mEcran->updateAllStates();

  // Mise à jour de l'IHM des CYD auxiliaires
  if (onMqttPublish != nullptr)
    onMqttPublish(mqttSubTopicCommand.c_str(), etat ? "ONR" : "OFFR");
}

//----------------------------------------------------------------------------------
// CNoeudChauffageIRSound::demandeOff()
//
// Séquence d'arrêt :
// 1. Noeud absent : le relais est déjà coupé, rien à faire.
// 2. Publication de "<noeud> OFF" sur le topic commande du noeud, qui éteint le chauffage par IR
//    et publie "OnOff 0" (ou IRKO en cas d'échec).
// 3. A réception de OnOff 0 : fin de la séquence, coupure du relais différée de
//    mulDelaiCoupureApresOff min (voir setEtatReelOnOff() et loop()) ; un ON dans ce délai
//    rallume par IR sans redémarrer le C3.
//    Sans OnOff 0 après mulDelaiCoupureForcee minutes : envoi immédiat du code relais.
//    IRKO ne coupe pas le relais tout de suite : couper le noeud ferait perdre le seul moyen
//    d'éteindre un chauffage resté allumé.
// 4. Vérification de la disparition du noeud (watchdog expiré) dans mulDelaiVerification s.
//    Sinon le code n'a pas été reçu : renvoi, jusqu'à muiNbEnvoisCode envois.
//    Le relais n'a qu'un code qui bascule : on ne renvoie jamais avant d'être sûr que
//    le précédent n'a pas été reçu (sinon le second annulerait le premier).
//
// Retour :
// 0 : RAS (séquence lancée ou rien à faire)
// -1 : Equipement inactif
// -2 : Arrêt déjà en cours
// -3 : Pas de noeud ou pas de MQTT
//----------------------------------------------------------------------------------
int CNoeudChauffageIRSound::demandeOff() {
  if (!active) {
    message(nomEquipement + ": inactif");
    return -1;
  }
  if (mEtatSequence != SEQ_REPOS) {
    message(nomEquipement + ": ignore: " + etapeSequence());
    return -2;
  }
  if (mNoeud == nullptr || onMqttPublish == nullptr) {
    message(nomEquipement + ": arret impossible (MQTT)");
    return -3;
  }
  if (!noeudPresent()) {
    message(nomEquipement + ": noeud absent, relais coupe");
    appliqueEtat(false); // Sinon le bouton resterait à ON (nœud disparu sans OnOff 0)
    return 0;
  }
  lanceArret(false);
  message(nomEquipement + ": arret demande");
  return 0;
}

// Publication de "<noeud> OFF" et attente de OnOff 0 (voir demandeOff())
// bCoupureImmediate : coupure du relais dès OnOff 0 (vérification avant coupure différée),
// sinon coupure différée de mulDelaiCoupureApresOff min
void CNoeudChauffageIRSound::lanceArret(bool bCoupureImmediate) {
  publieCommandeNoeud("OFF");
  mEtatSequence = ARRET_ATTENTE_ETAT0;
  mulDebutEtape = millis();
  muiEnvoisCode = 0;
  muiRenvoisOff = 0;
  mbNoeudPerdu = false;
  mbCoupureImmediate = bCoupureImmediate;
  mbCoupureDiffereeArmee = false;
}

// Commande "<noeud> ON|OFF" sur le topic commande du noeud (jamais retain)
void CNoeudChauffageIRSound::publieCommandeNoeud(const char* cmd) {
  if (mNoeud == nullptr || onMqttPublish == nullptr) return;
  String sCmd = mNoeud->nomEquipement + " " + cmd;
  onMqttPublish(mNoeud->mqttSubTopicCommand.c_str(), sCmd.c_str());
}

//----------------------------------------------------------------------------------
// CNoeudChauffageIRSound::demandeOn()
//
// Séquence de marche :
// 1. Noeud présent (relais déjà fermé, chauffage éteint après un IRKO ou à la main) :
//    publication de "<noeud> ON" sur le topic commande du noeud, puis étape 3.
// 2. Noeud absent : envoi du code relais. A la mise sous tension, le noeud allume
//    le chauffage par IR de lui-même. On attend son apparition (premier message)
//    pendant mulDelaiApparition s. Sinon le code n'a pas été reçu : renvoi, jusqu'à
//    muiNbEnvoisCode envois. Ce délai doit dépasser la durée de démarrage du C3 :
//    un renvoi alors que le premier code a été reçu couperait le noeud.
// 3. Attente de OnOff 1 pendant mulDelaiConfirmationOn s.
//    IRKO ou pas de réponse : message, le relais reste fermé (un nouvel ON passe par 1.).
//
// Retour :
// 0 : RAS (séquence lancée)
// -1 : Equipement inactif
// -2 : Séquence déjà en cours
// -3 : Pas de noeud ou pas de MQTT
//----------------------------------------------------------------------------------
int CNoeudChauffageIRSound::demandeOn() {
  if (!active) {
    message(nomEquipement + ": inactif");
    return -1;
  }
  if (mEtatSequence != SEQ_REPOS) {
    message(nomEquipement + ": ignore: " + etapeSequence());
    return -2;
  }
  if (mNoeud == nullptr || onMqttPublish == nullptr) {
    message(nomEquipement + ": marche impossible (MQTT)");
    return -3;
  }
  if (noeudPresent()) {
    mbCoupureDiffereeArmee = false; // Rallumage pendant le délai de coupure après OFF
    publieCommandeNoeud("ON");
    mEtatSequence = MARCHE_ATTENTE_ETAT1;
    mulDebutEtape = millis();
    message(nomEquipement + ": marche demandee (IR)");
    return 0;
  }
  message(nomEquipement + ": marche demandee");
  mbCoupureDiffereeArmee = false; // Coupure restée armée pendant l'absence du noeud
  muiEnvoisCode = 0;
  fermeRelais();
  return 0;
}

//----------------------------------------------------------------------------------
// CNoeudChauffageIRSound::loop()
//
// Déroulement des séquences de marche et d'arrêt (voir demandeOn() et demandeOff())
//
// Disparition du noeud (watchdog expiré) : l'état affiché passe à OFF.
// Hors séquence (prise coupée par sa télécommande, C3 débranché...), l'état réel du chauffage
// est inconnu (la prise n'alimente que le C3) ; OFF reste sûr : un ON relance le C3, qui
// vérifie l'état par le bip et réémet l'IR si besoin.
//----------------------------------------------------------------------------------
void CNoeudChauffageIRSound::loop() {
  bool bPresent = noeudPresent();
  bool bDisparition = mbNoeudPresent && !bPresent;
  bool bApparition = !mbNoeudPresent && bPresent;
  mbNoeudPresent = bPresent;
  EtatSequence etatAvant = mEtatSequence;

  switch (mEtatSequence) {
    case SEQ_REPOS:
      // Coupure du relais différée après un OFF (voir setEtatReelOnOff())
      // Une absence du noeud (trou MQTT, relais coupé par sa télécommande...) ne l'annule pas :
      // le code n'est envoyé que noeud présent (il bascule : noeud absent, il rallumerait le relais).
      // Au retour du noeud, le délai repart de zéro (un C3 qui redémarre publie OnOff 1 après son ALIVE).
      if (mbCoupureDiffereeArmee) {
        if (bApparition) {
          mulDebutCoupureDifferee = millis();
          message(nomEquipement + ": noeud revenu, coupure ds " + String(mulDelaiCoupureApresOff) + "min");
        }
        if (bPresent && millis() - mulDebutCoupureDifferee >= mulDelaiCoupureApresOff * 60000UL) {
          mbCoupureDiffereeArmee = false;
          if (!etat) {
            if (mbOffConfirme) {
              message(nomEquipement + ": OFF depuis " + String(mulDelaiCoupureApresOff) + "min, coupure prise");
              muiEnvoisCode = 0;
              coupeRelais();
            }
            else {
              // OFF non confirmé (bip hors séquence, peut-être un parasite) : on ne coupe pas
              // le relais avant d'avoir éteint le chauffage par IR, avec vérification par le bip
              message(nomEquipement + ": verif OFF avant coupure");
              lanceArret(true);
            }
          }
        }
      }
      break;

    case ARRET_ATTENTE_ETAT0:
      // Le code relais n'a pas encore été envoyé : une disparition du noeud n'est pas une
      // coupure du relais, mais un C3 planté ou déconnecté (chauffage peut-être encore allumé).
      // On continue d'attendre OnOff 0, jusqu'à la coupure forcée.
      if (!bPresent) {
        if (!mbNoeudPerdu) {
          mbNoeudPerdu = true;
          message(nomEquipement + ": noeud perdu, attente OnOff 0");
        }
      }
      else if (mbNoeudPerdu) {
        // Retour du noeud : la commande OFF a pu être perdue, on la renvoie
        mbNoeudPerdu = false;
        if (muiRenvoisOff < muiNbEnvoisCode) {
          muiRenvoisOff++;
          message(nomEquipement + ": noeud revenu, renvoi OFF");
          publieCommandeNoeud("OFF");
        }
      }
      if (millis() - mulDebutEtape >= mulDelaiCoupureForcee * 60000UL) {
        message(nomEquipement + ": pas de OnOff 0, coupure forcee");
        muiEnvoisCode = 0;
        coupeRelais();
      }
      break;

    case ARRET_ATTENTE_DISPARITION:
      if (!noeudPresent()) {
        message(nomEquipement + ": relais coupe");
        mEtatSequence = SEQ_REPOS;
      }
      else if (millis() - mulDebutEtape >= mulDelaiVerification * 1000UL) {
        if (muiEnvoisCode < muiNbEnvoisCode) {
          message(nomEquipement + ": renvoi code " + String(muiEnvoisCode + 1) + "/" + String(muiNbEnvoisCode));
          coupeRelais();
        }
        else {
          message(nomEquipement + ": ECHEC coupure relais");
          mEtatSequence = SEQ_REPOS;
        }
      }
      break;

    case MARCHE_ATTENTE_APPARITION:
      if (noeudPresent()) {
        message(nomEquipement + ": noeud present, attente IR");
        mEtatSequence = MARCHE_ATTENTE_ETAT1;
        mulDebutEtape = millis();
      }
      else if (millis() - mulDebutEtape >= mulDelaiApparition * 1000UL) {
        if (muiEnvoisCode < muiNbEnvoisCode) {
          message(nomEquipement + ": renvoi code " + String(muiEnvoisCode + 1) + "/" + String(muiNbEnvoisCode));
          fermeRelais();
        }
        else {
          message(nomEquipement + ": ECHEC marche, noeud absent");
          mEtatSequence = SEQ_REPOS;
        }
      }
      break;

    case MARCHE_ATTENTE_ETAT1:
      if (!noeudPresent()) {
        message(nomEquipement + ": noeud disparu");
        mEtatSequence = SEQ_REPOS;
      }
      else if (millis() - mulDebutEtape >= mulDelaiConfirmationOn * 1000UL) {
        message(nomEquipement + ": pas de confirmation ON");
        mEtatSequence = SEQ_REPOS;
        armeCoupureNonConfirmee();
      }
      break;
  }

  // Pendant l'attente de l'arrêt, le noeud perdu n'est pas un relais coupé : l'état affiché reste
  if (bDisparition && etatAvant != ARRET_ATTENTE_ETAT0) {
    if (etatAvant == SEQ_REPOS && etat)
      message(nomEquipement + ": noeud absent, chauffage inconnu");
    appliqueEtat(false);
  }
}

void CNoeudChauffageIRSound::coupeRelais() {
  muiEnvoisCode++;
  DBG(DBG_NOEUD, "[%s] %s : envoi du code relais %u/%u\n", mDateTime.getTime().c_str(), nomEquipement.c_str(), muiEnvoisCode, muiNbEnvoisCode);
  toggleDevice();
  mulDebutEtape = millis();
  mEtatSequence = ARRET_ATTENTE_DISPARITION;
}

void CNoeudChauffageIRSound::fermeRelais() {
  muiEnvoisCode++;
  DBG(DBG_NOEUD, "[%s] %s : envoi du code relais %u/%u (marche)\n", mDateTime.getTime().c_str(), nomEquipement.c_str(), muiEnvoisCode, muiNbEnvoisCode);
  toggleDevice();
  mulDebutEtape = millis();
  mEtatSequence = MARCHE_ATTENTE_APPARITION;
}

// Appelée à réception de "<noeud> IRKO"
//  - arrêt : le relais n'est pas coupé, on attend la coupure forcée (voir demandeOff())
//  - marche : le relais reste fermé (un nouvel ON rallume par IR sans redémarrer le C3),
//    fin de la séquence, coupure non confirmée programmée (voir armeCoupureNonConfirmee())
void CNoeudChauffageIRSound::signaleIrko() {
  if (mEtatSequence == MARCHE_ATTENTE_APPARITION || mEtatSequence == MARCHE_ATTENTE_ETAT1) {
    message(nomEquipement + ": echec IR, relais reste ferme");
    mEtatSequence = SEQ_REPOS;
    armeCoupureNonConfirmee();
  }
  else if (mEtatSequence == ARRET_ATTENTE_ETAT0) {
    unsigned long resteMin = (mulDelaiCoupureForcee * 60000UL - (millis() - mulDebutEtape)) / 60000UL;
    message(nomEquipement + ": echec IR, coupure ds " + String(resteMin) + "min");
  }
  else
    message(nomEquipement + ": echec IR");
}

// Etape de la séquence en cours et temps restant, pour la barre d'état
String CNoeudChauffageIRSound::etapeSequence() const {
  unsigned long ecoule = millis() - mulDebutEtape;
  switch (mEtatSequence) {
    case ARRET_ATTENTE_ETAT0: {
      if (mbNoeudPerdu) {
        unsigned long delai = mulDelaiCoupureForcee * 60000UL;
        unsigned long resteMin = ecoule < delai ? (delai - ecoule + 59999UL) / 60000UL : 0;
        return "noeud perdu, coupure ds " + String(resteMin) + "min";
      }
      return "arret IR en cours";
    }
    case ARRET_ATTENTE_DISPARITION: {
      unsigned long delai = mulDelaiVerification * 1000UL;
      unsigned long resteS = ecoule < delai ? (delai - ecoule + 999UL) / 1000UL : 0;
      return "coupure prise, verif " + String(resteS) + "s";
    }
    case MARCHE_ATTENTE_APPARITION: {
      unsigned long delai = mulDelaiApparition * 1000UL;
      unsigned long resteS = ecoule < delai ? (delai - ecoule + 999UL) / 1000UL : 0;
      return "demarrage noeud, " + String(resteS) + "s";
    }
    case MARCHE_ATTENTE_ETAT1: {
      return "allumage IR en cours";
    }
    default:
      return "sequence en cours";
  }
}

// Marche non confirmée (IRKO, pas de OnOff 1) : état du chauffage inconnu, le C3 reste alimenté.
// Sans nouvel ON, coupure dans mulDelaiCoupureApresOff min, précédée d'un arrêt par IR vérifié
// (comme pour un OFF spontané, voir loop()) : le C3 ne reste pas alimenté indéfiniment.
void CNoeudChauffageIRSound::armeCoupureNonConfirmee() {
  if (etat || mbCoupureDiffereeArmee) return;
  mbCoupureDiffereeArmee = true;
  mbOffConfirme = false;
  mulDebutCoupureDifferee = millis();
  DBG(DBG_NOEUD, "[%s] %s : coupure non confirmee programmee dans %lu min\n", mDateTime.getTime().c_str(), nomEquipement.c_str(), mulDelaiCoupureApresOff);
}

bool CNoeudChauffageIRSound::noeudPresent() const {
  return mNoeud != nullptr && mNoeud->estPresent();
}

void CNoeudChauffageIRSound::message(const String& s) {
  DBG(DBG_NOEUD, "[%s] %s\n", mDateTime.getTime().c_str(), s.c_str());
  if (mEcran != nullptr)
    mEcran->updateStatus(s);
}

//----------------------------------------------------------------------------------
// Commandes MQTT : ON/OFF (CYD auxiliaires, Home Assistant) passent par envoiOnOff()
//----------------------------------------------------------------------------------
void CNoeudChauffageIRSound::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
  if (cmd == "ON" || cmd == "OFF") {
    envoiOnOff();
    return;
  }
  CRCDevice::handleMqttCommand(payload);
}

void CNoeudChauffageIRSound::setup(const String pref) {
  CRCDevice::setup(pref);
  loadFromNVS();
  // L'état mémorisé en NVS n'est pas fiable au démarrage du CYD (nœud absent, ou présent sans
  // avoir encore publié OnOff) : OFF jusqu'au retour du nœud (voir loop())
  saveState(false);
}

void CNoeudChauffageIRSound::loadFromNVS() {
  CRCDevice::loadFromNVS();
  prefs.begin(nvs_namespace, true);
  mulDelaiCoupureForcee = prefs.getULong((mPrefixNVS+"offforc").c_str(), 20);
  mulDelaiCoupureApresOff = prefs.getULong((mPrefixNVS+"offdiff").c_str(), 15);
  mulDelaiVerification = prefs.getULong((mPrefixNVS+"offverif").c_str(), 90);
  mulDelaiApparition = prefs.getULong((mPrefixNVS+"onappar").c_str(), 90);
  mulDelaiConfirmationOn = prefs.getULong((mPrefixNVS+"onconf").c_str(), 60);
  muiNbEnvoisCode = prefs.getUInt((mPrefixNVS+"offnbcod").c_str(), 3);
  prefs.end();
}

void CNoeudChauffageIRSound::saveToNVS() {
  CRCDevice::saveToNVS();
  prefs.begin(nvs_namespace, false);
  prefs.putULong((mPrefixNVS+"offforc").c_str(), mulDelaiCoupureForcee);
  prefs.putULong((mPrefixNVS+"offdiff").c_str(), mulDelaiCoupureApresOff);
  prefs.putULong((mPrefixNVS+"offverif").c_str(), mulDelaiVerification);
  prefs.putULong((mPrefixNVS+"onappar").c_str(), mulDelaiApparition);
  prefs.putULong((mPrefixNVS+"onconf").c_str(), mulDelaiConfirmationOn);
  prefs.putUInt((mPrefixNVS+"offnbcod").c_str(), muiNbEnvoisCode);
  prefs.end();
}

// Bornes des paramètres (voir verifieParametres())
static const unsigned long DELAI_MAX_MIN = 1440;          // min : 24 h (et pas de débordement de x 60000UL)
static const unsigned long DELAI_MAX_S = 3600;            // s : 1 h
static const unsigned long MARGE_WATCHDOG_S = 30;         // s : marge au-delà du watchdog du noeud
static const unsigned long DELAI_MIN_CONFIRMATION_S = 30; // s : > séquence IR du noeud (3 x 3 s) + connexion MQTT
static const unsigned int NB_ENVOIS_CODE_MAX = 10;

// Valeur saisie ignorée si négative ou non numérique (toInt() renvoie 0 : rejeté sauf "0" explicite)
static void lireArgPositif(WebServer& server, const String& nom, unsigned long& val) {
  if (!server.hasArg(nom.c_str())) return;
  String s = server.arg(nom);
  s.trim();
  long v = s.toInt();
  if (v < 0 || (v == 0 && s != "0")) {
    DBG(DBG_NOEUD, "%s : valeur \"%s\" ignorée\n", nom.c_str(), s.c_str());
    return;
  }
  val = (unsigned long)v;
}

void CNoeudChauffageIRSound::loadFromWebServer (WebServer& server) {
  CRCDevice::loadFromWebServer(server);
  lireArgPositif(server, mPrefixNVS+"offforc", mulDelaiCoupureForcee);
  lireArgPositif(server, mPrefixNVS+"offdiff", mulDelaiCoupureApresOff);
  lireArgPositif(server, mPrefixNVS+"offverif", mulDelaiVerification);
  lireArgPositif(server, mPrefixNVS+"onappar", mulDelaiApparition);
  lireArgPositif(server, mPrefixNVS+"onconf", mulDelaiConfirmationOn);
  unsigned long ulNbEnvoisCode = muiNbEnvoisCode;
  lireArgPositif(server, mPrefixNVS+"offnbcod", ulNbEnvoisCode);
  muiNbEnvoisCode = ulNbEnvoisCode;
  // Les bornes dépendant du watchdog du noeud, chargé après : voir verifieParametres() dans MyWebServer::handleSave()
}

//----------------------------------------------------------------------------------
// CNoeudChauffageIRSound::verifieParametres()
//
// Ramène les délais dans leurs bornes. Le relais n'a qu'un code, qui bascule : un code
// renvoyé alors que le précédent a été reçu l'annule.
//  - mulDelaiVerification > watchdog du noeud : la coupure n'est constatée qu'à l'expiration
//    du watchdog ; renvoyer avant rallumerait le relais, et le C3 rallumerait le chauffage.
//  - mulDelaiApparition > watchdog du noeud : au démarrage, le premier message (OnOff/IRKO,
//    sinon ALIVE, dont la période est inférieure au watchdog) doit arriver avant le renvoi,
//    qui couperait le noeud.
//  - mulDelaiCoupureForcee >= 1 min : 0 couperait le noeud avant l'extinction par IR.
// Appelée une fois mNoeud et son watchdog chargés (setup_chauffageSb(), handleSave()).
//----------------------------------------------------------------------------------
void CNoeudChauffageIRSound::verifieParametres() {
  auto borne = [this](const char* nom, unsigned long& val, unsigned long vmin, unsigned long vmax) {
    unsigned long v = constrain(val, vmin, vmax);
    if (v == val) return;
    message(nomEquipement + ": " + nom + " " + String(val) + " -> " + String(v));
    val = v;
  };
  unsigned long minWdog = (mNoeud != nullptr) ? mNoeud->getWatchdogIntervalle() + MARGE_WATCHDOG_S : 0;
  borne("coupure forcee", mulDelaiCoupureForcee, 1, DELAI_MAX_MIN);
  borne("coupure apres OFF", mulDelaiCoupureApresOff, 0, DELAI_MAX_MIN);
  borne("verif coupure", mulDelaiVerification, minWdog, max(minWdog, DELAI_MAX_S));
  borne("attente apparition", mulDelaiApparition, minWdog, max(minWdog, DELAI_MAX_S));
  borne("confirmation ON", mulDelaiConfirmationOn, DELAI_MIN_CONFIRMATION_S, DELAI_MAX_S);
  unsigned long ulNbEnvoisCode = muiNbEnvoisCode;
  borne("nb envois code", ulNbEnvoisCode, 1, NB_ENVOIS_CODE_MAX);
  muiNbEnvoisCode = ulNbEnvoisCode;
}

String CNoeudChauffageIRSound::getHTML() {
  String html = CRCDevice::getHTML();
  // Insertion avant le "</div>" final de CRCDevice::getHTML()
  int pos = html.lastIndexOf("</div>");
  String fin = (pos >= 0) ? html.substring(pos) : "";
  if (pos >= 0) html = html.substring(0, pos);
  // Minimum des délais de renvoi du code : watchdog du noeud + marge (voir verifieParametres())
  String sMinWdog = (mNoeud != nullptr) ? String(mNoeud->getWatchdogIntervalle() + MARGE_WATCHDOG_S) : "?";
  html += "<div class=\"row\">"
            "<div><label>Coupure du relais après OFF (min, 0-" + String(DELAI_MAX_MIN) + ", rallumage rapide par IR d'ici là)</label><input type=\"number\" min=\"0\" name=" + (mPrefixNVS+"offdiff") + " value=\"" + String(mulDelaiCoupureApresOff) + "\"></div>"
            "<div><label>Coupure forcée si pas d'arrêt confirmé (min, 1-" + String(DELAI_MAX_MIN) + ")</label><input type=\"number\" min=\"1\" name=" + (mPrefixNVS+"offforc") + " value=\"" + String(mulDelaiCoupureForcee) + "\"></div>"
            "<div><label>Renvoi du code si le noeud répond encore après (s, &ge; watchdog noeud + " + String(MARGE_WATCHDOG_S) + " = " + sMinWdog + " s)</label><input type=\"number\" min=\"0\" name=" + (mPrefixNVS+"offverif") + " value=\"" + String(mulDelaiVerification) + "\"></div>"
          "</div>"
          "<div class=\"row\">"
            "<div><label>Renvoi du code si le noeud n'apparaît pas après (s, &ge; watchdog noeud + " + String(MARGE_WATCHDOG_S) + " = " + sMinWdog + " s)</label><input type=\"number\" min=\"0\" name=" + (mPrefixNVS+"onappar") + " value=\"" + String(mulDelaiApparition) + "\"></div>"
            "<div><label>Attente confirmation ON après apparition du noeud (s, " + String(DELAI_MIN_CONFIRMATION_S) + "-" + String(DELAI_MAX_S) + ")</label><input type=\"number\" min=\"" + String(DELAI_MIN_CONFIRMATION_S) + "\" name=" + (mPrefixNVS+"onconf") + " value=\"" + String(mulDelaiConfirmationOn) + "\"></div>"
            "<div><label>Nb max d'envois du code relais (1-" + String(NB_ENVOIS_CODE_MAX) + ")</label><input type=\"number\" min=\"1\" max=\"" + String(NB_ENVOIS_CODE_MAX) + "\" name=" + (mPrefixNVS+"offnbcod") + " value=\"" + String(muiNbEnvoisCode) + "\"></div>"
          "</div>";
  html += fin;
  return html;
}


/*void CNoeudChauffageIRSound::setup() {
  // On charge les infos de config depuis le NVS
  loadFromNVS();

}
*/
/*int CNoeudChauffageIRSound::activeEquipement() {
  int ret = 0;
  //setActive(!active);
  //ret = onMqttPublish(mqttSubTopicCommand.c_str(), active ? "ENABLER" : "DISABLER");

  return ret;
}*/

/*void CNoeudChauffageIRSound::setMqttPublishCallback(std::function<int(const char*, const char*)> cb) {
        onMqttPublish = cb;
    }*/
/*void CNoeudChauffageIRSound::loop() {
  //loop_envoi_trames();        
}*/

/*void CNoeudChauffageIRSound::loadFromNVS() {
  prefs.begin(nvs_namespace, true);

  domotique_prefix = prefs.getString("domo_prefix", default_domotique_topic_prefix);
  // Chaudiere
  nomEquipement = prefs.getString("chaud_nom", "XXX");
  //nomBoutonON = prefs.getString("chaud_nameBtnON", "Ch. ON");
  //nomBoutonOFF = prefs.getString("chaud_nameBtnOF", "Ch. OFF");
  frequency = prefs.getFloat("chaud_freq", 433.92);
  active = prefs.getBool("chaud_active", false);

  mqttSubTopic = prefs.getString("topic_prefix", default_topic_prefix);

  // On forme les subtopic MQTT pour la chaudière
  mqttSubTopicCommand = domotique_prefix + mqttSubTopic + "command";
  mqttSubTopicState   = domotique_prefix + mqttSubTopic + "state";
  mqttSubTopicStatus = domotique_prefix + mqttSubTopic + "status"; //mqqtInfo.subtopic_status;

  // État chaudière
  etat = prefs.getBool("boiler_state", default_boiler_state);
  etatStr = etat ? "ON" : "OFF";


  prefs.end();
}*/

/*void CNoeudChauffageIRSound::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  prefs.putString("chaud_nom", nomEquipement);
  prefs.putString("chaud_nameBtnON", nomBoutonON);
  prefs.putString("chaud_nameBtnOF", nomBoutonOFF);
  prefs.putFloat("chaud_freq", frequency);
  prefs.putBool("chaud_active", active);
  prefs.putBool("boiler_state", etat);

  prefs.putString("topic_prefix", mqttSubTopic);//topic_prefix);
  // Keep alive flag
  prefs.putBool("en_keep_alive", enableKeepAlive);

  

  prefs.end();
}*/

/*void CNoeudChauffageIRSound::print() const {
  DBG(DBG_CHAUDIERE, "[CHAUDIERE]\n");
  DBG(DBG_CHAUDIERE, "  [GENERAL]\n");
  DBG(DBG_CHAUDIERE, "     Nom            : %s\n", nomEquipement.c_str());
  DBG(DBG_CHAUDIERE, "     Actif          : %s\n", active ? "OUI" : "NON");
  DBG(DBG_CHAUDIERE, "     Nom btn ON     : %s\n", nomBoutonON.c_str());
  DBG(DBG_CHAUDIERE, "     Nom btn OFF    : %s\n", nomBoutonOFF.c_str());

  DBG(DBG_CHAUDIERE, "  [Protocole]\n");
  DBG(DBG_CHAUDIERE, "     Fréquence      : %f\n", frequency);

  DBG(DBG_CHAUDIERE, "  [MQTT]==\n");
  DBG(DBG_CHAUDIERE, "     MQTTSubTopic   : %s\n", mqttSubTopic.c_str());
  DBG(DBG_CHAUDIERE, "     Topic prefix   : %s\n", mqttSubTopic.c_str());//topic_prefix.c_str());
  DBG(DBG_CHAUDIERE, "     Command        : %s\n", mqttSubTopicCommand.c_str());
  DBG(DBG_CHAUDIERE, "     State          : %s\n", mqttSubTopicState.c_str());
  DBG(DBG_CHAUDIERE, "     Status         : %s\n", mqttSubTopicStatus.c_str());

  DBG(DBG_CHAUDIERE, "  [État chaudière]\n");
  DBG(DBG_CHAUDIERE, "     Dernier état   : %s\n", etatStr.c_str());



}*/

/*void CNoeudChauffageIRSound::saveState(bool state) {
  if (etat == state) return;

  prefs.begin(nvs_namespace, false);
  prefs.putBool("boiler_state", state);
  prefs.end();

  etat = state;
  etatStr = state ? "ON" : "OFF";

  prefs.end();
}*/
/*void CNoeudChauffageIRSound::setActive(bool state) {
  if (active == state) return;

  prefs.begin(nvs_namespace, false);
  prefs.putBool("chaud_active", state);
  prefs.end();

  active = state;
}*/

// Envoie l'état actif/inactif ainsi que ON/OFF
/*int CNoeudChauffageIRSound::remonteStatusParMqtt() {
  int ret = 0;
  DBG(DBG_CHAUDIERE, "CNoeudChauffageIRSound::remonteStatusParMqtt() - Envoi %s sur %s\n", this->active ? "ENABLER" : "DISABLER", this->mqttSubTopicCommand.c_str());
  onMqttPublish(this->mqttSubTopicCommand.c_str(), this->active ? "ENABLER" : "DISABLER");
  delay(200);
  DBG(DBG_CHAUDIERE, "CNoeudChauffageIRSound::remonteStatusParMqtt() - Envoi %s sur %s\n", this->etat ? "ONR" : "OFFR", this->mqttSubTopicCommand.c_str());
  onMqttPublish(this->mqttSubTopicCommand.c_str(), this->etat ? "ONR" : "OFFR");
  delay(200);
  return ret;
} */

//
// Etat réel de la chaudière
// Vient de l'observatoion de la DEL rouge
//  Eteint : 0
//  Allulmée : 1
//
/*void CNoeudChauffageIRSound::setEtatReelOnOff(bool state) {
  // On prévient les appareils distants
  //Serial.printf("void CNoeudChauffageIRSound::setEtatReelOnOff() - state=%d - etat=%d\n", state, etat);
  if (etat == state) return;
  String sEnvoi = state ? "ONR" : "OFFR";
  if (onMqttPublish != nullptr)
    onMqttPublish(this->mqttSubTopicCommand.c_str(), sEnvoi.c_str());

  saveState(state); // Sauvegarde NVS
  if (mEcran != nullptr) 
    mEcran->updateAllStates();

  // Si on a reçu OFF (state = false), et si bForcerDisable est à true, alors on remet l'équipement dans un état inactif (forcé)
  if (!state && mbForcerDisable) {
    setActive(false);
    if (onMqttPublish != nullptr)
      onMqttPublish(this->mqttSubTopicCommand.c_str(), "DISABLER");
    String s = nomEquipement + " : désactivee (force OFF)";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    DBGLN(DBG_CHAUDIERE, s);
    mbForcerDisable = false; // Réinitialisation du flag après utilisation
  }

}*/

/*void CNoeudChauffageIRSound::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
        
  // ON_FORCE et OFF_FORCE permettent de forcer ON ou OFF même si la chaudière est désactivée.
  if (cmd == "ON" || cmd == "OFF" || cmd == "ON_FORCE" || cmd == "OFF_FORCE") {
    bool state = (cmd == "ON" || cmd == "ON_FORCE") ? true : false; // 
    bool bForceEnable = (cmd == "ON_FORCE") ? true : false;
    bool bForceDisable = (cmd == "OFF_FORCE") ? true : false;
    bool mbOldActif = active; // Sauvegarde de l'état actif avant changement
    // Si c'est une commande ON ou OFF forcée, on active l'équipement même s'il est désactivé
    if (bForceEnable || bForceDisable) {
      setActive(true);
      if (onMqttPublish != nullptr)
        onMqttPublish(this->mqttSubTopicCommand.c_str(), "ENABLER");
      String s = nomEquipement + " : active (force ON)";
      if (mEcran != nullptr) mEcran->updateStatus(s, true);
      DBGLN(DBG_CHAUDIERE, s);
    }
    if (active) {    
      saveState(state); // Ajout 2.0
      if (state) { // On doit allumer la chaudière

        bSetEnvoyerTramesOFF(false);
        bSetEnvoyerTramesON(true);
        if (onMqttPublish != nullptr)
          onMqttPublish(this->mqttSubTopicCommand.c_str(), "ONR");
        //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      }
      else { // On doit éteindre la chaudière
        bSetEnvoyerTramesON(false);
        bSetEnvoyerTramesOFF(true);
        if (onMqttPublish != nullptr)
          onMqttPublish(this->mqttSubTopicCommand.c_str(), "OFFR");
        //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      }
      saveState(state);
      if (mEcran != nullptr) mEcran->updateAllStates();
      // Si l'équipement était inactif avant un OFF forcé, il faudra le remettre dans l'atat inactif au retour du résultat du OFF
      if (bForceDisable) {
        mbForcerDisable=true;
      }
    } // if (active) {
    else {
      String s = "L'equipement " + String(nomEquipement) + " n'est pas actif";
      if (mEcran != nullptr) mEcran->updateStatus(s);
      DBGLN(DBG_CHAUDIERE, s);
    }

  }
  else if (cmd == "ENABLE") {
    setActive(true);
    if (onMqttPublish != nullptr)
      onMqttPublish(this->mqttSubTopicCommand.c_str(), "ENABLER");
    String s = nomEquipement + " : active";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    DBGLN(DBG_CHAUDIERE, s);
  }
  else if (cmd == "DISABLE") {
    setActive(false);
    if (onMqttPublish != nullptr)
      onMqttPublish(this->mqttSubTopicCommand.c_str(), "DISABLER");
    String s = nomEquipement + " : désactivee";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    DBGLN(DBG_CHAUDIERE, s);
  }
}
*/
/*void CNoeudChauffageIRSound::handleMqttCommand(const String& payload) {
  String cmd = payload;
  cmd.toUpperCase();
  cmd.trim();
        
  if (cmd == "ON" || cmd == "OFF") {
    bool state = cmd == "ON" ? true : false;
    if (active) {    
      saveState(state); // Ajout 2.0
      if (state) {

        bSetEnvoyerTramesOFF(false);
        bSetEnvoyerTramesON(true);
        if (onMqttPublish != nullptr)
          onMqttPublish(this->mqttSubTopicCommand.c_str(), "ONR");
        //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      }
      else {
        bSetEnvoyerTramesON(false);
        bSetEnvoyerTramesOFF(true);
        if (onMqttPublish != nullptr)
          onMqttPublish(this->mqttSubTopicCommand.c_str(), "OFFR");
        //if (mEcran != nullptr) mEcran->updateBoilerStatus();
      }
      saveState(state);
      if (mEcran != nullptr) mEcran->updateAllStates();
    }
    else {
      String s = "L'equipement " + String(nomEquipement) + " n'est pas actif";
      if (mEcran != nullptr) mEcran->updateStatus(s);
      Serial.println(s);
    }

  }
  else if (cmd == "ENABLE") {
    setActive(true);
    onMqttPublish(this->mqttSubTopicCommand.c_str(), "ENABLER");
    String s = nomEquipement + " : active";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    Serial.println(s);
  }
  else if (cmd == "DISABLE") {
    setActive(false);
    onMqttPublish(this->mqttSubTopicCommand.c_str(), "DISABLER");
    String s = nomEquipement + " : désactivee";
    if (mEcran != nullptr) mEcran->updateStatus(s, true);
    Serial.println(s);
  }
}*/
   

/*void CNoeudChauffageIRSound::loadFromWebServer (WebServer& server) {
  if (server.hasArg("chaud_nom")) nomEquipement = server.arg("chaud_nom");
  if (server.hasArg("chaud_active")) active = true; else active = false;
  if (server.hasArg("en_keep_alive")) enableKeepAlive = true; else enableKeepAlive = false;//server.arg("en_keep_alive").toInt();
  if (server.hasArg("chaud_nameBtnON")) nomBoutonON = server.arg("chaud_nameBtnON");
  if (server.hasArg("chaud_nameBtnOF")) nomBoutonOFF = server.arg("chaud_nameBtnOF");
  if (server.hasArg("chaud_freq")) frequency = server.arg("chaud_freq").toFloat();

  // Lecture des 10 timings ON
  for (uint8_t i = 0; i < 10; i++) {
    String arg_name = "on_keep_" + String(i);
    if (server.hasArg(arg_name)) {
      onKeepalive[i] = server.arg(arg_name).toInt();
    }
  }

  // Lecture des 10 timings OFF
  for (uint8_t i = 0; i < 10; i++) {
    String arg_name = "off_keep_" + String(i);
    if (server.hasArg(arg_name)) {
      offKeepalive[i] = server.arg(arg_name).toInt();
    }
  }
}*/

/*String CNoeudChauffageIRSound::getHTML() {
  String html = "";
  html = "<h2>Configuration de la chaudière</h2>"
      "<div class=\"row\">"
        "<div><label>Nom</label><input type=\"text\" name=\"chaud_nom\" value=\"" + nomEquipement + "\"></div>"
        "<div><label>Topic prefix MQTT</label><input type=\"text\" name=\"topic_prefix\" value=\"" + mqttSubTopic + "\"></div>"
        "<div class=\"checkbox-row\"><label>Actif</label><input type=\"checkbox\" name=\"chaud_active\" value=\"1\"" + String(active ? " checked" : "") + "></div>"
      "</div>"
      "<div class=\"row\">"
        "<div><label>Nom bouton ON</label><input type=\"text\" name=\"chaud_nameBtnON\" value=\"" + nomBoutonON + "\"></div>"
        "<div><label>Nom bouton OFF</label><input type=\"text\" name=\"chaud_nameBtnOF\" value=\"" + nomBoutonOFF + "\"></div>"
      "</div>"
      "<label>Fréquence (MHz)</label><input type=\"number\" step=\"0.01\" name=\"chaud_freq\" value=\"" + String(frequency, 2) + "\">"

      // === KEEP-ALIVE ===
      "<h2>Keep-Alive Chaudière</h2>"
      "<div class=\"checkbox-row\"><label>Activer le keep-alive</label><input type=\"checkbox\" name=\"en_keep_alive\" value=\"1\"" + String(enableKeepAlive ? " checked" : "") + "></div>"
      "<h3>Timings ON (secondes)</h3><div class=\"timing-grid\">";
  for (uint8_t i = 0; i < 10; i++) {
    html += "<input type=\"number\" name=\"on_keep_" + String(i) + "\" value=\"" + String(onKeepalive[i]) + "\">";
  }
  html += "</div><h3>Timings OFF (secondes)</h3><div class=\"timing-grid\">";
  for (uint8_t i = 0; i < 10; i++) {
    html += "<input type=\"number\" name=\"off_keep_" + String(i) + "\" value=\"" + String(offKeepalive[i]) + "\">";
  }
  html += "</div>";

  return html;
}*/


