# Module Linky — Liste des composants

Liste d'achat pour **une carte** (voir `Schéma global Linky.md`). La liste exacte des composants dessinés sur le
circuit imprimé est `Linky_BOM.xlsx` (même dossier) ; cette page y ajoute ce qui n'est pas dessiné et ce qui est
hors carte.

## 1. Sur le circuit imprimé

| Repère | Composant | Référence | Qté |
|---|---|---|---|
| A1 | Seeed XIAO ESP32-C3, antenne Wi-Fi U.FL | – | 1 |
| U1 | Optocoupleur à entrée alternative (deux LED tête-bêche), DIP-4 | SFH620A-1 (Vishay) | 1 |
| R1, R3 | Résistance 1 kΩ 1/4 W | CF14JT1K00 | 2 |
| R2 | Résistance 10 kΩ 1/4 W | CF14JT10K0 | 1 |
| J1 | Bornier à vis 2 points, pas de 5,0 mm (bleu, en stock) | KF301-5.0-2P | 1 |
| J2 | Embase JST-XH 2 broches (LED) | B2B-XH-A(LF)(SN) | 1 |

## 2. À ajouter sur la carte (non dessiné)

| Pièce | Qté | Remarque |
|---|---|---|
| Support DIP-4 (tulipe ou standard), rangées à 7,62 mm | 1 | support de U1 (l'optocoupleur reste remplaçable) |
| Barrette femelle 1 × 7 au pas de 2,54 mm | 2 | support de A1 (le XIAO reste démontable) |
| Barrette mâle 1 × 7 au pas de 2,54 mm | 2 | à souder sous le XIAO (souvent fournie avec lui) |
| Vis M3 dans le PETG | 4 | fixation de la carte (trous aux 4 coins) |

## 3. Hors carte

| Pièce | Qté | Remarque |
|---|---|---|
| Adaptateur secteur USB 5 V + câble USB-C | 1 | branché sur le XIAO (bord droit de la carte, fente du boîtier) |
| Antenne Wi-Fi souple rectangulaire U.FL | 1 | celle des essais (−45 dBm près du compteur), collée à l'intérieur du boîtier |
| Paire de fils (torsadée de préférence), ≈ 30 cm | 1 | du bornier TIC du Linky (I1, I2) à J1 |
| LED 5 mm verte | 1 | couvercle du boîtier, sur fils vers J2 |
| Câble JST-XH 2 broches serti | 1 | J2 → LED |
| Boîtier imprimé en 3D | 1 | à dessiner, vissé à côté du compteur |
