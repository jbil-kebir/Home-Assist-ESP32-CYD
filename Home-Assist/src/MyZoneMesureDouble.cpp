
#include "global.h"
#include "MyZoneMesureDouble.h"

void CZoneMesureDouble::calculeCoordonnees () {
    muiLabelX = muiPosX + 4; // valeur expérimentale
    muiLabelY = muiPosY + 2; // valeur expérimentale
    muiLabelFont = 2;

    muiValMesure1X = muiPosX + 4; // valeur expérimentale
    muiValMesure1Y = muiLabelY + 15; // valeur expérimentale
    muiValMesure1Font = 4;

    // Cadre autour de la mesure 2
    muiPosMesure2X = muiPosX;
    muiPosMesure2Y = muiPosY + muiHight + 2; // En Ordonnée, on part du bloc du nom, on ajoute sa hauteur et 2 points de marge
    muiMesure2Width = muiWidth; // Même largeur que le nom
    muiMesure2Hight = MESURE_2_HAUTEUR; // = 20
    muiMesure2BgColor = muiBgColor; // = TFT_DARKCYAN

    muiValMesure2X = muiPosX + 4 + 26;
    muiValMesure2Y = muiPosMesure2Y + 2 ;
    muiValMesure2Font = 2;
    // Cadre entourant la mesure de la batterie
    muiPosBatX = muiPosX;
    muiPosBatY = muiPosMesure2Y + muiMesure2Hight + 2; // En Ordonnée, on part du bloc du nom, on ajoute sa hauteur et 2 points de marge
    muiBatWidth = muiMesure2Width; // Même largeur que le précédent
    muiBatHight = MESURE_2_HAUTEUR; // = 20
    muiBatBgColor = muiBgColor; // = TFT_DARKCYAN
    // Mesure de la batterie
    muiValBatterieX = muiPosX + 4 + 35; // Expérimental (pour aligner à droite)
    muiValBatterieY = muiValMesure2Y + 23 ; 
    muiValBatterieFont = 2;

    muiOKColorMesure = COLOR_OK_MESURE;
    muiKOColorMesure = COLOR_KO_MESURE;
    muiOKColorBatterie = COLOR_OK_BATTERIE;
    muiKOColorBatterie = COLOR_KO_BATTERIE;
  }


CZoneMesureDouble::CZoneMesureDouble(/*CConfig& cfg, */TFT_eSPI& tft, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bgcolor, String name/*=""*/) : /*mConfig(&cfg), */mTft(tft),
  muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiBgColor(bgcolor), msNom(name) {
    calculeCoordonnees();
  }

//
// Retour
// -1 : Mesure invalide, on ne fait rien
// 0 : RAS
//
int CZoneMesureDouble::drawMesure1(float val, const String& nom/*=""*/) { 
  if (val == -127.0 && mfOldMesure1 == -127.0) return -1; // Evite de trop nombreux raffraichisssements;
  mfOldMesure1 = val;

  mTft.fillRect(muiPosX, muiPosY, muiWidth, muiHight, muiBgColor);  // TFT_BLACK Efface zone
  if (val == -127.0) {
    mTft.setTextColor(muiKOColorMesure);
    mTft.drawString(nom, muiLabelX, muiLabelY, muiLabelFont);
    mTft.drawString("KO", muiValMesure1X, muiValMesure1Y, muiValMesure1Font);
  } 
  else if (val == -254.0) { // Watchdog Error
    mTft.setTextColor(muiKOColorMesure);
    mTft.drawString(nom, muiLabelX, muiLabelY, muiLabelFont);
    mTft.drawString("WDG", muiValMesure1X, muiValMesure1Y, muiValMesure1Font);
  } 
  else {
    mTft.setTextColor(muiOKColorMesure);
    mTft.drawString(nom, muiLabelX, muiLabelY, muiLabelFont);
    mTft.drawString(String(val, 1) + " °C", muiValMesure1X, muiValMesure1Y, muiValMesure1Font);
  }

return 0;

}

//
// Retour
// -1 : Mesure invalide, on ne fait rien
// 0 : RAS
//
int CZoneMesureDouble::drawMesure2(float val, const String& nom/*=""*/) { 
  //static float lastVal = 0;
  if (val == -1.0 && mfOldMesure2 == -1.0) return -1; // Evite de trop nombreux raffraichisssements;
  mfOldMesure2 = val;
  //eteindreEcran();
  mTft.fillRect(muiPosMesure2X, muiPosMesure2Y, muiMesure2Width, muiMesure2Hight, muiMesure2BgColor);  // TFT_BLACK Efface zone
  if (val == -127.0) {
    mTft.setTextColor(muiKOColorMesure);
    mTft.drawString("KO", muiValMesure2X, muiValMesure2Y, muiValMesure2Font);
  } 
  else if (val == -254.0) { // Watchdog Error
    mTft.setTextColor(muiKOColorMesure);
    mTft.drawString("WDG", muiValMesure2X, muiValMesure2Y, muiValMesure2Font);
  } 
  else {
    mTft.setTextColor(muiOKColorMesure);
    mTft.drawString(String(val, 1) + " %", muiValMesure2X, muiValMesure2Y, REMOTE_3_H_TEXTE_L1_F);
  }

return 0;

}
//
// Retour
// -1 : Mesure invalide, on ne fait rien
// 0 : RAS
//
int CZoneMesureDouble::drawEtatBatterie(float val, int iEtatBat, const String& nom/*=""*/) { 
  if (val == -1.0 && mfOldValBatterie == -1.0) return -1; // Evite de trop nombreux raffraichisssements;
  mfOldValBatterie = val;
  miEtatBatterie = iEtatBat;
  mTft.fillRect(muiPosBatX, muiPosBatY, muiBatWidth, muiBatHight, muiBatBgColor);  // TFT_BLACK Efface zone
  if (mfOldValBatterie == -1.0) {
    mTft.setTextColor(muiKOColorBatterie);
    mTft.drawString("KO", muiValBatterieX, muiValBatterieY, muiValBatterieFont);
  } 
  else if (mfOldValBatterie == -2.0) { // Watchdog Error
    mTft.setTextColor(muiKOColorBatterie);
    mTft.drawString("WDG", muiValBatterieX, muiValBatterieY, muiValBatterieFont);
  } 
  else {
    String sVal = String(mfOldValBatterie, 1) + " V";
    if (miEtatBatterie == 1) mTft.setTextColor(muiOKColorBatterie);
    else if (miEtatBatterie == 0) mTft.setTextColor(muiKOColorBatterie);
    else {
      mTft.setTextColor(muiKOColorBatterie);
      sVal = "KO";
    }
    mTft.drawString(sVal, muiValBatterieX, muiValBatterieY, muiValBatterieFont);
  }

  return 0;
}

