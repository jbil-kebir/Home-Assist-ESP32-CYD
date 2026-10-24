# Contrôleur HA (sans CYD) — Liste des composants

Liste d'achat pour **une carte** (voir `Schéma global contrôleur HA.md`). La liste exacte des composants dessinés sur le
circuit imprimé est l'export EasyEDA `Controleur_BOM.xlsx` (même dossier) ; cette page y ajoute ce qui n'est pas dessiné et
ce qui est hors carte.

## 1. Sur le circuit imprimé

| Repère | Composant | Référence | Qté |
|---|---|---|---|
| A1 | Module abaisseur MP1584EN 3 A (22 × 17 mm), réglé à 5,0 V | – | 1 |
| A2 | ESP32-WROOM-32 DevKit 38 broches, USB-C (rangées à 25,4 mm) | – | 1 |
| A3 | Seeed XIAO ESP32-S3 + Wio-SX1262 (kit LoRa), antenne Wi-Fi U.FL | – | 1 |
| A4 | Module CC1101 433 MHz EBYTE E07-M1101D + antenne SMA | – | 1 |
| DP, DA, DB | Diode Schottky | 1N5819 | 3 |
| R1 | Résistance 1 kΩ 1/4 W | CF14JT1K00 | 1 |
| C1, C8 | Électrolytique 100 µF 25 V (Ø 6,3, pas 2,5) | 25YXF100MEFC6.3X11 | 2 |
| C3 | Électrolytique 220 µF 16 V (Ø 6,3, pas 2,5) | KM221M016E110A | 1 |
| C6 | Électrolytique 10 µF 16 V (Ø 5, pas 2,0) | UVR1C100MDD | 1 |
| C2, C4, C5, C7, C9 | Céramique 100 nF, **pas de 2,54 mm** | C320C104K5R5TA | 5 |
| J1 | Jack d'alimentation femelle 5,5 × 2,1 mm pour circuit imprimé | DC-005-20 | 1 |
| J2, J4 | Embase JST-XH 2 broches (interrupteur, LED) | B2B-XH-A(LF)(SN) | 2 |
| J3 | Embase JST-XH 4 broches (DHT20) | B4B-XH-A(LF)(SN) | 1 |

## 2. À ajouter sur la carte (non dessiné)

| Pièce | Qté | Remarque |
|---|---|---|
| Barrette femelle 1 × 19 au pas de 2,54 mm | 2 | support de A2 (le DevKit reste démontable) |
| Barrette femelle 1 × 7 au pas de 2,54 mm | 2 | support de A3 (le XIAO reste démontable) |
| Barrette mâle 1 × 7 au pas de 2,54 mm | 2 | à souder sous le XIAO (souvent fournie avec lui) |
| Barrette femelle 2 × 4 au pas de 2,54 mm | 1 | support de A4 (PM254-2-04-Z-8.5) |
| Barrette femelle 1 broche au pas de 2,54 mm | 4 | support de A1 (une par borne ; le module a 2 trous par borne, un seul suffit) |
| Picots mâles pour A1 | 4 | à souder sous le module |
| Vis M3 dans le PETG | 4 | fixation de la carte (trous aux 3 coins libres + bord gauche) |

## 3. Hors carte

| Pièce | Qté | Remarque |
|---|---|---|
| Bloc secteur 12 V 1 A, fiche 5,5 × 2,1 mm, centre + | 1 | déjà possédé |
| Interrupteur à levier MTS-102 | 1 | façade du boîtier, sur fils vers J2 (à l'arrière de la carte) |
| LED 5 mm verte | 1 | façade, à côté de l'interrupteur, sur fils vers J4 |
| Câble JST-XH 2 broches serti | 2 | J2 (≈ 15 cm, la carte traverse le boîtier) et J4 |
| Module DHT20 | 1 | vissé à l'extérieur de la paroi gauche (vis M3 × 8 + écrou), câble par un trou vers J3 (JST-XH 4) |
| Câble JST-XH 4 broches serti | 1 | même brochage que les capteurs (1 3V3, 2 GND, 3 SCL, 4 SDA) |
| Antenne 433 MHz SMA | 1 | fournie avec le CC1101, vissée sur sa prise à travers la paroi arrière |
| Antenne LoRa souple U.FL 40 × 10 mm | 1 | fournie avec le kit Wio-SX1262, collée en haut de la paroi droite |
| Antenne Wi-Fi souple U.FL 40 × 20 mm | 1 | fournie avec le XIAO ESP32-S3, collée contre la paroi gauche |
| Boîtier imprimé en 3D | 1 | `Home-Assist/boitier/` (boîte + couvercle, PETG) |
| Vis M3 × 8 (couvercle) | 4 | dans les colonnes des coins |
| Vis à bois Ø 4 tête fraisée | 2 | oreilles, si le boîtier est fixé à l'étagère |
| Patins caoutchouc adhésifs | 4 | sous le fond |
