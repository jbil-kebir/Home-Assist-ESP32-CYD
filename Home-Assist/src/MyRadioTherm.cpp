#include <Arduino.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "MyRadioTherm.h"
#include "MyConfig.h"     // Pour FREQUENCE, etc.
#include "MyEcran.h"      // Si besoin d'updateStatus

#ifdef __LOCAL_MODE__

/*void CThermDevice::setup() {

    // Ajustements spécifiques pour réception + émission strings
    ELECHOUSE_cc1101.setMHZ(FREQUENCE); // C'est plutôt 433.96 MHz
    ELECHOUSE_cc1101.setModulation(2);       // Garde comme chaudière (ASK)
    ELECHOUSE_cc1101.setDRate(2.0);          // 2000 bps – adapté pour strings simples
    ELECHOUSE_cc1101.setRxBW(250.0);         // Bande passante raisonnable
    ELECHOUSE_cc1101.setPA(10);              // Puissance max
    ELECHOUSE_cc1101.SetRx();                // Démarre en réception

}

void CThermDevice::loop() {
    // 1. Gestion réception (polling non bloquant)
    if (ELECHOUSE_cc1101.CheckRxFifo(0)) {   // Paquet disponible ?
        rxLen = ELECHOUSE_cc1101.ReceiveData(rxBuffer);

        if (rxLen > 0) {
            // Termine la string (au cas où pas de \0 envoyé)
            if (rxLen < sizeof(rxBuffer)) {
                rxBuffer[rxLen] = '\0';
            } else {
                rxBuffer[sizeof(rxBuffer)-1] = '\0';
            }

            Serial.printf("Message reçu (%d bytes) : %s\n", rxLen, (char*)rxBuffer);

            // Callback si défini (ex: publish MQTT, update écran)
            if (onMessageReceived) {
                onMessageReceived((const char*)rxBuffer);
            }

            newMessageAvailable = true;

            // Option : update écran
            // if (mEcran) mEcran->updateStatus(String("RX: ") + (char*)rxBuffer);
        }

        // Retour en RX après lecture
        ELECHOUSE_cc1101.SetRx();
    }

    // 2. Émission périodique auto (optionnel – active si besoin)
    // if (millis() - lastTxTime >= txIntervalMs) {
    //     // Exemple : envoyer température si tu as un capteur local
    //     // transmitMessage("ThCave TEMP 28/01/2026 19:00:00 19.7");
    //     lastTxTime = millis();
    // }
}

// Émission d'un message texte simple
void CThermDevice::transmitMessage(const char* message) {
    if (!message || strlen(message) == 0) return;

    Serial.printf("Emission message : %s\n", message);

    digitalWrite(LED_RED, LED_ON);

    ELECHOUSE_cc1101.setMHZ(FREQUENCE);
    ELECHOUSE_cc1101.SetTx();
    delay(20);

    // Envoi direct (la lib gère le préambule/syncword automatiquement en mode packet)
    ELECHOUSE_cc1101.SendData((byte*)message, strlen(message) + 1);  // +1 pour inclure \0

    delay(50);
    ELECHOUSE_cc1101.SetRx();  // Retour en réception

    digitalWrite(LED_RED, LED_OFF);
    Serial.println("Emission message terminée");
}

// Émission d'une commande (ex: "FORCE_MEASURE", "SET_INTERVAL 60000")
void CThermDevice::transmitCommand(const char* cmd) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "CMD:%s", cmd);  // Préfixe pour distinguer
    transmitMessage(buffer);
}

// Récupère le dernier message reçu (consomme le flag)
bool CThermDevice::getReceivedMessage(char* buffer, size_t bufSize) {
    if (!newMessageAvailable || rxLen <= 0) return false;

    strncpy(buffer, (char*)rxBuffer, bufSize);
    buffer[bufSize-1] = '\0';

    newMessageAvailable = false;
    return true;
}*/

#endif // __LOCAL_MODE__