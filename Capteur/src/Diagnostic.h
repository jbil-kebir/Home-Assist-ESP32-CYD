#pragma once
#include <Arduino.h>

// Diagnostic d'un cycle d'éveil : horodatages millis() depuis le boot (0 = étape non atteinte).
// gDiag est rempli pendant le cycle en cours et recopié dans gDiagPrec (mémoire RTC, conservée
// pendant le deep sleep) juste avant de dormir. gDiagPrec est publié au réveil suivant dans le
// message REVEIL : on voit ainsi où un cycle muet s'est arrêté.
struct DiagCycle {
  uint32_t ulWifi;        // Wi-Fi connecté
  uint32_t ulNtp;         // Fin de la synchro NTP
  uint32_t ulMqtt;        // Première connexion MQTT réussie
  uint32_t ulPub;         // Toutes les mesures remontées
  uint32_t ulEcho;        // Écho reçu dans powerDown() (toutes les publications arrivées au broker)
  uint32_t ulFermeture;   // Connexion fermée par le broker après le DISCONNECT
  uint32_t ulSommeil;     // Entrée en deep sleep
  uint16_t uiEssaisMqtt;  // Nombre de tentatives de connexion MQTT
  int16_t  iDernierRc;    // Code du dernier échec MQTT (PubSubClient::state())
  uint8_t  ucWifiRapide;  // Canal mémorisé : 0 non utilisé, 1 utilisé, 2 échec (connexion normale)
};

extern DiagCycle gDiag;
extern DiagCycle gDiagPrec;

String diagToString(const DiagCycle& d);
