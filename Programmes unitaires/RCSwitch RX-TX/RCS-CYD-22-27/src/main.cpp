/*
 * CYD (ESP32 + ILI9341) + CC1101 – FULLY ORIENTÉ OBJET
 * CZoneInfo : première ligne en GFX_FONT_NUMBER (1), reste en INFO_GFX_FONT_NUMBER (2)
 * Sans __MODE_ECRAN__ (global.h) : sniffer seul, chaque trame reçue est affichée sur le port série.
 * Répétitions à l'envoi : 5 pour chaque équipement (tableau equipement[])
 */
#include "global.h"
#include <SPI.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>
#include "MyRCSwitch.h"

#ifdef __MODE_ECRAN__
#include "constantesEcran.h"
#include "MyBouton.h"
#include "MyZoneTexte.h"
#include "MyZoneInfo.h"
#include "MyEcran.h"
#endif

// Configuration des équipements
/*******************************************************************************************
 * Sonnette : 
 * Sonnette : 0x39CAB8, 199 us
 * Interrupteur simple : 0x3D3B72, 349 us
 * Interrupteur double gauche : 0xA882B8, 320 us
 * Interrupteur double milieu : 0xA882B4, 320 us
 * Interrupteur double droite : 0xA882B2, 713 us
 * Prise chauffage SdB : ON 0x41E9E1, OFF 0x41E9E2, 311 us
 * Guirlande (vraies valeurs) : 0xA882B2, protocole 5, 713 us
 * Chauffage chambre RdC : 0x5B0E64, 306 us (capture du 12/11/2026)
 * L'ordre de l'enum = l'ordre du tableau equipement[] = les boutons 1, 2, 3 de l'écran (libellés dans MyEcran.h)
 *******************************************************************************************/
enum {PROJECTEUR, PRISE_OFF, INTER_DOUBLE_G, NB_EQUIPEMENTS};
struct Equipement_RF {
  const char* name;
  unsigned long code;
  unsigned int protocol;
  unsigned int pulseWidth;
  unsigned int repetition;
  bool state; // Etat initial : false = éteint
};
Equipement_RF equipement[NB_EQUIPEMENTS] = {
  {"Projecteur", 0x3D3B72UL, 1, 349, 5, false},
  {"Prise OFF", 0x41E9E2UL, 1, 311, 5, false},       // Prise du chauffage SdB, code OFF
  {"Inter double G", 0xA882B8UL, 1, 320, 5, false}   // Interrupteur double, touche gauche
};


//===================================================================================
//                    OBJETS GLOBAUX 
//===================================================================================
#ifdef __MODE_ECRAN__
CEcran ecran;
#endif
CMyRCSwitch rcSwitch;

//===================================================================================
//                    SETUP PRINCIPAL 
//===================================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("\n=== Démarrage CYD – CZoneInfo avec police fine pour infos ==="));

  #ifdef __MODE_ECRAN__
  ecran.setup();
  #endif
  rcSwitch.setup();

  Serial.println(F("Système prêt !"));
}

//===================================================================================
//                    LOOP
//===================================================================================
void loop() {
  #ifdef __MODE_ECRAN__
  int btn = ecran.getPressedButton();
  #else
  int btn = -1;
  #ifdef __TEST_EMISSION__ // Voir global.h : désactivé par défaut (sniffer sourd pendant l'envoi)
  static unsigned long ulCompteurEnvoiCodeRcSwitch = 0L;

  if (millis() - ulCompteurEnvoiCodeRcSwitch >= 3000) {
      Serial.println(F(">>> EMISSION DE TEST : 0xABCDEF"));
      rcSwitch.send(0xABCDEF, 1, 200, 3);
      ulCompteurEnvoiCodeRcSwitch = millis();
  }
  #endif // __TEST_EMISSION__
  #endif

  if (btn > 0) {
    switch (btn) {
      case 1:
        Serial.println(F(">>> APPUI : PROJECTEUR"));
//        rcSwitch.send(PROJECTEUR_CODE, PROJECTEUR_PROTO, PROJECTEUR_PULSE, PROJECTEUR_REPEAT);
        rcSwitch.send(equipement[PROJECTEUR].code, equipement[PROJECTEUR].protocol, equipement[PROJECTEUR].pulseWidth, equipement[PROJECTEUR].repetition);
  #ifdef __MODE_ECRAN__
        ecran.updateLastSent(equipement[PROJECTEUR].name, equipement[PROJECTEUR].code, equipement[PROJECTEUR].protocol, equipement[PROJECTEUR].pulseWidth, equipement[PROJECTEUR].repetition);
  #endif
        break;
      case 2:
        Serial.println(F(">>> APPUI : PRISE OFF"));
        rcSwitch.send(equipement[PRISE_OFF].code, equipement[PRISE_OFF].protocol, equipement[PRISE_OFF].pulseWidth, equipement[PRISE_OFF].repetition);
  #ifdef __MODE_ECRAN__
        ecran.updateLastSent(equipement[PRISE_OFF].name, equipement[PRISE_OFF].code, equipement[PRISE_OFF].protocol, equipement[PRISE_OFF].pulseWidth, equipement[PRISE_OFF].repetition);
  #endif
        break;
      case 3:
        Serial.println(F(">>> APPUI : INTER DOUBLE G"));
        rcSwitch.send(equipement[INTER_DOUBLE_G].code, equipement[INTER_DOUBLE_G].protocol, equipement[INTER_DOUBLE_G].pulseWidth, equipement[INTER_DOUBLE_G].repetition);
  #ifdef __MODE_ECRAN__
        ecran.updateLastSent(equipement[INTER_DOUBLE_G].name, equipement[INTER_DOUBLE_G].code, equipement[INTER_DOUBLE_G].protocol, equipement[INTER_DOUBLE_G].pulseWidth, equipement[INTER_DOUBLE_G].repetition);
  #endif
        break;
        default:
        Serial.println(F(">>> OPERATION NON PRISE EN CHARGE"));
  #ifdef __MODE_ECRAN__
        ecran.updateLastSent("", 0, 0, 0, 0);
  #endif
        break;
    }
  }

  if (rcSwitch.available()) {
    unsigned long value = rcSwitch.getReceivedValue();
    if (value != 0) {
      int bits = rcSwitch.getReceivedBitlength();
      int proto = rcSwitch.getReceivedProtocol();
      int pulse = rcSwitch.getReceivedDelay();
      int rssi = rcSwitch.getRssi();

  #ifdef __MODE_ECRAN__
      ecran.updateLastReceived(value, bits, proto, pulse, rssi);
  #endif

      Serial.println(F("========== TRAME REÇUE =========="));
      Serial.printf("Hexa     : 0x%lX\n", value);
      Serial.printf("Bits     : %d\n", bits);
      Serial.printf("Proto    : %d\n", proto);
      Serial.printf("Pulse    : %d µs\n", pulse);
      Serial.printf("RSSI     : %d dBm\n", rssi);
      Serial.println(F("================================\n"));
    }
    rcSwitch.resetAvailable();
  }
  #ifdef __MODE_ECRAN__
  delay(10);
  #else
  //delay(3000);
  #endif
}