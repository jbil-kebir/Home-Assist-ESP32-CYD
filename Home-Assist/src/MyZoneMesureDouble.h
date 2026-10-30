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
#define COLOR_JOUR TFT_YELLOW   // Détecteur jour / nuit (à la place de la batterie, voir drawJourNuit()) : texte jaune sur le fond de la zone
#define COLOR_NUIT TFT_WHITE    // Nuit : texte blanc...
#define COLOR_FOND_NUIT TFT_NAVY // ... sur fond bleu marine
// Bandeau derrière le nom d'un nœud capteur : état instantané éveillé / endormi (voir drawMesure1())
#define EVEIL_SANS_BANDEAU   -2           // Pas de bandeau (zone qui n'est pas un nœud à deep sleep)
#define EVEIL_INCONNU        -1           // Rien reçu depuis le démarrage du CYD
#define EVEIL_ENDORMI         0           // Dernière trame : « <nom> DEEPSLEEP ON … »
#define EVEIL_EVEILLE         1           // Dernière trame : toute autre trame du nœud
#define COLOR_BANDEAU_EVEILLE TFT_GREEN   // Nom en noir
#define COLOR_BANDEAU_ENDORMI TFT_RED     // Nom en blanc
#define COLOR_BANDEAU_INCONNU TFT_DARKGREY // Nom en blanc
#define BANDEAU_NOM_HAUTEUR  17           // Hauteur du nom (police 2) ; la mesure commence en dessous
#
class CZoneMesureDouble {
    private:
        TFT_eSPI& mTft;

        String msNom="";
        float mfOldMesure1 = 0.0, mfOldMesure2 = 0.0, mfOldValBatterie = 0.0;
        int miEtatBatterie = 0;
        int miOldEclairement = -2, miOldEtatJourNuit = -2; // -2 : rien affiché
        int miOldEveil = -3; // -3 : rien affiché
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


        int drawMesure1(float val, const String& nom="", int eveil=EVEIL_SANS_BANDEAU);
        int drawMesure2(float val, const String& nom="");
        int drawEtatBatterie(float val, int iEtatBat, const String& nom="");
        int drawJourNuit(int eclairement, int etat);
        

};
#endif // __MY_ZONE_MESURE_DOUBLE_H__