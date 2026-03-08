#ifndef __MY_ZONE_STATUS_H__
#define __MY_ZONE_STATUS_H__

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "ecranConstantesPosition.h"

class CZoneStatus {
    private:
        TFT_eSPI& mTft;

        String msNom="";
        unsigned int muiPosX, muiPosY, muiWidth, muiHight, muiBgColor; // Cadre contenant le nom et la mesure. Initialisées par le constructeur
        // Valeurs déduites par tests
        unsigned int muiStatusX, muiStatusY, muiStatusFont; // Position et Police du texte (nom du capteur)
    public:
        CZoneStatus(TFT_eSPI& tft, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bgcolor, String name="") : mTft(tft),
         muiPosX(x), muiPosY(y), muiWidth(w), muiHight(h), muiBgColor(bgcolor), msNom(name) {
            muiStatusX = muiPosX + 3; // valeur expérimentale
            muiStatusY = muiPosY + 4; // valeur expérimentale
            muiStatusFont = 2;

         }

        String mstLastStatusMessage; // Dernier message affiché dans la barre de status. Utilisé après réveil de l'écran par un touch

        int drawStatus(const String& msg, bool memorise=false);
        int drawStatus();
        String setLastStatusMessage(String& s);
        String getLastStatusMessage();
        

};
#endif // __MY_ZONE_STATUS_H__