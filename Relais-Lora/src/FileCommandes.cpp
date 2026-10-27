#include "FileCommandes.h"

const SCapteurLora CFileCommandes::mCapteurs[] = FILE_CMD_CAPTEURS;
const int CFileCommandes::NB_CAPTEURS = sizeof(CFileCommandes::mCapteurs) / sizeof(CFileCommandes::mCapteurs[0]);

int CFileCommandes::indexTopic(const String& topicConfig) const {
  for (int i = 0; i < NB_CAPTEURS; i++)
    if (topicConfig == mCapteurs[i].topicConfig) return i;
  return -1;
}

int CFileCommandes::indexEquipement(const String& nomEquipement) const {
  String nom = nomEquipement; nom.toUpperCase();
  for (int i = 0; i < NB_CAPTEURS; i++) {
    String suffixe = mCapteurs[i].suffixe; suffixe.toUpperCase();
    if (nom.endsWith(suffixe)) return i;
  }
  return -1;
}

// Commande MQTT reçue sur le topic de config d'un capteur LoRa : remplace la précédente.
// Payload vide (effacement d'un retained) : ignoré.
void CFileCommandes::memorise(const String& topicConfig, const String& commande) {
  int i = indexTopic(topicConfig);
  if (i < 0 || commande.length() == 0) return;
  // « <nom> offline » : testament retenu d'un ancien firmware publié sur le topic command, pas une commande
  String sCmd = commande; sCmd.toLowerCase();
  if (sCmd.endsWith(" offline")) {
    Serial.printf("[File] %s : testament <%s> ignoré\n", topicConfig.c_str(), commande.c_str());
    return;
  }
  if (commande.equalsIgnoreCase("ANNULER")) {
    Serial.printf("[File] %s : commande <%s> annulée\n", topicConfig.c_str(), mEntrees[i].commande.c_str());
    retire(i);
    return;
  }
  mEntrees[i].commande = commande;
  mEntrees[i].nbEnvois = 0;
  mEntrees[i].bEnvoiArme = false;
  Serial.printf("[File] %s : commande mise en attente <%s>\n", topicConfig.c_str(), commande.c_str());
  publieAttente(i);
}

void CFileCommandes::retire(int i) {
  mEntrees[i].commande = "";
  mEntrees[i].bEnvoiArme = false;
  publieAttente(i);
}

// « STATUS (envoi 1/10) » ou « aucune », retenu : Home Assistant l'affiche même s'il démarre après
void CFileCommandes::publieAttente(int i) {
  if (!onMqttPublishRetenu) return;
  String sTopic = mCapteurs[i].topicConfig;
  sTopic.replace("/command", "/attente");
  const SEntree& e = mEntrees[i];
  String sMsg = (e.commande.length() > 0)
                ? e.commande + " (envoi " + String(e.nbEnvois) + "/" + String(FILE_CMD_MAX_ENVOIS) + ")"
                : String("aucune");
  onMqttPublishRetenu(sTopic.c_str(), sMsg.c_str(), true);
}

void CFileCommandes::publieAttentes() {
  for (int i = 0; i < NB_CAPTEURS; i++) publieAttente(i);
}

// Trame LoRa reçue d'un équipement : détecte l'accusé, compte les réveils et arme l'envoi.
void CFileCommandes::noteTrame(const String& topic, const String& nomEquipement, const String& payload) {
  int i = indexEquipement(nomEquipement);
  if (i < 0) return;
  SEntree& e = mEntrees[i];
  unsigned long ulMaintenant = millis();
  bool bNouveauReveil = !e.bDejaVu || (ulMaintenant - e.ulDerniereTrame >= FILE_CMD_NOUVEAU_REVEIL_MS);
  e.ulDerniereTrame = ulMaintenant;
  e.bDejaVu = true;
  if (e.nomCapteur.length() == 0) e.nomCapteur = nomEquipement;
  String sIP = payload.substring(payload.lastIndexOf(' ') + 1);
  IPAddress ipAddr;
  e.ip = (ipAddr.fromString(sIP) && sIP != "0.0.0.0") ? sIP : "";

  // Réponse « <nom> CONFIG cle:valeur ... [ERREUR ...] » à un GET ou un SET
  int iConfig = payload.indexOf(" CONFIG ");
  if (topic.endsWith("/state") && iConfig > 0) {
    noteConfig(e, payload, iConfig);
    if (e.commande.equalsIgnoreCase("GET")) {
      Serial.printf("[File] %s : config reçue, GET retiré\n", mCapteurs[i].topicConfig);
      retire(i);
      return;
    }
  }

  // Réponse « <nom> ETAT <date> <heure> | ... » à un STATUS (l'IP ajoutée par le relais est retirée)
  int iEtat = payload.indexOf(" ETAT ");
  if (topic.endsWith("/state") && iEtat > 0) {
    e.etat = payload.substring(iEtat + 6);
    if (ipAddr.fromString(sIP)) e.etat = e.etat.substring(0, e.etat.lastIndexOf(' '));
    e.ulEtatRecu = ulMaintenant;
  }

  // Config inconnue : on la demande une fois par démarrage du relais
  if (!e.bConfigConnue && !e.bGetAutoFait && e.commande.length() == 0) {
    e.bGetAutoFait = true;
    memorise(mCapteurs[i].topicConfig, "GET");
  }
  if (e.commande.length() == 0) return;

  // Accusé « COMMANDE reçue ... : <commande> ... » sur le topic d'état du capteur
  // (toUpperCase() laisse le « ç » UTF-8 intact : on cherche « COMMANDE RE »)
  String sCmd = e.commande; sCmd.toUpperCase();
  String sPayload = payload; sPayload.toUpperCase();
  if (topic.endsWith("/state") && sPayload.indexOf("COMMANDE RE") >= 0 && sPayload.indexOf(sCmd) >= 0) {
    Serial.printf("[File] %s : accusé reçu, commande <%s> retirée\n", mCapteurs[i].topicConfig, e.commande.c_str());
    retire(i);
    return;
  }

  if (bNouveauReveil && e.nbEnvois >= FILE_CMD_MAX_ENVOIS) {
    String sMsg = "ABANDON commande " + String(mCapteurs[i].topicConfig) + " : " + e.commande +
                  " (" + String(e.nbEnvois) + " réveils sans accusé)";
    Serial.println("[File] " + sMsg);
    if (onMqttPublish && topicEtatRelais.length() > 0) onMqttPublish(topicEtatRelais.c_str(), sMsg.c_str());
    retire(i);
    return;
  }

  // La trame DEEPSLEEP ON est la dernière avant le sommeil : trop tard pour envoyer
  e.bEnvoiArme = (sPayload.indexOf("DEEPSLEEP ON") < 0);
}

// Envoi de la commande en attente quand le capteur se tait depuis FILE_CMD_SILENCE_MS (fin de sa rafale de mesures)
void CFileCommandes::loop() {
  static_assert(sizeof(mCapteurs) / sizeof(SCapteurLora) <= FILE_CMD_NB_MAX_CAPTEURS,
                "FILE_CMD_CAPTEURS : augmenter FILE_CMD_NB_MAX_CAPTEURS");
  for (int i = 0; i < NB_CAPTEURS; i++) {
    SEntree& e = mEntrees[i];
    if (!e.bEnvoiArme || e.commande.length() == 0) continue;
    if (millis() - e.ulDerniereTrame < FILE_CMD_SILENCE_MS) continue;
    e.bEnvoiArme = false;
    e.nbEnvois++;
    Serial.printf("[File] %s : envoi %d/%d de <%s>\n", mCapteurs[i].topicConfig, e.nbEnvois, FILE_CMD_MAX_ENVOIS, e.commande.c_str());
    if (onLoraPublish) onLoraPublish(mCapteurs[i].topicConfig, e.commande.c_str());
    publieAttente(i);
  }
}

// « <topic de config> : dernière trame il y a 12 s — en attente : SET sleepI 60 (envoi 1/10) », une ligne par capteur
String CFileCommandes::getEtat() const {
  String s;
  for (int i = 0; i < NB_CAPTEURS; i++) {
    const SEntree& e = mEntrees[i];
    s += String(mCapteurs[i].topicConfig) + " : ";
    if (!e.bDejaVu) s += "jamais vu";
    else s += "dernière trame il y a " + String((millis() - e.ulDerniereTrame) / 1000) + " s";
    if (e.commande.length() > 0)
      s += " — en attente : " + e.commande + " (envoi " + String(e.nbEnvois) + "/" + String(FILE_CMD_MAX_ENVOIS) + ")";
    s += "\n";
  }
  return s;
}

// Garde les seuls jetons « cle:valeur » entre « CONFIG » et « ERREUR » (l'IP 0.0.0.0 ajoutée par le relais n'en est pas)
void CFileCommandes::noteConfig(SEntree& e, const String& payload, int iConfig) {
  e.nomCapteur = payload.substring(0, iConfig);
  String reste = payload.substring(iConfig + 8);
  int iErreur = reste.indexOf(" ERREUR");
  if (iErreur >= 0) reste = reste.substring(0, iErreur);
  String sConfig;
  reste.trim();
  while (reste.length() > 0) {
    int i = reste.indexOf(' ');
    String jeton = (i < 0) ? reste : reste.substring(0, i);
    reste = (i < 0) ? "" : reste.substring(i + 1); reste.trim();
    if (jeton.indexOf(':') > 0) sConfig += (sConfig.length() ? " " : "") + jeton;
  }
  e.config = sConfig;
  e.ulConfigRecue = millis();
  e.bConfigConnue = true;
}

// « 12 s », « 5 min », « 3 h », « 2 j »
static String duree(unsigned long ulMs) {
  unsigned long s = ulMs / 1000;
  if (s < 120) return String(s) + " s";
  if (s < 7200) return String(s / 60) + " min";
  if (s < 172800) return String(s / 3600) + " h";
  return String(s / 86400) + " j";
}

static String echappe(const String& s) {
  String r = s;
  r.replace("&", "&amp;"); r.replace("<", "&lt;"); r.replace(">", "&gt;"); r.replace("\"", "&quot;");
  return r;
}

static String libelle(const String& cle) {
  if (cle == "sleepA") return "Deep sleep actif (0 / 1)";
  if (cle == "sleepI") return "Intervalle deep sleep (s)";
  if (cle == "sleepS") return "Sommeil court SLEEPS (s)";
  if (cle == "sleepM") return "Sommeil moyen SLEEPM (s)";
  if (cle == "sleepL") return "Sommeil long SLEEPL (s)";
  if (cle == "wakeI")  return "Éveil maximum (s)";
  return cle;
}

String CFileCommandes::getHTML() const {
  String html;
  unsigned long ulMaintenant = millis();
  for (int i = 0; i < NB_CAPTEURS; i++) {
    const SEntree& e = mEntrees[i];
    String sNom = e.nomCapteur.length() ? e.nomCapteur : String("…") + mCapteurs[i].suffixe;
    // Entrée : seulement dans la commande libre (sinon le premier bouton du formulaire, « Annuler » par exemple, partirait)
    html += "<form class=\"device\" action=\"/capteurs_save\" method=\"POST\" autocomplete=\"off\" onkeydown=\"return event.key!='Enter'\">"
            "<input type=\"hidden\" name=\"cap\" value=\"" + String(i) + "\">"
            "<h2>" + echappe(sNom) + "</h2>"
            "<p>Topic : " + String(mCapteurs[i].topicConfig) + "<br>IP : " +
            (e.ip.length() ? "<a href=\"http://" + e.ip + "\">" + e.ip + "</a>" : String(e.bDejaVu ? "aucune (LoRa seul)" : "inconnue")) +
            "<br>Dernière trame : " +
            (e.bDejaVu ? "il y a " + duree(ulMaintenant - e.ulDerniereTrame) : String("jamais vu depuis le démarrage du relais")) +
            "<br>Config reçue : " + (e.bConfigConnue ? "il y a " + duree(ulMaintenant - e.ulConfigRecue) : String("inconnue")) + "</p>";
    bool bAttente = e.commande.length() > 0;
    String sBloque = bAttente ? " disabled" : "";
    if (bAttente)
      html += "<p class=\"attente\">En attente : " + echappe(e.commande) + " (envoi " + String(e.nbEnvois) + "/" +
              String(FILE_CMD_MAX_ENVOIS) + ") — appliquée au prochain réveil "
              "<button type=\"submit\" name=\"action\" value=\"annuler\" class=\"btn btn-off\">Annuler</button></p>";
    if (e.ulEtatRecu > 0) {
      String sEtat = echappe(e.etat); sEtat.replace(" | ", "<br>");
      html += "<p>État (il y a " + duree(ulMaintenant - e.ulEtatRecu) + ") :<br>" + sEtat + "</p>";
    }

    if (e.bConfigConnue) {
      html += "<div class=\"row\">";
      String reste = e.config;
      while (reste.length() > 0) {
        int iEsp = reste.indexOf(' ');
        String jeton = (iEsp < 0) ? reste : reste.substring(0, iEsp);
        reste = (iEsp < 0) ? "" : reste.substring(iEsp + 1);
        int p = jeton.indexOf(':');
        String cle = echappe(jeton.substring(0, p));
        String val = echappe(jeton.substring(p + 1));
        html += "<div><label>" + libelle(cle) + "</label>"
                "<input type=\"text\" name=\"k_" + cle + "\" value=\"" + val + "\">"
                "<input type=\"hidden\" name=\"o_" + cle + "\" value=\"" + val + "\"></div>";
      }
      html += "</div><button type=\"submit\" name=\"action\" value=\"enregistrer\" class=\"btn btn-on\"" + sBloque + ">Enregistrer</button>";
    }
    html += "<button type=\"submit\" name=\"action\" value=\"relire\" class=\"btn btn-cyan\"" + sBloque + ">Relire (GET)</button>"
            "<button type=\"submit\" name=\"action\" value=\"status\" class=\"btn btn-cyan\"" + sBloque + ">État (STATUS)</button>"
            "<button type=\"submit\" name=\"action\" value=\"reboot\" class=\"btn btn-off\"" + sBloque +
            " onclick=\"return confirm('Redémarrer " + echappe(sNom) + " à son prochain réveil ?')\">Redémarrer</button>"
            "<div class=\"row\"><div><label>Commande libre (topic " + String(mCapteurs[i].topicConfig) + ") "
            "<button type=\"button\" class=\"aide-btn\" onclick=\"aide(true)\">?</button></label>"
            "<input type=\"text\" name=\"libre\" placeholder=\"ex : WDOG 3600\""
            " onkeydown=\"if(event.key=='Enter'){event.stopPropagation();event.preventDefault();this.form.querySelector('[value=libre]').click();}\"></div></div>"
            "<button type=\"submit\" name=\"action\" value=\"libre\" class=\"btn btn-on\"" + sBloque + ">Envoyer</button></form>";
  }
  return html;
}

// Enregistrer : un seul SET avec les champs modifiés ; Relire : GET. Remplace la commande en attente du capteur.
void CFileCommandes::loadFromWebServer(WebServer& server) {
  int i = server.arg("cap").toInt();
  if (!server.hasArg("cap") || i < 0 || i >= NB_CAPTEURS) return;
  String sAction = server.arg("action");
  if (sAction == "annuler") { memorise(mCapteurs[i].topicConfig, "ANNULER"); return; }
  // Une seule commande en attente par capteur : pas de remplacement depuis la page (boutons grisés)
  if (mEntrees[i].commande.length() > 0) return;
  if (sAction == "relire") { memorise(mCapteurs[i].topicConfig, "GET"); return; }
  if (sAction == "status") { memorise(mCapteurs[i].topicConfig, "STATUS"); return; }
  if (sAction == "reboot") { memorise(mCapteurs[i].topicConfig, "REBOOT"); return; }
  if (sAction == "libre") {
    String sCmd = server.arg("libre"); sCmd.trim();
    memorise(mCapteurs[i].topicConfig, sCmd);  // Vide : ignorée par memorise()
    return;
  }
  String sSet;
  for (int a = 0; a < server.args(); a++) {
    String nom = server.argName(a);
    if (!nom.startsWith("k_")) continue;
    String cle = nom.substring(2);
    String val = server.arg(a); val.trim(); val.replace(" ", "");
    if (val.length() == 0 || val == server.arg("o_" + cle)) continue;
    sSet += " " + cle + ":" + val;
  }
  if (sSet.length() > 0) memorise(mCapteurs[i].topicConfig, "SET" + sSet);
}
