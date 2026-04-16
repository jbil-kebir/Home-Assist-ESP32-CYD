
#include "global.h"
#include "MyBoutonActif.h"
#include "EquipementBase.h"
//#include "MyRCDevice.h"
//#include "RemoteRCDevice.h"

bool CMyBoutonActif::setActif(bool act) {
    bool bLast = mbActif;
    mbActif = act;
    muiColor = mbActif ? TFT_RED : TFT_GREENYELLOW;
    return bLast;
}

void CMyBoutonActif::draw(unsigned int color) { 
    muiColor = color;
    draw();
}

void CMyBoutonActif::draw() { 
    if (mDevice == nullptr) {
        DBG(DBG_ECRAN, "void CMyBoutonActif::draw() - %s - mDevice == nullptr\n", msNom.c_str());
        return;
    }
    // Test
    bool mbActifAvant = mbActif;
    setActif(mDevice->active);
    //Serial.printf("mDevice->active (%s) : %d - mbActif avant %d et après : %d - couleur : 0x%x - Rouge : 0x%x - GreenYellow : 0x%x\n", msNom, mDevice->active, mbActifAvant, mbActif, muiColor, TFT_RED, TFT_GREENYELLOW);

    mTft.fillRoundRect(muiPosX, muiPosY, muiWidth, muiHight, 8, muiColor);
    mTft.drawRoundRect(muiPosX, muiPosY, muiWidth, muiHight, 8, TFT_WHITE);
    mTft.setTextColor(TFT_WHITE);
    mTft.drawCentreString(msNom, muiPosX + muiWidth/2, muiPosY + muiHight/2 - 8, 2.5);
}

void CMyBoutonActif::drawActive(bool bActiv) {
    mbActif = bActiv;
    muiColor = mbActif ? TFT_RED : TFT_GREENYELLOW;
    draw();
}

