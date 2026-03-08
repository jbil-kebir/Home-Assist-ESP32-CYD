#ifndef MY_IP_MODULE_H
#define MY_IP_MODULE_H

#include <Arduino.h>

class CIPModule {
    private:
        String msNom;
        String msIP;
    public:
        CIPModule(const String &nom, const String &ip);
        void setNom(const String& nom) {msNom = nom;}
        void setIP(const String& ip) {msIP = ip;}
        const String& getNom() const {return msNom;}
        const String& getIP() const {return msIP;}
};
#endif // MY_IP_MODULE_H