#ifndef __MY_ZONE_DATE_TIME_H__
#define __MY_ZONE_DATE_TIME_H__

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "ecranConstantesPosition.h"

class CConfig;

class CZoneDateTime {
    private:
        TFT_eSPI& mTft;
        CConfig* mConfig=nullptr;

        String msNom="";
        unsigned int muiPosX, muiPosY, muiWidth, muiHight, muiBgColor; // Cadre contenant le nom et la mesure. Initialisées par le constructeur
        unsigned int muiDateX, muiDateY, muiDateFont; // Position et Font pour la date
        unsigned int muiTimeX, muiTimeY, muiTimeFont; // Position et Font pour l'heure
    public:
        CZoneDateTime(CConfig& cfg, TFT_eSPI& tft, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bgcolor, String name="") : mConfig(&cfg), mTft(tft),
         muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiBgColor(bgcolor), msNom(name) {
            muiDateX =  muiPosX + 10;
            muiDateY =  muiPosY + 2; 
            muiDateFont = 2;

            muiTimeX = muiPosX + 10;
            muiTimeY = muiDateY + 15;
            muiTimeFont = 2.8;
        }


        int drawDateTime(const String& date, const String& heure);
        int drawDateTime();
        

};
#endif // __MY_ZONE_DATE_TIME_H__