
#include "global.h"
#include "MyBouton.h"
#include "EquipementBase.h"
//#include "RemoteRCDevice.h"


bool CMyBouton::setActif(bool act) {
    bool bLast = mbActif;
    mbActif = act;
    muiColor = mbActif ? TFT_RED : TFT_GREENYELLOW;
    return bLast;
}

bool CMyBouton::setOnOff(bool onOff) {
    bool bLast = mbOn;
    mbOn = onOff;
    unsigned int color = mbOn ? COULEUR_BTN_ON : COULEUR_BTN_OFF;
    muiColor = (mbActif ? color : TFT_DARKGREY);
    
    return bLast;
}

void CMyBouton::draw(unsigned int color) { 
    muiColor = color;
    draw();
}

void CMyBouton::draw() { 
    if (mDevice == nullptr) {
        DBG(DBG_ECRAN, "void CMyBouton::draw() - %s - mDevice == nullptr\n", msNom.c_str());
        return;
    }

    setActif(mDevice->active);
    setOnOff(mDevice->etat);

    mTft.fillRoundRect(muiPosX, muiPosY, muiWidth, muiHight, 8, muiColor);
    mTft.drawRoundRect(muiPosX, muiPosY, muiWidth, muiHight, 8, TFT_WHITE);
    mTft.setTextColor(TFT_WHITE);
    mTft.drawCentreString(msNom, muiPosX + muiWidth/2, muiPosY + muiHight/2 - 8, 2.5);
}

void CMyBouton::drawActive(bool bActiv) {
    mbActif = bActiv;
    muiColor = mbActif ? TFT_RED : TFT_GREENYELLOW;
    draw();
}

void CMyBouton::drawOnOff(bool bOn) {
    mbOn = bOn;
    muiColor = mbOn ? COULEUR_BTN_ON : COULEUR_BTN_OFF;
    draw();
}
