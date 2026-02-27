#ifndef __MY_ZONE_TITLE_H__
#define __MY_ZONE_TITLE_H__

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "ecranConstantesPosition.h"

class CConfig;

class CZoneTitle {
    private:
        TFT_eSPI& mTft;
        CConfig* mConfig=nullptr;

        String msNom="";
        unsigned int muiPosX, muiPosY, muiWidth, muiHight, muiBgColor; // Cadre contenant le nom et la mesure. Initialisées par le constructeur
        // Valeurs déduites par tests
        unsigned int muiTitleX, muiTitleY, muiTitleFont; // Position et Police du texte 
    public:
        CZoneTitle(CConfig& cfg, TFT_eSPI& tft, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bgcolor, String name="") : mConfig(&cfg), mTft(tft),
         muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiBgColor(bgcolor), msNom(name) {
            muiTitleX = TITRE_MSG_POS_X; // valeur expérimentale
            muiTitleY = TITRE_MSG_POS_Y; // valeur expérimentale
            muiTitleFont = 2;

         }

        int drawTitle(const String& ip);
        int drawTitle();

};
#endif // __MY_ZONE_TITLE_H__