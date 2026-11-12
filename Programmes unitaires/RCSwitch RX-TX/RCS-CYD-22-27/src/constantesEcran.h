
#ifndef __CONSTANTESECRAN_H__
#define __CONSTANTESECRAN_H__

// =================== HARDWARE ===================
// Touch HSPI personnalisé
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
// Backlight pin
#define TFT_BL        21


// =================== DIMENSIONS DE L'ECRAN ===================
#define SCREEN_W      320
#define SCREEN_H      240

// =================== NUMÉROS DE POLICE ===================
#define GFX_FONT_NUMBER       1   // Police par défaut (gras pour titres)
#define INFO_GFX_FONT_NUMBER  2   // Police plus fine pour les infos détaillées

// Touch calibration
#define TS_MINX       150
#define TS_MINY       150
#define TS_MAXX       3850
#define TS_MAXY       3850

#endif // __CONSTANTESECRAN_H__


