#ifndef __MYBOUTON_BASE_H__
#define __MYBOUTON_BASE_H__

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "ecranConstantesPosition.h"

class CEquipementBase;

class CMyBoutonBase {
    protected:

        TFT_eSPI& mTft;
        XPT2046_Touchscreen& touch;

        String msNom="";
        unsigned int muiPosX, muiPosY, muiWidth, muiHight, muiColor;

    public:
        CMyBoutonBase(TFT_eSPI& tft, XPT2046_Touchscreen& _touch, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color, String name) : 
            mTft(tft), touch(_touch), 
            muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiColor(color), msNom(name) {
            
         }

        CEquipementBase *mDevice=nullptr;
        bool isPressed();
        void waitRelease();

};
#endif // __MYBOUTON_BASE_H__