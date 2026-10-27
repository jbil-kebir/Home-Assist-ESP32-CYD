#ifndef __MYDATETIME_H__
#define __MYDATETIME_H__

#include <Arduino.h>
#include <time.h>

class CMyDateTime {
    private:
        // Configuration NTP
        const char* ntpServer = "pool.ntp.org";
        const char* tzFrance = "CET-1CEST,M3.5.0,M10.5.0/3"; // Heure d'été/hiver automatique
        const uint32_t NTP_TIMEOUT_MS = 5000; // Attente maximale de la 1re synchro (mise sous tension)
    public:
        int setup();
        String getDateTime();
        String getDate();
        String getTime();
        String getDateTimeISO();
        unsigned long getAbsoluteSecondes() {return time(NULL);} // Retourne le nb de secondes depuis le 01/01/1970 00:00
        // Heure synchronisée par NTP depuis la mise sous tension (conservée pendant un deep sleep) ? Ne bloque pas.
        bool isTimeValid();

};
#endif // __MYDATETIME_H__