#ifndef __MYBOUTON_H__
#define __MYBOUTON_H__

#define BTN_X         20
#define BTN_W         280
#define BTN_H         35
#define BTN_RADIUS    8

#define BTN1_Y        40
#define BTN2_Y        80
#define BTN3_Y        120

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// =================== CLASSE CBouton ===================
class CBouton {
private:
  TFT_eSPI& tft;
  XPT2046_Touchscreen& touch;
  const char* label;
  int x, y, w, h, radius;
  uint16_t color;

public:
  CBouton(TFT_eSPI& _tft, XPT2046_Touchscreen& _touch, const char* _label,
          int _x, int _y, int _w, int _h, int _radius, uint16_t _color)
    : tft(_tft), touch(_touch), label(_label), x(_x), y(_y), w(_w), h(_h), radius(_radius), color(_color) {
        
    }

  void setup();
  void draw();
  bool isPressed();
  void waitRelease();
};
#endif // __MYBOUTON_H__
