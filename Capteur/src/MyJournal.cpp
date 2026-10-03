// Pas de global.h ici : Serial désigne le vrai port série (voir MyJournal.h)
#include "MyJournal.h"
#include <esp_system.h>
#include <esp_attr.h>
#include <Preferences.h>

static const char* JOURNAL_NVS_NAMESPACE = "journal";
static const char* JOURNAL_NVS_CLE       = "session";

CJournal gJournal;
CSerialJournal gSerialJournal;

//------------------------------------------------------------------------------
// Copie des dernières lignes en mémoire RTC (survit à un redémarrage logiciel)
//------------------------------------------------------------------------------
static const uint32_t JOURNAL_MAGIC_RTC = 0x4A524E4C; // "JRNL"
RTC_NOINIT_ATTR static uint32_t rtcMagic;
RTC_NOINIT_ATTR static uint8_t  rtcTete;
RTC_NOINIT_ATTR static uint8_t  rtcNb;
RTC_NOINIT_ATTR static char     rtcLignes[JOURNAL_NB_LIGNES_RTC][JOURNAL_LONG_LIGNE_RTC];

static void rtcRaz() {
  rtcTete = 0;
  rtcNb = 0;
  rtcMagic = JOURNAL_MAGIC_RTC;
}

static void rtcAjoute(const char* ligne) {
  if (rtcMagic != JOURNAL_MAGIC_RTC) rtcRaz();
  strncpy(rtcLignes[rtcTete], ligne, JOURNAL_LONG_LIGNE_RTC - 1);
  rtcLignes[rtcTete][JOURNAL_LONG_LIGNE_RTC - 1] = '\0';
  rtcTete = (rtcTete + 1) % JOURNAL_NB_LIGNES_RTC;
  if (rtcNb < JOURNAL_NB_LIGNES_RTC) rtcNb++;
}

static const char* nomCauseRedemarrage(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:   return "mise sous tension";
    case ESP_RST_EXT:       return "reset externe";
    case ESP_RST_SW:        return "redemarrage logiciel (ESP.restart)";
    case ESP_RST_PANIC:     return "PLANTAGE (exception / panic)";
    case ESP_RST_INT_WDT:   return "WATCHDOG interruption";
    case ESP_RST_TASK_WDT:  return "WATCHDOG tache";
    case ESP_RST_WDT:       return "WATCHDOG";
    case ESP_RST_DEEPSLEEP: return "reveil de deep sleep";
    case ESP_RST_BROWNOUT:  return "BROWNOUT (chute de tension d'alimentation)";
    case ESP_RST_SDIO:      return "reset SDIO";
    default:                return "inconnue";
  }
}

//------------------------------------------------------------------------------
// CJournal
//------------------------------------------------------------------------------
void CJournal::demarrage() {
  if (mDemarre) return;
  mDemarre = true;

  reprendSessionNVS();

  esp_reset_reason_t cause = esp_reset_reason();
  bool rtcValide = (rtcMagic == JOURNAL_MAGIC_RTC && rtcNb <= JOURNAL_NB_LIGNES_RTC && rtcTete < JOURNAL_NB_LIGNES_RTC);
  if (cause != ESP_RST_POWERON && rtcValide && rtcNb > 0) {
    ajouteBrut("---- Dernieres lignes avant le redemarrage ----");
    uint8_t debut = (rtcNb < JOURNAL_NB_LIGNES_RTC) ? 0 : rtcTete;
    for (uint8_t i = 0; i < rtcNb; i++) {
      char* l = rtcLignes[(debut + i) % JOURNAL_NB_LIGNES_RTC];
      l[JOURNAL_LONG_LIGNE_RTC - 1] = '\0';
      ajouteBrut(l);
    }
    ajouteBrut("---- Fin des lignes avant le redemarrage ----");
  }
  rtcRaz();

  char buf[JOURNAL_LONG_LIGNE];
  snprintf(buf, sizeof(buf), "Demarrage - cause : %s (%d)", nomCauseRedemarrage(cause), (int)cause);
  ajoute(buf);
}

// Session sauvegardée avant la dernière coupure (voir sauvegarde()) : reprise une seule fois
void CJournal::reprendSessionNVS() {
  Preferences prefs;
  if (!prefs.begin(JOURNAL_NVS_NAMESPACE, false)) return;
  if (prefs.isKey(JOURNAL_NVS_CLE)) {
    String s = prefs.getString(JOURNAL_NVS_CLE, "");
    prefs.remove(JOURNAL_NVS_CLE);
    if (s.length() > 0) {
      ajouteBrut("---- Session precedente (sauvegardee avant coupure) ----");
      int debut = 0;
      while (debut < (int)s.length()) {
        int fin = s.indexOf('\n', debut);
        if (fin < 0) fin = s.length();
        if (fin > debut) ajouteBrut(s.substring(debut, fin).c_str());
        debut = fin + 1;
      }
      ajouteBrut("---- Fin de la session precedente ----");
    }
  }
  prefs.end();
}

//------------------------------------------------------------------------------
// CJournal::sauvegarde()
//
// Enregistre en NVS les dernières lignes du journal (au plus JOURNAL_TAILLE_NVS octets),
// juste avant une coupure d'alimentation annoncée (OnOff 0, IRKO).
// Usure de la flash : une même raison n'est pas sauvegardée plus d'une fois par minute
// (republications tant que MQTT n'est pas connecté), deux sauvegardes sont espacées d'au moins 2 s.
// Retour : true si la sauvegarde a été faite
//------------------------------------------------------------------------------
bool CJournal::sauvegarde(const char* raison) {
  if (!mbSauvegardeActive) return false;
  unsigned long now = millis();
  if (mulDerniereSauvegarde != 0) {
    unsigned long ecart = now - mulDerniereSauvegarde;
    if (ecart < 2000UL) return false;
    if (msDerniereRaison == raison && ecart < 60000UL) return false;
  }
  mulDerniereSauvegarde = now;
  msDerniereRaison = raison;

  char buf[JOURNAL_LONG_LIGNE];
  snprintf(buf, sizeof(buf), "Sauvegarde du journal avant coupure (%s)", raison);
  Serial.println(buf);
  ajoute(buf);

  // Lignes les plus récentes qui tiennent dans JOURNAL_TAILLE_NVS
  uint8_t debut = (mNb < JOURNAL_NB_LIGNES) ? 0 : mTete;
  size_t taille = 0;
  uint8_t nbGardees = 0;
  for (int i = mNb - 1; i >= 0; i--) {
    size_t l = strlen(mLignes[(debut + i) % JOURNAL_NB_LIGNES]) + 1;
    if (taille + l > JOURNAL_TAILLE_NVS) break;
    taille += l;
    nbGardees++;
  }
  String s;
  s.reserve(taille + 1);
  for (uint8_t i = mNb - nbGardees; i < mNb; i++) {
    s += mLignes[(debut + i) % JOURNAL_NB_LIGNES];
    s += '\n';
  }

  Preferences prefs;
  if (!prefs.begin(JOURNAL_NVS_NAMESPACE, false)) return false;
  bool ok = prefs.putString(JOURNAL_NVS_CLE, s) > 0;
  prefs.end();
  Serial.printf("Journal sauvegarde : %u lignes, %u octets en %lu ms%s\n", nbGardees, (unsigned)taille, millis() - now, ok ? "" : " - ECHEC");
  return ok;
}

void CJournal::ajoute(const char* ligne) {
  char buf[JOURNAL_LONG_LIGNE];
  unsigned long ms = millis();
  snprintf(buf, sizeof(buf), "[%lu.%03lu] %s", ms / 1000, ms % 1000, ligne);
  ajouteBrut(buf);
  rtcAjoute(buf);
}

void CJournal::ajouteBrut(const char* ligne) {
  char* dest = mLignes[mTete];
  strncpy(dest, ligne, JOURNAL_LONG_LIGNE - 1);
  dest[JOURNAL_LONG_LIGNE - 1] = '\0';
  mTete = (mTete + 1) % JOURNAL_NB_LIGNES;
  if (mNb < JOURNAL_NB_LIGNES) mNb++;
}

String CJournal::getContenu() const {
  if (mNb == 0) return "(journal vide)\n";
  String s;
  s.reserve(mNb * 60);
  uint8_t debut = (mNb < JOURNAL_NB_LIGNES) ? 0 : mTete;
  for (uint8_t i = 0; i < mNb; i++) {
    s += mLignes[(debut + i) % JOURNAL_NB_LIGNES];
    s += '\n';
  }
  return s;
}

void CJournal::vide() {
  mTete = 0;
  mNb = 0;
}

//------------------------------------------------------------------------------
// CSerialJournal
//------------------------------------------------------------------------------
void CSerialJournal::begin(unsigned long baud) {
  Serial.begin(baud);
  gJournal.demarrage();
}

void CSerialJournal::flush() {
  Serial.flush();
}

CSerialJournal::operator bool() const {
  return (bool)Serial;
}

size_t CSerialJournal::write(uint8_t c) {
  Serial.write(c);
  if (c == '\n') finLigne();
  else if (c != '\r') {
    if (mLong >= JOURNAL_LONG_LIGNE - 1) finLigne(); // Ligne trop longue : coupée
    mLigne[mLong++] = (char)c;
  }
  return 1;
}

size_t CSerialJournal::write(const uint8_t* buffer, size_t size) {
  Serial.write(buffer, size);
  for (size_t i = 0; i < size; i++) {
    uint8_t c = buffer[i];
    if (c == '\n') finLigne();
    else if (c != '\r') {
      if (mLong >= JOURNAL_LONG_LIGNE - 1) finLigne();
      mLigne[mLong++] = (char)c;
    }
  }
  return size;
}

void CSerialJournal::finLigne() {
  mLigne[mLong] = '\0';
  if (mLong > 0) gJournal.ajoute(mLigne);
  mLong = 0;
}
