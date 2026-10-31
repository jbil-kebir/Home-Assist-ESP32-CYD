#include "MyVersion.h"
#include "global.h"

// __DATE__ = « Oct 31 2026 » (jour complété par une espace : « Oct  5 2026 »), __TIME__ = « 14:05:27 »
String versionDateCompilation() {
  static const char* mois[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const char* d = __DATE__;
  int iMois = 0;
  for (int i = 0; i < 12; i++)
    if (strncmp(d, mois[i], 3) == 0) { iMois = i + 1; break; }
  int iJour = atoi(d + 4);
  char s[20];
  snprintf(s, sizeof(s), "%02d/%02d/%s %.5s", iJour, iMois, d + 7, __TIME__);
  return String(s);
}

String versionComplete() {
  return String(VERSION) + " " + versionDateCompilation();
}
