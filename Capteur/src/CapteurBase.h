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
//   intervalle normal + retard toléré + DELAI_ALERTE_MARGE_S
//   - deep sleep actif : le forçage n'est vérifié qu'au réveil. Nombre de réveils pour atteindre le forçage
//     (compté avec le sommeil seul, cas le plus défavorable) × (sommeil + DELAI_ALERTE_EVEIL_S) ;
//   - sans deep sleep : le forçage lui-même.
//   Le retard toléré (NVS <préfixe>retard, page Web, SET <préfixe>retard) exprime l'importance du capteur.
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
  virtual void saveToNVS() = 0;   // Après SET <préfixe>retard (CConfig::handleMqttCommand())

  unsigned long getDelaiAlerteS() const;
  unsigned long getRetardTolereS() const { return mulRetardTolereS; }
  // « <préfixe>force:<s> <préfixe>retard:<s> » pour la trame CONFIG
  String getParamsAlerte() const;
  // SET <préfixe>force <s> (FORCAGE_MIN_S à FORCAGE_MAX_S) ou <préfixe>retard <s> (0 à DELAI_ALERTE_RETARD_MAX_S).
  // Retour 0 : modifié (en RAM ; saveToNVS() à la charge de l'appelant) ; -1 : autre clé ; -2 : valeur invalide
  int setParamAlerte(const String& cle, const String& valeur);

protected:
  CCapteurBase(unsigned long forcageDefautS) :
    mulIntervalleForcageRemonteeMesure(forcageDefautS), mulDefaultIntervalleForcageRemonteeMesure(forcageDefautS) {}

  // En secondes : intervalle de forçage de la remontée de mesure, même si la valeur n'a pas changé
  unsigned long mulIntervalleForcageRemonteeMesure, mulDefaultIntervalleForcageRemonteeMesure;
  unsigned long mulRetardTolereS = DELAI_ALERTE_RETARD_DEFAUT_S;

  // À appeler par loadFromNVS() / saveToNVS() (prefs déjà ouvert) et loadFromWebServer() de chaque classe
  void chargeRetardNVS(Preferences& prefs);
  void enregistreRetardNVS(Preferences& prefs);
  void litRetardWeb(WebServer& server);
  // Ligne de la page Web : champ « Retard toléré » + « → alerte si aucune nouvelle pendant … »
  String getHTMLRetard() const;
  // « alerte:<s> » à ajouter en fin de chaque trame de mesure (précédé d'une espace)
  String suffixeAlerte() const { return " alerte:" + String(getDelaiAlerteS()); }

private:
  static const CConfig* smConfig;
};

#endif // __CAPTEUR_BASE_H__
