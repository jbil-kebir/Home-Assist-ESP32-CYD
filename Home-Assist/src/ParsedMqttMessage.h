#ifndef PARSED_MQTT_MESSAGE_H
#define PARSED_MQTT_MESSAGE_H

#include <Arduino.h>
#include <vector>

class ParsedMqttMessage {
public:
    String msExpediteur;                    // ex: "ThSdb"
    std::vector<String> mvsMesure;          // ex: ["Temp"] ou ["Tension", "DECHARGEE"]
    int miTailleMesure = 0;
    String msDate;                          // ex: "27/02/2026"
    String msTime;                          // ex: "12:18:48"
    String msIp;                            // ex: "192.168.1.67"
    float mfValeur = 0.0f;                  // la valeur numérique extraite (ex: 23.5)
    bool mbForce = false;
    bool mbIsValid = false;
    // Délai d'alerte annoncé par le nœud (Capteur 3.9 : « alerte:<s> » en fin de trame de mesure), 0 si absent.
    // Jamais rangé dans mvsMesure (les classes Remote lisent la valeur dans mvsMesure.back()).
    unsigned long miDelaiAlerteS = 0;

    ParsedMqttMessage() = default;

    bool isDelaiAlerte(const String& s) const;
    bool isForce(const String& s) const;
    bool isIp(const String& s) const;
    bool isDate(const String& s) const;
    bool isTime(const String& s) const;    
    bool parse(const String& payload);

    void printDebug() const;
};

#endif