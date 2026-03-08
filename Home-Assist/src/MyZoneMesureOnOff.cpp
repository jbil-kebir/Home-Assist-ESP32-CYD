
#include "global.h"
#include "MyZoneMesureOnOff.h"

void CZoneMesureOnOff::calculeCoordonnees () {
    muiLabelX = muiPosX + 4; // valeur expérimentale
    muiLabelY = muiPosY + 2; // valeur expérimentale
    muiLabelFont = 2;

    muiValMesureX = muiPosX + 4; // valeur expérimentale
    muiValMesureY = muiLabelY + 15; // valeur expérimentale
    muiValMesureFont = 4;
    // Cadre entourant la mesure de la batterie
    /*muiPosBatX = muiPosX;
    muiPosBatY = muiPosY + muiHight + 2; // En Ordonnée, on part du bloc du nom, on ajoute sa hauteur et 2 points de marge
    muiBatWidth = muiWidth; // Même largeur que le nom
    muiBatHight = MESURE_BATTERIE_HAUTEUR; // = 20
    muiBatBgColor = muiBgColor; // = TFT_DARKCYAN
    // Mesure de la batterie
    muiValBatterieX = muiPosX + 4 + 35; // Expérimental (pour aligner à droite)
    muiValBatterieY = muiPosBatY + 2 ; // Même niveau que le cadre + 2
    muiValBatterieFont = 2;*/

    muiOKColorMesure = COLOR_OK_MESURE;
    muiKOColorMesure = COLOR_KO_MESURE;
    muiOKColorBatterie = COLOR_OK_BATTERIE;
    muiKOColorBatterie = COLOR_KO_BATTERIE;
}

CZoneMesureOnOff::CZoneMesureOnOff(/*CConfig& cfg, */TFT_eSPI& tft, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bgcolor, String name/*=""*/) : /*mConfig(&cfg), */mTft(tft),
  muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiBgColor(bgcolor), msNom(name) {
    calculeCoordonnees();
  }


//
// Retour
// -1 : Mesure invalide, on ne fait rien
//
int CZoneMesureOnOff::drawMesure(int val, const String& nom/*=""*/) { 

  if (val == -1.0 && mfOldMesure == -1.0) return -1; // Evite de trop nombreux raffraichisssements;
  mfOldMesure = val;

  //Serial.printf("int CZoneMesureOnOff::drawMesure() - OnOff %d = %s\n", val, val == 0 ? "Rouge" : "Vert");
  mTft.fillRect(muiPosX, muiPosY, muiWidth-20, muiHight, muiBgColor);  // TFT_BLACK Efface zone
  mTft.fillRect(muiPosX+muiWidth-18, muiPosY+2, 18, muiHight-2, val == 0 ? TFT_RED : TFT_GREEN);  // TFT_BLACK Efface zone

  String sLabel = nom.substring(0, 8);
  if (val == -1.0) {
    mTft.setTextColor(muiKOColorMesure);
    mTft.drawString(sLabel, muiLabelX, muiLabelY, muiLabelFont);
    //mTft.drawString("KO", muiValMesureX, muiValMesureY, muiValMesureFont);
  } 
  else if (val == -10.0) { // Watchdog Error
    mTft.setTextColor(muiKOColorMesure);
    mTft.drawString(sLabel, muiLabelX, muiLabelY, muiLabelFont);
    //mTft.drawString("WDG", muiValMesureX, muiValMesureY, muiValMesureFont);
  } 
  else {
    mTft.setTextColor(muiOKColorMesure);
    mTft.drawString(sLabel, muiLabelX, muiLabelY, muiLabelFont);
    //mTft.drawString(String(val), muiValMesureX, muiValMesureY, muiValMesureFont);
  }

return 0;

}



