
#include "global.h"
#include "MyZoneAffichageIP.h"

void CZoneAffichageIP::calculeCoordonnees () {
    muiLabelFont = 2;
    /*muiLabelX = muiPosX + 4; // valeur expérimentale
    muiLabelY = muiPosY + 2; // valeur expérimentale

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
    muiKOColorBatterie = COLOR_KO_BATTERIE;*/
  }


CZoneAffichageIP::CZoneAffichageIP(/*CConfig& cfg, */TFT_eSPI& tft, std::vector<CIPModule> *ctrl, std::vector<CIPModule> *escl, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bgcolor, String name/*=""*/) : 
  /*mConfig(&cfg), */mTft(tft),
  mvsControleurs(ctrl), mvsEsclaves(escl),
  muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiBgColor(bgcolor), msNom(name) {
    mvsControleurs = ctrl;
    mvsEsclaves = escl;
    calculeCoordonnees();
  }

//
// Retour
// -1 : Pointeur non affecté
// 0 : RAS
//
int CZoneAffichageIP::drawEquipements () { 
  int ret = 0;
  if (mvsControleurs == nullptr) {
    DBG(DBG_ECRAN, "int CZoneAffichageIP::drawEquipements () - Pointeur mvsControleurs null\n");
    return -1;
  }
  if (mvsEsclaves == nullptr) {
    DBG(DBG_ECRAN, "int CZoneAffichageIP::drawEquipements () - Pointeur mvsEsclaves null\n");
    return -1;
  }

  unsigned int x = muiPosX;
  unsigned int y = muiPosY;

  //Serial.printf("int CZoneAffichageIP::drawEquipements () - mvsControleurs %d\n", mvsControleurs->size());
  if (mvsControleurs->empty()) {
    DBG(DBG_ECRAN, "int CZoneAffichageIP::drawEquipements () - mvsControleurs vide\n");
  }

  mTft.setTextColor(TFT_ORANGE, TFT_BLACK);
  mTft.drawString("==================================================", x, y, muiLabelFont);
  y+=10;
  mTft.setTextColor(TFT_WHITE, TFT_BLACK);
  mTft.drawString("Controleurs et clients connus", x, y, muiLabelFont);
  y+=14;
  mTft.setTextColor(TFT_ORANGE, TFT_BLACK);
  mTft.drawString("==================================================", x, y, muiLabelFont);
  y+=10;
  mTft.setTextColor(TFT_WHITE, TFT_BLACK);
  for (const auto& ctrl : *mvsControleurs) {
    String sCtrl = ctrl.getNom().substring(0, 60);
    String sIp = ctrl.getIP();
    mTft.drawString(sCtrl, x, y, muiLabelFont);
    //mTft.drawString(" → ", x+50, muiLabelFont);
    mTft.drawString(sIp, x+190, y, muiLabelFont);
    //Serial.printf("int CZoneAffichageIP::drawEquipements () - x = %d, y = %d, nom : %s, IP : %s\n", x, y, sCtrl.c_str(), sIp.c_str());
    y += 15;
   }

  /*mTft.drawString("=================================", x, y, muiLabelFont);
  y+=10;
  mTft.drawString("Clients connus", x, y, muiLabelFont);
  y+=10;
  mTft.drawString("=================================", x, y, muiLabelFont);
  y+=10;
  for (const auto& ctrl : *mvsEsclaves) {
    String sCtrl = ctrl.getNom();
    String sIp = ctrl.getIP();
    mTft.drawString(sCtrl, x, y, muiLabelFont);
    mTft.drawString(" → ", x+50, muiLabelFont);
    mTft.drawString(sIp, x+100, y, muiLabelFont);
    //Serial.printf("int CZoneAffichageIP::drawEquipements () - x = %d, y = %d, nom : %s, IP : %s\n", x, y, sCtrl.c_str(), sIp.c_str());
    y += 15;
  }*/

   return ret;

return 0;

}


