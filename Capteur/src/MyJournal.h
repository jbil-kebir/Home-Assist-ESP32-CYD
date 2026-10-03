#ifndef MYJOURNAL_H
#define MYJOURNAL_H

//------------------------------------------------------------------------------
// Journal consultable sur la page web du capteur (http://<IP>/logs)
//
// global.h redirige Serial vers gSerialJournal : tout ce qui est écrit sur Serial part sur le
// port série ET dans le journal, ligne par ligne, horodaté (secondes depuis le démarrage).
//
// Les dernières lignes sont aussi copiées en mémoire RTC (RTC_NOINIT), qui survit à un
// redémarrage logiciel (plantage, watchdog, ESP.restart(), souvent brownout) : au démarrage
// suivant, elles sont reprises dans le journal, avec la cause du redémarrage.
// Après une mise sous tension, la mémoire RTC est perdue.
//
// Sauvegarde avant coupure (case « Sauvegarde du journal avant coupure » de la page web) :
// un noeud alimenté par un relais sait quand il va être coupé (juste après avoir publié
// OnOff 0 ou IRKO). sauvegarde() enregistre alors les dernières lignes en NVS ; elles sont
// reprises au démarrage suivant ("Session precedente"), puis effacées de la NVS.
//
// Ce fichier ne doit pas inclure global.h (MyJournal.cpp utilise le vrai Serial).
//------------------------------------------------------------------------------
#include <Arduino.h>

constexpr uint8_t JOURNAL_NB_LIGNES      = 100;
constexpr uint8_t JOURNAL_LONG_LIGNE     = 160;
constexpr uint8_t JOURNAL_NB_LIGNES_RTC  = 24;  // Mémoire RTC limitée (8 Ko sur ESP32-C3)
constexpr uint8_t JOURNAL_LONG_LIGNE_RTC = 120;
constexpr size_t  JOURNAL_TAILLE_NVS     = 3800; // Octets max sauvegardés en NVS (chaîne NVS : 4000 max)

class CJournal {
public:
  void demarrage();               // Cause du redémarrage + reprise des lignes RTC et NVS (appelée par Serial.begin())
  void ajoute(const char* ligne); // Ligne sans '\n', horodatée ici
  String getContenu() const;
  void vide();
  void setSauvegardeActive(bool b) { mbSauvegardeActive = b; }
  bool sauvegarde(const char* raison); // Enregistre les dernières lignes en NVS (coupure imminente)
private:
  char mLignes[JOURNAL_NB_LIGNES][JOURNAL_LONG_LIGNE];
  uint8_t mTete = 0;
  uint8_t mNb = 0;
  bool mDemarre = false;
  bool mbSauvegardeActive = true;
  unsigned long mulDerniereSauvegarde = 0;
  String msDerniereRaison;
  void ajouteBrut(const char* ligne);
  void reprendSessionNVS();
};

extern CJournal gJournal;

// Remplaçant de Serial (voir global.h) : port série + journal
class CSerialJournal : public Print {
public:
  void begin(unsigned long baud);
  void flush();
  operator bool() const;
  size_t write(uint8_t c) override;
  size_t write(const uint8_t* buffer, size_t size) override;
  using Print::write;
private:
  char mLigne[JOURNAL_LONG_LIGNE];
  size_t mLong = 0;
  void finLigne();
};

extern CSerialJournal gSerialJournal;

#endif // MYJOURNAL_H
