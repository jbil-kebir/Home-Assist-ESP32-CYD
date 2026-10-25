# Capteur S3 (variante cuve sur piles) — Boîtier

Version du 2026-10-25. **État : deuxième tracé (structure empilée, comme le boîtier du Capteur C3 Xiao DHT20-Piles),
d'après la carte routée (`../../pcb/Capteur S3/CapteurS3.md` § 8) ; pas encore imprimé.** Le premier tracé (deux
compartiments côte à côte, vissé sur du bois, commit 5c013bc) est remplacé.

Source OpenSCAD commentée : `boitier_s3.scad` (réglages dans Fenêtre > Customizer). Pièces à imprimer :
`s3_boite.stl`, `s3_couvercle.stl`, `s3_socle.stl` et `s3_trappe.stl` (PETG opaque). Aperçus : `apercu_eclate.png`,
`apercu_coupe.png`, `apercu_dessus.png`, `apercu_dessous.png`.

## 1. Besoins (décidés le 2026-10-25)

| Besoin | Réponse |
|---|---|
| Variante | **cuve** : 4 piles AA, interrupteur, flotteur, DHT20 ; avec le **Wio LoRa** |
| Pose | à l'abri de l'eau, **posé** (4 patins), pas vissé |
| Disposition | **deux pièces empilées** : boîte de la carte en haut, **socle des piles en dessous**, réunis par 4 vis M3 |
| Piles | support 4 × AA (`Composants\Alimentation\Boitier 4 x AA\Mesures.md`) vissé sous le fond de la boîte, piles vers le bas, **trappe à glissière sous le socle**, sans outil, cran, sortie par le **côté gauche** |
| Interrupteur | **MTS-102** dans la paroi basse |
| Fil du flotteur | sort **vers le bas** (paroi basse), fente + serre-câble à l'intérieur |
| DHT20 | **dehors, sur la paroi haute** (on retourne le boîtier pour changer les piles) |
| Antennes | Wi-Fi et LoRa souples, à l'intérieur |
| Prise USB | **pas de fente** : on ouvre le couvercle pour programmer (interrupteur sur ARRÊT avant de brancher l'USB, aucune protection sur la carte) |
| Matière | PETG opaque, parois de 2 mm |

## 2. Dimensions

| Cote | Valeur |
|---|---|
| Boîte (en haut) | intérieur 64 × 83 × 33 mm |
| Socle (en bas) | même contour, 21 mm de haut (support 16,5 + jeu 1 + trappe 2 + rebords 1,5) |
| Extérieur | 76 × 95 mm (colonnes comprises) × 58 mm de haut (couvercle compris) |
| Parois, fond, couvercle | 2 mm |

Vue de côté (posé) :

```
   ┌──────────────────────────────┐ ← couvercle (4 vis M3)
   │  carte + XIAO S3 + Wio       │   boîte : intérieur 64 × 83 × 33
   │  ──plots──   ▫ bossage       │
   ├══════════════════════════════┤ ← fond de la boîte (support vissé dessous)
   │  support 4 × AA, piles vers  │   socle : 21 mm
   │  le bas                      │
   └═══[ trappe → sort à gauche ]═┘ ← dessous ; patins aux 4 coins
```

Vue de dessus (couvercle retiré) :

```
              paroi haute : DHT20 dehors ─ ┌──────────┐o═ broches → câble
   ┌───────────────────────────────────────└──────────┘──┐
   │ ░░░░░░░ antenne Wi-Fi ░░░░░░                         │
 ░ │ ┌─────────────────────────────┐                      │
 L │ │ XIAO S3 + Wio          [J1] │                      │
 o │ │                             │                      │
 R │ │      carte 46 × 65          │                      │
 a │ │      (tournée de 180°)      │                      │
   │ │ [J5]   [J4]   [J3]          │                      │
   │ └─────────────────────────────┘          ▯ fils piles│
   │   pont                                               │
   └────┼──────[MTS]──────────────────────────────────────┘
        ↓ flotteur   ↑ interrupteur (paroi basse)
```

Coordonnées ci-dessous : X depuis la paroi gauche intérieure, Y depuis la paroi basse intérieure, Z depuis le fond
intérieur de la boîte (négatif dans le socle).

## 3. Détail

### 3.1 Carte

- Vissée sur **4 plots** Ø 7 × 4 mm (vis M3 dans le PETG, avant-trou Ø 2,5), aux trous de la carte (3,5 mm des coins).
- **Tournée de 180°** par rapport au dessin EasyEDA : borniers J3 (piles), J4 (interrupteur), J5 (flotteur) en bas,
  J1 (DHT20) à droite, XIAO en haut à gauche.
- Bord de la carte à 2 mm de la paroi gauche (antenne LoRa), 16 mm de la paroi droite, **15 mm de la paroi basse** (fils
  des borniers, serre-câble, interrupteur) et 3 mm de la paroi haute.
- XIAO + Wio empilés sur barrettes femelles : dessus du Wio **estimé à 25 mm** au-dessus de la carte (Z ≈ 30,6), soit
  2,4 mm sous le couvercle.

### 3.2 Fil du flotteur (paroi basse)

- Fente de 5 mm **ouverte vers le couvercle** (il la referme), à X = 14,5 mm (sous J5), fond arrondi à 8 mm du fond
  intérieur.
- **Pont de serre-câble** entre la fente et J5 (Y = 6) : bloc 9 × 6 × 7 mm, traversé sous son dessus par un passage pour
  un collier de 2,5 mm.

### 3.3 Interrupteur MTS-102 (paroi basse)

- Trou Ø 6,2 à X = 30 mm, **Z = 25 mm** : le corps (≈ 18 mm derrière la paroi avec les cosses) passe au-dessus des
  borniers et de la fiche J4 ; fils vers J4.

### 3.4 DHT20 (paroi haute, dehors)

- Plaquette à plat contre la paroi haute, **capteur à gauche, broches coudées à droite**, centrée à Z = 16,5 mm
  (milieu de la boîte fermée) ; de X = 21,5 à 50 mm.
- Trou Ø 3,4 à X = 43,5 mm pour la vis M3 + écrou (à l'intérieur, à droite de l'antenne Wi-Fi).
- **Trou du câble Ø 6** à X = 57 mm, sous la pointe des broches ; les fils vont à J1 ; reboucher le trou au montage.

### 3.5 Antennes

- **Wi-Fi** (40 × 20) contre la paroi haute, de X = 1 à 41 mm, bord bas sur un appui de 1 mm à Z = 12.
- **LoRa** (40 × 10) contre la paroi gauche, de Y = 20 à 60 mm, bord bas sur un appui de 1 mm à Z = 20.

### 3.6 Support de piles (sous la boîte)

- Support 62 × 57,2 × 16,5 **vissé sous le fond** de la boîte, piles vers le bas (vers la trappe), de X = 1 à 63 et de
  Y = 14 à 71,2.
- 2 vis **M3 × 5** à tête plate, mises **depuis l'intérieur du support**, à travers le fond, dans 2 **bossages** Ø 7 × 2
  côté carte (X = 10 et 54, Y = 42,6) : 4 mm de PETG pour la vis. Le bossage de gauche est sous la carte, juste sous les
  soudures de C7 (flotteur) : 2 mm de jeu jusqu'au dessous de la carte.
- Ses fils sortent du grand côté bas, à 6 mm du coin droit, et montent par le **trou de 8 × 8 mm du fond**
  (X = 52 à 60, Y = 3 à 11) jusqu'au bornier J3.

### 3.7 Socle et trappe

- **Socle** : cadre du même contour que la boîte, 21 mm de haut, fixé sous la boîte par **4 vis M3 × 25** passées par
  en dessous dans les colonnes des coins (lamage Ø 6,5 × 3,5 pour les têtes, avant-trous de 10 mm dans les colonnes de
  la boîte).
- **Trappe** (2 mm) sous le support, retenue par des **rebords** de 1,5 mm le long des parois basse et haute et par un
  rebord de 6 mm au bout droit ; **appuis** de 1,5 mm au-dessus (chanfreinés pour s'imprimer sans support).
- On l'ouvre en la poussant **vers la gauche** avec le pouce (5 stries creuses près du bout droit) : elle sort par une
  **fente de la paroi gauche** du socle.
- **Cran** : une bossette de 0,5 mm sous la trappe, près du bout droit, tombe dans une rainure du rebord droit.
- **Patins** autocollants sous les 4 coins du socle (ils cachent les têtes des vis).

### 3.8 Couvercle

- 4 vis **M3** dans les colonnes des coins (avant-trous Ø 2,5 sur 10 mm), passages Ø 3,4 ; rebord de centrage de 1,5 mm
  (jeu 0,3 mm). Rien dessus.

## 4. Montage

1. Imprimer la boîte, le couvercle, le socle et la trappe (PETG, remplissage ≥ 30 %). Le socle s'imprime rebords de
   la trappe sur le plateau, le couvercle face extérieure sur le plateau (les STL sont déjà orientés).
2. Monter l'interrupteur dans la paroi basse (écrou dehors), souder ses fils (vers J4).
3. Visser le support de piles sous le fond (2 vis M3 × 5, depuis l'intérieur du support), passer ses fils par le trou du
   fond.
4. Visser la carte sur les plots (4 vis M3 × 6) ; brancher les piles sur J3 (+ / −), l'interrupteur sur J4.
5. Coller l'antenne Wi-Fi contre la paroi haute, l'antenne LoRa contre la paroi gauche ; brancher leurs câbles.
6. Visser le DHT20 sur la paroi haute (vis M3 + écrou), passer son câble par le trou, brancher J1, reboucher le trou.
7. Poser le fil du flotteur dans la fente, sur le pont, le serrer avec le collier, le visser dans J5.
8. Visser le socle sous la boîte (4 vis M3 × 25), coller les patins.
9. **Interrupteur sur ARRÊT**, fermer le couvercle (4 vis M3 × 8).
10. Piles : retourner le boîtier (sur un côté), pousser la trappe vers la gauche, mettre les 4 piles, refermer jusqu'au
    cran.

## 5. À vérifier avant impression

- Hauteur réelle du dessus du Wio sur la carte montée (25 mm prévus ; le couvercle est à 27,4 mm au-dessus de la carte).
- Diamètre du fil du flotteur (fente de 5 mm prévue).
- Sous la carte, au-dessus du bossage gauche (X = 10, Y = 42,6, soit PCB ≈ 38 ; 37) : soudures de C7 (PCB 35,2 ; 39,2)
  coupées à moins de 2 mm sous la carte.
- Têtes des vis du support de piles : assez plates pour ne pas gêner les piles.
- Longueur des vis du socle (M3 × 25 prévues).
