# Capteur C3 Xiao — Liste des composants

Liste d'achat pour **une carte**, par variante (chaudière / cuve, voir `CapteurC3Xiao.md` § 3).
La liste exacte des composants dessinés sur le circuit imprimé est l'export EasyEDA `BOM_Capteur C3 Xiao.xlsx`
(même dossier) ; cette page y ajoute ce qui n'est pas dessiné et ce qui est hors carte.

## 1. Sur le circuit imprimé

| Repère | Composant | Référence | Qté | Chaudière | Cuve |
|---|---|---|---|---|---|
| A1 | Seeed XIAO ESP32-C3 (antenne externe U.FL) | – | 1 | ✔ | ✔ |
| C1 | Céramique 100 nF | C320C104K5R5TA | 1 | ✔ | ✔ |
| C2 | Électrolytique 10 µF 16 V (Ø 5, pas 2,0) | UVR1C100MDD | 1 | ✔ | ✔ |
| TP1–TP4 | Points de test 3V3, GND, SDA, SCL | barrette mâle 1 broche PZ254V-11-01P | 4 | ✔ | ✔ |
| J1 | Embase JST-XH 4 broches (DHT20) | B4B-XH-A(LF)(SN) | 1 | ✔ | ✔ |
| J2 | Embase JST-XH 5 broches (TCS34725) | B5B-XH-A(LF)(SN) | 1 | ✔ | – |
| R1 | Résistance 100 Ω 1/4 W | PCF14JT100R | 1 | ✔ | – |
| D1 | LED bleue 5 mm | XL-502UBC | 1 | ✔ | – |
| J3 | Bornier à vis 2 points pas 5,0 (piles) | XY301V-A-5.0-2P | 1 | – | ✔ |
| D2 | Diode Schottky | 1N5819 | 1 | – | ✔ |
| J4 | Embase JST-XH 2 broches (interrupteur) | B2B-XH-A(LF)(SN) | 1 | – | ✔ |
| C3 | Électrolytique 10 µF 16 V (Ø 5, pas 2,0) | UVR1C100MDD | 1 | – | ✔ |
| U1 | Régulateur 3,3 V TO-92 | MCP1700-3302E/TO | 1 | – | ✔ |
| C4 | Céramique 1 µF | RDER71H105K2K1H03B | 1 | – | ✔ |
| C5 | Électrolytique 470 µF 10 V (Ø 6,3, pas 2,5) | UVR1A471MED1TA | 1 | – | ✔ |
| R2 | Résistance 200 kΩ 1/4 W | PCF14JT200K | 1 | – | ✔ |
| R3 | Résistance 100 kΩ 1/4 W | PCF14JT100K | 1 | – | ✔ |
| C6 | Céramique 100 nF | C320C104K5R5TA | 1 | – | ✔ |
| J5 | Bornier à vis 2 points pas 5,0 (flotteur) | XY301V-A-5.0-2P | 1 | – | ✔ (1) |
| R4 | Résistance 10 kΩ 1/4 W | PCF14JT10K0 | 1 | – | ✔ (1) |
| R5 | Résistance 1 kΩ 1/4 W | PCF14JT1K00 | 1 | – | ✔ (1) |
| C7 | Céramique 100 nF | C320C104K5R5TA | 1 | – | ✔ (1) |

(1) Pas montés dans la variante cuve sans flotteur.

## 2. À ajouter sur la carte (non dessiné)

| Pièce | Qté | Remarque |
|---|---|---|
| Barrette femelle 1 × 7 au pas de 2,54 mm | 2 | support de A1 (le XIAO reste démontable) |
| Barrette mâle 1 × 7 au pas de 2,54 mm | 2 | à souder sous le XIAO (souvent fournie avec lui) |
| Vis M3 (+ entretoises si besoin) | 4 | fixation de la carte, trous à 3,5 mm des coins |

## 3. Hors carte

| Pièce | Qté | Chaudière | Cuve | Remarque |
|---|---|---|---|---|
| Module DHT20 | 1 | ✔ | ✔ | fixé à l'extérieur de la paroi (vis M3) |
| Câble JST-XH 4 broches serti, 20 cm | 1 | ✔ | ✔ | vers J1, fils soudés sur le module (`CapteurChaudiereSdb.md` § 7.7) |
| Module TCS34725 | 1 | ✔ | – | collé contre le voyant de la chaudière, cache opaque conseillé |
| Câble JST-XH 5 broches serti, 20 cm | 1 | ✔ | – | vers J2 (broche 1 = VIN du module) |
| Bloc USB 5 V + câble USB-C | 1 | ✔ | – | alimentation par la prise du XIAO |
| Support 4 piles AA + 4 piles AA alcalines | 1 | – | ✔ | deux fils vers J3 |
| Interrupteur à levier MTS-102 (comme la borne Squash) | 1 | – | ✔ | en paroi (trou de 6 mm, écrou), 2 cosses sur 3 utilisées, sur fil vers J4 ; **sur arrêt avant de brancher l'USB** |
| Câble JST-XH 2 broches serti | 1 | – | ✔ | vers l'interrupteur |
| Flotteur normalement ouvert (fermé = cuve pleine), fils de 50 cm | 1 | – | ✔ | vers J5 |
| Antenne Wi-Fi U.FL | 1 | ✔ | ✔ | fournie avec le XIAO ESP32-C3 |
| Gaine thermorétractable | – | ✔ | ✔ | soudures des modules |
| Boîtier imprimé en 3D | 1 | ✔ | ✔ | moins haut que pour la carte S3 (pas de Wio) ; compter C5 (11 mm) et le support de piles |
