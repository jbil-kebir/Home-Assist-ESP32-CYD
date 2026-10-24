// =============================================================================
//  boitier_linky.scad – Boîtier du module Linky (XIAO ESP32-C3 + SFH620A-1), vissé à côté du compteur
// -----------------------------------------------------------------------------
//  Pièces :
//    - BOÎTE (PETG) : fond + parois, 4 plots de la carte, fente de la prise USB-C (paroi gauche), fente des fils
//      TIC ouverte vers le haut (paroi droite) + pont de serre-câble à l'intérieur, appui de l'antenne souple
//      (paroi haute), 4 colonnes de vis du couvercle aux coins (à l'extérieur), 2 oreilles de fixation au mur ;
//    - COUVERCLE (PETG) : plaque vissée (4 vis M3), rebord de centrage, trou et collier de la LED « allumé ».
//
//  Repère (boîtier posé à plat contre le mur, vu de face, couvercle vers soi) : X vers la droite, Y vers le haut,
//  Z vers soi ; origine = coin bas gauche de l'INTÉRIEUR, au ras du fond intérieur.
//  La carte (58 × 33) est vue composants vers soi, tournée de 180° par rapport au dessin EasyEDA :
//  prise USB-C du XIAO à GAUCHE, bornier J1 (fils TIC) à DROITE, côté du compteur.
//
//  Réglages : menu Fenêtre > Customizer. Fiche : Linky/boitier/Boîtier Linky.md
// =============================================================================

/* [Pièce à afficher] */
// ensemble, eclate, coupe : contrôle ; boite, couvercle : pièces à exporter en STL
piece = "eclate"; // [ensemble, eclate, coupe, boite, couvercle]

/* [Boîte] */
// Intérieur : longueur (X), largeur (Y), hauteur (Z, du fond intérieur au dessous du couvercle)
interieur = [71, 43, 28];
paroi = 2;
fond = 2;
couvercle_epaisseur = 2;

/* [Circuit imprimé] */
carte = [58, 33, 1.6];
// Coin bas gauche de la carte dans le boîtier (1 mm de la paroi gauche : la prise USB-C dépasse de 1,5 mm)
carte_position = [1, 1];
// Trous M3 de la carte (coordonnées sur la carte, symétriques : la rotation de 180° ne les change pas)
carte_trous = [[3.5, 3.5], [54.5, 3.5], [3.5, 29.5], [54.5, 29.5]];
plot_diametre = 7;
plot_hauteur = 4;
// Avant-trou pour une vis M3 vissée dans le PETG
vis_pilote = 2.5;
vis_profondeur = 6;

/* [Prise USB-C du XIAO (paroi gauche)] */
// Centre de la prise : Y = milieu de la carte ; Z = plots 4 + carte 1,6 + 16,5 mesuré (XIAO sur barrette femelle)
usb_y = 17.5;
usb_hauteur = 22.1;
// Fente (Y × Z) : la fiche du câble branché à demeure y entre
usb_fente = [13, 8];

/* [Fils TIC (paroi droite)] */
// Fente ouverte vers le haut (le couvercle la referme) : centre Y = bornier J1, largeur, bas de la fente
tic_y = 17.5;
tic_largeur = 4;
tic_bas = 10;
// Pont de serre-câble (collier de 2,5 mm) entre la fente et le bornier : centre X, taille, passage du collier
pont_x = 65;
pont = [6, 10, 9];
pont_passage = [3.5, 2];
pont_passage_z = 4;

/* [Antenne Wi-Fi souple (paroi haute)] */
// Antenne 40 × 20 mm collée contre la paroi haute (câble u.FL de 52 mm) ; appui sous son bord bas
antenne = [40, 20];
antenne_x = 15;
antenne_bas = 6;
appui = [1, 1];

/* [Vis du couvercle] */
// Colonnes aux 4 coins, À L'EXTÉRIEUR de l'intérieur (la carte est à 1 mm des parois gauche et basse)
colonne_diametre = 7;
colonne_decalage = 2.5;
vis_passage = 3.4;
vis_colonne_profondeur = 10;

/* [Couvercle] */
rebord_hauteur = 1.5;
rebord_epaisseur = 1.2;
jeu = 0.3;
// LED 5 mm : position (X, Y), trou, collier sous le couvercle (diamètre extérieur, hauteur) ; la collerette
// de la LED (Ø 5,8) bute sous le collier, le dôme dépasse d'environ 2,5 mm
led = [36, 30];
led_trou = 5.1;
led_collier = [8.5, 3];

/* [Fixation au mur] */
// 2 oreilles (haut et bas, au milieu) : saillie, largeur, épaisseur ; vis à bois Ø 4 à tête fraisée
// (trou à 6 mm de la paroi : la tête Ø 8,5 ne touche pas la paroi)
oreille = [13, 14, 4];
oreille_trou = 4.5;
oreille_fraisure = 8.5;

$fn = 48;
L = interieur[0]; W = interieur[1]; H = interieur[2];
colonnes = [[-colonne_decalage, -colonne_decalage], [L + colonne_decalage, -colonne_decalage],
            [-colonne_decalage, W + colonne_decalage], [L + colonne_decalage, W + colonne_decalage]];

// --- Contour extérieur, en 2D : rectangle (parois de 2 mm) + colonnes en saillie aux coins, raccords arrondis
// (pas de hull : il épaissirait toutes les parois jusqu'aux colonnes)
module contour() {
    offset(r = -1) offset(r = 1) union() {
        translate([-paroi, -paroi]) square([L + 2 * paroi, W + 2 * paroi]);
        for (c = colonnes) translate(c) circle(d = colonne_diametre);
    }
}

module oreilles() {
    yt = -(oreille[0] - oreille[1] / 2);   // centre du trou, depuis la face extérieure de la paroi
    for (s = [0, 1]) translate([L / 2, s ? W + paroi : -paroi, -fond]) mirror([0, s, 0])
        difference() {
            hull() {
                translate([-oreille[1] / 2, -0.01, 0]) cube([oreille[1], 0.01, oreille[2]]);
                translate([0, yt, 0]) cylinder(d = oreille[1], h = oreille[2]);
            }
            translate([0, yt, -1]) {
                cylinder(d = oreille_trou, h = oreille[2] + 2);
                // fraisure à 90° sur le dessus (tête de la vis à bois au ras)
                translate([0, 0, 1 + oreille[2] - oreille_fraisure / 2 + oreille_trou / 2])
                    cylinder(d1 = oreille_trou, d2 = oreille_fraisure + 0.2, h = (oreille_fraisure - oreille_trou) / 2 + 0.1);
            }
        }
}

module pont_serre_cable() {
    translate([pont_x - pont[0] / 2, tic_y - pont[1] / 2, 0]) difference() {
        cube(pont);
        // passage du collier sous le dessus du pont, perpendiculaire aux fils
        translate([(pont[0] - pont_passage[0]) / 2, -1, pont_passage_z]) cube([pont_passage[0], pont[1] + 2, pont_passage[1]]);
    }
}

module boite() {
    difference() {
        union() {
            translate([0, 0, -fond]) linear_extrude(fond + H) contour();
            oreilles();
        }
        // intérieur
        cube([L, W, H + 1]);
        // fente USB-C (paroi gauche)
        translate([-paroi - 1, usb_y, usb_hauteur]) rotate([0, 90, 0])
            linear_extrude(paroi + 2) offset(r = 1.5) square([usb_fente[1] - 3, usb_fente[0] - 3], center = true);
        // fente des fils TIC (paroi droite), ouverte vers le haut
        translate([L - 1, tic_y - tic_largeur / 2, tic_bas + tic_largeur / 2]) cube([paroi + 2, tic_largeur, H]);
        translate([L - 1, tic_y, tic_bas + tic_largeur / 2]) rotate([0, 90, 0]) cylinder(d = tic_largeur, h = paroi + 2);
        // avant-trous des vis du couvercle
        for (c = colonnes) translate([c[0], c[1], H - vis_colonne_profondeur]) cylinder(d = vis_pilote, h = vis_colonne_profondeur + 1);
    }
    // plots de la carte
    for (t = carte_trous) translate([carte_position[0] + t[0], carte_position[1] + t[1], 0]) difference() {
        cylinder(d = plot_diametre, h = plot_hauteur);
        translate([0, 0, plot_hauteur - vis_profondeur]) cylinder(d = vis_pilote, h = vis_profondeur + 1);
    }
    pont_serre_cable();
    // appui sous le bord bas de l'antenne (paroi haute)
    translate([antenne_x, W - appui[0], antenne_bas - appui[1]]) cube([antenne[0], appui[0], appui[1]]);
}

module couvercle() {   // dessiné en place (dessous à Z = H) ; à imprimer retourné, face extérieure sur le plateau
    difference() {
        union() {
            translate([0, 0, H]) linear_extrude(couvercle_epaisseur) contour();
            // rebord de centrage, à l'intérieur des parois
            translate([0, 0, H - rebord_hauteur]) linear_extrude(rebord_hauteur) difference() {
                offset(delta = -jeu) square([L, W]);
                offset(delta = -jeu - rebord_epaisseur) square([L, W]);
            }
            // collier de la LED
            translate([led[0], led[1], H - led_collier[1]]) cylinder(d = led_collier[0], h = led_collier[1]);
        }
        for (c = colonnes) translate([c[0], c[1], H - 1]) cylinder(d = vis_passage, h = couvercle_epaisseur + 2);
        translate([led[0], led[1], H - led_collier[1] - 1]) cylinder(d = led_trou, h = led_collier[1] + couvercle_epaisseur + 2);
    }
}

// --- Composants (contrôle seulement) : carte, XIAO, bornier, antenne
module composants() {
    z0 = plot_hauteur;
    color("green") translate([carte_position[0], carte_position[1], z0]) cube(carte);
    // XIAO : centre des broches à 47,45 du bord J1 sur la carte → à 10,55 du bord USB ; plaquette 21 × 17,8,
    // dessus à ≈ 18 mm de la carte ; prise USB-C 9 × 3,2, face à 1,5 mm au-delà du bord de la carte
    xc = carte_position[0] + 10.55; yc = usb_y;
    color("dimgray") translate([xc - 10.5, yc - 8.9, usb_hauteur - 3.5]) cube([21, 17.8, 5.1]);
    color("silver") translate([carte_position[0] - 1.5, yc - 4.5, usb_hauteur - 1.6]) cube([7.5, 9, 3.2]);
    // bornier J1 (10 × 8 × 10) au bord droit de la carte, entrée des fils vers la droite
    color("royalblue") translate([carte_position[0] + carte[0] - 8.2, tic_y - 5.5, z0 + carte[2]]) cube([8, 11, 10]);
    // optocoupleur, LED et JST (repères visuels)
    color("black") translate([carte_position[0] + 58 - 18 - 3.3, carte_position[1] + 33 - 16.5 - 4.6, z0 + carte[2]]) cube([6.6, 9.2, 4]);
    color("white") translate([carte_position[0] + 58 - 15 - 3.8, carte_position[1] + 33 - 6.1 - 3, z0 + carte[2]]) cube([7.6, 6, 12]);
    color("orange", 0.6) translate([antenne_x, W - 0.4, antenne_bas]) cube([antenne[0], 0.3, antenne[1]]);
}

if (piece == "boite") boite();
else if (piece == "couvercle") translate([0, 0, H + couvercle_epaisseur]) mirror([0, 0, 1]) couvercle();
else if (piece == "ensemble") { color("lightgray") boite(); composants(); color("gainsboro", 0.5) couvercle(); }
else if (piece == "eclate") { color("lightgray") boite(); composants(); translate([0, 0, 25]) color("gainsboro") couvercle(); }
else if (piece == "coupe") difference() {
    union() { color("lightgray") boite(); composants(); color("gainsboro") couvercle(); }
    translate([-50, usb_y, -50]) cube([200, 100, 100]);
}
