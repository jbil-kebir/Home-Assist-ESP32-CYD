#ifndef __GLOBAL_H__
#define __GLOBAL_H__

// Interface tactile (3 boutons d'envoi + zone d'infos). Commenté : sniffer seul, trames reçues sur le port série.
#define __MODE_ECRAN__

// Sans écran : émission d'une trame de test (0xABCDEF, protocole 1, 200 µs) toutes les 3 s. À NE PAS activer pendant
// une capture : le sniffer est sourd pendant chaque envoi et il pollue la bande 433 MHz.
//#define __TEST_EMISSION__

#endif // __GLOBAL_H__
