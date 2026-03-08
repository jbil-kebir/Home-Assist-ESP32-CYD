#ifndef MYCHAUDIERE_H
#define MYCHAUDIERE_H

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "MyRadioTx.h"
#include "global.h"

#define MAX_KEEPALIVE 10

class CEcran;
class CRadioTX;

class CChaudiere : public CRadioTX, public CEquipementBase {
private:
  CEcran *mEcran=nullptr;

public:
  CChaudiere() = default;
  CChaudiere(const String& nomEqu, CEcran& ecran,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(&ecran), onMqttPublish(cbonMqttPublish) {
    nomEquipement = nomEqu;
    mPrefixNVS = nomEquipement.substring(0, 2);
    #ifndef __CYD__
    mEcran = nullptr;
    #endif
   
  }

  // Publication MQTT
  std::function<int(const char*, const char*)> onMqttPublish; 
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  const bool default_boiler_state = false;
  const uint8_t default_enable_keep_alive = true;


  // Timings par défaut (10 cases, les dernières à 0)
  const uint32_t default_on_keepalive[MAX_KEEPALIVE] = {7, 14, 56, 85, 626, 0, 0, 0, 0, 0};
  const uint32_t default_off_keepalive[MAX_KEEPALIVE] = {30, 64, 77, 626, 0, 0, 0, 0, 0, 0};
  const char* default_topic_prefix = "chaudiere/";

  String nomBoutonON = "CHAUD. ON";
  String nomBoutonOFF = "CHAUD. OFF";

  float frequency = 434.038;
  bool enableKeepAlive = true;
  uint32_t onKeepalive[MAX_KEEPALIVE] = {7, 14, 56, 85, 626, 0, 0, 0, 0, 0};
  uint32_t offKeepalive[MAX_KEEPALIVE] = {30, 64, 77, 626, 0, 0, 0, 0, 0, 0};
  uint8_t on_keepalive_count = 5;
  uint8_t off_keepalive_count = 4;
  //bool etat = false;
  String etatStr = "OFF";


  void setup();
  void loop();
  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void saveState(bool state);
  void setActive(bool state);
  int  remonteStatusParMqtt();
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
  void loop_envoi_trames();
  int isetUpStatus(bool bStatus);
  bool bSetEnvoyerTramesON(bool b);
  bool bSetEnvoyerTramesOFF(bool b);
  bool bGetEnvoiEnCours(void);

  void envoiCommande(const uint16_t* pulses, int nbPulses, const char* action);
  //-----------------------------------------------------------------
  //  SECTION POUR l'ENVOI DES TRAMES ------ FIN
  //-----------------------------------------------------------------

};

#endif // MYCHAUDIERE_H