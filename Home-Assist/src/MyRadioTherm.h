#ifndef __MYRADIOTHERM_H__
#define __MYRADIOTHERM_H__

#include "MyRadioTx.h"   // Hérite de CRadioTX pour réutiliser setup_CC1101 et transmitPulses si besoin

class CThermDevice : public CRadioTX {
private:
    // Pour réception
    byte rxBuffer[72];           // Buffer un peu plus grand que les messages
    int  rxLen = 0;
    bool newMessageAvailable = false;

    // Pour émission température (exemple)
    unsigned long lastTxTime = 0;
    unsigned long txIntervalMs = 30000;  // 30s par défaut

    // Callback pour traiter les messages reçus (ex: MQTT publish, affichage écran)
    std::function<void(const char* msg)> onMessageReceived = nullptr;

public:
    CThermDevice() = default;

    // Constructeur avec callback optionnel
    CThermDevice(std::function<void(const char* msg)> cb) : onMessageReceived(cb) {}

    void setup();
    void loop();

    // Émission d'un message texte (température, etc.)
    //void transmitMessage(const char* message);

    // Émission d'une commande (ex: "FORCE_MEASURE", "SET_INTERVAL 60000")
    //void transmitCommand(const char* cmd);

    // Récupérer le dernier message reçu (si disponible)
    //bool getReceivedMessage(char* buffer, size_t bufSize);

    // Configurer intervalle d'émission auto (si tu veux auto-tx)
    //void setTxInterval(unsigned long ms) { txIntervalMs = ms; }

    // Callback pour nouveaux messages
    void setMessageCallback(std::function<void(const char* msg)> cb) {
        onMessageReceived = cb;
    }

    // Pour debug / status
    //bool hasNewMessage() const { return newMessageAvailable; }
};

#endif // __MYRADIOTHERM_H__