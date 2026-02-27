#ifndef __MYDATETIME_H__
#define __MYDATETIME_H__

#include <Arduino.h>
#include <time.h>

class CMyDateTime {
    private:
        // Configuration NTP
        const char* ntpServer = "pool.ntp.org";
        const long gmtOffset_sec = 3600;  // GMT+1 pour la France
        const int daylightOffset_sec = 3600;  // Heure d'été
    public:
        int setup();
        String getDateTime();
        String getDate();
        String getTime();
        String getDateTimeISO();
        unsigned long getAbsoluteSecondes() {return time(NULL);} // Retourne le nb de secondes depuis le 01/01/1970 00:00

};
#endif // __MYDATETIME_H__