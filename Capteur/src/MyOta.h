#ifndef __MY_OTA_H__
#define __MY_OTA_H__

#include <Arduino.h>
#include <functional>
#include <WebServer.h>

//------------------------------------------------------------------------------
// Mise à jour du firmware à distance (OTA), 21/11/2026 — Relais-Lora 2.9, Home-Assist (CYD) 4.3, Capteur 4.5 ;
// copie identique dans chaque projet (comme MyJournal) : toute correction est à reporter dans les autres
//
// Deux façons d'envoyer le firmware, protégées par le même mot de passe (OTA_MOT_DE_PASSE, secret.h) :
//   - A : PlatformIO par le réseau (ArduinoOTA, profil *_ota de platformio.ini) ;
//   - B : page Web /update (navigateur, téléphone) : le fichier doit porter l'étiquette de cet appareil
//         (ETIQUETTE_FIRMWARE, voir MyOta.cpp), sinon il est refusé.
// Retour arrière (chargeur Arduino 3.x : CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE) : un nouveau firmware est « à
// l'essai » ; il n'est validé qu'une fois MQTT connecté (confirme()). Pas de connexion en OTA_DELAI_VALIDATION_S
// (minuterie indépendante de loop(), qui couvre aussi un setup() bloqué), ou plantage avant : retour à l'ancien.
// Chaque étape est publiée, retenue, sur home/ota/<appareil> (Home Assistant : notification + mail du résultat) :
//   DEBUT <version> <date heure> (page Web | PlatformIO)
//   ECHEC <version> <date heure> : <raison>
//   TRANSFERT <version> <date heure> : terminé, redémarrage
//   OK <nouvelle version> <date heure>                      (après redémarrage, MQTT connecté)
//   RETOUR <version> <date heure> : <raison>                (l'ancien firmware a repris la main)
//   ATTENTE <version> <date heure> : <détail>               (nœud : commande OTA, prêt à recevoir, annonce())
// Nœud LoRa : pas de MQTT ; publie() passe par le relais (trame LoRa « home/ota/<nom> … »), confirme() est appelé
// après la 1re émission LoRa, et ArduinoOTA ne démarre qu'avec le point d'accès (demarreArduinoOta()).
//------------------------------------------------------------------------------
#define OTA_DELAI_VALIDATION_S  120   // Délai laissé au nouveau firmware pour se connecter à MQTT
#define OTA_TOPIC_RACINE        "home/ota/"

class COta {
public:
  // Tout début de setup() : nouveau firmware à l'essai → minuterie de retour arrière armée
  void debutDemarrage();
  // Après la connexion Wi-Fi. nomTopic : fin du topic (home/ota/<nomTopic>) ; publie(topic, message) : publication
  // MQTT retenue ; horodatage() : « 21/11/2026 15:02 »
  // demarrerArduinoOta = false : réseau pas encore là (nœud LoRa) ; demarreArduinoOta() plus tard
  void begin(const String& nomTopic, const char* motDePasse, const String& version,
             std::function<void(const char*, const char*)> publie, std::function<String()> horodatage,
             bool demarrerArduinoOta = true);
  void demarreArduinoOta();
  void annonce(const String& etape, const String& detail) { publie(etape, detail); }
  bool estConfirme() const { return mbConfirme; }
  // MQTT connecté : valide le firmware à l'essai, publie OK ou RETOUR (une fois)
  void confirme();
  void ajoutePagesWeb(WebServer& server);  // /update (GET : page, POST : envoi du fichier)
  void loop();                             // ArduinoOTA

private:
  String msNomTopic, msMotDePasse, msVersion;
  std::function<void(const char*, const char*)> mPublie;
  std::function<String()> mHorodatage;
  bool mbConfirme = false;
  // Page /update : étiquette cherchée dans le flux reçu (la fin du paquet précédent est gardée)
  String msFinPrecedente;
  String msEtiquetteTrouvee;   // Appareil annoncé par le fichier ("" : aucune étiquette)
  bool mbEtiquetteOk = false;
  bool mbEnvoiRefuse = false;
  String msErreurEnvoi;

  void publie(const String& etape, const String& detail);
  void noteMiseAJour(const char* source);   // Avant le redémarrage : ce qu'il faudra confirmer ensuite
  void chercheEtiquette(const uint8_t* data, size_t len);
};

extern COta gOta;

#endif // __MY_OTA_H__
