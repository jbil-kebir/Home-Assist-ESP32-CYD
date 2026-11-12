#ifndef __MYZONEINFO_H__
#define __MYZONEINFO_H__

// =================== CZoneInfo ===================
#define INFO_X          10
#define INFO_Y          165
#define INFO_W          300
#define INFO_H          (SCREEN_H - INFO_Y - 10)
#define INFO_RADIUS     8

#define INFO_TITLE_Y    (INFO_Y + 8)

#define INFO_LINE1_Y    INFO_TITLE_Y //(INFO_TITLE_Y + 20)
#define INFO_LINE2_Y    (INFO_TITLE_Y + 20)
#define INFO_LINE3_Y    (INFO_TITLE_Y + 40)
#define INFO_LINE4_Y    (INFO_TITLE_Y + 60)
#define INFO_LINE5_Y    (INFO_TITLE_Y + 80)

#define INFO_MARGIN_X   10

#define COLOR_INFO_BG     TFT_DARKGREY
#define COLOR_INFO_TEXT   TFT_CYAN


// =================== CLASSE CZoneInfo ===================
class CZoneInfo {
private:
  TFT_eSPI& tft;

public:
  CZoneInfo(TFT_eSPI& _tft) : tft(_tft) {}

  void setup();
  void clearContent();
  void updateLastSent(const char* name, unsigned long code, int proto, int pulse, int repeat);
  void updateLastReceived(unsigned long code, int bits, int proto, int pulse, int rssi);
};
#endif // __MYZONEINFO_H__