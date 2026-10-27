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
  mEntrees[i].commande = commande;
  mEntrees[i].nbEnvois = 0;
  mEntrees[i].bEnvoiArme = false;
  Serial.printf("[File] %s : commande mise en attente <%s>\n", topicConfig.c_str(), commande.c_str());
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
  if (e.commande.length() == 0) return;

  // Accusé « COMMANDE reçue ... : <commande> ... » sur le topic d'état du capteur
  // (toUpperCase() laisse le « ç » UTF-8 intact : on cherche « COMMANDE RE »)
  String sCmd = e.commande; sCmd.toUpperCase();
  String sPayload = payload; sPayload.toUpperCase();
  if (topic.endsWith("/state") && sPayload.indexOf("COMMANDE RE") >= 0 && sPayload.indexOf(sCmd) >= 0) {
    Serial.printf("[File] %s : accusé reçu, commande <%s> retirée\n", mCapteurs[i].topicConfig, e.commande.c_str());
    e.commande = "";
    e.bEnvoiArme = false;
    return;
  }

  if (bNouveauReveil && e.nbEnvois >= FILE_CMD_MAX_ENVOIS) {
    String sMsg = "ABANDON commande " + String(mCapteurs[i].topicConfig) + " : " + e.commande +
                  " (" + String(e.nbEnvois) + " réveils sans accusé)";
    Serial.println("[File] " + sMsg);
    if (onMqttPublish && topicEtatRelais.length() > 0) onMqttPublish(topicEtatRelais.c_str(), sMsg.c_str());
    e.commande = "";
    e.bEnvoiArme = false;
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
