# Capteur chaudière Sdb — Boîtier

Version du 2026-10-24. **État : premier tracé, d'après la carte routée (`../../pcb/Capteur chaudiere Sdb/CapteurChaudiereSdb.md`
§ 7) ; pas encore imprimé.**

Source OpenSCAD commentée : `boitier_chaudiere.scad` (réglages dans Fenêtre > Customizer). Pièces à imprimer :
`chaudiere_boite.stl` et `chaudiere_couvercle.stl` (PETG opaque). Aperçus : `apercu_eclate.png`, `apercu_coupe.png`,
`apercu_dessus.png`, `apercu_dessous.png`.

## 1. Besoins (décidés le 2026-10-24)

| Besoin | Réponse |
|---|---|
| Pose | au bout d'un coffrage en bois fixé au mur, au-dessus du voyant de la chaudière ; boîtier à plat sur le coffrage, **couvercle vers la pièce** |
| Fixation | 4 oreilles, vis à bois Ø 4 à tête fraisée ; cales éventuelles entre boîtier et coffrage décidées au montage (réglage de la distance au mur) |
| TCS34725 | **dehors, sous la paroi basse, centré**, capteur vers le bas (vers le voyant) ; face sans composants posée à plat sur la paroi, **vissé par ses 2 trous (vis M2 + écrou à l'intérieur)**, sans entretoise ; fenêtre dans la paroi pour les fils ; centre du capteur à 11 mm de la face arrière (LED du voyant à 15 mm du coffrage) |
| DHT20 | **dehors, sur le couvercle**, côté droit près de J1, vis M3 + écrou ; capteur vers le haut, broches vers le bas ; trou du câble rebouché |
| Prise USB-C | à gauche, fente |
| LED bleue D1 | trou dans la paroi gauche, pattes pliées à 90° |
| Antennes souples | Wi-Fi 40 × 20 contre la paroi haute ; LoRa 40 × 10 contre la paroi droite, non collée |
| Couvercle | vissé (4 vis M3) |
| Matière | PETG opaque, parois de 2 mm |

Pièces : TCS34725 (`Composants\Capteurs\TCS34725\Mesures.md`), DHT20 (`Composants\Capteurs\DHT20\Mesures.md`).

## 2. Dimensions

| Cote | Valeur |
|---|---|
| Intérieur | 57 × 55 × 33 mm (hauteur : du fond intérieur au dessous du couvercle) |
| Extérieur, fermé | 69 × 67 × 37 mm (colonnes des coins comprises) ; 85 mm de haut avec les oreilles |
| Parois, fond, couvercle | 2 mm |

Vue de face (posé sur le coffrage, couvercle vers soi) :

```
        (oreille)                              (oreille)
   ◯──────────────────────────────────────────────◯
   │  ░░░░░ antenne Wi-Fi (paroi haute) ░░░░        │
   │ ┌──────────────────────────────[J2]──────┐     │
   │ │                                         │  ░  │
  ═╡ │ ┌────────┐                   ┌───────┐  │  ░ LoRa
 USB │ │ XIAO + │                   │ DHT20 │  │  ░ (paroi
   │ │ │  Wio   │                   │(sur le│  │  ░ droite)
   │ │ └────────┘                   │couv.) │  │     │
  o╡ │              TP TP TP TP     └─┬┬┬┬──┘[J1]    │
 LED │                                o (câble)│     │
   │ └─────────────────────────────────────────┘     │
   ◯────────────┬─────────────────┬────────────────◯
   (oreille)    │    TCS34725     │       (oreille)
                └─────────────────┘  ← dessous, capteur vers le voyant
```

Coordonnées ci-dessous : X depuis la paroi gauche intérieure, Y depuis la paroi basse intérieure, Z depuis le fond
intérieur.

## 3. Détail

### 3.1 Carte

- Vissée sur **4 plots** Ø 7 × 4 mm (vis M3 dans le PETG, avant-trou Ø 2,5), aux trous de la carte (3,5 mm des coins).
- Carte vue composants vers soi, **tournée de 180°** par rapport au dessin EasyEDA : XIAO, prise USB-C et LED bleue à
  gauche, J1 (DHT20) à droite, J2 (TCS34725) en haut. Le placement de la carte n'a pas été modifié.
- Bord de la carte à 1 mm de la paroi gauche, 10 mm de la paroi droite (fiche J1), 4 mm de la paroi basse et 10 mm de
  la paroi haute (fiche J2).
- XIAO + Wio empilés sur barrettes femelles : dessus du Wio **estimé à 25 mm** au-dessus de la carte (Z ≈ 30,6), soit
  2,4 mm sous le couvercle.

### 3.2 Prise USB-C (paroi gauche)

- Fente 13 × 8 mm à coins arrondis, centrée sur la prise : Y = 27 mm, **Z = 22,1 mm** (plots 4 + carte 1,6 + 16,5 mm,
  valeur estimée comme pour le contrôleur HA).

### 3.3 LED bleue D1 (paroi gauche)

- Trou Ø 5,3 à Y = 13,5 mm, Z = 12 mm.
- Pattes : environ 6,5 mm à la verticale depuis la carte, puis pliées à 90° vers la paroi (environ 4,8 mm) ; la
  collerette (Ø 5,9) bute contre l'intérieur de la paroi, le dôme dépasse d'environ 6 mm dehors. Une goutte de colle.

### 3.4 TCS34725 (sous la paroi basse)

- Module 20 × 20, **face sans composants contre la paroi**, capteur vers le bas, centré en X (28,5 mm).
- **Profondeur** : centre du capteur à **11 mm de la face arrière** du boîtier (réglage `tcs_centre`). Le centre de la
  LED du voyant est à 15 mm de la surface du coffrage (mesuré le 2026-10-24) : 4 mm de marge, rattrapés par des cales
  sous les oreilles. Module de 1 à 21 mm depuis la face arrière (Z = −1 à 19).
- **Trous côté couvercle, broches côté fond** : 2 trous Ø 2,4 à X = 21 et 36 mm, Z = 17 mm.
- **Écrous M2** dans 2 logements hexagonaux (4,6 sur plats, 1,8 de profondeur), creusés dans 2 bossages de
  8 × 3 × 8 mm à l'intérieur de la paroi : on pose l'écrou de l'intérieur, il ne tourne pas, on visse de l'extérieur
  (**vis M2 × 8**). Les bossages sont au-dessus de la carte : accessibles carte en place.
- **Fenêtre des fils** 19 × 3 mm sur la rangée de broches (Z = −0,5 à 2,5), cachée par le module ; la rangée de broches
  étant presque au niveau du fond intérieur, la fenêtre entaille le fond de 0,5 mm le long de la paroi (il reste 1,5 mm).
- Les fils soudés côté face sans composants entrent au ras du fond, remontent dans les 4 mm entre la paroi basse et la
  carte, puis passent au-dessus de la moitié droite de la carte (libre : seulement les points de test et J1) jusqu'à J2.

### 3.5 Antennes

- **Wi-Fi** (40 × 20) contre la paroi haute, de X = 1 à 41 mm, bord bas sur un appui de 1 mm à Z = 12 (au-dessus de la
  fiche J2).
- **LoRa** (40 × 10) contre la paroi droite, de Y = 7 à 47 mm, bord bas sur un appui de 1 mm à Z = 20 (au-dessus de la
  fiche J1), non collée.

### 3.6 Couvercle (vers la pièce)

- 4 vis **M3** dans des colonnes Ø 7 aux 4 coins, à l'extérieur de l'intérieur, avant-trous Ø 2,5 sur 10 mm ;
  passages Ø 3,4. Rebord de centrage de 1,5 mm sous le couvercle (jeu 0,3 mm).
- **DHT20**, dehors, plaquette verticale au-dessus de J1 (centre X = 47) :
  - trou Ø 3,4 à Y = 34,5 pour la vis M3 + écrou (sous le couvercle) ;
  - plaquette de Y = 28 (bord côté broches) à 56,5, capteur ajouré en haut ;
  - **trou du câble Ø 6** à Y = 21, sous la pointe des broches coudées (7 mm) : les fils entrent tout droit, près de J1 ;
    reboucher le trou au montage (colle chaude ou mastic).
- À imprimer face extérieure sur le plateau (le STL est déjà retourné) : pas de support.

### 3.7 Fixation sur le coffrage

- 4 oreilles (2 en haut, 2 en bas) à X = 8 et 49 mm, hors de la zone du TCS34725 (X = 18,5 à 38,5) ; 14 mm de large,
  4 mm d'épaisseur, au niveau du fond ; trou Ø 4,5 à 6 mm de la paroi, **fraisé à 90°** (Ø 8,5) pour une vis à bois Ø 4
  à tête fraisée.
- Distance entre le TCS34725 et le mur : réglable par des cales sous les oreilles, choisies au montage (≈ 4 mm pour
  amener le centre du capteur de 11 à 15 mm du coffrage, face à la LED du voyant).

## 4. Montage

1. Imprimer la boîte et le couvercle (PETG, remplissage ≥ 30 %).
2. Souder les fils du câble J2 sur le TCS34725 (VIN, GND, SCL, SDA, LED), côté face sans composants
   (`CapteurChaudiereSdb.md` § 7.7).
3. Passer les fils par la fenêtre de la paroi basse, poser les 2 écrous M2 dans leurs logements, visser le module
   (2 vis M2 × 8).
4. Visser la carte sur les plots (4 vis M3 × 6), brancher J2.
5. Plier les pattes de D1 et passer la LED dans son trou (goutte de colle).
6. Coller l'antenne Wi-Fi contre la paroi haute, poser l'antenne LoRa contre la paroi droite ; brancher leurs câbles.
7. Visser le boîtier sur le coffrage (4 vis à bois Ø 4, avec des cales d'environ 4 mm sous les oreilles, à ajuster),
   TCS34725 face au voyant.
8. Couvercle : visser le DHT20 (vis M3 + écrou), passer son câble par le trou, brancher J1, reboucher le trou.
9. Brancher le câble USB-C, fermer le couvercle (4 vis M3 × 8).

## 5. À vérifier avant impression

- Hauteur réelle de la prise USB-C du XIAO avec le Wio (16,5 mm au-dessus de la carte prévus) et du dessus du Wio
  (25 mm prévus ; le couvercle est 27,4 mm au-dessus de la carte).
- Fiche du câble USB-C utilisé (fente 13 × 8).
