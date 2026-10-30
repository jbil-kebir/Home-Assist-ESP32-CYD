# LoRa — Bandes de fréquences, choix et performances

Version du 2026-10-29. Liaison LoRa point à point (sans LoRaWAN) entre le **Relais-Lora** (XIAO ESP32-S3 + Wio-SX1262,
passerelle LoRa ↔ MQTT) et les **capteurs LoRa** (même kit), aujourd'hui ThRemise et ThCave.

## 1. Rappels

| Terme | Sens |
|---|---|
| SF (facteur d'étalement) | 7 à 12 : chaque cran de plus gagne ≈ 2,5 dB de sensibilité mais **double** le temps d'émission |
| BW (largeur de bande) | 125 kHz ici ; plus étroit = plus sensible mais plus lent |
| RSSI | puissance reçue (dBm) : -30 = très fort, -120 = limite |
| SNR | rapport signal / bruit (dB) : LoRa décode **sous** le bruit, jusqu'à -7,5 dB en SF7, -20 dB en SF12 |
| Cycle d'utilisation | part du temps pendant laquelle un émetteur a le droit d'émettre, **par heure glissante** (1 % = 36 s / h) |
| ERP / EIRP | puissance rayonnée, rapportée à un dipôle (ERP) ou à une antenne isotrope (EIRP = ERP + 2,15 dB) |

## 2. Bandes possibles

Cadre européen : recommandation **ERC/REC 70-03** (annexe 1, appareils à courte portée) et norme **ETSI EN 300 220**.

| Bande | Puissance max | Cycle max | Avantages | Inconvénients |
|---|---|---|---|---|
| **433,05 – 434,79 MHz** | 10 mW ERP (10 dBm) | 10 % | traverse mieux murs et dalles ; portée un peu meilleure à puissance égale | **impossible avec le Wio-SX1262** (adapté 862 – 930 MHz : il faudrait d'autres modules et antennes) ; antennes plus grandes ; bande **encombrée** (télécommandes, sondes météo) et **déjà utilisée chez nous** par le CC1101 du contrôleur et les prises 433 MHz ; puissance plus faible |
| 863,0 – 865,0 MHz | 25 mW ERP (14 dBm) | 0,1 % | peu fréquentée | 0,1 % très contraignant (3,6 s / h) |
| 865,0 – 868,0 MHz | 25 mW ERP | 1 % | – | partagée avec le RFID UHF (forte puissance) : brouillage possible |
| **868,0 – 868,6 MHz (g1)** | **25 mW ERP** | **1 %** | **bande LoRa de référence** (canaux LoRaWAN 868,1 / 868,3 / 868,5) ; matériel et antennes prévus pour elle ; 1 % largement suffisant pour nos trames | bande la plus utilisée par LoRa (réseaux LoRaWAN voisins, compteurs) ; puissance limitée à 14 dBm |
| 868,7 – 869,2 MHz (g2) | 25 mW ERP | 0,1 % | – | 0,1 % |
| **869,4 – 869,65 MHz (g3)** | **500 mW ERP (27 dBm)** | **10 %** | la plus généreuse : jusqu'à 22 dBm avec le SX1262 (+ 8 dB de bilan) et 10 % de cycle | bande étroite (250 kHz) ; plus de consommation à 22 dBm ; souvent réservée aux liaisons descendantes LoRaWAN |
| 869,7 – 870,0 MHz (g4) | 5 mW ERP sans limite, ou 25 mW à 1 % | – / 1 % | – | très faible puissance sans contrainte de temps |
| 902 – 928 MHz | – | – | bande LoRa des Amériques | **interdite en Europe** (c'était notre réglage d'origine, 915 MHz) |
| 2,4 GHz (SX1280) | 10 mW EIRP | – | mondiale, débit plus élevé | autre puce ; portée bien plus faible à travers les murs ; partagée avec le Wi-Fi |

## 3. Choix retenu : 868,1 MHz

Décidé le 2026-10-29 (migration depuis 915 MHz, commit f37c91b ; Capteur 3.0, Relais-Lora 2.0).

| Paramètre | Valeur | Macro (`global.h` du capteur et du relais) |
|---|---|---|
| Fréquence | **868,1 MHz** (sous-bande g1) | `LORA_FREQUENCE` |
| Largeur de bande | 125 kHz | `LORA_BANDWIDTH` |
| Facteur d'étalement | **SF7** | `LORA_SPREADING_FACTOR` |
| Taux de codage | 4/5 | `LORA_CODING_RATE` |
| Mot de synchronisation | 0x12 (privé) | `LORA_SYNC_WORD` |
| Puissance | **14 dBm** | `LORA_OUTPUT_POWER` |
| Préambule | 8 symboles | `LORA_PREAMBLE_LENGTH` |
| TCXO | 1,8 V (Wio-SX1262) | `LORA_TCXO_VOLTAGE` |

Pourquoi :
- **Seule bande légale compatible avec le matériel** (Wio-SX1262 adapté 862 – 930 MHz) sans changer de module.
- **Le cycle de 1 % est très loin d'être atteint.** Une trame de 30 à 40 octets dure ≈ 90 à 110 ms en SF7 / 125 kHz.
  Un capteur en veille profonde se réveille toutes les **600 s** et envoie ≈ 4 à 5 trames (température, humidité,
  batterie, jour / nuit…) : ≈ 0,5 s toutes les 10 min, soit **≈ 0,08 %** du temps (≈ 3 s / h pour 36 s permises).
  Le relais émet encore moins (commandes et accusés seulement).
- **14 dBm** à la sortie du SX1262 + antenne souple (≈ 2 dBi) ≈ 16 dBm EIRP = la limite de 25 mW ERP : on est au
  maximum autorisé, sans le dépasser.
- **SF7** = trames les plus courtes (moins de consommation, moins de collisions) ; suffisant vu les mesures (§ 4).
- Le passage par 869,525 MHz (g3) reste la réserve si la portée manquait un jour (§ 5).

Contrainte : **tous les nœuds LoRa changent ensemble** (fréquence en dur dans `global.h`, pas en NVS) ; un nœud resté
en 915 MHz n'est plus entendu. Firmwares 915 MHz de secours : `Firmwares/firmware-Relais-Lora-1.3.bin`,
`Firmwares/firmware-Capteur-S3-LoRa-flotteur-915-2.1.bin`.

## 4. Performances mesurées (2026-10-29, 868,1 MHz, SF7)

Mesures lues sur le relais (journal `/logs`). Limites du SX1262 en SF7 / 125 kHz : sensibilité ≈ **-124 dBm**,
SNR minimal ≈ **-7,5 dB**. La **marge de SNR** (SNR mesuré + 7,5) est l'indicateur le plus parlant.

| Nœud | Situation | RSSI | SNR | Marge de SNR | Verdict |
|---|---|---|---|---|---|
| ThRemise | sur la table, à côté du relais | -29 dBm | 12,5 dB | – | essai de fonctionnement (aller-retour GET OK) |
| ThRemise | en place, **relais loin** (provisoire) | -94 à -100 dBm | -4,25 à +7 dB | 3 à 14 dB | **faible** : pertes probables |
| ThRemise | en place, **relais au centre de la maison** | **-86 / -87 dBm** | **7 à 12,75 dB** | **≥ 14 dB** | **bon** |
| ThCave | sur la table (relais au centre) | -68 à -76 dBm | 12 à 13 dB | – | trames et SET reçus, acquittés |
| ThCave | **en place à la cave** (relais au centre) | **-73 / -74 dBm** | **12,5 dB** | **≈ 20 dB** | **très bon** |

Constats :
- **L'emplacement du relais compte plus que la fréquence** : ThRemise gagne ≈ 10 dB de RSSI et ≈ 8 dB de SNR quand
  le relais passe au centre de la maison. Le relais doit rester au centre.
- Avec le relais au centre, **SF7 suffit pour les deux nœuds**, avec au moins 14 dB de marge.
- Pas de comparaison avec 915 MHz possible : la mesure de référence (T0) n'a pas été faite avant la migration.
- Au-delà du SNR de 10 dB environ, le SNR plafonne : c'est alors le RSSI qui renseigne sur la marge (ThCave).

## 5. Suite et pistes

Mesure des pertes (2026-10-30, ThRemise, sommeil 30 s, 35 min d'écoute MQTT sans toucher au relais) : **63 réveils,
1 trame perdue sur 315** (0,3 %) ; trame du flotteur et accusé du contrôleur reçus 63 fois sur 63 au premier envoi.
Pendant une surveillance qui interrogeait la page `/logs/data` du relais toutes les 3 s, environ une trame sur dix se perdait :
pendant qu'il sert une page Web, le relais ne lit plus le module radio, et une trame de la rafale (150 ms d'écart) est
écrasée par la suivante. **Ne pas laisser la page Journal du relais ouverte** (rafraîchissement automatique) ou la mettre
en Pause. Amélioration possible : lire la trame dès l'interruption de réception (jugée inutile à ce taux de pertes).

À faire :
- Compter les pertes sur 24 h avec un sommeil de 600 s (ThRemise, ThCave).
- Veille profonde remise sur ThRemise et ThCave le 2026-10-30 ; vérifier la tension de batterie de ThCave (≈ 3 V attendus)
  quand il passera sur piles.
- Refaire les mesures si un nouveau capteur LoRa est posé loin du centre.

Si la portée manquait (marge de SNR < 5 dB, pertes régulières), dans cet ordre :

| Piste | Gain | Prix |
|---|---|---|
| Déplacer / surélever l'antenne du relais ou du capteur | quelques dB à 10 dB | aucun |
| **SF9** | ≈ +5 dB | temps d'émission × 4 (≈ 0,3 % de cycle : encore loin de 1 %), plus de consommation |
| **869,525 MHz (g3)** jusqu'à 22 dBm | ≈ +8 dB | consommation à l'émission plus forte ; changer tous les nœuds ensemble |
| SF12 | ≈ +12 dB | temps × 32 : à éviter (cycle et piles) |
