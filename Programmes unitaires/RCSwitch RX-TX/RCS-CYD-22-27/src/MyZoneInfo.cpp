#include "global.h"
#ifdef __MODE_ECRAN__
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "constantesEcran.h"
#include "MyZoneInfo.h"



  //CZoneInfo::CZoneInfo(TFT_eSPI& _tft) : tft(_tft) {}

  void CZoneInfo::setup() {
    tft.fillRoundRect(INFO_X, INFO_Y, INFO_W, INFO_H, INFO_RADIUS, COLOR_INFO_BG);
    tft.setTextColor(TFT_WHITE);
    //tft.setFreeFont(&FreeSansBold9pt7b);
    tft.drawString("Infos :", INFO_X + INFO_MARGIN_X, INFO_TITLE_Y, GFX_FONT_NUMBER);  // Titre en gras
    clearContent();
  }

  void CZoneInfo::clearContent() {
    tft.fillRect(INFO_X + 5, INFO_TITLE_Y /*+ 20*/, INFO_W - 10, INFO_H - 30, COLOR_INFO_BG);
  }

  void CZoneInfo::updateLastSent(const char* name, unsigned long code, int proto, int pulse, int repeat) {
    clearContent();
    tft.setTextColor(TFT_YELLOW);
    //tft.setFreeFont(&FreeSansBold9pt7b);
    tft.drawString("Dernier envoi :", INFO_X + INFO_MARGIN_X, INFO_LINE1_Y, GFX_FONT_NUMBER);  // Gras

    tft.setTextColor(COLOR_INFO_TEXT);
    //tft.setFreeFont(&FreeSans9pt7b);
    char buf[100];
    sprintf(buf, "%s", name);
    tft.drawString(buf, INFO_X + INFO_MARGIN_X, INFO_LINE2_Y, INFO_GFX_FONT_NUMBER);
    sprintf(buf, "Code: 0x%lX", code);
    tft.drawString(buf, INFO_X + INFO_MARGIN_X, INFO_LINE3_Y, INFO_GFX_FONT_NUMBER);
    sprintf(buf, "Proto: %d  Pulse: %dµs", proto, pulse);
    tft.drawString(buf, INFO_X + INFO_MARGIN_X, INFO_LINE4_Y, INFO_GFX_FONT_NUMBER);
    sprintf(buf, "Répétitions: %d", repeat);
    tft.drawString(buf, INFO_X + INFO_MARGIN_X, INFO_LINE5_Y, INFO_GFX_FONT_NUMBER);
  }

  void CZoneInfo::updateLastReceived(unsigned long code, int bits, int proto, int pulse, int rssi) {
    clearContent();
    tft.setTextColor(TFT_CYAN);
    //tft.setFreeFont(&FreeSansBold9pt7b);
    tft.drawString("Dernière réception :", INFO_X + INFO_MARGIN_X, INFO_LINE1_Y, GFX_FONT_NUMBER);  // Gras

    tft.setTextColor(COLOR_INFO_TEXT);
    //tft.setFreeFont(&FreeSans9pt7b);
    char buf[100];
    sprintf(buf, "Code : 0x%lX Bits : %d  Proto : %d", code, bits, proto);
    tft.drawString(buf, INFO_X + INFO_MARGIN_X, INFO_LINE2_Y, INFO_GFX_FONT_NUMBER);
    sprintf(buf, "Pulse : %dus RSSI : %d dBm", pulse, rssi);
    tft.drawString(buf, INFO_X + INFO_MARGIN_X, INFO_LINE3_Y, INFO_GFX_FONT_NUMBER);
  
  }
#endif