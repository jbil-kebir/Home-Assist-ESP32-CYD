# NoeudChauffageSdb — Liste des composants

Liste d'achat pour **une carte**. La liste exacte des composants dessinés sur le circuit imprimé est l'export EasyEDA
`BOM_NoeudChauffageSdb.xlsx` (même dossier) ; cette page y ajoute ce qui n'est pas dessiné et ce qui est hors carte.

## 1. Sur le circuit imprimé

| Repère | Composant | Référence | Qté |
|---|---|---|---|
| A1 | Seeed XIAO ESP32-C3 (antenne externe U.FL) | – | 1 |
| J1 | Bornier à vis 2 points pas 5,0 (bloc secteur 12 V) | XY301V-A-5.0-2P | 1 |
| D1, DA | Diode Schottky | 1N5819 | 2 |
| C1 | Électrolytique 47 µF 25 V (Ø 5, pas 2,0) | 25YXF47MEFC5X11 | 1 |
| U1 | Régulateur à découpage 5 V 1 A | R-78E5.0-1.0 (Recom) | 1 |
| C6, C10 | Électrolytique 100 µF 16 V (Ø 6,3, pas 2,5) | KM101M016E110A | 2 |
| C7, C9 | Céramique 100 nF | C320C104K5R5TA | 2 |
| C8 | Électrolytique 10 µF 16 V (Ø 5, pas 2,0) | UVR1C100MDD | 1 |
| JP1, JP2 | Barrette mâle 2 broches (mesure du courant 5 V et IR) | PZ254V-11-02P | 2 |
| J5 | Embase JST-XH 3 broches (micro) | B3B-XH-A(LF)(SN) | 1 |
| J6 | Embase JST-XH 2 broches (LED IR) | B2B-XH-A(LF)(SN) | 1 |
| Q1 | Transistor NPN TO-92 (E-B-C) | 2N2222A | 1 |
| R1 | Résistance 220 Ω 1/4 W | PCF14JT220R | 1 |
| R2 | Résistance 10 kΩ 1/4 W | PCF14JT10K0 | 1 |
| R3 | Résistance 22 Ω 1/4 W | PCF14JT22R0 | 1 |
| R4 | Résistance 100 Ω 1/4 W | PCF14JT100R | 1 |
| R5 | Résistance 330 Ω 1/4 W | PCF14JT330R | 1 |
| D6 | LED bleue 5 mm (son) | XL-502UBC | 1 |
| D7 | LED rouge 5 mm (IR) | XL-502SURC | 1 |
| TP1–TP6 | Points de test V12, +5V, 3V3, MICRO, IR_K, GND | barrette mâle 1 broche PZ254V-11-01P | 6 |

## 2. À ajouter sur la carte (non dessiné)

| Pièce | Qté | Remarque |
|---|---|---|
| Barrette femelle 1 × 7 au pas de 2,54 mm | 2 | support de A1 (le XIAO reste démontable) |
| Barrette mâle 1 × 7 au pas de 2,54 mm | 2 | à souder sous le XIAO (souvent fournie avec lui) |
| Cavalier au pas de 2,54 mm | 2 | sur JP1 et JP2 (à retirer pour mesurer le courant) |
| Vis M3 (+ entretoises si besoin) | 4 | fixation de la carte (64 × 48 mm), trous à 3,5 mm des coins |

## 3. Hors carte

| Pièce | Qté | Remarque |
|---|---|---|
| Bloc secteur 12 V (tension à vide < 16 V) | 1 | deux fils vers J1 |
| Module micro MAX4466 | 1 | sur fil, face au haut-parleur du chauffage |
| Câble JST-XH 3 broches serti | 1 | vers J5 (3V3, GND, MICRO) |
| LED infrarouge HIR204C | 2 | en série, sur fil, face au récepteur du chauffage |
| Câble JST-XH 2 broches serti | 1 | vers J6 |
| Antenne Wi-Fi U.FL | 1 | fournie avec le XIAO ESP32-C3 |
| Gaine thermorétractable | – | soudures du micro et des LED IR |
| Boîtier imprimé en 3D | 1 | LED témoins visibles (fenêtre ou couvercle translucide) |
| Option : boîtier Hammond 1554C2GYCL + 3 presse-étoupes PG7 | 1 | voir `NoeudChauffageSdb.md` § 6 |
