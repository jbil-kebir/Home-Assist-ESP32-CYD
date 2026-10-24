# Capteur S3 (variante cuve) — Boîtier

Version du 2026-10-25. **État : premier tracé, d'après la carte routée (`../../pcb/Capteur S3/CapteurS3.md` § 8) ; pas
encore imprimé.**

Source OpenSCAD commentée : `boitier_s3.scad` (réglages dans Fenêtre > Customizer). Pièces à imprimer :
`s3_boite.stl`, `s3_couvercle.stl` et `s3_trappe.stl` (PETG opaque). Aperçus : `apercu_eclate.png`, `apercu_coupe.png`,
`apercu_dessus.png`, `apercu_dessous.png`.

## 1. Besoins (décidés le 2026-10-25)

| Besoin | Réponse |
|---|---|
| Variante | **cuve** : 4 piles AA, interrupteur, flotteur, DHT20 ; avec le **Wio LoRa** |
| Pose | à l'abri de l'eau, **vissé sur du bois** (4 oreilles, vis à bois Ø 4 fraisées), couvercle vers soi |
| Disposition | **deux compartiments côte à côte** : carte à gauche, piles à droite |
| Piles | support 4 × AA (`Composants\Alimentation\Boitier 4 x AA\Mesures.md`) derrière une **trappe à glissière**, sans outil, retenue par un cran |
| Interrupteur | **MTS-102** dans la paroi basse |
| Fil du flotteur | sort **vers le bas**, trou + serre-câble à l'intérieur |
| DHT20 | **dehors, sur le couvercle**, vis M3 + écrou ; capteur vers le haut, broches vers le bas ; trou du câble rebouché |
| Antennes | Wi-Fi et LoRa souples, à l'intérieur |
| Prise USB | **pas de fente** : on ouvre le couvercle pour programmer (interrupteur sur ARRÊT avant de brancher l'USB, aucune protection sur la carte) |
| Matière | PETG opaque, parois de 2 mm |

## 2. Dimensions

| Cote | Valeur |
|---|---|
| Intérieur | 121 × 83 × 33 mm : compartiment de la carte 57 mm, cloison 2 mm, compartiment des piles 62 mm |
| Extérieur, fermé | 133 × 95 × 37 mm (colonnes comprises) ; 113 mm de haut avec les oreilles |
| Parois, fond, couvercle, cloison | 2 mm |

Vue de face (posé sur le bois, couvercle vers soi) :

```
          (oreille)                                     (oreille)
   ◯──────────────────────────────◯──────────────────────────────────────◯
   │ ░░░░ antenne Wi-Fi ░░░░        │  ═══ fente de sortie de la trappe ═══ │
 ░ │ ┌────────────────────────┐     │ ┌──────────────────────────────────┐ │
 L │ │ XIAO + Wio         ┌───┐│     │ │                                  │ │
 o │ │                    │DHT││     │ │      support 4 × AA              │ │
 R │ │                    │20 ││[J1] │ │      (piles horizontales)        │ │
 a │ │      carte         └┬┬┬┘│     │ │                                  │ │
   │ │      46 × 65         o  │     │ │                                  │ │
   │ │ [J5]   [J4]   [J3]      │     │ │                                  │ │
   │ └────────────────────────┘    ▯│←fils                                 │ │
   │   pont                         │ └──────────────────────────────────┘ │
   ◯────┼──────[MTS]──────────────◯──────────────────────────────────────◯
        ↓ flotteur   ↑ interrupteur          (oreille)          (oreille)
```

Coordonnées ci-dessous : X depuis la paroi gauche intérieure, Y depuis la paroi basse intérieure, Z depuis le fond
intérieur.

## 3. Détail

### 3.1 Carte

- Vissée sur **4 plots** Ø 7 × 4 mm (vis M3 dans le PETG, avant-trou Ø 2,5), aux trous de la carte (3,5 mm des coins).
- Carte vue composants vers soi, **tournée de 180°** par rapport au dessin EasyEDA : borniers J3 (piles), J4
  (interrupteur), J5 (flotteur) en bas, J1 (DHT20) à droite, XIAO en haut à gauche. Le placement n'a pas été modifié.
- Bord de la carte à 2 mm de la paroi gauche (antenne LoRa), 9 mm de la cloison (fiche J1), **15 mm de la paroi basse**
  (fils des borniers, serre-câble, interrupteur) et 3 mm de la paroi haute.
- XIAO + Wio empilés sur barrettes femelles : dessus du Wio **estimé à 25 mm** au-dessus de la carte (Z ≈ 30,6).

### 3.2 Fil du flotteur (paroi basse)

- Fente de 5 mm **ouverte vers le couvercle** (il la referme), à X = 14,5 mm (sous J5), fond arrondi à 8 mm du fond
  intérieur.
- **Pont de serre-câble** entre la fente et J5 (Y = 6) : bloc 9 × 6 × 7 mm, traversé sous son dessus par un passage pour
  un collier de 2,5 mm. Une traction sur le fil ne tire pas sur le bornier.

### 3.3 Interrupteur MTS-102 (paroi basse)

- Trou Ø 6,2 à X = 30 mm, **Z = 25 mm** : le corps (≈ 18 mm derrière la paroi avec les cosses) passe au-dessus des
  borniers ; fils vers J4.

### 3.4 Compartiment des piles

- Support 4 × AA (62 × 57,2 × 16,5) **vissé sur 2 plots** Ø 7 × 3 mm par ses 2 trous (entraxe 44, au milieu), centre à
  X = 91, Y = 33 ; piles horizontales.
- Sortie de ses fils sur le grand côté gauche, à 6 mm du coin bas, en face du **trou de 10 × 10 mm de la cloison**
  (Y = 3 à 13, Z = 4 à 14) ; les fils (15 cm) vont au bornier J3.
- Dessus des piles à environ 19,5 mm du fond : environ 11 mm libres sous la trappe pour les doigts.

### 3.5 Trappe à glissière

- Plaque de 2 mm (61,4 mm de large, 84,7 mm de long), juste sous le couvercle, posée sur **2 appuis** de 1,5 mm (sur la
  cloison et sur la paroi droite, chanfreinés dessous pour s'imprimer sans support).
- Les **bords de l'ouverture du couvercle** (1,5 mm de chaque côté, 6 mm en bas, 3 mm en haut) la retiennent.
- On l'ouvre en la poussant **vers le haut** avec le pouce (5 stries creuses près du bas) : elle sort par une **fente de
  la paroi haute**.
- **Cran** : une bossette de 0,5 mm sur la trappe, près du bas, tombe dans une rainure sous le couvercle quand la trappe
  est fermée.
- Couvercle vissé ou non, la trappe s'ouvre seule.

### 3.6 Antennes

- **Wi-Fi** (40 × 20) contre la paroi haute, de X = 2 à 42 mm, bord bas sur un appui de 1 mm à Z = 12.
- **LoRa** (40 × 10) contre la paroi gauche, de Y = 20 à 60 mm, bord bas sur un appui de 1 mm à Z = 20 (loin des piles).

### 3.7 Couvercle (sur toute la boîte)

- **6 vis M3** dans des colonnes Ø 7 à l'extérieur (4 coins + 2 au milieu des grands côtés, à X = 56), avant-trous
  Ø 2,5 sur 10 mm ; passages Ø 3,4. Rebord de centrage de 1,5 mm autour du compartiment de la carte (jeu 0,3 mm).
- **DHT20**, dehors, plaquette verticale (centre X = 49,5) :
  - trou Ø 3,4 à Y = 58,5 pour la vis M3 + écrou ;
  - plaquette de Y = 52 (bord côté broches) à 80,5, capteur ajouré en haut ;
  - **trou du câble Ø 6** à Y = 45, sous la pointe des broches coudées ; reboucher le trou au montage.
- Ouverture au-dessus de la trappe, rainure du cran dessous.
- À imprimer face extérieure sur le plateau (le STL est déjà retourné) : pas de support.

### 3.8 Fixation sur le bois

- 4 oreilles (2 en haut, 2 en bas) à X = 45 et 105 mm, loin du fil du flotteur et de l'interrupteur ; 14 mm de large,
  4 mm d'épaisseur, au niveau du fond ; trou Ø 4,5 à 6 mm de la paroi, **fraisé à 90°** (Ø 8,5) pour une vis à bois Ø 4
  à tête fraisée.

## 4. Montage

1. Imprimer la boîte, le couvercle et la trappe (PETG, remplissage ≥ 30 %).
2. Visser le boîtier sur le bois (4 vis à bois Ø 4).
3. Monter l'interrupteur dans la paroi basse (écrou dehors), souder ses fils (vers J4).
4. Visser le support de piles sur ses plots (2 vis M3 × 6 à tête plate), passer ses fils par le trou de la cloison.
5. Visser la carte sur les plots (4 vis M3 × 6) ; brancher les piles sur J3 (+ / −), l'interrupteur sur J4.
6. Coller l'antenne Wi-Fi contre la paroi haute, l'antenne LoRa contre la paroi gauche ; brancher leurs câbles.
7. Poser le fil du flotteur dans la fente, sur le pont, le serrer avec le collier, le visser dans J5.
8. Couvercle : visser le DHT20 (vis M3 + écrou), passer son câble par le trou, brancher J1, reboucher le trou.
9. **Interrupteur sur ARRÊT**, fermer le couvercle (6 vis M3 × 8), glisser la trappe par le haut jusqu'au cran.
10. Piles : pousser la trappe vers le haut, mettre les 4 piles, refermer.

## 5. À vérifier avant impression

- Hauteur réelle du dessus du Wio sur la carte montée (25 mm prévus ; le couvercle est à 27,4 mm au-dessus de la carte).
- Diamètre du fil du flotteur (fente de 5 mm prévue).
- Tête des vis du support de piles : assez plate pour ne pas gêner les piles (sinon vis à tête fraisée).
- Position exacte de la sortie des fils du support (6 mm du coin) en face du trou de la cloison.
