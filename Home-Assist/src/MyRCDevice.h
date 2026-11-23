#ifndef MYRCDEVICE_H
#define MYRCDEVICE_H

#include "global.h"
#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include "MyRCSwitch.h"
#include "EquipementBase.h"
#include "RemotePresenceReseauElectrique.h"

class CEcran;  // Forward declaration

class CRCDevice : public CMyRCSwitch, public CEquipementBase  {
protected:
  CEcran* mEcran = nullptr;
  // Valeurs par défaut quand le NVS est vide (setDefauts()) ; valeurs historiques pour projecteur et guirlande
  unsigned long mulCodeDefaut = 0x3D3B72;
  int miProtocoleDefaut = 1;
  int miImpulsionDefaut = 349;
  String msNomDefaut = "";       // vide : celui de CEquipementBase::loadFromNVS()
  String msLibelleDefaut = "Projecteur";
  String msSubTopicDefaut = "";  // vide : celui de CEquipementBase::loadFromNVS()
  void branchePresence();        // Callbacks de mPresence vers l'appareil (setup())

public:
  CRCDevice() = default;

  CRCDevice(const String& nomEqu, CEcran& ecran,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(&ecran), onMqttPublish(cbonMqttPublish)
  {
    nomEquipement = nomEqu;
    mPrefixNVS = nomEquipement.substring(0, 2) + "_";  // Ex : "Pr_" pour Projecteur
    #ifndef __CYD__
    mEcran = nullptr;
    #endif
  }

  // Publication MQTT
  std::function<int(const char*, const char*)> onMqttPublish; 

  // Nœud de présence sur le réseau électrique (PrGuirlande...) : état réel de l'appareil, renvoi du code jusqu'à
  // ce que l'état voulu soit atteint. Sans nœud associé (nom vide sur la page Web) : fonctionnement historique.
  CRemotePresenceReseauElectrique mPresence;

  //bool etat = false;
  String etatStr = "OFF";
  String buttonName = "Appareil";

  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  void setDefauts(unsigned long codeDef, int protocoleDef, int impulsionDef, const String& nomDef,
                  const String& libelleDef, const String& subTopicDef); // Avant setup()
  void setup(const String pref);
  void loop();
  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  void saveState(bool state);
  void handleMqttCommand(const String& payload);
  int remonteStatusParMqtt(); // Envoie l'état actif/inactif ainsi que ON/OFF
  void print() const;
  String getHTML();
  virtual int envoiOnOff();
  int commande(bool voulu);       // État voulu (ON / OFF) : code envoyé seulement si l'état mesuré diffère
  int activeEquipement();

  //------------------------------------------------------------------------------
  // Mode maintenance (Home-Assist 4.4) : le nœud de l'appareil (PrGuirlande, NoeudChauffageSdb) est entretenu
  // (téléversement, page Web) ; le CYD alimente sa prise, suspend la logique de l'appareil, refuse ses commandes,
  // publie l'état ; à la fin (heure de fin ou OFF), remet l'appareil comme il faut (sortMaintenance()).
  //------------------------------------------------------------------------------
  std::function<int(const char*, const char*, bool)> onMqttPublishRetenu = nullptr;
  String msTopicMaintenance;       // Ordre de HA (vide : pas de maintenance)
  String msTopicMaintenanceEtat;   // État publié par le CYD (msTopicMaintenance + "/etat")
  void setTopicMaintenance(const String& topic);
  void handleMaintenance(const String& payload);
  bool refuseEnMaintenance();      // Commande refusée (message) si l'appareil est en maintenance
  void publieMaintenanceAux();     // « MAINTENANCER ON|OFF » pour les CYD auxiliaires

protected:
  int commandeDirecte(bool voulu); // commande() sans le contrôle de la maintenance
  // OFF_FORCE (4.8, comme CChaudiere) : appareil réactivé pour l'extinction, puis désactivé une fois éteint
  bool mbForcerDisable = false;
  void finForcerDisable();         // Désactivation demandée par OFF_FORCE, une fois la commande terminée
  bool mbOrdreMaintenanceLu = false;          // 4.7 : ordre retenu déjà lu (annonce aux CYD aux seulement après)
  bool mbEtatAvantMaintenance = false;
  bool mbEntreeMaintenanceEnAttente = false;  // Guirlande : état réel pas encore connu (démarrage du CYD)
  bool mbSortieMaintenanceEnAttente = false;  // Guirlande : état réel vérifié (PING) avant de remettre l'état d'avant
  String msEtatMaintenancePublie;
  void loopMaintenance();
  void publieEtatMaintenance(const String& sEtat);
  void quitteMaintenance(const String& sRaison);
  virtual String raisonRefusMaintenance();   // "" : maintenance possible
  virtual void entreMaintenance(bool bReprise);
  virtual void sortMaintenance();
  virtual String detailMaintenance();
};

#endif