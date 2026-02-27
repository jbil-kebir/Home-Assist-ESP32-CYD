#ifndef __MYBOUTON_H__
#define __MYBOUTON_H__

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "MyBoutonBase.h"

#include "ecranConstantesPosition.h"

class CConfig;
class CEquipementBase;

class CMyBouton : public CMyBoutonBase {
    private:
        //CConfig* mConfig=nullptr;
        //TFT_eSPI& mTft;

        //String msNom="";
        //unsigned int muiPosX, muiPosY, muiWidth, muiHight, muiColor;
        bool mbActif = false; // Enabled / Disabled
        bool mbOn = false; // ON/OFF
    
    public:
        CMyBouton(CConfig& cfg, TFT_eSPI& tft, XPT2046_Touchscreen& _touch, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color, String name) : 
            CMyBoutonBase(tft, _touch, x, y, w, h, color, name)/*,
            mConfig(&cfg), mTft(tft),
            muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiColor(color), msNom(name)*/ {
            
         }

        //CEquipementBase *mDevice=nullptr;

        void draw();
        void draw(unsigned int color);
        void drawActive(bool bActiv);
        void drawOnOff(bool bOn);
        bool setActif(bool act);
        bool setOnOff(bool onOff);

};
#endif // __MYBOUTON_H__