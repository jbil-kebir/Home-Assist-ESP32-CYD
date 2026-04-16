#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <WiFi.h>

#include "MyConfig.h"
#include "global.h"
#include "MyEcran.h"

  CEcran::CEcran(CConfig& cfg) : tft(TFT_eSPI()), touch(XPT2046_CS, XPT2046_IRQ), touchSPI(HSPI), mConfig(cfg), 
    mBtnProjecteur(cfg, tft, touch, BOUTON_P_X, BOUTON_P_Y, BOUTON_P_W, BOUTON_P_H, TFT_DARKGREY, BOUTON_P_N), 
    mBtnGuirlande(cfg, tft, touch, BOUTON_G_X, BOUTON_G_Y, BOUTON_G_W, BOUTON_G_H, TFT_DARKGREY, BOUTON_G_N),
    mBtnChauffageON(cfg, tft, touch, BOUTON_SB_ON_X, BOUTON_SB_ON_Y, BOUTON_SB_ON_W, BOUTON_SB_ON_H, TFT_DARKGREY, BOUTON_SB_ON_N),
    mBtnChauffageOFF(cfg, tft, touch, BOUTON_SB_OFF_X, BOUTON_SB_OFF_Y, BOUTON_SB_OFF_W, BOUTON_SB_OFF_H, TFT_DARKGREY, BOUTON_SB_OFF_N),
    mBtnChaudiereON(cfg, tft, touch, BOUTON_ON_X, BOUTON_ON_Y, BOUTON_ON_W, BOUTON_ON_H, TFT_DARKGREY, BOUTON_ON_N),
    mBtnChaudiereOFF(cfg, tft, touch, BOUTON_OFF_X, BOUTON_OFF_Y, BOUTON_OFF_W, BOUTON_OFF_H, TFT_DARKGREY, BOUTON_OFF_N),

    mBtnProjecteurActif(cfg, tft, touch, BOUTON_PA_X, BOUTON_PA_Y, BOUTON_PA_W, BOUTON_PA_H, TFT_GREENYELLOW, BOUTON_PA_N),
    mBtnGuirlandeActif(cfg, tft, touch, BOUTON_GA_X, BOUTON_GA_Y, BOUTON_GA_W, BOUTON_GA_H, TFT_GREENYELLOW, BOUTON_GA_N),
    mBtnChauffageONActif(cfg, tft, touch, BOUTON_SBA_ON_X, BOUTON_SBA_ON_Y, BOUTON_SBA_ON_W, BOUTON_SBA_ON_H, TFT_GREENYELLOW, BOUTON_SBA_ON_N),
    mBtnChauffageOFFActif(cfg, tft, touch, BOUTON_SBA_OFF_X, BOUTON_SBA_OFF_Y, BOUTON_SBA_OFF_W, BOUTON_SBA_OFF_H, TFT_GREENYELLOW, BOUTON_SBA_OFF_N),
    mBtnChaudiereONActif(cfg, tft, touch, BOUTON_ONA_X, BOUTON_ONA_Y, BOUTON_ONA_W, BOUTON_ONA_H, TFT_GREENYELLOW, BOUTON_ONA_N),
    mBtnChaudiereOFFActif(cfg, tft, touch, BOUTON_OFFA_X, BOUTON_OFFA_Y, BOUTON_OFFA_W, BOUTON_OFFA_H, TFT_GREENYELLOW, BOUTON_OFFA_N),
      
    mBtnSeries(tft, touch, BOUTON_SERIE_X, BOUTON_SERIE_Y, BOUTON_SERIE_W, BOUTON_SERIE_H, TFT_BLUE, "0"),
    
    mZoneMesureOuest(tft, REMOTE_1_X, REMOTE_1_Y, REMOTE_1_W, REMOTE_1_H, REMOTE_1_BG_C),
    mZoneMesureDoubleOuest(tft, REMOTE_1_X, BOUTON_PA_Y, REMOTE_1_W, REMOTE_1_H, REMOTE_1_BG_C),
    
    mZoneMesureCentre(tft, REMOTE_2_X, REMOTE_2_Y, REMOTE_2_W, REMOTE_2_H, REMOTE_2_BG_C),
    mZoneMesureDoubleCentre(tft, REMOTE_2_X, REMOTE_2_Y, REMOTE_2_W, REMOTE_2_H, REMOTE_2_BG_C),

    mZoneMesureEst(tft, LOCAL_1_X, LOCAL_1_Y, LOCAL_1_W, LOCAL_1_H, LOCAL_1_BG_C),
    mZoneMesureDoubleEst(tft, REMOTE_3_X, REMOTE_3_Y, REMOTE_3_W, REMOTE_3_H, REMOTE_3_BG_C),

    mZoneStatus(tft, STATUS_INT_X, STATUS_INT_Y, STATUS_INT_W, STATUS_INT_H, STATUS_INT_C),
    mZoneStatusBas(tft, STATUS_INT_X, STATUS_INT_Y+70, STATUS_INT_W-50, STATUS_INT_H, STATUS_INT_C),
    mZoneDateTime(cfg, tft, DATE_HEURE_X, DATE_HEURE_Y, DATE_HEURE_W, DATE_HEURE_H, DATE_HEURE_BG_C),
    mZoneTitle(cfg, tft, TITRE_X, TITRE_Y, TITRE_W, TITRE_H, TITRE_C),
    mZoneFlotteur(tft, REMOTE_2_X, (REMOTE_2_Y + REMOTE_2_H + 2 + MESURE_2_HAUTEUR + 2), REMOTE_2_W, REMOTE_2_H, REMOTE_2_BG_C),
    mZoneTorNomade(tft, REMOTE_2_X, (REMOTE_2_Y + REMOTE_2_H + 2 + MESURE_2_HAUTEUR + 2 + MESURE_2_HAUTEUR + 2), REMOTE_2_W, REMOTE_2_H, REMOTE_2_BG_C),
    mZoneCouleurSdb(tft, REMOTE_1_X, (REMOTE_1_Y + REMOTE_1_H + 2 + MESURE_2_HAUTEUR + 2), REMOTE_1_W, REMOTE_1_H, REMOTE_1_BG_C),
    mZoneHa1(tft, SERIE4_COL_RIGHT_X, SERIE4_ROW1_Y, SERIE4_COL_W, SERIE4_ROW_H, REMOTE_1_BG_C),
    mZoneHa2(tft, SERIE4_COL_RIGHT_X, SERIE4_ROW2_Y, SERIE4_COL_W, SERIE4_ROW_H, REMOTE_1_BG_C),
    mZoneAffichageIP(tft, mvsControleurs, mvsEsclaves, PREMIERE_LIGNE_DE_BOUTONS_X, PREMIERE_LIGNE_DE_BOUTONS_Y, RESOLUTION_X-PREMIERE_LIGNE_DE_BOUTONS_X, RESOLUTION_Y-PREMIERE_LIGNE_DE_BOUTONS_Y, REMOTE_1_BG_C)
    {
    }

  void CEcran::setup(const String pref, std::vector<CIPModule> *ctrl, std::vector<CIPModule> *escl) {
    mPrefixNVS = pref;
    mvsControleurs = ctrl;
    mvsEsclaves = escl;
    mZoneAffichageIP.mvsControleurs = mvsControleurs;
    mZoneAffichageIP.mvsEsclaves = mvsEsclaves;

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    #ifdef __LOCAL_MODE__
    mBtnProjecteur.mDevice = mConfig.projecteur;
    mBtnGuirlande.mDevice = mConfig.guirlande;
    mBtnChauffageON.mDevice = mConfig.chauffageSb;
    mBtnChauffageOFF.mDevice = mConfig.chauffageSb;
    mBtnChaudiereON.mDevice = mConfig.chaudiere;
    mBtnChaudiereOFF.mDevice = mConfig.chaudiere;

    mBtnProjecteurActif.mDevice = mConfig.projecteur;
    mBtnGuirlandeActif.mDevice = mConfig.guirlande;
    mBtnChauffageONActif.mDevice = mConfig.chauffageSb;
    mBtnChauffageOFFActif.mDevice = mConfig.chauffageSb;
    mBtnChaudiereONActif.mDevice = mConfig.chaudiere;
    mBtnChaudiereOFFActif.mDevice = mConfig.chaudiere;
    #else
    mBtnProjecteur.mDevice = mConfig.mRemoteProjecteur;
    mBtnGuirlande.mDevice = mConfig.mRemoteGuirlande;
    mBtnChauffageON.mDevice = mConfig.mRemoteChauffage;
    mBtnChauffageOFF.mDevice = mConfig.mRemoteChauffage;
    mBtnChaudiereON.mDevice = mConfig.mRemoteChaudiere;
    mBtnChaudiereOFF.mDevice = mConfig.mRemoteChaudiere;

    mBtnProjecteurActif.mDevice = mConfig.mRemoteProjecteur;
    mBtnGuirlandeActif.mDevice = mConfig.mRemoteGuirlande;
    mBtnChauffageONActif.mDevice = mConfig.mRemoteChauffage;
    mBtnChauffageOFFActif.mDevice = mConfig.mRemoteChauffage;
    mBtnChaudiereONActif.mDevice = mConfig.mRemoteChaudiere;
    mBtnChaudiereOFFActif.mDevice = mConfig.mRemoteChaudiere;
    #endif

    int i = 0;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnProjecteurActif; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnProjecteur; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnGuirlandeActif; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnGuirlande; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnChauffageONActif; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnChauffageON; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnChauffageOFFActif; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnChauffageOFF; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnChaudiereONActif; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnChaudiereON; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnChaudiereOFFActif; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnChaudiereOFF; i++;
    mListBoutonsPtr[i] = (CMyBoutonBase*)&mBtnSeries; i++;

    // Charge les paramètres liés à l'écran
    //loadFromNVS ();

    // Initiallisation de l'écran
    tft.init();
//    tft.setRotation(mucOrientationEcran);
    tft.fillScreen(TFT_NAVY);
    tft.setTextColor(TFT_YELLOW);
    tft.drawCentreString("Home Assistant", 160, 40, 4);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString("Initialisation...", 160, 80, 2);

    // Initiallisation de la partie tactile de l'écran
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);

    // Charge les paramètres liés à l'écran
    loadFromNVS ();
    tft.setRotation(mucOrientationEcran);
    touch.setRotation(mucOrientationEcran);

    mbEcranAllume = true; // Pas de veille à l'allumage
    mulDernierTouch = millis();

    updateStatus("Pret");
  }
  void CEcran::loop() {
    // === Gestion de la veille légère ===
    isTouched();
    if (sleep_timeout > 0) {  // Veille activée
      if (mbEcranAllume && millis() - mulDernierTouch > (unsigned long)sleep_timeout * 1000UL) {
        updateStatus("Veille ecran...");
        delay(1000);
        digitalWrite(TFT_BL, LOW);
        mbEcranAllume = false;
      }
    }

    // Date et heure
    static unsigned long ulIntervalleDateHeure=0;
    unsigned long tempsRafraichissement = 1000UL;
    if (millis() - ulIntervalleDateHeure >= tempsRafraichissement) { // On affiche l'heure régulièrement
      mZoneDateTime.drawDateTime();
      ulIntervalleDateHeure = millis();
    }
  }

unsigned char CEcran::setOrientationEcran(unsigned char orientation/*=DEFAUT_ORIENTATION_ECRAN*/) {
  unsigned char ucLastOrientation = mucOrientationEcran;
  if (orientation >= 0 && orientation <= 3) {
    tft.setRotation(orientation);
    touch.setRotation(orientation);
    drawMainInterface();  // Redessine tout
    DBGLN(DBG_ECRAN, "Rotation ecran change a : " + String(orientation));
  }

  mucOrientationEcran = orientation;
  prefs.begin(nvs_namespace, false);
  // Ecriture de l'orientation de l'écran
  prefs.putInt("or_ecran", mucOrientationEcran);
  prefs.end();
 
  return ucLastOrientation;
}
unsigned char CEcran::getOrientationEcran() {
  return mucOrientationEcran;
}
void CEcran::forceSleep() {
  updateStatus("Veille forcee par MQTT", false);
  digitalWrite(TFT_BL, LOW);
  mbEcranAllume = false;
}

void CEcran::forceWake() {
  mulDernierTouch = millis();
  digitalWrite(TFT_BL, HIGH);
  mbEcranAllume = true;
  drawStatus("Reveille par MQTT", false);
  delay(2000);
}

void CEcran::updateSleepTimeout(int32_t st) {
  forceWake();
  String s = "Mise en veille : " + String(st) + "s a partir de maintenant";
  drawStatus(s, false);
  mulDernierTouch = millis(); // On compte à partir de maintenant
}
void CEcran::desactiveChaudiere(bool bDrawInterface/*=true*/) {
  drawMainInterface();
  DBG(DBG_ECRAN, "CEcran::desactiveChaudiere()\n");
  drawStatus("Chaudiere desactivee", false);
}
void CEcran::activeChaudiere(bool activ/*=true*/, bool bDrawInterface/*=true*/){
  if (!activ) {
    desactiveChaudiere (bDrawInterface);
    return;
  }
  drawMainInterface();
  DBG(DBG_ECRAN, "CEcran::activeChaudiere()\n");
  drawStatus("Chaudiere activee", false);
}
void CEcran::desactiveProjecteur(bool bDrawInterface/*=true*/) {
  drawMainInterface();
  drawStatus("Projecteur desactive", false);
}
void CEcran::activeProjecteur(bool activ/*=true*/, bool bDrawInterface/*=true*/){
  if (!activ) {
    desactiveProjecteur (bDrawInterface);
    return;
  }
  drawMainInterface();
  drawStatus("Projecteur active", false);
}
void CEcran::desactiveGuirlande(bool bDrawInterface/*=true*/) {
  drawMainInterface();
  drawStatus("Guirlande desactivee", false);
}
void CEcran::activeGuirlande(bool activ/*=true*/, bool bDrawInterface/*=true*/) {
  if (!activ) {
    desactiveGuirlande (bDrawInterface);
    return;
  }
  drawMainInterface();
  drawStatus("Guirlande activee", false);
}
void CEcran::desactiveChauffageSb(bool bDrawInterface/*=true*/) {
  drawMainInterface();
  drawStatus("Chauffage desactive", false);
}
void CEcran::activeChauffageSb(bool activ/*=true*/, bool bDrawInterface/*=true*/) {
  DBG(DBG_ECRAN, "activ : %d - drwInterface : %d\n", activ, bDrawInterface);
  if (!activ) {
    desactiveChauffageSb (bDrawInterface);
    return;
  }
  drawMainInterface();
  drawStatus("Chauffage active", false);
}


int CEcran::drawStatus(const String& msg, bool memorise/*=false*/) {
  /*if (mucSerieAffichageEnCours == 4)
    mZoneStatusBas.drawStatus(msg, memorise);
  else */mZoneStatus.drawStatus(msg, memorise);
  return 0;
}

int CEcran::drawStatus() {
  if (mucSerieAffichageEnCours == 4)
    mZoneStatusBas.drawStatus();
  else mZoneStatus.drawStatus();
  return 0;
}

bool CEcran::isTouched() {
  if (touch.tirqTouched() && touch.touched()) {
    mulDernierTouch = millis();  // Réinitialise le timer
  // Si l'écran était éteint, on le rallume
    if (!mbEcranAllume) {
      digitalWrite(TFT_BL, HIGH);
      mbEcranAllume = true;
      drawMainInterface();
      drawStatus();
    }
    return true;
  }
  return false;
}


int CEcran::getPressedButton() {

  for (int i = BTN_PROJECTEUR_ACTIVE; i < _TOTAL_BTN_; i++) {
    CMyBoutonBase *btnPtr = mListBoutonsPtr[i];
    if (btnPtr == nullptr) continue;
    if (btnPtr->isPressed()) return i;
  } 
  return -1;
}



void CEcran::print() const {

  DBG(DBG_ECRAN, "[Veille ecran]\n");
  if (sleep_timeout <= 0) {
    DBG(DBG_ECRAN, "  Désactivee\n");
  } else {
    DBG(DBG_ECRAN, "  Timeout      : %ld secondes (%ld minutes)\n", sleep_timeout, sleep_timeout / 60);
  }
  DBG(DBG_ECRAN, "\n");
  DBG(DBG_ECRAN, "[Orientation ecran]\n");
  switch(mucOrientationEcran) {
    case 0:
    DBG(DBG_ECRAN, "Portrait\n");
    break;
    case 1:
    DBG(DBG_ECRAN, "Paysage\n");
    break;
    case 2:
    DBG(DBG_ECRAN, "Portrait inverse\n");
    break;
    case 3:
    DBG(DBG_ECRAN, "Paysage inverse\n");
    break;
    default:
    DBG(DBG_ECRAN, "Orientation inconnue %d\n", mucOrientationEcran);
    break;
  }
  DBG(DBG_ECRAN, "\n");
  DBG(DBG_ECRAN, "[Série d'affichage]\n");
  DBG(DBG_ECRAN, "  Max      : %d\n", mucNbAffichagesMax);
  DBG(DBG_ECRAN, "  En cours : %d\n", mucSerieAffichageEnCours);

}

void CEcran::saveToNVS() {
  prefs.begin(nvs_namespace, false);

  // Écriture timeout veille
  prefs.putInt("sleep_timeout", sleep_timeout);
  // Ecriture de l'orientation de l'écran
  prefs.putInt("or_ecran", mucOrientationEcran);
  // Ecriture des paramètre de srie d'affichage
  prefs.putUChar("aff_max", mucNbAffichagesMax);
  prefs.putUChar("aff", mucSerieAffichageEnCours);

  prefs.end();
}
void CEcran::loadFromNVS() {
  prefs.begin(nvs_namespace, false);  // read/write

  // Timeout veille
  sleep_timeout = prefs.getInt("sleep_timeout", default_sleep_timeout);
  // Orientation écran
  mucOrientationEcran = prefs.getInt("or_ecran", default_orientation_ecran);
  // Lecture des paramètre de srie d'affichage
  mucNbAffichagesMax = prefs.getUChar("aff_max", 1);
  unsigned char ucSerieAffichageEnCours = prefs.getUChar("aff", 1);
  if (ucSerieAffichageEnCours > mucNbAffichagesMax) 
    ucSerieAffichageEnCours = mucNbAffichagesMax;

  setSerieAffichageEnCours(ucSerieAffichageEnCours);
  //mucSerieAffichageEnCours = ucSerieAffichageEnCours;
 
  prefs.end();  
}
void CEcran::setSleepTimeout(int32_t st) {
  sleep_timeout = st;
  prefs.begin(nvs_namespace, false);

  // Écriture timeout veille
  prefs.putInt("sleep_timeout", sleep_timeout);

  prefs.end(); 
  updateSleepTimeout(st);
}

void CEcran::loadFromWebServer (WebServer& server) {
  if (server.hasArg((mPrefixNVS+"sleep_timeout").c_str())) sleep_timeout = server.arg((mPrefixNVS+"sleep_timeout").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"or_ecran").c_str())) mucOrientationEcran = server.arg((mPrefixNVS+"or_ecran").c_str()).toInt();
  if (server.hasArg((mPrefixNVS+"aff_max").c_str())) mucNbAffichagesMax = server.arg((mPrefixNVS+"aff_max")).toInt();
  if (server.hasArg((mPrefixNVS+"aff").c_str())) mucSerieAffichageEnCours = server.arg((mPrefixNVS+"aff")).toInt();
  if (mucSerieAffichageEnCours > mucNbAffichagesMax) 
  setSerieAffichageEnCours(mucNbAffichagesMax);

}


String CEcran::getHTML() {
  String html = "";
  html =   "<h2>Ecran</h2>"
  "<div class=\"row\">"
    "<div><label>Timeout veille (secondes, -1 = désactivée)</label><input type=\"number\" name=" + mPrefixNVS + String("sleep_timeout") + " value=\"" + String(sleep_timeout) + "\"></div>"
    "<div><label>Orientation (0 à 3)</label><input type=\"number\" name=" + (mPrefixNVS + "or_ecran").c_str() + " value=\"" + String(mucOrientationEcran) + "\"></div>"
  "</div>"
  "<div class=\"row\">"
    "<div><label>Nb max de séries d'affichages (1-255)</label><input type=\"text\" name=" + (mPrefixNVS + "aff_max").c_str() + " value=\"" + mucNbAffichagesMax + "\"></div>"
    "<div><label>Série en cours</label><input type=\"text\" name=" + (mPrefixNVS + "aff").c_str() + " value=\"" + mucSerieAffichageEnCours + "\"></div>"
  "</div>";

  return html;
}

void CEcran::drawMainInterface() {
  tft.fillScreen(COULEUR_FOND_ECRAN);
  mZoneTitle.drawTitle();


  /*Serial.printf("void CEcran::drawMainInterface() mucSerieAffichageEnCours %d (YEst, YCentre, YOuest) = (%d, %d, %d) (MsgYEst, MsgYCentre, MsgYOuest) = (%d, %d, %d)\n", mucSerieAffichageEnCours, 
      mZoneMesureEst.muiPosY, mZoneMesureCentre.muiPosY, mZoneMesureOuest.muiPosY, mZoneMesureEst.muiValMesureY, mZoneMesureCentre.muiValMesureY, mZoneMesureOuest.muiValMesureY);*/

  if (mucSerieAffichageEnCours < 3) {
    mBtnProjecteurActif.draw();
    mBtnProjecteur.draw();

    mBtnGuirlandeActif.draw();
    mBtnGuirlande.draw();

    mBtnChauffageONActif.draw();
    mBtnChauffageON.draw();
    mBtnChauffageOFFActif.draw();
    mBtnChauffageOFF.draw();

    mBtnChaudiereONActif.draw();
    mBtnChaudiereON.draw();
    mBtnChaudiereOFFActif.draw();
    mBtnChaudiereOFF.draw();
  }
  if (mucSerieAffichageEnCours <= 3) {
    switch(mucSerieAffichageEnCours) {
    case 1: // RAS
      mZoneMesureDoubleEst.muiPosY = LOCAL_1_Y;
      mZoneMesureDoubleCentre.muiPosY = REMOTE_2_Y;
    break;
    case 2: // RAS
      mZoneMesureDoubleEst.muiPosY = LOCAL_1_Y;
      mZoneMesureDoubleCentre.muiPosY = REMOTE_2_Y;
    break;
    case 3: // On change la disposition des zones d'affichage
      mZoneMesureDoubleEst.muiPosY = BOUTON_PA_Y;
      mZoneMesureDoubleCentre.muiPosY = BOUTON_PA_Y;
    break;
    }
    mZoneMesureDoubleEst.calculeCoordonnees();
    mZoneMesureDoubleCentre.calculeCoordonnees();
    tft.drawRect(STATUS_EXT_X, STATUS_EXT_Y, STATUS_EXT_W, STATUS_EXT_H, STATUS_EXT_C);
    updateAppareilsDeMesure();

  }
  else if (mucSerieAffichageEnCours == 4) {
    tft.drawRect(STATUS_EXT_X, STATUS_EXT_Y, STATUS_EXT_W, STATUS_EXT_H, STATUS_EXT_C);
    updateSerie4();
  }
  else if (mucSerieAffichageEnCours == 5) {
    updateControleursEtEsclaves();
  }

  // Changement de série d'affichage
  mBtnSeries.draw(mucSerieAffichageEnCours);

}

void CEcran::serieAffichagePlusUn() {
  unsigned char val = mucSerieAffichageEnCours + 1;
  setSerieAffichageEnCours(val);
}

void CEcran::setSerieAffichageEnCours(unsigned char num) {
  mucSerieAffichageEnCours = num%(mucNbAffichagesMax+1);
  //Serial.printf("void CEcran::setSerieAffichageEnCours() - mucSerieAffichageEnCours %d/%d\n", mucSerieAffichageEnCours, mucNbAffichagesMax);
  if (mucSerieAffichageEnCours == 0) mucSerieAffichageEnCours = 1;
  drawMainInterface();

  prefs.begin(nvs_namespace, false);
  prefs.putUChar("aff", mucSerieAffichageEnCours);
  prefs.end();
  remonteEcranSerieParMqtt();
}

int CEcran::remonteEcranSerieParMqtt() {
  int ret = 0;
  String s = "ECRAN SERIE " + String(mucSerieAffichageEnCours);
  if (mConfig.onMqttPublish != nullptr) {
    DBG(DBG_ECRAN, "void CEcran::remonteStatusParMqtt() %s : %s\n", mConfig.topic_config_state.c_str(), s.c_str());
    mConfig.onMqttPublish(mConfig.topic_config_state.c_str(), s.c_str());
  }
  return ret;
} 
int CEcran::remonteOrientationEcranParMqtt() {
  int ret = 0;
  String s = "ECRAN " + String(mucOrientationEcran);
  if (mConfig.onMqttPublish != nullptr) {
    DBG(DBG_ECRAN, "void CEcran::remonteOrientationEcranParMqtt() %s : %s\n", mConfig.topic_config_state.c_str(), s.c_str());
    mConfig.onMqttPublish(mConfig.topic_config_state.c_str(), s.c_str());
  }
  return ret;
} 
int CEcran::remonteVeilleEcranParMqtt() {
  int ret = 0;
  String s = "VEILLE " + String(sleep_timeout);
  if (mConfig.onMqttPublish != nullptr) {
    DBG(DBG_ECRAN, "void CEcran::remonteVeilleEcranParMqtt() %s : %s\n", mConfig.topic_config_state.c_str(), s.c_str());
    mConfig.onMqttPublish(mConfig.topic_config_state.c_str(), s.c_str());
  }
  return ret;
} 
int CEcran::remonteStatusParMqtt() {
  int ret = 0;
  ret = remonteEcranSerieParMqtt();
  ret = remonteOrientationEcranParMqtt();
  ret = remonteVeilleEcranParMqtt();
  delay(200);
  return ret;
} 

void CEcran::updateAppareilsDeMesure() {
  #ifdef __LOCAL_MODE__
  updateThermometreLocal();
  #else
  updateRemoteDevice_DS18B20(mConfig.mRemoteThMain->nomEquipement, mConfig.mRemoteThMain->getLastTemperature());
  updateRemoteBat_DS18B20(mConfig.mRemoteBatMain->nomEquipement, mConfig.mRemoteBatMain->miLastEtatBatterie, mConfig.mRemoteBatMain->getLastTension());
  #endif
  updateRemoteDevice_ThCh1er(mConfig.mRemoteThCh1er->nomEquipement, mConfig.mRemoteThCh1er->getLastTemperature());
  updateRemoteBat_ThCh1er(mConfig.mRemoteBatThCh1er->nomEquipement, mConfig.mRemoteBatThCh1er->miLastEtatBatterie, mConfig.mRemoteBatThCh1er->getLastTension());

  updateRemoteDevice_ThSdb(mConfig.mRemoteThSdb->nomEquipement, mConfig.mRemoteThSdb->getLastTemperature());
  updateRemoteDevice_ThSdbDel(mConfig.mRemoteCoulSdb->nomEquipement, mConfig.mRemoteCoulSdb->getLastLedStatus());
  updateRemoteBat_ThSdb(mConfig.mRemoteBatSdb->nomEquipement, mConfig.mRemoteBatSdb->miLastEtatBatterie, mConfig.mRemoteBatSdb->getLastTension());

  updateRemoteDevice_ThCave(mConfig.mRemoteThCave->nomEquipement, mConfig.mRemoteThCave->getLastTemperature());
  updateRemoteDevice_ThCaveH(mConfig.mRemoteThCave->nomEquipement, mConfig.mRemoteThCave->getLastHumidite());
  updateRemoteDevice_ThCaveTor(mConfig.mRemoteTor->nomEquipement, mConfig.mRemoteTor->getLastMesure());
  updateRemoteBat_ThCave(mConfig.mRemoteBatCave->nomEquipement, mConfig.mRemoteBatCave->miLastEtatBatterie, mConfig.mRemoteBatCave->getLastTension());

  updateRemoteDevice_ThNomade(mConfig.mRemoteThNomade->nomEquipement, mConfig.mRemoteThNomade->getLastTemperature());
  updateRemoteDevice_ThNomadeH(mConfig.mRemoteThNomade->nomEquipement, mConfig.mRemoteThNomade->getLastHumidite());
  updateRemoteDevice_ThNomadeTor(mConfig.mRemoteTorNomade->nomEquipement, mConfig.mRemoteTorNomade->getLastMesure());
  updateRemoteBat_ThNomade(mConfig.mRemoteBatNomade->nomEquipement, mConfig.mRemoteBatNomade->miLastEtatBatterie, mConfig.mRemoteBatNomade->getLastTension());  

  updateRemoteDevice_ThRemise(mConfig.mRemoteThRemise->nomEquipement, mConfig.mRemoteThRemise->getLastTemperature());
  updateRemoteDevice_ThRemiseH(mConfig.mRemoteThRemise->nomEquipement, mConfig.mRemoteThRemise->getLastHumidite());
  updateRemoteBat_ThRemise(mConfig.mRemoteBatRemise->nomEquipement, mConfig.mRemoteBatRemise->miLastEtatBatterie, mConfig.mRemoteBatRemise->getLastTension());

  updateRemoteDevice_NewNas(mConfig.mRemoteNewNas->nomEquipement, mConfig.mRemoteNewNas->getLastMesure());
  updateRemoteDevice_BigNas(mConfig.mRemoteBigNas->nomEquipement, mConfig.mRemoteBigNas->getLastMesure());

}
//============================================================================================
// Affichage mesure et batterie - emplacement EST (1 mesure + batterie) => Série 1 sur LOCAL
//============================================================================================
void CEcran::updateThermometreLocal() { 
  if (mucSerieAffichageEnCours != 1 && mucSerieAffichageEnCours != 3) return;
  String nom = "";
  float temp = 0.0;
  #ifdef __LOCAL_MODE__  
  #ifdef __LOCAL_DS18B20__
  nom = mConfig.ds18b20->nomEquipement;
  temp = mConfig.ds18b20->getLastTemperature();
  #else
  nom = "";
  temp = -127.0;
  #endif
  #else
  nom = "";
  temp = -127.0;
  #endif
  mZoneMesureEst.drawMesure(temp, nom);
}
//============================================================================================
// Affichage mesure et batterie - emplacement OUEST
//============================================================================================
//-------------------------------------- DS18B20 (ThChRDC) --------------------------------------
void CEcran::updateRemoteDevice_DS18B20(const String& nom, float val) {
  if (mucSerieAffichageEnCours != 1 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureEst.drawMesure(val, nom);
}
void CEcran::updateRemoteBat_DS18B20(const String& nom, int etatBatterie, float val) {
  if (mucSerieAffichageEnCours != 1 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureEst.drawEtatBatterie(val, etatBatterie, nom);
}

//-------------------------------------- ThCh1er --------------------------------------
void CEcran::updateRemoteDevice_ThCh1er(const String& nom, float val) {
  if (mucSerieAffichageEnCours == 4 || mucSerieAffichageEnCours == 5) return;
  mZoneMesureOuest.drawMesure(val, nom);
}
void CEcran::updateRemoteBat_ThCh1er(const String& nom, int etatBatterie, float val) {
  if (mucSerieAffichageEnCours == 4 || mucSerieAffichageEnCours == 5) return;
  mZoneMesureOuest.drawEtatBatterie(val, etatBatterie, nom);
}


//============================================================================================
// Affichage mesure et batterie - emplacement CENTRE
//============================================================================================
//-------------------------------------- ThSdb --------------------------------------
void CEcran::updateRemoteDevice_ThSdb(const String& nom, float val) {
  if (mucSerieAffichageEnCours != 1 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureCentre.drawMesure(val, nom);
}
// Batterie du thermomètre SDB
void CEcran::updateRemoteBat_ThSdb(const String& nom, int etatBatterie, float val) {
  if (mucSerieAffichageEnCours != 1 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureCentre.drawEtatBatterie(val, etatBatterie, nom);
}
// Couleur
void CEcran::updateRemoteDevice_ThSdbDel(const String& nom, int val) {
  //Serial.printf("void CEcran::updateRemoteDevice_ThSdbDel() - nom = %s, val = 0x%lx\n", nom.c_str(), val);
  #ifdef __LOCAL_MODE__
  mConfig.chaudiere->setEtatReelOnOff(val);
  #else
  mConfig.mRemoteChaudiere->setEtatReelOnOff(val);
  #endif
  //updateAllStates();
  if (mucSerieAffichageEnCours > 4) return;
  if (mucSerieAffichageEnCours == 4) {
    mZoneCouleurSdb.muiPosX = SERIE4_COL_LEFT_X;
    mZoneCouleurSdb.muiPosY = SERIE4_ROW1_Y;
    mZoneCouleurSdb.muiWidth = SERIE4_COL_W;
    mZoneCouleurSdb.muiHight = REMOTE_B_1_H; //SERIE4_ROW_H;
    mZoneCouleurSdb.calculeCoordonnees();
  } else {
    mZoneCouleurSdb.muiPosX = REMOTE_1_X;
    mZoneCouleurSdb.muiPosY = (REMOTE_1_Y + REMOTE_1_H + 2 + MESURE_2_HAUTEUR + 2);
    mZoneCouleurSdb.muiWidth = REMOTE_1_W;
    mZoneCouleurSdb.muiHight = REMOTE_1_H;
    mZoneCouleurSdb.calculeCoordonnees();
  }
  mZoneCouleurSdb.drawMesure(val, nom);
}

//-------------------------------------- ThRemise --------------------------------------
void CEcran::updateRemoteDevice_ThRemise(const String& nom, float val) {
  if (mucSerieAffichageEnCours != 2 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureDoubleCentre.drawMesure1(val, nom);
}
// Humidité
void CEcran::updateRemoteDevice_ThRemiseH(const String& nom, float val) {
  if (mucSerieAffichageEnCours != 2 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureDoubleCentre.drawMesure2(val, nom);
}
// Batterie du thermomètre Remise (à la place de thermomètre Chambre 1er)
void CEcran::updateRemoteBat_ThRemise(const String& nom, int etatBatterie, float val) {
  if (mucSerieAffichageEnCours != 2 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureDoubleCentre.drawEtatBatterie(val, etatBatterie, nom);
}

//============================================================================================
// Affichage mesure et batterie - emplacement EST (2 mesures + batterie) => Série 2
//============================================================================================
void CEcran::updateRemoteDevice_ThCave(const String& nom, float val) {
  if (mucSerieAffichageEnCours != 2 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureDoubleEst.drawMesure1(val, nom);
}
// Humidité
void CEcran::updateRemoteDevice_ThCaveH(const String& nom, float val) {
  if (mucSerieAffichageEnCours != 2 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureDoubleEst.drawMesure2(val, nom);
}
// Tout ou Rien
void CEcran::updateRemoteDevice_ThCaveTor(const String& nom, int val) {
  if (mucSerieAffichageEnCours == 5) return;
  //Serial.printf("void CEcran::updateRemoteDevice_ThCaveTor(nom, val) = (%s, %d)\n", nom.c_str(), val);
  if (mucSerieAffichageEnCours == 4) {
    mZoneFlotteur.muiPosX = SERIE4_COL_LEFT_X;
    mZoneFlotteur.muiPosY = SERIE4_ROW2_Y;
    mZoneFlotteur.muiWidth = SERIE4_COL_W;
    mZoneFlotteur.muiHight = REMOTE_B_1_H; //SERIE4_ROW_H;
    mZoneFlotteur.calculeCoordonnees();
  } else {
    mZoneFlotteur.muiPosX = REMOTE_2_X;
    mZoneFlotteur.muiPosY = (REMOTE_2_Y + REMOTE_2_H + 2 + MESURE_2_HAUTEUR + 2);
    mZoneFlotteur.muiWidth = REMOTE_2_W;
    mZoneFlotteur.muiHight = REMOTE_2_H;
    mZoneFlotteur.calculeCoordonnees();
  }
  mZoneFlotteur.drawMesure(val, nom);
}

// Batterie du thermomètre Cave (à la place de thermomètre Chambre 1er)
void CEcran::updateRemoteBat_ThCave(const String& nom, int etatBatterie, float val) {
  if (mucSerieAffichageEnCours != 2 && mucSerieAffichageEnCours != 3) return;
  mZoneMesureDoubleEst.drawEtatBatterie(val, etatBatterie, nom);
}

//============================================================================================
// Affichage mesure et batterie - emplacement OUEST HAUT (2 mesures + batterie) => Série 3
//============================================================================================
void CEcran::updateRemoteDevice_ThNomade(const String& nom, float val) {
  if (mucSerieAffichageEnCours != 3) return;
  mZoneMesureDoubleOuest.drawMesure1(val, nom);
}
// Humidité
void CEcran::updateRemoteDevice_ThNomadeH(const String& nom, float val) {
  if (mucSerieAffichageEnCours != 3) return;
  mZoneMesureDoubleOuest.drawMesure2(val, nom);
}
// Tout ou Rien
void CEcran::updateRemoteDevice_ThNomadeTor(const String& nom, int val) {
  if (mucSerieAffichageEnCours == 5) return;
  //Serial.printf("void CEcran::updateRemoteDevice_ThNomadeTor(nom, val) = (%s, %d)\n", nom.c_str(), val);
  if (mucSerieAffichageEnCours == 4) {
    mZoneTorNomade.muiPosX = SERIE4_COL_LEFT_X;
    mZoneTorNomade.muiPosY = SERIE4_ROW3_Y;
    mZoneTorNomade.muiWidth = SERIE4_COL_W;
    mZoneTorNomade.muiHight = REMOTE_B_1_H; //SERIE4_ROW_H;
    mZoneTorNomade.calculeCoordonnees();
    mZoneTorNomade.drawMesure(val, nom);
  } else {
    /*mZoneTorNomade.muiPosX = REMOTE_2_X;
    mZoneTorNomade.muiPosY = (REMOTE_2_Y + REMOTE_2_H + 2 + MESURE_2_HAUTEUR + 2);
    mZoneTorNomade.muiWidth = REMOTE_2_W;
    mZoneTorNomade.muiHight = REMOTE_2_H;
    mZoneTorNomade.calculeCoordonnees();*/
  }
}

// Batterie du thermomètre Nomade
void CEcran::updateRemoteBat_ThNomade(const String& nom, int etatBatterie, float val) {
  if (mucSerieAffichageEnCours != 3) return;
  mZoneMesureDoubleOuest.drawEtatBatterie(val, etatBatterie, nom);
}

//============================================================================================
// Série 4 : coulSdb / Flotteur / Ha1 / Ha2
//============================================================================================
void CEcran::updateRemoteDevice_NewNas(const String& nom, int val) {
  if (mucSerieAffichageEnCours != 4) return;
  mZoneHa1.drawMesure(val, nom);
}
void CEcran::updateRemoteDevice_BigNas(const String& nom, int val) {
  if (mucSerieAffichageEnCours != 4) return;
  mZoneHa2.drawMesure(val, nom);
}
void CEcran::updateSerie4() {
  if (mucSerieAffichageEnCours != 4) return;
  updateRemoteDevice_ThSdbDel(mConfig.mRemoteCoulSdb->nomEquipement, mConfig.mRemoteCoulSdb->getLastLedStatus());
  updateRemoteDevice_ThCaveTor(mConfig.mRemoteTor->nomEquipement, (int)mConfig.mRemoteTor->getLastMesure());
  if (mConfig.mRemoteNewNas != nullptr)
    updateRemoteDevice_NewNas(mConfig.mRemoteNewNas->nomEquipement, (int)mConfig.mRemoteNewNas->getLastMesure());
  if (mConfig.mRemoteBigNas != nullptr)
    updateRemoteDevice_BigNas(mConfig.mRemoteBigNas->nomEquipement, (int)mConfig.mRemoteBigNas->getLastMesure());
}

//============================================================================================
// Affichage de la date et de l'heure
//============================================================================================
void CEcran::updateDateHeure(const String& date, const String& heure) {
  mZoneDateTime.drawDateTime(date, heure);
}
void CEcran::updateDateHeure() {
  mZoneDateTime.drawDateTime();
}

//============================================================================================
// Affichage de la liste des contrôleurs et des esclaves
//============================================================================================
void CEcran::updateControleursEtEsclaves() {
  mZoneAffichageIP.drawEquipements();
}

// Mets à jour la zone de messages d'état des équipements
void CEcran::updateAllStates() {
  drawMainInterface();
}

  void CEcran::updateStatus(const String& msg, bool drawInterface/*=false*/, bool memorise/*=false*/) {
    if (drawInterface)
      drawMainInterface();
    
    drawStatus(msg, memorise);
  }  
  void CEcran::updateTitleWithIP(const String& ip) {
    mZoneTitle.drawTitle(ip);
}

