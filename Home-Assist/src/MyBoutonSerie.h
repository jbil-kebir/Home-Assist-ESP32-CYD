#ifndef __MYBOUTON_SERIE_H__
#define __MYBOUTON_SERIE_H__

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "ecranConstantesPosition.h"
#include "MyBoutonBase.h"

class CEquipementBase;
//class CMyBoutonBase;

class CMyBoutonSerie : public CMyBoutonBase {
    private:

        //TFT_eSPI& mTft;

        //String msNom="";
        //unsigned int muiPosX, muiPosY, muiWidth, muiHight, muiColor;

        unsigned char mucSerieAffichageEnCours = 1;
    
    public:
        CMyBoutonSerie(/*CConfig& cfg, */TFT_eSPI& tft, XPT2046_Touchscreen& _touch, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color, String name) : 
        CMyBoutonBase(tft, _touch, x, y, w, h, color, name)/* ,
        mTft(tft), muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiColor(color), msNom(name) */{
            
         }

        //CEquipementBase *mDevice=nullptr;

        void draw();
        void draw(unsigned char s);

        unsigned char setSerie(unsigned char s);

};
#endif // __MYBOUTON_SERIE_H__