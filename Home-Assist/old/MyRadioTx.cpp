#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "global.h"
#include "MyConfig.h"
#include "MyEcran.h"

// === VALEURS MOYENNES (µs) ===
#define A 210,205
#define B 210,453
#define C 449,205
#define D 449,453
#define LAST_PULSE_HIGH 449

// === TRAMES ===
#define NB_CARACTERES_ON_1 73
#define TAILLE_BUFFER_ON_1 (2 * NB_CARACTERES_ON_1 + 1)
const uint16_t pulseBufferON[TAILLE_BUFFER_ON_1] = {
  A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
  D,A,D,A,A,A,A,A,B,A,A,C,A,B,A,D,
  C,A,B,A,C,B,A,C,A,A,A,A,A,
  A,A,A,A,A,A,A,
  B,C,A,B,C,A,A,A,A,A,A,A,
  B,C,A,B,C,A,A,A,B,
  LAST_PULSE_HIGH
};

#define NB_CARACTERES_ACK_ON 73
#define TAILLE_BUFFER_ACK_ON (2 * NB_CARACTERES_ACK_ON + 1)
const uint16_t pulseBufferACK_ON[TAILLE_BUFFER_ACK_ON] = {
  A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
  D,A,D,A,A,A,A,A,B,A,A,C,A,B,A,D,
  C,A,B,A,C,B,A,C,A,A,A,A,A,
  B,C,A,A,A,A,
  B,C,A,B,C,A,A,A,A,A,A,A,
  B,C,A,B,C,A,A,A,A,B,
  LAST_PULSE_HIGH
};

#define NB_CARACTERES_OFF_1 74
#define TAILLE_BUFFER_OFF_1 (2 * NB_CARACTERES_OFF_1 + 1)
const uint16_t pulseBufferOFF[TAILLE_BUFFER_OFF_1] = {
  A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
  D,A,D,A,A,A,A,A,B,A,A,C,A,B,A,D,
  C,A,B,A,C,B,A,C,A,A,A,A,A,
  A,A,A,A,A,A,A,
  B,C,A,B,C,A,A,A,A,A,A,A,
  B,A,C,A,B,A,A,A,A,A,
  LAST_PULSE_HIGH
};

#define NB_CARACTERES_ACK_OFF 74
#define TAILLE_BUFFER_ACK_OFF (2 * NB_CARACTERES_ACK_OFF + 1)
const uint16_t pulseBufferACK_OFF[TAILLE_BUFFER_ACK_OFF] = {
  A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,
  D,A,D,A,A,A,A,A,B,A,A,C,A,B,A,D,
  C,A,B,A,C,B,A,C,A,A,A,A,A,
  B,C,A,A,A,A,
  B,C,A,B,C,A,A,A,A,A,A,A,
  B,A,C,A,B,A,A,A,A,D,
  LAST_PULSE_HIGH
};

#include "MyRadioTx.h"

// === CLASSES ===

  void CRadioTX::setup() {
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setMHZ(FREQUENCE);
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.setPA(10);
    ELECHOUSE_cc1101.SetRx();

    pinMode(CC1101_GDO0, INPUT);
    pinMode(LED_RED, OUTPUT);
    digitalWrite(LED_RED, LED_OFF);

    // Aucune transmission n'est en cours
    mbEnvoyerTramesON = false; 
    mbEnvoyerTramesOFF = false;
    // Obsolète
    miCompteurEnvois = 0;
    // Keep alive
    //mbKeepAliveActive = config.chaudiere.enableKeepAlive == 0 ? false : true; 
    mbKeepAliveActive = false;  // Ce paramètre sera ajusté après envoi de la première trame ON ou OFF
    mbTrameCommandeEnvoyee = false;

    // On regarde l'état de la chaudière dans la config
    // car s'il y a eu un redémarrage inoppiné de notre
    // microcontrolleur, il faudra repartir du dernier
    // état
    if (config.etat) { // ON
      mbEnvoyerTramesON = true; // On demande l'envoi d'une trame ON
      mbEnvoyerTramesOFF = false;
      mbTrameCommandeEnvoyee = false; // La trame de commande n'a pas encore été envoyée
      muiKeepAliveIndexEnCours = 0; // Pour les keep-alive, on recommence à 0
      mbKeepAliveActive = false;  // Ce paramètre sera ajusté après envoi de la première trame ON ou OFF
    }
    else {
      mbEnvoyerTramesON = false;
      mbEnvoyerTramesOFF = true;
      mbTrameCommandeEnvoyee = false;
      muiKeepAliveIndexEnCours = 0;
      mbKeepAliveActive = false;
    }    
    
    
     
 
  }

  void CRadioTX::loop() {
    static unsigned long delaiAvantEnvoi = millis();
    static unsigned long lastKeepAliveTime = 0;

    if (  mbEnvoyerTramesON && // Si envoi trame ON demandé
          !mbTrameCommandeEnvoyee // et pas encore envoyée
    ) {
      Serial.printf("Demande ON et pas encore envoyee...\n");
      // On envoi la trame ON
      transmitPulses(pulseBufferON, TAILLE_BUFFER_ON_1, "Commande ON");
      mbTrameCommandeEnvoyee = true; // On signale que la première trame a été envoyée
      Serial.printf("\t...Demande ON envoyee\n");
      // Activation keep-alive après première trame ON
      mbKeepAliveActive = config.chaudiere.enableKeepAlive == 0 ? false : true;
      muiKeepAliveIndexEnCours = 0;
      lastKeepAliveTime = millis();
    }
    else if ( mbEnvoyerTramesOFF && // Si envoi trame OFF demandé
              !mbTrameCommandeEnvoyee // et pas encore envoyée
    ) {
      Serial.printf("Demande OFF et pas encore envoye\n");
      // On envoi la trame ON
      transmitPulses(pulseBufferOFF, TAILLE_BUFFER_OFF_1, "Commande OFF");
      mbTrameCommandeEnvoyee = true; // On signale que la première trame a été envoyée
      Serial.printf("\t...Demande OFF envoyee\n");
      // Activation keep-alive après première trame OFF
      mbKeepAliveActive = config.chaudiere.enableKeepAlive == 0 ? false : true;
      muiKeepAliveIndexEnCours = 0;
      lastKeepAliveTime = millis();
    }
    else if (  mbEnvoyerTramesON && // Si envoi trame ON demandé
          mbTrameCommandeEnvoyee && // et trame déjà envoyée
          mbKeepAliveActive   // et Keep active activé
    ) {
      if (millis() - lastKeepAliveTime > config.chaudiere.onKeepalive[muiKeepAliveIndexEnCours] * 1000UL) { // S'il est temps d'envoyer un keep alive
        Serial.printf("Demande ON deja encore envoye - Keep alive actif\n");
        if (muiKeepAliveIndexEnCours + 1 >= config.chaudiere.on_keepalive_count) { // Si le dernier Keppe alive du tableau a été envoyé, on le renvoit
          muiKeepAliveIndexEnCours = config.chaudiere.on_keepalive_count - 1;  // Répète le dernier (626s)
          //Serial.printf("\t...Envoi ON keep alive %d\n", muiKeepAliveIndexEnCours);
        }
        String s = "Keep-Alive ON " + String(config.chaudiere.onKeepalive[muiKeepAliveIndexEnCours]) + "s";
        Serial.printf("..Envoi ON Keep-Alive : %d - %s\n", muiKeepAliveIndexEnCours, s.c_str());
        transmitPulses(pulseBufferON, TAILLE_BUFFER_ACK_ON, s.c_str());
        muiKeepAliveIndexEnCours++;
        lastKeepAliveTime = millis();

      }
    }
    else if (  mbEnvoyerTramesOFF && // Si envoi trame OFF demandé
          mbTrameCommandeEnvoyee && // et trame déjà envoyée
          mbKeepAliveActive   // et Keep active activé
    ) {
      if (millis() - lastKeepAliveTime > config.chaudiere.onKeepalive[muiKeepAliveIndexEnCours] * 1000UL) { // S'il est temps d'envoyer un keep alive
        Serial.printf("Demande OFF deja encore envoye - Keep alive actif\n");
        if (muiKeepAliveIndexEnCours + 1 >= config.chaudiere.off_keepalive_count) { // Si le dernier Keppe alive du tableau a été envoyé, on le renvoit
          muiKeepAliveIndexEnCours = config.chaudiere.off_keepalive_count - 1;  // Répète le dernier (626s)
          //Serial.printf("\t...Envoi OFF keep alive %d\n", muiKeepAliveIndexEnCours);
        }
        String s = "Keep-Alive OFF " + String(config.chaudiere.offKeepalive[muiKeepAliveIndexEnCours]) + "s";
        Serial.printf("..Envoi OFF Keep-Alive : %d - %s\n", muiKeepAliveIndexEnCours, s.c_str());
        transmitPulses(pulseBufferOFF, TAILLE_BUFFER_ACK_OFF, s.c_str());
        muiKeepAliveIndexEnCours++;
        lastKeepAliveTime = millis();

      }

    }

  }
  // bStatus = true si Chaudiere ON, false sinon.
  // On met en conséquence les variables dans les 
  // bons états.
  // Utile lors du setup surtout en cas de redémarrage inoppiné
/*  int CRadioTX::isetUpStatus(bool bStatus) {
    int ret = 0;
    if (bStatus) {
      mbEnvoyerTramesON = true;
      mbEnvoyerTramesOFF = false;
      mbTrameCommandeEnvoyee = false;
      mbKeepAliveActive = false;
      muiKeepAliveIndexEnCours = 0; 
    }
    else {
      mbEnvoyerTramesON = false;
      mbEnvoyerTramesOFF = true;
      mbTrameCommandeEnvoyee = false;
      mbKeepAliveActive = false;
      muiKeepAliveIndexEnCours = 0; 
    }
    return ret;
  }*/

  bool CRadioTX::bSetEnvoyerTramesON(bool b) {
    bool bLastVal = mbEnvoyerTramesON;
    Serial.printf("bSetEnvoyerTramesON ancien : %d - nouveau : %d\n", bLastVal, b);
    mbEnvoyerTramesON = b;
    mbTrameCommandeEnvoyee = false;
    mbKeepAliveActive = false;
    muiKeepAliveIndexEnCours = 0;
    return bLastVal;
  }
  bool CRadioTX::bSetEnvoyerTramesOFF(bool b) {
    bool bLastVal = mbEnvoyerTramesOFF;
    Serial.printf("bSetEnvoyerTramesOFF ancien : %d - nouveau : %d\n", bLastVal, b);
    mbEnvoyerTramesOFF = b;
    mbTrameCommandeEnvoyee = false;
    mbKeepAliveActive = false;
    muiKeepAliveIndexEnCours = 0;
    return bLastVal;
  }
  bool CRadioTX::bGetEnvoiEnCours(void) { 
    return mbEnvoyerTramesON || mbEnvoyerTramesOFF;
  }

  void CRadioTX::transmitPulses(const uint16_t* pulses, int nbPulses, const char* action) {
    Serial.printf("\n=== EMISSION %s : %d pulses ===\n", action, nbPulses);
    ecran->updateStatus(String("Emission ") + action + "...");
    digitalWrite(LED_RED, LED_ON);

    ELECHOUSE_cc1101.SetTx();
    delay(20);
    pinMode(CC1101_GDO0, OUTPUT);
    digitalWrite(CC1101_GDO0, LOW);
    delay(50);

    bool state = HIGH;
    for (int i = 0; i < nbPulses; i++) {
      digitalWrite(CC1101_GDO0, state);
      delayMicroseconds(pulses[i]);
      state = !state;
    }

    digitalWrite(CC1101_GDO0, LOW);
    delay(50);

    pinMode(CC1101_GDO0, INPUT);
    ELECHOUSE_cc1101.SpiStrobe(0x36);
    delay(10);
    ELECHOUSE_cc1101.SetRx();

    digitalWrite(LED_RED, LED_OFF);
    ecran->updateStatus(String("Emission ") + action + " terminee");
    Serial.println("Emission terminee\n");
  }
