# Contrôleur HA — Boîtier

Version du 2026-10-24. **État : premier jet, d'après la carte routée (`../pcb/Schéma global contrôleur HA.md` § 7).
Hauteurs des modules ESTIMÉES : ne pas imprimer avant les vérifications du § 5.**

Source OpenSCAD commentée : `boitier_controleur.scad` (réglages dans Fenêtre > Customizer). Pièces à imprimer :
`controleur_boite.stl` et `controleur_couvercle.stl` (PETG opaque). Aperçus : `apercu_eclate.png`, `apercu_coupe.png`,
`apercu_avant.png`, `apercu_arriere.png`, `apercu_gauche.png`, `apercu_droite.png`.

## 1. Besoins (décidés les 2026-10-23 et 2026-10-24)

| Besoin | Réponse |
|---|---|
| Pose | sur une étagère ; vis prévues quand même |
| Couvercle | vissé, sur le dessus |
| Avant | interrupteur MTS-102 et LED « allumé », côte à côte au milieu |
| Arrière | jack 12 V (J1), prise SMA du CC1101 (antenne 433 MHz vissée dehors) |
| Côté droit | prises USB-C du DevKit et du XIAO (programmation sans ouvrir), aération |
| Côté gauche | antenne gravée du DevKit (rien en face), DHT20 à l'extérieur (vis M3, comme prévu pour les capteurs), aération |
| Antennes du XIAO S3 + Wio | souples, à l'intérieur : Wi-Fi 40 × 20 (paroi gauche), LoRa 40 × 10 (paroi droite, en haut) ; câbles u.FL de 52 mm |
| Matière | PETG opaque, parois de 2 mm |

## 2. Dimensions

| Cote | Valeur |
|---|---|
| Intérieur | 60 × 86 × 36 mm (hauteur : du fond intérieur au dessous du couvercle) |
| Extérieur, fermé | 72 × 98 × 40 mm (colonnes des coins comprises) ; 90 mm de large avec les oreilles |
| Parois, fond, couvercle | 2 mm |

Vue de dessus, couvercle retiré :

```
                        ARRIÈRE : jack 12 V, SMA 433 du CC1101
     ┌────────────────────────────────────────────────────────┐
     ░ antenne                                                │
     ░ Wi-Fi   ┌CC1101┐  J1   [MP1584EN]                      │
     ░ (paroi  │      │                           (J2)        │
     ░ gauche) └──────┘                  ┌────────────┐  USB ═╡ aération
 ════ oreille  aération                  │ XIAO S3+Wio│       ╞════ oreille
 DHT20 (dehors)                          └────────────┘       ▓ antenne LoRa
     │   ┌──────────────────────────────────────────────┐ USB═╡ (en haut)
     │   │ DevKit ESP32                                 │     ▓
     │   └──────────────────────────────────────────────┘     │
     │   J3 DHT20      J4 LED                                 │
     └──────────[ interrupteur ]──[ LED ]─────────────────────┘
                         AVANT
```

## 3. Détail

### 3.1 Carte

- Vissée sur **4 plots** Ø 7 × 4 mm (vis M3 dans le PETG, avant-trou Ø 2,5), aux trous de la carte.
- Posée telle que dessinée dans EasyEDA (avant en bas), à **2 mm** de la paroi gauche (antenne Wi-Fi), **1 mm** de la
  paroi droite (les prises USB-C, qui dépassent d'environ 1,6 mm, arrivent dans l'épaisseur de la paroi) et 1 mm de l'avant
  et de l'arrière.

### 3.2 Façade avant

- **Interrupteur** MTS-102 (trou Ø 6,2) à X = 24 mm et **LED** 5 mm (trou Ø 5,1 + collier Ø 8,5 × 3 derrière la façade,
  contre lequel bute la collerette) à X = 36 mm.
- Tous deux à **28 mm** au-dessus du fond intérieur : le corps de l'interrupteur (≈ 18 mm derrière la façade avec ses
  cosses) passe au-dessus du DevKit (≈ 21 mm) et des fiches JST de J3 et J4.

### 3.3 Paroi arrière

- **Jack 12 V** : trou Ø 11 (la fiche entre jusqu'au jack), axe à 6,5 mm au-dessus de la carte, face à J1.
- **SMA du CC1101** : trou Ø 8, axe estimé à 13 mm au-dessus de la carte ; l'antenne 433 MHz se visse dehors.

### 3.4 Côté droit

- **Fentes USB-C** 13 × 8 mm du DevKit (axe estimé à 14,6 mm au-dessus de la carte) et du XIAO (16,5 mm).
- **Aération** : 4 fentes 3 × 8 mm vers l'arrière (MP1584EN).
- **Antenne LoRa** souple collée en haut, à l'intérieur, au-dessus de la fente USB du DevKit (appui de 1 mm à 25 mm
  du fond intérieur), loin de l'antenne 433 MHz (arrière gauche).

### 3.5 Côté gauche

- **DHT20** vissé à l'extérieur, capteur vers l'avant : trou Ø 3,4 pour une **vis M3 × 8 + écrou** (à l'intérieur), à
  28 mm au-dessus du fond intérieur ; **trou du câble** Ø 6 juste derrière les broches. Loin du MP1584EN (arrière).
- **Aération** : 5 fentes 3 × 8 mm en bas, vers l'arrière.
- **Antenne Wi-Fi** souple collée à l'intérieur, au-dessus des fentes (appui de 1 mm à 14 mm du fond intérieur), dans
  la moitié arrière ; l'antenne gravée du DevKit (avant gauche) n'a rien en face.

### 3.6 Couvercle et fixation

- Couvercle vissé par 4 vis **M3** dans des colonnes Ø 7 aux 4 coins, à l'extérieur (comme le module Linky), avec un
  rebord de centrage de 1,5 mm. À imprimer face extérieure sur le plateau (le STL est déjà retourné).
- 2 **oreilles** à gauche et à droite, au milieu, au niveau du fond, trou Ø 4,5 fraisé pour vis à bois Ø 4 (si on fixe
  le boîtier à l'étagère) ; patins caoutchouc sous le fond sinon.

## 4. Montage

1. Imprimer la boîte et le couvercle (PETG, remplissage ≥ 30 %).
2. Monter l'interrupteur et la LED en façade, le DHT20 sur la paroi gauche (vis M3 + écrou), passer son câble.
3. Coller les antennes Wi-Fi (paroi gauche) et LoRa (paroi droite, en haut), sur leurs appuis.
4. Poser la carte, prise SMA du CC1101 et jack face à leurs trous, visser (4 vis M3 × 6).
5. Brancher les câbles (J2 interrupteur, J3 DHT20, J4 LED, antennes u.FL), visser l'antenne 433 MHz dehors.
6. Fermer le couvercle (4 vis M3 × 8).

## 5. À vérifier avant d'imprimer (réglages du Customizer)

| Réglage | Valeur actuelle | À mesurer |
|---|---|---|
| `devkit_usb`, `devkit_dessus` | 14,6 / 15,7 mm | DevKit sur barrettes femelles : milieu de la prise USB-C, dessus du module WROOM |
| `xiao_usb`, `xiao_dessus` | 16,5 / 25 mm | XIAO S3 + Wio empilés sur barrettes : milieu de la prise USB-C, dessus de l'empilement |
| `sma_hauteur` | 13 mm | CC1101 sur sa barrette 2 × 4 : axe de la prise SMA |
| `jack_hauteur`, `jack_x` | 6,5 / 21,6 mm | jack DC-005-20 soudé : axe de la fiche |
| `facade_hauteur` | 28 mm | interrupteur et LED au-dessus du DevKit et des fiches JST |
| `dht_trou` | Ø 3,4 | trou du DHT20 (M3 prévu, à confirmer sur le module) |
