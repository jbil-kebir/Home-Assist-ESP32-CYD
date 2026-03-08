#ifndef __MY_ZONE_MESURE_DOUBLE_H__
#define __MY_ZONE_MESURE_DOUBLE_H__

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "ecranConstantesPosition.h"

#define MESURE_2_HAUTEUR 20

#define COLOR_OK_MESURE TFT_WHITE
#define COLOR_KO_MESURE TFT_RED
#define COLOR_OK_BATTERIE TFT_GREEN
#define COLOR_KO_BATTERIE TFT_RED
#
class CZoneMesureDouble {
    private:
        TFT_eSPI& mTft;

        String msNom="";
        float mfOldMesure1 = 0.0, mfOldMesure2 = 0.0, mfOldValBatterie = 0.0;
        int miEtatBatterie = 0;
        // Valeurs déduites par tests
        unsigned int muiLabelX, muiLabelY, muiLabelFont; // Position et Police du texte (nom du capteur)

        unsigned int muiValMesure1X, muiValMesure1Y, muiValMesure1Font; // Position et Police de la mesure 1 (température, ...)
        unsigned int muiValMesure2X, muiValMesure2Y, muiValMesure2Font; // Position et Police de la mesure 2 (température, ...)


        unsigned int muiPosBatX, muiPosBatY, muiBatWidth, muiBatHight, muiBatBgColor; // Cadre contenant la mesure batterie. Calculé
        unsigned int muiValBatterieX, muiValBatterieY, muiValBatterieFont; // Position et Police de la valeur batterie 

        unsigned int muiOKColorMesure, muiKOColorMesure, muiOKColorBatterie, muiKOColorBatterie; // Couleurs
    
    public:
        unsigned int muiPosX, muiPosY, muiWidth, muiHight, muiBgColor; // Cadre contenant le nom et la mesure. Initialisées par le constructeur
        unsigned int muiPosMesure2X, muiPosMesure2Y, muiMesure2Width, muiMesure2Hight, muiMesure2BgColor; // Cadre contenant la mesure 2. Calculé

        void calculeCoordonnees();
        CZoneMesureDouble(/*CConfig& cfg, */TFT_eSPI& tft, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bgcolor, String name="");


        int drawMesure1(float val, const String& nom="");
        int drawMesure2(float val, const String& nom="");
        int drawEtatBatterie(float val, int iEtatBat, const String& nom="");
        

};
#endif // __MY_ZONE_MESURE_DOUBLE_H__