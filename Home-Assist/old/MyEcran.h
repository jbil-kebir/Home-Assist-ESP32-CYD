#ifndef __MYECRAN_H__
#define __MYECRAN_H__

#define TS_MINX 300
#define TS_MINY 400
#define TS_MAXX 3800
#define TS_MAXY 3750

// Ecran
#define RESOLUTION_X  320
#define RESOLUTION_Y  240

// Coordonnées des objets
#define BOUTON_2H_X 20
#define BOUTON_2H_Y 50
#define BOUTON_2H_W 90
#define BOUTON_2H_H 60
#define BOUTON_2H_C TFT_BLUE
#define BOUTON_2H_N "2H"

#define BOUTON_ON_X 115
#define BOUTON_ON_Y 50
#define BOUTON_ON_W 90
#define BOUTON_ON_H 60
#define BOUTON_ON_C TFT_ORANGE //TFT_GREEN
#define BOUTON_ON_N "ON"

#define BOUTON_OFF_X 210
#define BOUTON_OFF_Y 50
#define BOUTON_OFF_W 90
#define BOUTON_OFF_H 60
#define BOUTON_OFF_C TFT_RED
#define BOUTON_OFF_N "OFF"

#define COULEUR_TEXTE_DE_BASE TFT_BLACK
#define COULEUR_FOND_ECRAN TFT_BLACK

// Zone de titre
#define TITRE_X 0
#define TITRE_Y 0
#define TITRE_W 320
#define TITRE_H 30
#define TITRE_C TFT_DARKGREEN

// Texte de titre centré
#define TITRE_MSG_POS_X 160
#define TITRE_MSG_POS_Y 7
#define TITRE_MSG_C TFT_WHITE
#define TITRE_MSG_FONT 2


// Zone de status
#define STATUS_EXT_X 10
#define STATUS_EXT_Y 120
#define STATUS_EXT_W 300
#define STATUS_EXT_H 30
#define STATUS_EXT_C TFT_WHITE

#define STATUS_INT_X (STATUS_EXT_X+2)
#define STATUS_INT_Y (STATUS_EXT_Y+2)
#define STATUS_INT_W (STATUS_EXT_W-4)
#define STATUS_INT_H (STATUS_EXT_H-4)
#define STATUS_INT_C TFT_BLACK
// Message de status
#define STATUS_MSG_POS_X 15
#define STATUS_MSG_POS_Y 128
#define STATUS_MSG_FONT 2

// Zone de status 2
#define STATUS_2_EXT_X 10
#define STATUS_2_EXT_Y 160
#define STATUS_2_EXT_W 300
#define STATUS_2_EXT_H 75
#define STATUS_2_EXT_C TFT_WHITE

#define STATUS_2_INT_X (STATUS_2_EXT_X+5)
#define STATUS_2_INT_Y (STATUS_2_EXT_Y+10)
#define STATUS_2_INT_W (STATUS_2_EXT_W-10)
#define STATUS_2_INT_H (STATUS_2_EXT_H-15)
#define STATUS_2_INT_C TFT_BLACK
// Message de status 2
#define STATUS_2_MSG_POS_X 15
#define STATUS_2_MSG_POS_Y 165
#define STATUS_2_MSG_ON_C TFT_GREEN // Couleur lorsque c'est ON
#define STATUS_2_MSG_OFF_C TFT_RED // Couleur lorsque c'est OFF
#define STATUS_2_MSG_FONT 2

// En minutes. A mettre dans la config NVS
// Mettre une valeur négative ou nulle pour 
// pas de veille
#define TEMPS_VEILLE_MINUTES 0.5 

class CEcran {
private:
  TFT_eSPI tft;
  XPT2046_Touchscreen touch;
  SPIClass touchSPI;  // Membre pour que l'objet survive
  CConfig& mConfig;
  bool mbEcranAllume;  // État du rétroéclairage pour la gestion de la veille légère
  unsigned long mulDernierTouch;

public:
  CEcran(CConfig& cfg) : tft(TFT_eSPI()), touch(XPT2046_CS, XPT2046_IRQ), touchSPI(HSPI), mConfig(cfg) {}

  void setup();
  void loop();
  void drawMainInterface();
  void drawButton(int x, int y, int w, int h, uint16_t color, const char* label);
  void updateStatus(const String& msg);
  bool isTouched(int& x, int& y);
  void updateBoilerStatus(const String& status) {
    tft.drawRect(STATUS_2_EXT_X, STATUS_2_EXT_Y, STATUS_2_EXT_W, STATUS_2_EXT_H, STATUS_2_EXT_C);

    tft.fillRect(STATUS_2_INT_X, STATUS_2_INT_Y, STATUS_2_INT_W, STATUS_2_INT_H, STATUS_2_INT_C);
    uint16_t color = status == "ON" ? STATUS_2_MSG_ON_C : STATUS_2_MSG_OFF_C;
    tft.setTextColor(color, COULEUR_TEXTE_DE_BASE);

    tft.drawString("Chaudiere : " + status, STATUS_2_MSG_POS_X, STATUS_2_MSG_POS_Y, STATUS_2_MSG_FONT);   
     
  }
  void updateTitleWithIP(const String& ip) {
    // Efface la ligne du titre
    tft.fillRect(TITRE_X, TITRE_Y, TITRE_W, TITRE_H, TITRE_C);
    Serial.printf("updateTitleWithIP - ip : %s\n", ip.c_str());
    tft.setTextColor(TITRE_MSG_C);
    
    String title = "AD200 - " + String(FREQUENCE, 3) + " MHz";
    if (ip.length() > 0) {
      title += " - " + ip;
    } else {
      title += " - ...";
    }

    tft.drawCentreString(title.c_str(), TITRE_MSG_POS_X, TITRE_MSG_POS_Y, TITRE_MSG_FONT);
  }
};

#endif // __MYECRAN_H__
