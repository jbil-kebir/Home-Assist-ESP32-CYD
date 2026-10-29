#ifndef MY_IP_MODULE_H
#define MY_IP_MODULE_H

#include <Arduino.h>

class CIPModule {
    private:
        String msNom;
        String msIP;
        unsigned long mulDernierContact = 0; // millis() du dernier message reçu de ce module
        // Silence maximal (s) avant retrait de la liste (série 5). 0 : jamais retiré (CYD principal, application, puce non configurée).
        // Capteur : son intervalle de watchdog. CYD auxiliaire : "Attente max des CYD auxiliaires".
        unsigned long mulAttenteMax = 0;
    public:
        CIPModule(const String &nom, const String &ip);
        void setNom(const String& nom) {msNom = nom;}
        void setIP(const String& ip) {msIP = ip;}
        const String& getNom() const {return msNom;}
        const String& getIP() const {return msIP;}
        void touche() {mulDernierContact = millis();}
        unsigned long getDernierContact() const {return mulDernierContact;}
        void setAttenteMax(unsigned long s) {mulAttenteMax = s;}
        unsigned long getAttenteMax() const {return mulAttenteMax;}
};
#endif // MY_IP_MODULE_H