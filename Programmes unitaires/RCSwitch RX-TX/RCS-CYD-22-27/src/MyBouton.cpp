#include "global.h"
#ifdef __MODE_ECRAN__
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "constantesEcran.h"
#include "MyBouton.h"


/*
CBouton::CBouton(TFT_eSPI& _tft, XPT2046_Touchscreen& _touch, const char* _label,
        int _x, int _y, int _w, int _h, int _radius, uint16_t _color)
: tft(_tft), touch(_touch), label(_label), x(_x), y(_y), w(_w), h(_h), radius(_radius), color(_color) {

}*/

void CBouton::setup() {
    tft.fillRoundRect(x, y, w, h, radius, color);
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.setTextColor(TFT_WHITE);
    draw();
}

void CBouton::draw() {
    tft.drawCentreString(label, x + w / 2, y + 10, GFX_FONT_NUMBER);
}

bool CBouton::isPressed() {
    if (!touch.touched()) return false;

    TS_Point p = touch.getPoint();
    int ty = map(p.y, TS_MINY, TS_MAXY, 0, SCREEN_H);

    return (ty >= y && ty <= y + h);
}

void CBouton::waitRelease() {
    while (touch.touched()) delay(10);
}

#endif