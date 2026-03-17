#ifndef __REMOTECHAUDIERE_H__
#define __REMOTECHAUDIERE_H__

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "global.h"
#include "EquipementBase.h"


class CEcran;

class CRemoteChaudiere : public CEquipementBase {
private:
  CEcran *mEcran=nullptr;

public:
  CRemoteChaudiere() = default;
  CRemoteChaudiere(const String& nomEqu, CEcran& ecran,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(&ecran), onMqttPublish(cbonMqttPublish) {
    nomEquipement = nomEqu;
    mPrefixNVS = nomEquipement.substring(0, 2);
   
  }

  // Publication MQTT
  std::function<int(const char*, const char*)> onMqttPublish; 
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT
  
  const bool default_boiler_state = false;
  const uint8_t default_enable_keep_alive = true;

  const char* default_topic_prefix = "chaudiere/";

  String nomBoutonON = "CHAUD. ON";
  String nomBoutonOFF = "CHAUD. OFF";

  //bool active = true;

  //bool etat = false;
  String etatStr = "OFF";

  void setup();
  void loop();
  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void setEtatReelOnOff(bool state);
  void saveState(bool state);
  //void setActive(bool state);
  int envoiTrameON(); // Bouton ON appuyé
  int envoiTrameOFF(); // Bouton OFF appuyé
  int activeEquipement();
  int allumer();
  int eteindre();
  String getHTML();

  void handleMqttCommand(const String& payload);
  void print() const;

  //-----------------------------------------------------------------
  //  SECTION POUR l'ENVOI DES TRAMES
  //
  //  
  //-----------------------------------------------------------------
  bool mbEnvoyerTramesON = false;
  bool mbEnvoyerTramesOFF = false;
  int miCompteurEnvois = 0;
  int muiKeepAliveIndexEnCours = 0; // Index du keep-alive anvoyé
  bool mbKeepAliveActive = false;
  bool mbTrameCommandeEnvoyee = false; // Premiere trame ON ou OFF déjà envoyée

  void setup_envoi_trames();
  bool bSetEnvoyerTramesON(bool b);
  bool bSetEnvoyerTramesOFF(bool b);
  bool bGetEnvoiEnCours(void);

  void envoiCommande(const uint16_t* pulses, int nbPulses, const char* action);
  //-----------------------------------------------------------------
  //  SECTION POUR l'ENVOI DES TRAMES ------ FIN
  //-----------------------------------------------------------------

};

#endif // __REMOTECHAUDIERE_H__