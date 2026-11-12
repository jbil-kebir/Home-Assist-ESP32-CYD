#ifndef __MYECRAN_H__
#define __MYECRAN_H__
#include "global.h"
#ifdef __MODE_ECRAN__

#define TITLE_X       (SCREEN_W / 2)
#define TITLE_Y       10

// Couleurs
#define COLOR_BG          TFT_BLACK
#define COLOR_TITLE       TFT_YELLOW
#define COLOR_PROJECTEUR  TFT_BLUE
#define COLOR_SONNETTE    TFT_GREEN
#define COLOR_GUIRLANDE   TFT_RED


//===================================================================================
//                    CLASSE CEcran 
//===================================================================================
class CEcran {
private:
  TFT_eSPI tft;
  SPIClass touchSPI;
  XPT2046_Touchscreen touch;

  CZoneTexte mZoneTitre;
  CBouton mBtnProjecteur;
  CBouton mBtnSonnette;
  CBouton mBtnGuirlande;
  CZoneInfo mZoneInfo;

public:
  CEcran()
    : touch(XPT2046_CS), touchSPI(HSPI),
      mZoneTitre(tft, "Contrôle 433 MHz", TITLE_X, TITLE_Y, &FreeSansBold9pt7b, COLOR_TITLE),
      mBtnProjecteur(tft, touch, "PROJECTEUR", BTN_X, BTN1_Y, BTN_W, BTN_H, BTN_RADIUS, COLOR_PROJECTEUR),
      mBtnSonnette(tft, touch, "PRISE OFF", BTN_X, BTN2_Y, BTN_W, BTN_H, BTN_RADIUS, COLOR_SONNETTE),        // equipement[PRISE_OFF]
      mBtnGuirlande(tft, touch, "INTER DOUBLE G", BTN_X, BTN3_Y, BTN_W, BTN_H, BTN_RADIUS, COLOR_GUIRLANDE), // equipement[INTER_DOUBLE_G]
      mZoneInfo(tft) {}

  void setup();
  int getPressedButton();
  void updateLastSent(const char* name, unsigned long code, int proto, int pulse, int repeat);
  void updateLastReceived(unsigned long code, int bits, int proto, int pulse, int rssi);
};
#endif
#endif // __MYECRAN_H__
