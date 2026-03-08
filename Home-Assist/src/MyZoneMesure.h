#ifndef __MY_ZONE_MESURE_H__
#define __MY_ZONE_MESURE_H__

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "ecranConstantesPosition.h"

#define MESURE_BATTERIE_HAUTEUR 20
#define COLOR_OK_MESURE TFT_WHITE
#define COLOR_KO_MESURE TFT_RED
#define COLOR_OK_BATTERIE TFT_GREEN
#define COLOR_KO_BATTERIE TFT_RED
#
class CZoneMesure {
    private:
        TFT_eSPI& mTft;

        String msNom="";
        float mfOldMesure = 0.0, mfOldValBatterie = 0.0;
        int miEtatBatterie = 0;
        // Valeurs déduites par tests
        unsigned int muiLabelX, muiLabelY, muiLabelFont; // Position et Police du texte (nom du capteur)

        unsigned int muiPosBatX, muiPosBatY, muiBatWidth, muiBatHight, muiBatBgColor; // Cadre contenant la mesure batterie. Calculé
        unsigned int muiValBatterieX, muiValBatterieY, muiValBatterieFont; // Position et Police de la batterie

        unsigned int muiOKColorMesure, muiKOColorMesure, muiOKColorBatterie, muiKOColorBatterie; // Couleurs
    
    public:
        unsigned int muiPosX, muiPosY, muiWidth, muiHight, muiBgColor; // Cadre contenant le nom et la mesure. Initialisées par le constructeur
        unsigned int muiValMesureX, muiValMesureY, muiValMesureFont; // Position et Police de la mesure 1 (température, ...)

        void calculeCoordonnees();
        CZoneMesure(/*CConfig& cfg, */TFT_eSPI& tft, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bgcolor, String name=""); 

        int drawMesure(float val, const String& nom="");
        int drawEtatBatterie(float val, int iEtatBat, const String& nom="");
        

};
#endif // __MY_ZONE_MESURE_H__