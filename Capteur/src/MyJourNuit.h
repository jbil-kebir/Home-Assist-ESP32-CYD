#ifndef __MYJOURNUIT_H__
#define __MYJOURNUIT_H__

#include <functional>
#include <WebServer.h>
#include <Preferences.h>
#include "global.h"
#include "MyDateTime.h"

//------------------------------------------------------------------------------
// Détecteur jour / nuit : photorésistance alimentée par une GPIO (coupée hors mesure et en deep sleep)
//
//   JOUR_NUIT_ALIM_PIN ── photorésistance ──┬── JOUR_NUIT_PIN (ADC1)
//                                           └── 10 kΩ ── GND
//
// Éclairement = tension mesurée en % de la tension d'alimentation (0 = obscurité, 100 = pleine lumière).
// État décidé par une fenêtre d'hystérésis (page Web) :
//   éclairement >= seuil haut -> JOUR ; éclairement <= seuil bas -> NUIT ; entre les deux : état inchangé.
//
// Trames (sur mqttSubTopicState), dès que l'état ou l'éclairement change, et au forçage périodique :
//   « <nom> Jour <date> <heure> 1|0 »              (toujours)
//   « <nom> Eclairement <date> <heure> <pct> »     (si « Remonter l'éclairement » est coché)
//
// Mémorisation (voir MyJourNuit.cpp) :
//   - mémoire RTC (conservée en deep sleep, effacée à la coupure / au reset, sans usure) : état, éclairement,
//     heure du dernier envoi ;
//   - NVS : état seulement, écrit à chaque bascule (environ deux fois par jour) pour survivre à une coupure.
//------------------------------------------------------------------------------
#define JOUR_NUIT_STABILISATION_MS  5     // Attente entre la mise sous tension du pont et la mesure
#define JOUR_NUIT_NB_ECHANTILLONS   8     // Mesures moyennées
#define JOUR_NUIT_TENSION_ALIM_MV   3300  // Tension de la GPIO d'alimentation (100 %)
#define JOUR_NUIT_SEUIL_BAS         20    // % : passage jour -> nuit
#define JOUR_NUIT_SEUIL_HAUT        30    // % : passage nuit -> jour
#define JOUR_NUIT_ETAT_INCONNU      -1
#define JOUR_NUIT_ETAT_NUIT         0
#define JOUR_NUIT_ETAT_JOUR         1
// Bornes acceptées par setParam() (commande SET) et la page Web, en secondes
#define JOUR_NUIT_INTER_MIN_SEC     1
#define JOUR_NUIT_INTER_MAX_SEC     3600
#define JOUR_NUIT_FORCE_MIN_SEC     60
#define JOUR_NUIT_FORCE_MAX_SEC     86400

class CJourNuit {
private:
  Preferences prefs;
  unsigned char mucPin;      // Entrée ADC1
  unsigned char mucPinAlim;  // Alimentation du pont
  int miLastEclairement = -1, miNewEclairement = -1; // En %, -1 : aucune mesure
  int8_t mcLastEtat = JOUR_NUIT_ETAT_INCONNU, mcNewEtat = JOUR_NUIT_ETAT_INCONNU;
  int8_t mcEtatNVS = JOUR_NUIT_ETAT_INCONNU; // État enregistré en NVS (écrit seulement s'il change)
  int miSeuilBas = JOUR_NUIT_SEUIL_BAS;
  int miSeuilHaut = JOUR_NUIT_SEUIL_HAUT;
  int miSeuilBasValide = JOUR_NUIT_SEUIL_BAS;   // Dernière fenêtre valide : restaurée par valideSeuils() si
  int miSeuilHautValide = JOUR_NUIT_SEUIL_HAUT; // une commande SET laisse bas >= haut
  bool mbRemonterEclairement = true;
  const char* nvs_namespace = NVS_NAME_SPACE;
  String mPrefixNVS = "jn_";
  CMyDateTime *mDateTime=nullptr;

  int8_t calculeEtat(int eclairement) const;
  void memoriseEtatNVS();
  bool publie(const String& sMesure, const String& sValeur, bool force);

public:
  CJourNuit(uint8_t gpioPin, uint8_t gpioPinAlim, CMyDateTime& dateTime) :
    mucPin(gpioPin), mucPinAlim(gpioPinAlim), mDateTime(&dateTime) {}

  String domotique_prefix;
  String nomEquipement = "Jn";
  String mqttSubTopic = "thermometre/";
  bool active = false; // Voir loadFromNVS()
  bool mbMesureRemontee = false; // Permet d'empêcher le deep sleep tant qu'une mesure n'a pas été remontée
  String mqttSubTopicCommand;
  String mqttSubTopicState;
  unsigned long mulIntervalleMesure = 30UL, mulDefaultIntervalleMesure = 30UL; // En secondes
  // En secondes, compté à travers le deep sleep (heure système, voir loop())
  unsigned long mulIntervalleForcageRemonteeMesure = 600UL, mulDefaultIntervalleForcageRemonteeMesure = 600UL;

  // MQTT callback
  std::function<int(const char*, const char*)> onMqttPublish = nullptr;
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cb) { onMqttPublish = cb; }
  // Lora P2P callback
  std::function<int(const char*)> onLoraP2PPublish = nullptr;
  void setLoraP2PPublishCallback(std::function<int(const char* payload)> cb) { onLoraP2PPublish = cb; }

  void loadFromNVS();
  void loadFromWebServer(WebServer& server);
  void saveToNVS();
  String getHTML();

  int setup(const String pref);
  int loop();
  bool readEclairement();
  int getLastEclairement() const { return miLastEclairement; }
  int8_t getLastEtat() const { return mcLastEtat; }
  String getLastEtatTexte() const;
  void coupeAlimentation();
  void print() const;
  int handleMqttCommand(const String& payload);
  int readAndPublish(bool force=false);
  // Commande SET (voir CConfig::setParam()) : clés jn_sbas, jn_shaut, jn_ecl, jn_inter, jn_force
  int setParam(const String& cle, const String& valeur);
  bool valideSeuils();
  String getParams() const;
};

#endif // __MYJOURNUIT_H__
