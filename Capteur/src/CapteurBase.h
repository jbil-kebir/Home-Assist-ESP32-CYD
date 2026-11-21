#ifndef __CAPTEUR_BASE_H__
#define __CAPTEUR_BASE_H__

#include "global.h"
#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>

class CConfig;

//------------------------------------------------------------------------------
// Superclasse des capteurs (Capteur 3.9) : pour l'instant, seulement ce qui concerne le délai d'alerte.
//
// Délai d'alerte d'un capteur = durée sans nouvelles au-delà de laquelle le contrôleur (CYD) le signale.
// Calculé ici, et nulle part ailleurs (getDelaiAlerteS()) :
//   délai saisi (NVS / page Web / SET <préfixe>alerte, en MINUTES, Capteur 4.4), relevé au minimum si plus court ;
//   minimum = intervalle normal + DELAI_ALERTE_MARGE_S (getDelaiMinimumS()) :
//   - deep sleep actif : le forçage n'est vérifié qu'au réveil. Nombre de réveils pour atteindre le forçage
//     (compté avec le sommeil seul, cas le plus défavorable) × (sommeil + DELAI_ALERTE_EVEIL_S) ;
//   - sans deep sleep : le forçage lui-même.
//   0 = « au plus tôt » : le minimum.
// Avant la 4.4 : « retard toléré » (<préfixe>retard, s) ajouté au minimum ; converti au 1er démarrage
// (chargeAlerteNVS()) : retard 0 → 0, sinon (retard + forçage) / 60, arrondi (7200 + 3600 → 180 min).
// Le délai est joint à chaque mesure (suffixeAlerte() : « alerte:<s> », en dernier, avant l'IP ajoutée par
// CMqtt::publish()). Une incohérence avec le rythme du nœud est impossible par construction : rien à refuser.
//
// Le forçage (mulIntervalleForcageRemonteeMesure) est ici ; chaque classe garde sa valeur par défaut (constructeur)
// et sa propre lecture / écriture NVS et page Web du forçage.
//------------------------------------------------------------------------------
class CCapteurBase {
public:
  // Sommeil du nœud (deep sleep actif, durée), lu dans CConfig : à appeler une fois au démarrage
  static void setConfig(const CConfig* config) { smConfig = config; }
  // Durée lisible : « 45 s », « 21 min », « 1 h 01 », « 2 j 3 h 05 »
  static String formateDuree(unsigned long s);

  virtual ~CCapteurBase() {}
  virtual String getPrefixNVS() const = 0;
  virtual void saveToNVS() = 0;   // Après SET <préfixe>alerte (CConfig::handleMqttCommand())

  unsigned long getDelaiAlerteS() const;    // Délai annoncé : saisi, relevé au minimum
  unsigned long getDelaiMinimumS() const;   // Plus court délai possible avec le rythme actuel du nœud
  unsigned long getDelaiAlerteSaisiMin() const { return mulDelaiAlerteMin; }
  // « <préfixe>force:<s> <préfixe>alerte:<min> » pour la trame CONFIG
  String getParamsAlerte() const;
  // SET <préfixe>force <s> (FORCAGE_MIN_S à FORCAGE_MAX_S) ou <préfixe>alerte <min> (0 à DELAI_ALERTE_MAX_MIN).
  // Retour 0 : modifié (en RAM ; saveToNVS() à la charge de l'appelant) ; -1 : autre clé ; -2 : valeur invalide
  int setParamAlerte(const String& cle, const String& valeur);

protected:
  CCapteurBase(unsigned long forcageDefautS) :
    mulIntervalleForcageRemonteeMesure(forcageDefautS), mulDefaultIntervalleForcageRemonteeMesure(forcageDefautS) {}

  // En secondes : intervalle de forçage de la remontée de mesure, même si la valeur n'a pas changé
  unsigned long mulIntervalleForcageRemonteeMesure, mulDefaultIntervalleForcageRemonteeMesure;
  unsigned long mulDelaiAlerteMin = DELAI_ALERTE_DEFAUT_MIN;   // Délai d'alerte saisi, en minutes (0 : au plus tôt)

  // À appeler par loadFromNVS() / saveToNVS() (prefs déjà ouvert) et loadFromWebServer() de chaque classe
  // (chargeAlerteNVS : après la lecture du forçage, qui sert à convertir l'ancien retard toléré)
  void chargeAlerteNVS(Preferences& prefs);
  void enregistreAlerteNVS(Preferences& prefs);   // Efface aussi l'ancienne clé <préfixe>retard
  void litAlerteWeb(WebServer& server);
  // Ligne de la page Web : champ « Délai d'alerte » + « → alerte après … »
  String getHTMLAlerte() const;
  // « alerte:<s> » à ajouter en fin de chaque trame de mesure (précédé d'une espace)
  String suffixeAlerte() const { return " alerte:" + String(getDelaiAlerteS()); }

private:
  static const CConfig* smConfig;
};

#endif // __CAPTEUR_BASE_H__
