#include "RemoteCapteurBase.h"

std::function<void(const String&, bool, unsigned long)> CRemoteCapteurBase::onAlerte = nullptr;
bool CRemoteCapteurBase::sbTrameCopie = false;

// « home/thermometre/state » → « home/thermometre/copie » (topic sans « /state » final : « <topic>/copie »)
String CRemoteCapteurBase::topicCopie(const String& topicState) {
  if (topicState.endsWith("/state")) return topicState.substring(0, topicState.length() - 6) + "/copie";
  return topicState + "/copie";
}
std::function<void()> CRemoteCapteurBase::onChangementRetard = nullptr;

std::vector<CRemoteCapteurBase*>& CRemoteCapteurBase::registre() {
  static std::vector<CRemoteCapteurBase*> v;
  return v;
}

void CRemoteCapteurBase::verifieTous() {
  for (CRemoteCapteurBase* c : registre())
    if (c->active) c->verifieWatchdog();
}

String CRemoteCapteurBase::formateDuree(unsigned long s) {
  if (s < 60) return String(s) + " s";
  unsigned long m = (s + 30) / 60;  // Arrondi à la minute
  if (m < 60) return String(m) + " min";
  unsigned long h = m / 60; m %= 60;
  String sMin = (m < 10 ? "0" : "") + String(m);
  if (h < 24) return String(h) + " h " + sMin;
  return String(h / 24) + " j " + String(h % 24) + " h " + sMin;
}

// « ... alerte:<s> ... » (mot entier, casse indifférente). Lu ici plutôt que par ParsedMqttMessage :
// CRemoteBatterieAA a son propre découpage des trames.
unsigned long CRemoteCapteurBase::extraitDelaiAlerte(const String& payload) {
  String s = " " + payload + " ";
  s.toLowerCase();
  int i = s.indexOf(" alerte:");
  if (i < 0) return 0;
  i += 8;
  unsigned long ul = 0;
  bool bChiffre = false;
  while (i < (int)s.length() && isDigit(s[i])) { ul = ul * 10 + (s[i] - '0'); i++; bChiffre = true; }
  if (!bChiffre || s[i] != ' ') return 0;
  return ul;
}

void CRemoteCapteurBase::noteReception(const String& payload) {
  mulWatchDog = millis();
  unsigned long ul = extraitDelaiAlerte(payload);
  if (ul > 0 && ul != mulDelaiAnnonceS) {
    DBG(DBG_CAPTEURS, "CRemoteCapteurBase::noteReception() - %s : délai d'alerte annoncé %lu s (avant : %lu s)\n",
        nomEquipement.c_str(), ul, mulDelaiAnnonceS);
    mulDelaiAnnonceS = ul;
    prefs.begin(nvs_namespace, false);
    prefs.putULong((mPrefixNVS + "wdann").c_str(), mulDelaiAnnonceS);
    prefs.end();
  }
  if (mbEnRetard) {
    mbEnRetard = false;
    DBG(DBG_CAPTEURS, "CRemoteCapteurBase::noteReception() - %s : de retour\n", nomEquipement.c_str());
    if (onAlerte != nullptr && mulDelaiAnnonceS > 0) { onAlerte(nomEquipement, false, 0); mbEtatPublie = true; }
    if (onChangementRetard != nullptr) onChangementRetard();
  }
  else if (!mbEtatPublie && onAlerte != nullptr && mulDelaiAnnonceS > 0) {
    // 1re trame depuis le démarrage du CYD : OK publié pour resynchroniser HA (sans mail si HA était déjà à OK)
    onAlerte(nomEquipement, false, 0);
    mbEtatPublie = true;
  }
}

// Capteur inactif (case « Actif » décochée) : jamais en retard. Contrôlé ici et pas seulement dans verifieTous() :
// CRemoteDHT20::loop() et CRemoteTCS34725::loop() appellent verifieWatchdog() sans tester « active ».
bool CRemoteCapteurBase::verifieWatchdog() {
  if (!active) return false;
  if (!delaiDepasse()) return false;
  if (!mbEnRetard) {
    mbEnRetard = true;
    DBG(DBG_CAPTEURS, "CRemoteCapteurBase::verifieWatchdog() - %s : en retard (rien reçu depuis %lu s)\n",
        nomEquipement.c_str(), getWatchdogIntervalle());
    if (onAlerte != nullptr && mulDelaiAnnonceS > 0) { onAlerte(nomEquipement, true, (millis() - mulWatchDog) / 1000UL); mbEtatPublie = true; }
    if (onChangementRetard != nullptr) onChangementRetard();
  }
  return true;
}

void CRemoteCapteurBase::chargeWatchdogNVS() {
  mulWatchdogIntervalle = prefs.getLong((mPrefixNVS + "wdog").c_str(), mulWatchdogDefaultIntervalle);
  mulDelaiAnnonceS = prefs.getULong((mPrefixNVS + "wdann").c_str(), 0);
}

// Le délai annoncé n'est pas écrit ici : il l'est à sa réception (noteReception())
void CRemoteCapteurBase::enregistreWatchdogNVS() {
  if (!mbDelaiSaisi) return; // Champ absent de la page : valeur NVS laissée telle quelle
  prefs.putLong((mPrefixNVS + "wdog").c_str(), mulWatchdogIntervalle);
}

void CRemoteCapteurBase::litWatchdogWeb(WebServer& server) {
  if (mbDelaiSaisi && server.hasArg((mPrefixNVS + "wdog").c_str())) mulWatchdogIntervalle = server.arg((mPrefixNVS + "wdog")).toInt();
}

String CRemoteCapteurBase::getHTMLWatchdog() const {
  String sProvenance = (mulDelaiAnnonceS > 0) ? "annoncé par le nœud"
                     : mbDelaiSaisi ? "saisi ici : l'équipement n'annonce rien"
                     : "par défaut : aucune annonce reçue du nœud";
  String s = "<div><label>Délai d'alerte : " + formateDuree(getWatchdogIntervalle()) + " (" + sProvenance + ")</label></div>";
  if (mbDelaiSaisi)
    s += "<div><label>Intervalle Watchdog (s)</label><input type=\"text\" name=" + (mPrefixNVS + "wdog") +
         " value=\"" + String(mulWatchdogIntervalle) + "\"></div>";
  return s;
}
