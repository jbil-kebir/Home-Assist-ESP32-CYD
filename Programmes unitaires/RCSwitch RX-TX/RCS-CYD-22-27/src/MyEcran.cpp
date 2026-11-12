#include "global.h"
#ifdef __MODE_ECRAN__
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "constantesEcran.h"
#include "MyBouton.h"
#include "MyZoneTexte.h"
#include "MyZoneInfo.h"
#include "MyEcran.h"

//===================================================================================
//                    CLASSE CEcran 
//===================================================================================

  /*CEcran::CEcran()
    : touch(XPT2046_CS), touchSPI(HSPI),
      mZoneTitre(tft, "Contrôle 433 MHz", TITLE_X, TITLE_Y, &FreeSansBold9pt7b, COLOR_TITLE),
      mBtnProjecteur(tft, touch, "PROJECTEUR", BTN_X, BTN1_Y, BTN_W, BTN_H, BTN_RADIUS, COLOR_PROJECTEUR),
      mBtnSonnette(tft, touch, "SONNETTE", BTN_X, BTN2_Y, BTN_W, BTN_H, BTN_RADIUS, COLOR_SONNETTE),
      mBtnGuirlande(tft, touch, "GUIRLANDE", BTN_X, BTN3_Y, BTN_W, BTN_H, BTN_RADIUS, COLOR_GUIRLANDE),
      mZoneInfo(tft) {
      }*/

  void CEcran::setup() {
    // Backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    Serial.println(F("Backlight ON"));

    // TFT
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(COLOR_BG);

    // Touch HSPI
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(3);

    // Dessin IHM
    mZoneTitre.setup();
    mBtnProjecteur.setup();
    mBtnSonnette.setup();
    mBtnGuirlande.setup();
    mZoneInfo.setup();

    Serial.println(F("CEcran::setup() terminé – IHM complète"));
  }

  int CEcran::getPressedButton() {
    if (mBtnProjecteur.isPressed()) {
      mBtnProjecteur.waitRelease();
      return 1;
    }
    if (mBtnSonnette.isPressed()) {
      mBtnSonnette.waitRelease();
      return 2;
    }
    if (mBtnGuirlande.isPressed()) {
      mBtnGuirlande.waitRelease();
      return 3;
    }
    return 0;
  }

  void CEcran::updateLastSent(const char* name, unsigned long code, int proto, int pulse, int repeat) {
    mZoneInfo.updateLastSent(name, code, proto, pulse, repeat);
  }

  void CEcran::updateLastReceived(unsigned long code, int bits, int proto, int pulse, int rssi) {
    mZoneInfo.updateLastReceived(code, bits, proto, pulse, rssi);
  }
#endif