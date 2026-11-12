#ifndef __MYBOUTON_H__
#define __MYBOUTON_H__

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <functional>

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

        // Pastille d'état dans le coin haut droit (optionnelle) :
        // 1 : vert, 0 : rouge, -1 : gris (inconnu). Non définie : pas de pastille.
        std::function<int()> mfEtatPastille = nullptr;
        // Variante : couleur TFT de la pastille fournie directement (prioritaire sur mfEtatPastille), -1 : pas de pastille
        std::function<int()> mfCouleurPastille = nullptr;
        void drawPastille();
        // Couleur du bouton d'après un état MESURÉ au lieu de la dernière commande (appareil sans retour, ex. lumière
        // cave mesurée par JnCave) : 1 : vert (ON), 0 : rouge (OFF), -1 : gris clair (inconnu). Bouton désactivé
        // (cadenas) : gris foncé, comme les autres. Non définie : couleur selon mDevice->etat.
        std::function<int()> mfEtatMesure = nullptr;

        void draw();
        void draw(unsigned int color);
        void drawActive(bool bActiv);
        void drawOnOff(bool bOn);
        bool setActif(bool act);
        bool setOnOff(bool onOff);

};
#endif // __MYBOUTON_H__