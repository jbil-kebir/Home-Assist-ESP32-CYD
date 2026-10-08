// MyActionneurIR.h
#pragma once

#ifndef __CACTIONNEURIR_H__
#define __CACTIONNEURIR_H__
#include "global.h"
#include <functional>
#include <Preferences.h>
#include <Arduino.h>
#include <WebServer.h>
#include "MyDateTime.h"

#ifdef ACTIONNEUR_IR
#ifdef CAPTEUR_MICRO_MAX4466
#include "MyMax4466.h"
#endif

//----------------------------------------------------------------------------
// Actionneur IR du noeud chauffage SdB (repris de Archives/ChauffageIR 1.1, bibliothèque IRremote)
//
// La télécommande du chauffage n'a qu'un code, qui bascule ON/OFF. Le micro (CMax4466) écoute
// le bip du chauffage : court = ON, long = OFF. Séquence lancée par demandeEtat(consigne) :
//   émission IR → attente du bip (muiAttenteBip ms)
//   → bip conforme à la consigne : publication "<nom> OnOff <date> <heure> 0|1" (via le micro)
//   → bip contraire ou pas de bip : réémission, jusqu'à muiNbTentatives émissions
//   → échec : publication "<nom> IRKO <date> <heure>", l'état OnOff n'est pas modifié
// Les publications sont retentées tant que MQTT n'est pas connecté.
//
// Le noeud n'est alimenté que lorsque le relais 433 MHz est fermé : la mise sous tension vaut demande ON.
//
// IRremote.hpp ne doit être inclus que dans MyActionneurIR.cpp (définitions multiples au link sinon).
//----------------------------------------------------------------------------
#define IR_DEFAULT_PROTOCOLE        1       // 1 NEC, 2 SONY, 3 RC5, 4 RC6
#define IR_DEFAULT_ADRESSE          0xCDAB  // Télécommande du chauffage SdB
#define IR_DEFAULT_COMMANDE         0x1A    // Code unique ON/OFF (bascule)
#define IR_DEFAULT_REPETITIONS      0       // L'émission est bloquante : chaque répétition (~110 ms) rend le micro sourd
#define IR_DEFAULT_ATTENTE_BIP      3000    // ms
#define IR_DEFAULT_NB_TENTATIVES    3
#define IR_INTERVALLE_REPUBLICATION 2000    // ms entre deux tentatives de publication (MQTT pas encore connecté)
// Bornes (voir bornesParametres())
#define IR_MIN_ATTENTE_BIP          1000    // ms : > durée max d'un bip long (MAX4466_DEFAULT_LONG_MAX, 750 ms) + réaction du chauffage
#define IR_MAX_ATTENTE_BIP          10000   // ms
#define IR_MIN_NB_TENTATIVES        2       // Code bascule : si le chauffage est déjà dans l'état demandé, la 1re émission l'inverse
#define IR_MAX_NB_TENTATIVES        10

class CActionneurIR {
private:
  bool _initialized = false;
  unsigned char mucPin = ACTIONNEUR_IR_PIN;
  // Paramètres (NVS + page Web)
  uint8_t mucProtocole = IR_DEFAULT_PROTOCOLE;
  uint16_t muiAdresse = IR_DEFAULT_ADRESSE;
  uint8_t mucCommande = IR_DEFAULT_COMMANDE;
  uint8_t mucRepetitions = IR_DEFAULT_REPETITIONS;
  unsigned int muiAttenteBip = IR_DEFAULT_ATTENTE_BIP;
  unsigned int muiNbTentatives = IR_DEFAULT_NB_TENTATIVES;
  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;
  String mPrefixNVS = "ir_";  // Préfixe par défaut
  CMyDateTime *mDateTime=nullptr;

  // Séquence émission IR / attente du bip
  enum EtatSequence { SEQ_REPOS, SEQ_ATTENTE_BIP };
  EtatSequence mEtatSequence = SEQ_REPOS;
  int miConsigne = -1;              // 0 = OFF, 1 = ON
  unsigned int muiTentative = 0;    // Nb d'émissions de la séquence en cours
  unsigned long mulDebutAttente = 0L;

  // Publications en attente (MQTT pas encore connecté)
  int miEtatAPublier = -1;          // -1 : rien, 0/1 : état OnOff
  bool mbIrkoAPublier = false;
  unsigned long mulDerniereTentativePublication = 0L;

  void emet();
  void finSequence(int etatObtenu); // -1 = échec
  void gerePublications();
  bool publieIrko();
  void bornesParametres();

public:
  CActionneurIR(CMyDateTime& dateTime) : mDateTime(&dateTime) {}
  bool active = false;
  #ifdef CAPTEUR_MICRO_MAX4466
  CMax4466 *mMicro = nullptr; // Ecoute du bip et publication de l'état OnOff
  #endif

  // MQTT callback (publication IRKO)
  std::function<int(const char*, const char*)> onMqttPublish;
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish);

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void setActive(bool state);
  void setPrefixNVS(const char* pr) {mPrefixNVS = pr;}
  String getHTML();

  bool setup(const String pref, int ledPin = -1); // ledPin : LED allumée pendant l'émission (-1 = aucune)
  int loop();

  void envoie();                  // Emission brute du code IR
  int demandeEtat(int consigne);  // Lance la séquence émission / vérification par le bip
  bool onBip(int val);            // Appelée par le micro à chaque bip valide. true = bip pris en charge
  bool sequenceEnCours() const { return mEtatSequence != SEQ_REPOS; }
  int handleMqttCommand(const String& payload); // "<nom du micro> OFF|ON" → demandeEtat(0|1)

  void print() const;
};
#endif // ACTIONNEUR_IR
#endif // __CACTIONNEURIR_H__
