#include "CapteurBase.h"
#include "MyConfig.h"

const CConfig* CCapteurBase::smConfig = nullptr;

//
// Délai d'alerte (s) : voir CapteurBase.h
//
unsigned long CCapteurBase::getDelaiMinimumS() const {
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
  return intervalle + DELAI_ALERTE_MARGE_S;
}

// Délai saisi (minutes), relevé au minimum s'il est plus court (0 : au plus tôt)
unsigned long CCapteurBase::getDelaiAlerteS() const {
  unsigned long saisi = mulDelaiAlerteMin * 60UL;
  unsigned long minimum = getDelaiMinimumS();
  return saisi > minimum ? saisi : minimum;
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
         " " + getPrefixNVS() + "alerte:" + String(mulDelaiAlerteMin);
}

int CCapteurBase::setParamAlerte(const String& cle, const String& valeur) {
  String c = cle; c.toUpperCase();
  String p = getPrefixNVS(); p.toUpperCase();
  if (!c.startsWith(p)) return -1;
  c = c.substring(p.length());
  if (c != "FORCE" && c != "ALERTE") return -1;
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
    if (ul > DELAI_ALERTE_MAX_MIN) return -2;
    mulDelaiAlerteMin = ul;
  }
  Serial.println("CCapteurBase::setParamAlerte() - " + cle + " = " + v);
  return 0;
}

void CCapteurBase::chargeAlerteNVS(Preferences& prefs) {
  String cle = getPrefixNVS() + "alerte", ancienne = getPrefixNVS() + "retard";
  if (prefs.isKey(cle.c_str())) mulDelaiAlerteMin = prefs.getULong(cle.c_str(), DELAI_ALERTE_DEFAUT_MIN);
  else if (prefs.isKey(ancienne.c_str())) {
    // Firmware antérieur à la 4.4 : retard toléré (s) converti ; écrit en NVS au prochain enregistrement
    unsigned long retard = prefs.getULong(ancienne.c_str(), 0);
    mulDelaiAlerteMin = (retard == 0) ? 0 : (retard + mulIntervalleForcageRemonteeMesure + 30UL) / 60UL;
    Serial.printf("CCapteurBase::chargeAlerteNVS() - %s : retard toléré %lu s converti en délai d'alerte %lu min\n",
                  getPrefixNVS().c_str(), retard, mulDelaiAlerteMin);
  }
  else mulDelaiAlerteMin = DELAI_ALERTE_DEFAUT_MIN;
  if (mulDelaiAlerteMin > DELAI_ALERTE_MAX_MIN) mulDelaiAlerteMin = DELAI_ALERTE_MAX_MIN;
}

void CCapteurBase::enregistreAlerteNVS(Preferences& prefs) {
  prefs.putULong((getPrefixNVS() + "alerte").c_str(), mulDelaiAlerteMin);
  String ancienne = getPrefixNVS() + "retard";
  if (prefs.isKey(ancienne.c_str())) prefs.remove(ancienne.c_str());
}

// Mêmes contrôles que SET : une valeur invalide laisse l'ancienne
void CCapteurBase::litAlerteWeb(WebServer& server) {
  String cle = getPrefixNVS() + "alerte";
  if (server.hasArg(cle.c_str())) setParamAlerte(cle, server.arg(cle.c_str()));
}

String CCapteurBase::getHTMLAlerte() const {
  unsigned long delai = getDelaiAlerteS(), minimum = getDelaiMinimumS();
  String sResultat = "&rarr; alerte après " + formateDuree(delai) + " sans nouvelle";
  if (mulDelaiAlerteMin == 0) sResultat += " (au plus tôt)";
  else if (delai == minimum) sResultat += " (relevé au minimum)";
  else sResultat += " (minimum " + formateDuree(minimum) + ")";
  return "<div class=\"row\">"
           "<div><label>Délai d'alerte (min, 0 = au plus tôt, max " + String(DELAI_ALERTE_MAX_MIN) + ")</label><input type=\"text\" name=" +
             (getPrefixNVS() + "alerte") + " value=\"" + String(mulDelaiAlerteMin) + "\"></div>"
           "<div><label>" + sResultat + "</label></div>"
         "</div>";
}
