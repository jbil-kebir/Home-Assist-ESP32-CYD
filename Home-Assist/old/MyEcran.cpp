#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <WiFi.h>

#include "MyConfig.h"
#include "global.h"
#include "MyEcran.h"

  void CEcran::setup() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_NAVY);
    tft.setTextColor(TFT_YELLOW);
    tft.drawCentreString("AD200 TX", 160, 40, 4);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString("Initialisation...", 160, 80, 2);

    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(3);
    mbEcranAllume = true; // Pas de veille à l'allumage
    Serial.printf("Setup mulDernierTouch\n");
    mulDernierTouch = millis();
  }
  void CEcran::loop() {
    // === Gestion de la veille légère ===
    if (mConfig.sleep_timeout > 0) {  // Veille activée
      if (mbEcranAllume && millis() - mulDernierTouch > (unsigned long)mConfig.sleep_timeout * 1000UL) {
        updateStatus("Veille écran...");
        delay(1000);
        digitalWrite(TFT_BL, LOW);
        mbEcranAllume = false;
      }
  }

  }



  void CEcran::drawMainInterface() {
    tft.fillScreen(COULEUR_FOND_ECRAN);

//    updateTitleWithIP("");
    updateTitleWithIP(WiFi.localIP().toString());

    drawButton(BOUTON_2H_X,  BOUTON_2H_Y, BOUTON_2H_W, BOUTON_2H_H, BOUTON_2H_C,  BOUTON_2H_N);
    drawButton(BOUTON_ON_X, BOUTON_ON_Y, BOUTON_ON_W, BOUTON_ON_H, BOUTON_ON_C, BOUTON_ON_N);
    drawButton(BOUTON_OFF_X, BOUTON_OFF_Y, BOUTON_OFF_W, BOUTON_OFF_H, BOUTON_OFF_C,   BOUTON_OFF_N);

    tft.drawRect(STATUS_EXT_X, STATUS_EXT_Y, STATUS_EXT_W, STATUS_EXT_H, STATUS_EXT_C);
    updateStatus("Pret");

    updateBoilerStatus(mConfig.etatStr.c_str());
  }

  void CEcran::drawButton(int x, int y, int w, int h, uint16_t color, const char* label) {
    tft.fillRoundRect(x, y, w, h, 8, color);
    tft.drawRoundRect(x, y, w, h, 8, TFT_WHITE);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString(label, x + w/2, y + h/2 - 8, 2.5);
  }

  void CEcran::updateStatus(const String& msg) {
    tft.fillRect(STATUS_INT_X, STATUS_INT_Y, STATUS_INT_W, STATUS_INT_H, STATUS_INT_C);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
//    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(msg.c_str(), STATUS_MSG_POS_X, STATUS_MSG_POS_Y, STATUS_MSG_FONT);
  }

  bool CEcran::isTouched(int& x, int& y) {
    if (touch.tirqTouched() && touch.touched()) {
      mulDernierTouch = millis();  // Réinitialise le timer
    // Si l'écran était éteint, on le rallume
      if (!mbEcranAllume) {
        digitalWrite(TFT_BL, HIGH);
        mbEcranAllume = true;
        drawMainInterface();
        //mqtt.publishState(mqtt.getCurrentState());  // Optionnel : republie l’état
      }
      else {
        TS_Point p = touch.getPoint();
        x = map(p.x, TS_MINX, TS_MAXX, 0, RESOLUTION_X);
        y = map(p.y, TS_MINY, TS_MAXY, 0, RESOLUTION_Y);
        x = constrain(x, 0, RESOLUTION_X-1);
        y = constrain(y, 0, RESOLUTION_Y-1);
      }
      return true;
    }
    return false;
  }
