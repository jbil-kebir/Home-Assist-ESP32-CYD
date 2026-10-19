# Capteur chaudière Sdb — Liste des composants

Liste d'achat pour **une carte**. La liste exacte des composants dessinés sur le circuit imprimé est l'export EasyEDA
`BOM_CapteurChaudiereSdb.xlsx` (même dossier) ; cette page y ajoute ce qui n'est pas dessiné et ce qui est hors carte.

## 1. Sur le circuit imprimé

| Repère | Composant | Référence | Qté |
|---|---|---|---|
| A1 | Seeed XIAO ESP32-S3 + Wio-SX1262 (kit LoRa) | – | 1 |
| C1 | Céramique 100 nF | C320C104K5R5TA | 1 |
| C2 | Électrolytique 10 µF 16 V (Ø 5, pas 2,0) | UVR1C100MDD | 1 |
| D1 | LED bleue 5 mm | XL-502UBC | 1 |
| R1 | Résistance 100 Ω 1/4 W | PCF14JT100R | 1 |
| J1 | Embase JST-XH 4 broches (DHT20) | B4B-XH-A(LF)(SN) | 1 |
| J2 | Embase JST-XH 5 broches (TCS34725) | B5B-XH-A(LF)(SN) | 1 |
| TP1–TP4 | Points de test 3V3, GND, SDA, SCL | barrette mâle 1 broche PZ254V-11-01P | 4 |

## 2. À ajouter sur la carte (non dessiné)

| Pièce | Qté | Remarque |
|---|---|---|
| Barrette femelle 1 × 7 au pas de 2,54 mm | 2 | support de A1 (le XIAO reste démontable) |
| Barrette mâle 1 × 7 au pas de 2,54 mm | 2 | à souder sous le XIAO (souvent fournie avec lui) |
| Vis M3 (+ entretoises si besoin) | 4 | fixation de la carte, trous à 3,5 mm des coins |

## 3. Hors carte

| Pièce | Qté | Remarque |
|---|---|---|
| Module DHT20 | 1 | fixé à l'extérieur de la paroi par son trou (vis M3) |
| Câble JST-XH 4 broches serti, 20 cm | 1 | vers J1, fils soudés sur le module (§ 7.7) |
| Module TCS34725 | 1 | collé contre le voyant (adhésif double face, cache opaque conseillé) |
| Câble JST-XH 5 broches serti, 20 cm | 1 | vers J2 (broche 1 = VIN du module) |
| Bloc USB 5 V + câble USB-C | 1 | alimentation par la prise du XIAO |
| Antennes Wi-Fi et LoRa | – | fournies avec le XIAO et le Wio-SX1262 |
| Gaine thermorétractable | – | soudures des modules |
| Boîtier imprimé en 3D, fermé | 1 | hauteur intérieure ≥ 25 mm (XIAO + Wio sur barrettes) |
