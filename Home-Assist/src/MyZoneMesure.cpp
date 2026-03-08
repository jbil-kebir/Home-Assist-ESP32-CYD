
#include "global.h"
#include "MyZoneMesure.h"

void CZoneMesure::calculeCoordonnees () {
    muiLabelX = muiPosX + 4; // valeur expérimentale
    muiLabelY = muiPosY + 2; // valeur expérimentale
    muiLabelFont = 2;

    muiValMesureX = muiPosX + 4; // valeur expérimentale
    muiValMesureY = muiLabelY + 15; // valeur expérimentale
    muiValMesureFont = 4;
    // Cadre entourant la mesure de la batterie
    muiPosBatX = muiPosX;
    muiPosBatY = muiPosY + muiHight + 2; // En Ordonnée, on part du bloc du nom, on ajoute sa hauteur et 2 points de marge
    muiBatWidth = muiWidth; // Même largeur que le nom
    muiBatHight = MESURE_BATTERIE_HAUTEUR; // = 20
    muiBatBgColor = muiBgColor; // = TFT_DARKCYAN
    // Mesure de la batterie
    muiValBatterieX = muiPosX + 4 + 35; // Expérimental (pour aligner à droite)
    muiValBatterieY = muiPosBatY + 2 ; // Même niveau que le cadre + 2
    muiValBatterieFont = 2;

    muiOKColorMesure = COLOR_OK_MESURE;
    muiKOColorMesure = COLOR_KO_MESURE;
    muiOKColorBatterie = COLOR_OK_BATTERIE;
    muiKOColorBatterie = COLOR_KO_BATTERIE;
}

CZoneMesure::CZoneMesure(/*CConfig& cfg, */TFT_eSPI& tft, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bgcolor, String name/*=""*/) : /*mConfig(&cfg), */mTft(tft),
  muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiBgColor(bgcolor), msNom(name) {
    calculeCoordonnees();
  }


//
// Retour
// -1 : Mesure invalide, on ne fait rien
//
int CZoneMesure::drawMesure(float val, const String& nom/*=""*/) { 

  if (val == -127.0 && mfOldMesure == -127.0) return -1; // Evite de trop nombreux raffraichisssements;
  mfOldMesure = val;

  mTft.fillRect(muiPosX, muiPosY, muiWidth, muiHight, muiBgColor);  // TFT_BLACK Efface zone
  if (val == -127.0) {
    mTft.setTextColor(muiKOColorMesure);
    mTft.drawString(nom, muiLabelX, muiLabelY, muiLabelFont);
    mTft.drawString("KO", muiValMesureX, muiValMesureY, muiValMesureFont);
  } 
  else if (val == -254.0) { // Watchdog Error
    mTft.setTextColor(muiKOColorMesure);
    mTft.drawString(nom, muiLabelX, muiLabelY, muiLabelFont);
    mTft.drawString("WDG", muiValMesureX, muiValMesureY, muiValMesureFont);
  } 
  else {
    mTft.setTextColor(muiOKColorMesure);
    mTft.drawString(nom, muiLabelX, muiLabelY, muiLabelFont);
    mTft.drawString(String(val, 1) + " °C", muiValMesureX, muiValMesureY, muiValMesureFont);
  }

return 0;

}

//
// Retour
// -1 : Mesure invalide, on ne fait rien
//
int CZoneMesure::drawEtatBatterie(float val, int iEtatBat, const String& nom/*=""*/) { 

  //static float lastVal = 0;
  if (val == -1.0 && mfOldValBatterie == -1.0) return -1; // Evite de trop nombreux raffraichisssements;
  mfOldValBatterie = val;
  //eteindreEcran();
  mTft.fillRect(muiPosBatX, muiPosBatY, muiBatWidth, muiBatHight, muiBatBgColor);  // REMOTE_B_1_H, REMOTE_B_1_BG_C = 20, TFT_DARKCYAN
  if (val == -1.0) {
    mTft.setTextColor(muiKOColorBatterie);
    mTft.drawString("KO", muiValBatterieX, muiValBatterieY, muiValBatterieFont);
  } 
  else if (val == -2.0) { // Watchdog Error
    mTft.setTextColor(muiKOColorBatterie);
    mTft.drawString("WDG", muiValBatterieX, muiValBatterieY, muiValBatterieFont);
  } 
  else {
    String sVal = String(val, 1) + " V";
    if (iEtatBat == 1) mTft.setTextColor(muiOKColorBatterie);
    else if (iEtatBat == 0) mTft.setTextColor(muiKOColorBatterie);
    else {
      mTft.setTextColor(muiKOColorBatterie);
      sVal = "KO";
    }
    mTft.drawString(sVal, muiValBatterieX, muiValBatterieY, muiValBatterieFont);
  }
  return 0;


}

