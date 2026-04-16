#ifndef __REMOTE_TCS34725_H__
#define __REMOTE_TCS34725_H__

#include <OneWire.h>
#include <WebServer.h>
#include <Preferences.h>
#include <functional>
#include "global.h"
#include "MyEcran.h"
#include "RCSwitchCodeAnalyzer.h"
#include "EquipementBase.h"

class CRemoteTCS34725 : public CRCSwitchCodeAnalyzer, public CEquipementBase  {
private:
  CEcran *mEcran=nullptr;
  // On mémorise la température pour n'afficher que les changements
  unsigned long lastRgb = 0x0UL;  // Valeur invalide par défaut. Une couleur valide sera de la forme 0xFFRRGGBB
  unsigned long newRgb = 0x0UL;  // Valeur invalide par défaut
  unsigned int muiLastR = 0;
  unsigned int muiLastG = 0;
  unsigned int muiLastB = 0;
  unsigned int muiNewR = 0;
  unsigned int muiNewG = 0;
  unsigned int muiNewB = 0;

  // On mémorise la température pour n'afficher que les changements
  unsigned int muiLastLux = 0x0;  // Valeur invalide par défaut. Une luminosité sera envoyée sous la forme 0xFFFFFFLM (LM = Luminosité)
  unsigned int muiNewLux = 0x0;  // Valeur invalide par défaut
  // On mémorise la température pour n'afficher que les changements
  unsigned int muiLastLuxBrut = 0x0;  // Valeur invalide par défaut. Une luminosité sera envoyée sous la forme 0xFFFFFFLM (LM = Luminosité)
  unsigned int muiNewLuxBrut = 0x0;  // Valeur invalide par défaut
  // Etet ON/OFF
  int mbNewEtatOnOff = -1; // Mis à jour par calcul. N'a pas besoin d'être dans le NVS. true = 1 ON, false = 0 OFF, -1 valeur invalide
  int mbLastEtatOnOff = -1; // Mis à jour par calcul. N'a pas besoin d'être dans le NVS. true = 1 ON, false = 0 OFF, -1 valeur invalide

  //====================== Commandes MQTT ======================
  String sMqttCommandMesure = "MESURE"; // Force une mesure

  // En secondes pour les tests. Si l'appareil ne répond pas au delà de cet intervalle, 
  // il est considéré comme KO
  unsigned long mulWatchdogIntervalle = 60; 
  unsigned long mulWatchdogDefaultIntervalle = 60; 
  unsigned long mulWatchDog;
  unsigned char mucCapteurID=0;
  // Codes RCSwitch
  #define NB_MESURES  2 // Température et humidité
  #define NB_CODES_RCS  (2*NB_MESURES) // MSB et LSB ==> donc 2 codes
  unsigned long mulCodes[NB_CODES_RCS];
  _ST_MESURE_ mStMesures[NB_CODES_RCS]; // Mesures décodées
  unsigned char mucNbCodesRecus = 0; // Nombre de codes reçus

public:
  CRemoteTCS34725(const String& nomEqu, CEcran* ecran, 
      std::function<void(const String&, unsigned long)> callbackCouleur = nullptr, 
      std::function<void(const String&, unsigned long)> callbackLuminosite = nullptr,
      std::function<int(const char*, const char*)> cbonMqttPublish = nullptr) : mEcran(ecran), onCouleurChanged(callbackCouleur), onLuminositeChanged(callbackLuminosite), onMqttPublish(cbonMqttPublish) {
      nomEquipement = nomEqu;
    } 

  
  // Callback : void(String expediteur, float temperature)
  std::function<void(const String&, unsigned long)> onCouleurChanged;    
  std::function<void(const String&, unsigned long)> onLuminositeChanged;    
  // MQTT
  std::function<int(const char*, const char*)> onMqttPublish;    

  void setDisplayCallbackCouleur(std::function<void(const String&, unsigned long)> cb); // Pour mise à jour de l'affichage
  void setDisplayCallbackLuminosite(std::function<void(const String&, unsigned long)> cb); // Pour mise à jour de l'affichage
  void setMqttPublishCallback(std::function<int(const char* topic, const char* payload)> cbMqttPublish); // Pour publication MQTT

  void loadFromNVS();
  void loadFromWebServer (WebServer& server);
  void saveToNVS();
  String getHTML();

  int begin(const String pref);
  int loop();
  bool readMesures();
  int getLastLedStatus() const;
  unsigned long getLastCouleur() const;
  unsigned long getLastLuminosite() const;
  unsigned long getLastLuminositeBrut() const;
  void print() const;
  void printMesures();
  void handleMqttCommand(const String& payload);
  void handleMqttState(const String& payload);
  int handleRCSwitchCode(unsigned long code);
  bool remonteStatusParMqtt();
  bool remonteCouleurParMqtt();
  bool remonteLuminositeParMqtt();
  bool couleurValide(unsigned long coul);
  bool lumValide(unsigned long lum);

//  bool remonteTensionParMqtt();

};

#endif // __REMOTE_TCS34725_H__