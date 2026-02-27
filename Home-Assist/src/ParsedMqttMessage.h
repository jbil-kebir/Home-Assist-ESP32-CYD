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

    ParsedMqttMessage() = default;

    bool isForce(const String& s) const;
    bool isIp(const String& s) const;
    bool isDate(const String& s) const;
    bool isTime(const String& s) const;    
    bool parse(const String& payload);

    void printDebug() const;
};

#endif