#ifndef MEMOIRE_RTC_H
#define MEMOIRE_RTC_H

//------------------------------------------------------------------------------
// Dernière mesure envoyée gardée en mémoire RTC pendant le deep sleep (modèle : CJourNuit)
//
// Chaque capteur garde dans son .cpp, en RTC_DATA_ATTR : un indicateur de validité, la dernière valeur
// ENVOYÉE (mise à jour seulement après un envoi réussi) et l'heure système de ce dernier envoi.
// - Réveil de deep sleep (reveilDeepSleep()) et mémoire valide : la dernière valeur est reprise, la mesure
//   n'est envoyée que si elle a changé ou si le forçage est échu.
// - Mise sous tension ou reset : la mémoire RTC n'est pas relue, la 1re mesure est toujours envoyée.
// - Forçage compté en heure système (time()), qui continue d'avancer pendant le deep sleep (millis() repart
//   de 0 à chaque réveil, ce qui empêchait tout forçage en deep sleep).
// - Mesure inchangée : « rien à remonter » (mbMesureRemontee = true), le capteur peut s'endormir.
//------------------------------------------------------------------------------
#include <time.h>
#include <esp_system.h>

// Espacement des tentatives de forçage quand l'envoi échoue (évite une tentative à chaque tour de loop())
#define RTC_RELANCE_FORCAGE_MS  10000UL

inline bool reveilDeepSleep() {
  return esp_reset_reason() == ESP_RST_DEEPSLEEP;
}

// Heure système réglée (NTP) : après le 01/09/2020. Avant la synchro, time() compte depuis le démarrage (1970).
#define RTC_HEURE_VALIDE        ((time_t)1598918400)

// Forçage échu : plus de ulIntervalleS secondes depuis le dernier envoi réussi.
// ulDerniereTentative (millis(), 0 = aucune) espace les essais si l'envoi forcé échoue.
// À la mise sous tension, tDernierEnvoi est noté avant la synchro NTP : quand l'heure est réglée, le saut
// (1970 -> 2026) faisait croire le délai écoulé (trames FORCE au démarrage). Référence alors recalée sur l'heure réglée.
inline bool forcageEchu(time_t& tDernierEnvoi, unsigned long ulIntervalleS, unsigned long& ulDerniereTentative) {
  time_t tMaintenant = time(nullptr);
  if (tDernierEnvoi < RTC_HEURE_VALIDE && tMaintenant >= RTC_HEURE_VALIDE) {
    tDernierEnvoi = tMaintenant;
    return false;
  }
  if ((unsigned long)(tMaintenant - tDernierEnvoi) < ulIntervalleS) return false;
  if (ulDerniereTentative != 0 && millis() - ulDerniereTentative < RTC_RELANCE_FORCAGE_MS) return false;
  ulDerniereTentative = millis();
  if (ulDerniereTentative == 0) ulDerniereTentative = 1;
  return true;
}

#endif
