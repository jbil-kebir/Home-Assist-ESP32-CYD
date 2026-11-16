#ifndef __REMOTE_CAPTEUR_BASE_H__
#define __REMOTE_CAPTEUR_BASE_H__

#include <functional>
#include <vector>
#include <WebServer.h>
#include <Preferences.h>
#include "global.h"
#include "EquipementBase.h"

//------------------------------------------------------------------------------
// Superclasse des capteurs distants (Home-Assist 3.9) : pour l'instant, seulement le chien de garde.
//
// Chien de garde : le capteur est signalé quand rien n'a été reçu de lui pendant getWatchdogIntervalle() secondes :
//   - délai d'alerte ANNONCÉ par le nœud (Capteur 3.9 : « alerte:<s> » en fin de chaque trame de mesure), calculé
//     par le nœud d'après son sommeil, son forçage et le retard toléré réglé sur sa page Web ; le dernier reçu est
//     gardé en NVS (<préfixe>wdann, écrit seulement s'il change) pour être juste dès le redémarrage du CYD ;
//   - sinon (ancien firmware, actionneur, état HA...) : délai SAISI sur la page du CYD (<préfixe>wdog), en secours.
//     À supprimer quand tous les capteurs seront passés en 3.9.
// Passage en retard et retour : onAlerte(nom, enRetard), défini par le CYD principal seul (publication MQTT retenue
// sur home/alerte/<nom>, voir main.cpp), et SEULEMENT pour un capteur qui a annoncé son délai : ni les anciens
// firmwares, ni le nœud chauffage (sa disparition, relais coupé, est normale), ni les états HA (NewNas, BigNas).
// Tous les capteurs sont vérifiés par verifieTous() (main.cpp) : le loop() de beaucoup d'entre eux n'est pas appelé
// (batteries, flotteurs, couleur...).
//------------------------------------------------------------------------------
class CRemoteCapteurBase : public CEquipementBase {
public:
  // Passage en retard (true) ou retour (false) d'un capteur ; silenceS = secondes écoulées depuis son dernier message
  // (ou depuis le démarrage du CYD s'il n'a jamais rien reçu), 0 au retour. Défini par le contrôleur principal seulement.
  static std::function<void(const String& nom, bool enRetard, unsigned long silenceS)> onAlerte;
  // Passage en retard ou retour de n'importe quel capteur (tous les CYD) : redessin de l'écran (WDG, bandeau orange)
  static std::function<void()> onChangementRetard;
  // Vérifie le chien de garde de tous les capteurs actifs (passage en retard). À appeler par loop() du programme.
  static void verifieTous();
  // Copies des dernières valeurs republiées par le CYD principal pour un auxiliaire qui démarre (« DEVICE CYD ») :
  // publiées sur « …/copie » au lieu de « …/state » (home/thermometre/copie, home/noeud/copie), écoutées par les
  // auxiliaires seuls. Sur le topic d'état, le principal (bandeau d'éveil, chien de garde) et HA (Éveillé, Dernière
  // trame) les prenaient pour des trames des nœuds (16/11/2026).
  static String topicCopie(const String& topicState);
  // true pendant le traitement d'une copie (auxiliaires, CMqtt::callback()) : pas d'effet sur le bandeau d'éveil
  static bool sbTrameCopie;
  // Durée lisible : « 45 s », « 21 min », « 1 h 01 », « 2 j 3 h 05 »
  static String formateDuree(unsigned long s);

  // Délai appliqué (s) : annoncé par le nœud s'il en a annoncé un, sinon saisi
  unsigned long getWatchdogIntervalle() const { return mulDelaiAnnonceS > 0 ? mulDelaiAnnonceS : mulWatchdogIntervalle; }
  unsigned long getDelaiAnnonce() const { return mulDelaiAnnonceS; }
  bool estEnRetard() const { return mbEnRetard; }

protected:
  CRemoteCapteurBase(unsigned long wdogDefautS) :
    mulWatchdogIntervalle(wdogDefautS), mulWatchdogDefaultIntervalle(wdogDefautS) { registre().push_back(this); }
  // Pas de copie (le registre garde l'adresse de chaque capteur)
  CRemoteCapteurBase(const CRemoteCapteurBase&) = delete;
  CRemoteCapteurBase& operator=(const CRemoteCapteurBase&) = delete;

  // En secondes. Si l'appareil ne répond pas au delà de cet intervalle, il est considéré comme KO
  unsigned long mulWatchdogIntervalle;          // Saisi (secours)
  unsigned long mulWatchdogDefaultIntervalle;
  unsigned long mulDelaiAnnonceS = 0;           // Annoncé par le nœud, 0 : jamais
  unsigned long mulWatchDog = 0;                // millis() du dernier message reçu (ou du démarrage)
  bool mbEnRetard = false;                      // Retard signalé (onAlerte), pas encore revenu
  bool mbEtatPublie = false;                    // Un état (RETARD / OK) a été publié depuis le démarrage du CYD : sinon,
                                                // la 1re trame publie OK (HA garde le dernier RETARD retenu avant le redémarrage)

  // Message reçu du capteur : remet le chien de garde à zéro, retient le délai annoncé (« alerte:<s> »)
  // et signale le retour si le capteur était en retard
  void noteReception(const String& payload);
  // Délai dépassé (sans effet de bord)
  bool delaiDepasse() const { return (millis() - mulWatchDog) >= getWatchdogIntervalle() * 1000UL; }
  // À appeler par loop() : délai dépassé ? Signale le passage en retard (une fois)
  bool verifieWatchdog();

  // À appeler par loadFromNVS() / saveToNVS() (prefs déjà ouvert) et loadFromWebServer()
  void chargeWatchdogNVS();
  void enregistreWatchdogNVS();
  void litWatchdogWeb(WebServer& server);
  // Bloc de la page : délai appliqué et sa provenance, champ de secours
  String getHTMLWatchdog() const;

  static unsigned long extraitDelaiAlerte(const String& payload); // 0 si absent

private:
  // Tous les capteurs (objets globaux : variable statique locale, construite au premier appel)
  static std::vector<CRemoteCapteurBase*>& registre();
};

#endif // __REMOTE_CAPTEUR_BASE_H__
