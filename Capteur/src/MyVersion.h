#ifndef __MYVERSION_H__
#define __MYVERSION_H__

#include <Arduino.h>

// Version du logiciel (VERSION, global.h) et date et heure de compilation.
// MyVersion.cpp est recompilé à chaque compilation (script version_build.py, extra_scripts de platformio.ini) :
// sans cela, __DATE__ et __TIME__ garderaient la date de sa dernière compilation.
String versionDateCompilation();  // « 31/10/2026 14:05 »
String versionComplete();         // « 3.4 31/10/2026 14:05 »

#endif // __MYVERSION_H__
