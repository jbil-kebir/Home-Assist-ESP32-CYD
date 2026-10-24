# Module Linky — Boîtier

Version du 2026-10-24. **État : premier tracé, d'après la carte routée (`../pcb/Schéma global Linky.md` § 7) ;
pas encore imprimé.**

Source OpenSCAD commentée : `boitier_linky.scad` (réglages dans Fenêtre > Customizer). Pièces à imprimer :
`linky_boite.stl` et `linky_couvercle.stl` (PETG opaque). Aperçus : `apercu_eclate.png`, `apercu_coupe.png`,
`apercu_dessus.png`.

## 1. Besoins (décidés le 2026-10-24)

| Besoin | Réponse |
|---|---|
| Pose | à plat contre le panneau, à côté du compteur (compteur à droite) |
| Fixation | sans ouvrir le boîtier : 2 oreilles, vis à bois Ø 4 à tête fraisée |
| Prise USB-C | à gauche, câble branché à demeure dans une fente |
| Fils TIC | à droite : fente + serre-câble à l'intérieur |
| Couvercle | vissé |
| Antenne Wi-Fi souple | 40 × 20 mm, câble u.FL de 52 mm, collée contre une paroi |
| LED « allumé » | sur le couvercle |
| Matière | PETG opaque, parois de 2 mm |

## 2. Dimensions

| Cote | Valeur |
|---|---|
| Intérieur | 71 × 43 × 28 mm (hauteur : du fond intérieur au dessous du couvercle) |
| Extérieur, fermé | 83 × 55 × 32 mm (colonnes des coins comprises) ; 73 mm de haut avec les oreilles |
| Parois, fond, couvercle | 2 mm |

Vue de face (posé contre le mur, couvercle vers soi) :

```
                     (oreille, vis à bois Ø 4)
  ◯───────────────────────────────────────────────◯
  │  ░░░░░░░░░░ antenne 40 × 20 ░░░░░░░░░           │  paroi haute
  │                          (LED sur le couvercle)  │
  │ ┌────────────────────────────────────┐          │
 ═╡ │USB  XIAO            U1      J1 TIC │ [pont]   ╞═ fils TIC
  │ └────────────────────────────────────┘ serre-câble (fente ouverte vers le haut)
  │                                                  │
  ◯───────────────────────────────────────────────◯
                     (oreille, vis à bois Ø 4)
```

## 3. Détail

### 3.1 Carte

- Vissée sur **4 plots** Ø 7 × 4 mm (vis M3 dans le PETG, avant-trou Ø 2,5), aux trous de la carte (3,5 mm des bords).
- Carte vue composants vers soi, **tournée de 180°** par rapport au dessin EasyEDA : XIAO et prise USB-C à gauche,
  bornier J1 à droite.
- Bord de la carte à 1 mm des parois gauche et basse : la prise USB-C (qui dépasse de 1,5 mm) arrive dans l'épaisseur
  de la paroi gauche.

### 3.2 Prise USB-C (paroi gauche)

- Fente 13 × 8 mm à coins arrondis, centrée sur la prise : à 17,5 mm de la paroi basse et à **22,1 mm** au-dessus
  du fond intérieur (plots 4 + carte 1,6 + 16,5 mm mesurés, XIAO sur barrette femelle).

### 3.3 Fils TIC (paroi droite)

- Fente de 4 mm de large, **ouverte vers le haut** (on pose les fils sans les enfiler, le couvercle referme la fente),
  fond arrondi à 10 mm au-dessus du fond intérieur, face au bornier J1.
- **Pont de serre-câble** entre la fente et J1 : bloc 6 × 10 × 9 mm, traversé sous son dessus par un passage
  3,5 × 2 mm pour un collier de 2,5 mm. Les fils posés dessus sont serrés par le collier : une traction sur les fils
  ne tire pas sur le bornier.

### 3.4 Antenne Wi-Fi

- Collée contre la **paroi haute** (loin du compteur et du plan de masse), de X = 15 à 55 mm, bord bas sur un appui de
  1 mm à 6 mm du fond intérieur. Carte à 9 mm de cette paroi.
- Le câble de 52 mm atteint largement l'antenne : la prise u.FL est au bout du XIAO opposé à l'USB, à environ 25 mm.

### 3.5 Couvercle

- 4 vis **M3** dans des colonnes Ø 7 aux 4 coins, **à l'extérieur** de l'intérieur (la carte est à 1 mm des parois
  gauche et basse : pas de place pour une colonne à l'intérieur), avant-trous Ø 2,5 sur 10 mm ; passages Ø 3,4.
- Rebord de centrage de 1,5 mm sous le couvercle, à l'intérieur des parois (jeu 0,3 mm).
- **LED « allumé »** : trou Ø 5,1 et collier Ø 8,5 × 3 mm dessous ; la collerette de la LED (Ø 5,8) bute sous le
  collier, le dôme dépasse d'environ 2,5 mm. Position : X = 36, Y = 30, au-dessus de R3, près de J2.
  **Écart à la proposition** (LED au-dessus du XIAO) : le dessus du XIAO est à environ 24 mm du fond intérieur, soit
  4 mm sous le couvercle, trop peu pour la LED, ses pattes et le câble ; à côté de J2, le câble de la LED est court.
  Position réglable (`led`).
- À imprimer face extérieure sur le plateau (le STL est déjà retourné).

### 3.6 Fixation au mur

- 2 oreilles (haut et bas, au milieu), 14 mm de large, 4 mm d'épaisseur, au niveau du fond ; trou Ø 4,5 à 6 mm de la
  paroi, **fraisé à 90°** (Ø 8,5) pour une vis à bois Ø 4 à tête fraisée. On visse le boîtier sans l'ouvrir.

## 4. Montage

1. Imprimer la boîte et le couvercle (PETG, remplissage ≥ 30 %).
2. Visser le boîtier sur le panneau (2 vis à bois Ø 4).
3. Visser la carte sur les plots (4 vis M3 × 6).
4. Coller l'antenne contre la paroi haute (appui en bas), brancher son câble sur le XIAO.
5. Passer la LED dans le couvercle (collerette sous le collier, goutte de colle), brancher son câble sur J2.
6. Poser les fils TIC dans la fente, sur le pont, les serrer avec le collier, les visser dans J1 (I1, I2).
7. Brancher le câble USB-C, fermer le couvercle (4 vis M3 × 8).

## 5. À vérifier

- Hauteur réelle de la prise USB-C sur la carte montée (22,1 mm prévu, mesure du 2026-10-20 sur une autre carte).
- Hauteur de la fiche JST-XH de la LED (≈ 12 mm prévus au-dessus de la carte).
- Taille de la fiche du câble USB-C utilisé (fente 13 × 8).
