#include "global.h"
#ifdef __MODE_ECRAN__
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "constantesEcran.h"
#include "MyZoneTexte.h"

/*CZoneTexte::CZoneTexte(TFT_eSPI& _tft, const char* _text, int _x, int _y, const GFXfont* _font, uint16_t _color)
    : tft(_tft), text(_text), x(_x), y(_y), font(_font), color(_color) {
    }*/

void CZoneTexte::setup() {
    tft.setFreeFont(font);
    tft.setTextColor(color);
    draw();
}

void CZoneTexte::draw() {
    tft.drawCentreString(text, x, y, GFX_FONT_NUMBER);
}
#endif