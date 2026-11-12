#ifndef __MYZONETEXTE_H__
#define __MYZONETEXTE_H__

// =================== CLASSE CZoneTexte ===================
class CZoneTexte {
private:
  TFT_eSPI& tft;
  const char* text;
  int x, y;
  const GFXfont* font;
  uint16_t color;

public:
  CZoneTexte(TFT_eSPI& _tft, const char* _text, int _x, int _y, const GFXfont* _font, uint16_t _color)
    : tft(_tft), text(_text), x(_x), y(_y), font(_font), color(_color) {
        
    }

  void setup();
  void draw();
};
#endif // __MYZONETEXTE_H__
