#ifndef __MYECRAN_H__
#define __MYECRAN_H__

#include "global.h"
#include <Preferences.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "MyConfig.h"

#include "ecranConstantesPosition.h"
#include "MyBouton.h"
#include "MyBoutonActif.h"
#include "MyBoutonSerie.h"
#include "MyZoneMesure.h"
#include "MyZoneMesureDouble.h"
#include "MyZoneStatus.h"
#include "MyZoneDateTime.h"
#include "MyZoneTitle.h"
#include "MyZoneMesureOnOff.h"

enum {
  BTN_PROJECTEUR_ACTIVE,
  BTN_PROJECTEUR_TOGGLE,
  BTN_GUIRLANDE_ACTIVE,
  BTN_GUIRLANDE_TOGGLE,
  BTN_CHAUFFAGE_ON_ACTIVE,
  BTN_CHAUFFAGE_ON,
  BTN_CHAUFFAGE_OFF_ACTIVE,
  BTN_CHAUFFAGE_OFF,
  BTN_CHAUDIERE_ON_ACTIVE,
  BTN_CHAUDIERE_ON,
  BTN_CHAUDIERE_OFF_ACTIVE,
  BTN_CHAUDIERE_OFF,
  BTN_SERIE,
  _TOTAL_BTN_
};

class CEcran {
private:
  TFT_eSPI tft;
  XPT2046_Touchscreen touch;
  SPIClass touchSPI;
  CConfig& mConfig;
  CMyBouton   mBtnProjecteur,
              mBtnGuirlande,
              mBtnChauffageON,
              mBtnChauffageOFF,
              mBtnChaudiereON,
              mBtnChaudiereOFF;

  CMyBoutonActif  mBtnProjecteurActif,
                  mBtnGuirlandeActif,
                  mBtnChauffageONActif,
                  mBtnChauffageOFFActif,
                  mBtnChaudiereONActif,
                  mBtnChaudiereOFFActif;

  CMyBoutonSerie mBtnSeries;

  CZoneMesure mZoneMesureOuest,
              mZoneMesureCentre,
              mZoneMesureEst;

  CZoneMesureDouble mZoneMesureDoubleEst;
  CZoneMesureDouble mZoneMesureDoubleCentre;

  CZoneMesureOnOff  mZoneFlotteur;

  CZoneStatus mZoneStatus;
  CZoneDateTime mZoneDateTime;
  CZoneTitle  mZoneTitle;

  CMyBoutonBase *mListBoutonsPtr[_TOTAL_BTN_]; // Liste de pointeurs vers les boutons.

  bool mbEcranAllume;
  // S'il y a plus de trois mesures à afficher, ces deux paramètres permettent de choisir la série à afficher
  unsigned char mucNbAffichagesMax = 1;
  unsigned char mucSerieAffichageEnCours = 1;

  unsigned long mulDernierTouch;

  Preferences prefs;
  const char* nvs_namespace = NVS_NAME_SPACE;
  String mPrefixNVS = "ecr_";  // Préfixe par défaut

public:
  CEcran(CConfig& cfg);
  const int32_t default_sleep_timeout = 300;  // -1 pour désactiver

  // Timeout avant écran de veille
  int32_t sleep_timeout = 300;
  const int8_t default_orientation_ecran = DEFAUT_ORIENTATION_ECRAN;  // Landscape inversé
  unsigned char mucOrientationEcran;

  void setPrefixNVS(const char* pr) {mPrefixNVS = pr;}

  void setup(const String pref);
  void loop();
  void drawMainInterface();

  void updateStatus(const String& msg, bool drawInterface=false, bool memorise=false);
  bool isTouched();
//  bool isTouched(int& x, int& y);
  int getPressedButton();

  void updateTitleWithIP(const String& ip);
  void updateAllStates();  // Nouvelle méthode pour afficher les 4 états
  unsigned char setOrientationEcran(unsigned char orientation=DEFAUT_ORIENTATION_ECRAN);
  unsigned char getOrientationEcran();
  void forceSleep();
  void forceWake();
  void desactiveChaudiere(bool bDrawInterface=true);
  void activeChaudiere(bool activ=true, bool bDrawInterface=true);
  void desactiveProjecteur(bool bDrawInterface=true);
  void activeProjecteur(bool activ=true, bool bDrawInterface=true);
  void desactiveGuirlande(bool bDrawInterface=true);
  void activeGuirlande(bool activ=true, bool bDrawInterface=true);
  void desactiveChauffageSb(bool bDrawInterface=true);
  void activeChauffageSb(bool activ=true, bool bDrawInterface=true);
  void updateSleepTimeout(int32_t st);
  String setLastStatusMessage(String& s);
  String getLastStatusMessage();
  void print() const;
  void setSerieAffichageEnCours(unsigned char num);
  void serieAffichagePlusUn();

  void saveToNVS();
  void loadFromNVS();
  void setSleepTimeout(int32_t st);
  void loadFromWebServer (WebServer& server);
  String getHTML();

  void updateAppareilsDeMesure();
  void updateThermometreLocal();
  void updateRemoteDevice_DS18B20(const String& nom, float val);
  void updateRemoteBat_DS18B20(const String& nom, int etatBatterie, float val);
  void updateRemoteDevice_ThCh1er(const String& nom, float val);
  void updateRemoteBat_ThCh1er(const String& nom, int etatBatterie, float val);
  void updateRemoteDevice_ThSdb(const String& nom, float val);
  void updateRemoteBat_ThSdb(const String& nom, int etatBatterie, float val);
  void updateRemoteDevice_ThCave(const String& nom, float val);
  void updateRemoteDevice_ThCaveH(const String& nom, float val);
  void updateRemoteDevice_ThCaveTor(const String& nom, int val);
  void updateRemoteBat_ThCave(const String& nom, int etatBatterie, float val);
  void updateRemoteDevice_ThNomade(const String& nom, float val);
  void updateRemoteDevice_ThNomadeH(const String& nom, float val);
  void updateRemoteBat_ThNomade(const String& nom, int etatBatterie, float val);
  void updateDateHeure(const String& date, const String& heure);
  void updateDateHeure();

  void eteindreEcran() {pinMode(TFT_BL, OUTPUT); digitalWrite(TFT_BL, LOW);}
  void allumerEcran() {pinMode(TFT_BL, OUTPUT); digitalWrite(TFT_BL, HIGH); delay(200);}
};

#endif // __MYECRAN_H__