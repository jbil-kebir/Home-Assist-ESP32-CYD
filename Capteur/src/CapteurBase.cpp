#include "CapteurBase.h"
#include "MyConfig.h"

const CConfig* CCapteurBase::smConfig = nullptr;

//
// Délai d'alerte (s) : voir CapteurBase.h
//
unsigned long CCapteurBase::getDelaiAlerteS() const {
  unsigned long forcage = mulIntervalleForcageRemonteeMesure > 0 ? mulIntervalleForcageRemonteeMesure : 1UL;
  unsigned long intervalle = forcage;
  if (smConfig != nullptr && smConfig->mbDeepSleepActive && smConfig->getSommeilEnCours() > 0) {
    unsigned long sommeil = smConfig->getSommeilEnCours(); // Durée en cours (temporaire SLEEPS/M/L ou sleepI)
    // Nombre de réveils compté avec le sommeil seul (au moins un) : si l'éveil réel est plus court que
    // DELAI_ALERTE_EVEIL_S, il peut falloir un réveil de plus qu'en comptant le cycle complet
    unsigned long nbReveils = (forcage + sommeil - 1) / sommeil;
    if (nbReveils == 0) nbReveils = 1;
    intervalle = nbReveils * (sommeil + DELAI_ALERTE_EVEIL_S);
  }
  return intervalle + mulRetardTolereS + DELAI_ALERTE_MARGE_S;
}

String CCapteurBase::formateDuree(unsigned long s) {
  if (s < 60) return String(s) + " s";
  unsigned long m = (s + 30) / 60;  // Arrondi à la minute
  if (m < 60) return String(m) + " min";
  unsigned long h = m / 60; m %= 60;
  String sMin = (m < 10 ? "0" : "") + String(m);
  if (h < 24) return String(h) + " h " + sMin;
  return String(h / 24) + " j " + String(h % 24) + " h " + sMin;
}

String CCapteurBase::getParamsAlerte() const {
  return getPrefixNVS() + "force:" + String(mulIntervalleForcageRemonteeMesure) +
         " " + getPrefixNVS() + "retard:" + String(mulRetardTolereS);
}

int CCapteurBase::setParamAlerte(const String& cle, const String& valeur) {
  String c = cle; c.toUpperCase();
  String p = getPrefixNVS(); p.toUpperCase();
  if (!c.startsWith(p)) return -1;
  c = c.substring(p.length());
  if (c != "FORCE" && c != "RETARD") return -1;
  String v = valeur; v.trim();
  bool bNombre = v.length() > 0 && v.length() <= 7;
  for (char ch : v) if (!isdigit(ch)) bNombre = false;
  if (!bNombre) return -2;
  unsigned long ul = strtoul(v.c_str(), nullptr, 10);
  if (c == "FORCE") {
    if (ul < FORCAGE_MIN_S || ul > FORCAGE_MAX_S) return -2;
    mulIntervalleForcageRemonteeMesure = ul;
  }
  else {
    if (ul > DELAI_ALERTE_RETARD_MAX_S) return -2;
    mulRetardTolereS = ul;
  }
  Serial.println("CCapteurBase::setParamAlerte() - " + cle + " = " + v);
  return 0;
}

void CCapteurBase::chargeRetardNVS(Preferences& prefs) {
  mulRetardTolereS = prefs.getULong((getPrefixNVS() + "retard").c_str(), DELAI_ALERTE_RETARD_DEFAUT_S);
  if (mulRetardTolereS > DELAI_ALERTE_RETARD_MAX_S) mulRetardTolereS = DELAI_ALERTE_RETARD_DEFAUT_S;
}

void CCapteurBase::enregistreRetardNVS(Preferences& prefs) {
  prefs.putULong((getPrefixNVS() + "retard").c_str(), mulRetardTolereS);
}

// Mêmes contrôles que SET : une valeur invalide laisse l'ancienne
void CCapteurBase::litRetardWeb(WebServer& server) {
  String cle = getPrefixNVS() + "retard";
  if (server.hasArg(cle.c_str())) setParamAlerte(cle, server.arg(cle.c_str()));
}

String CCapteurBase::getHTMLRetard() const {
  return "<div class=\"row\">"
           "<div><label>Retard toléré (s, 0 à " + String(DELAI_ALERTE_RETARD_MAX_S) + ")</label><input type=\"text\" name=" +
             (getPrefixNVS() + "retard") + " value=\"" + String(mulRetardTolereS) + "\"></div>"
           "<div><label>&rarr; alerte si aucune nouvelle pendant " + formateDuree(getDelaiAlerteS()) + "</label></div>"
         "</div>";
}
