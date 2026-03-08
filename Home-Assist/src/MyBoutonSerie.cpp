
#include "global.h"
#include "MyBoutonSerie.h"

unsigned char CMyBoutonSerie::setSerie(unsigned char s) {
    unsigned char lastSerie = mucSerieAffichageEnCours;
    mucSerieAffichageEnCours = s;
    msNom = String(mucSerieAffichageEnCours);
    return lastSerie;
}
void CMyBoutonSerie::draw() { 
    
    mTft.fillRoundRect(muiPosX, muiPosY, muiWidth, muiHight, 8, muiColor);
    mTft.drawRoundRect(muiPosX, muiPosY, muiWidth, muiHight, 8, TFT_WHITE);
    mTft.setTextColor(TFT_WHITE);
    mTft.drawCentreString(msNom, muiPosX + muiWidth/2, muiPosY + muiHight/2 - 8, 2.5);
}

void CMyBoutonSerie::draw(unsigned char s) { 
    //Serial.printf("void CMyBoutonSerie::draw() - mucSerieAffichageEnCours (%d)\n", s);
    setSerie(s);
    draw();
}

