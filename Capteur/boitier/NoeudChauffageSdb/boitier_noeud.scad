// =============================================================================
//  boitier_noeud.scad – Boîtier du NoeudChauffageSdb (XIAO ESP32-C3, micro MAX4466, 2 LED IR), vissé au mur
// -----------------------------------------------------------------------------
//  Pièces :
//    - BOÎTE (PETG) : fond + parois, 4 plots de la carte, fente de la prise USB-C (paroi gauche), fente du câble
//      12 V ouverte vers le haut (paroi droite) + pont de serre-câble à l'intérieur, appui de l'antenne souple
//      (paroi basse), 4 colonnes de vis du couvercle aux coins (à l'extérieur), 4 oreilles de fixation au mur ;
//    - COUVERCLE (PETG), face tournée vers le chauffage : plaque vissée (4 vis M3), rebord de centrage,
//      grille + 2 plots M2 du micro, 2 trous + colliers des LED IR, 2 trous + tubes des LED témoins D6 / D7.
//
//  Repère (boîtier posé à plat contre le mur, vu de face, couvercle vers soi) : X vers la droite, Y vers le haut,
//  Z vers soi ; origine = coin bas gauche de l'INTÉRIEUR, au ras du fond intérieur.
//  La carte (64 × 48) est vue composants vers soi, tournée de 180° par rapport au dessin EasyEDA :
//  prise USB-C du XIAO à GAUCHE, prises J1 / J5 / J6 en HAUT, LED témoins en BAS.
//  Les positions des composants sont celles du PCB (repère EasyEDA, mm) ; carte() les passe dans le boîtier.
//
//  Réglages : menu Fenêtre > Customizer. Fiche : Capteur/boitier/NoeudChauffageSdb/Boîtier NoeudChauffageSdb.md
// =============================================================================

/* [Pièce à afficher] */
// ensemble, eclate, coupe : contrôle ; boite, couvercle : pièces à exporter en STL
piece = "eclate"; // [ensemble, eclate, coupe, boite, couvercle]

/* [Boîte] */
// Intérieur : longueur (X), largeur (Y), hauteur (Z, du fond intérieur au dessous du couvercle)
interieur = [66, 62, 30];
paroi = 2;
fond = 2;
couvercle_epaisseur = 2;

/* [Circuit imprimé] */
carte = [64, 48, 1.6];
// Coin bas gauche de la carte dans le boîtier : 1 mm de la paroi gauche (USB-C), 4 mm de la paroi basse (antenne)
carte_position = [1, 4];
// Trous M3 de la carte (symétriques : la rotation de 180° ne les change pas)
carte_trous = [[3.5, 3.5], [60.5, 3.5], [3.5, 44.5], [60.5, 44.5]];
plot_diametre = 7;
plot_hauteur = 4;
// Avant-trou pour une vis M3 vissée dans le PETG
vis_pilote = 2.5;
vis_profondeur = 6;

/* [Prise USB-C du XIAO (paroi gauche)] */
// Centre de la prise : Y = axe du XIAO (Y PCB 29) ; Z = plots 4 + carte 1,6 + 16,5 mesuré (XIAO sur barrette femelle)
usb_hauteur = 22.1;
// Fente (Y × Z) : la fiche du câble y entre
usb_fente = [13, 8];

/* [Câble 12 V (paroi droite)] */
// Fente ouverte vers le haut (le couvercle la referme) : centre Y, largeur (diamètre du câble), bas de la fente
alim_y = 57;
alim_largeur = 5;
alim_bas = 8;
// Pont de serre-câble (collier de 2,5 mm) entre la fente et le bornier J1 : centre X, taille, passage du collier
pont_x = 60.5;
pont = [6, 9, 7];
pont_passage = [3.5, 2];
pont_passage_z = 3;

/* [Antenne Wi-Fi souple (paroi basse)] */
// Antenne 40 × 20 mm collée contre la paroi basse (câble u.FL de 52 mm) ; appui sous son bord
antenne = [40, 20];
antenne_x = 13;
antenne_bas = 7;
appui = [1, 1];

/* [Vis du couvercle] */
// Colonnes aux 4 coins, À L'EXTÉRIEUR de l'intérieur (la carte est à 1 mm des parois gauche et droite)
colonne_diametre = 7;
colonne_decalage = 2.5;
vis_passage = 3.4;
vis_colonne_profondeur = 10;

/* [Couvercle] */
rebord_hauteur = 1.5;
rebord_epaisseur = 1.2;
jeu = 0.3;

/* [Micro MAX4466 (sous le couvercle)] */
// Plaquette 13,2 × 20, capsule vers le couvercle, broches vers le haut (vers J5) ; position PCB du centre de la
// capsule ; trous M2 à 10 mm d'entraxe, à 4,5 mm sous le centre de la capsule (2,2 du bord, capsule à 6,7)
micro_capsule = [28, 23.3];
micro_plaquette = [13.2, 20, 1.6];
micro_entraxe = 10;
micro_trous_dy = -4.5;
// Plots M2 : diamètre (limité par la capsule Ø 10), hauteur = capsule 5,3 + 0,5 de jeu (rondelle de mousse)
micro_plot = [3.4, 5.8];
micro_vis_pilote = 1.6;
// Grille : 7 trous (un au centre, 6 autour)
grille_trou = 1.5;
grille_rayon = 2.8;

/* [LED IR IR333C-A (couvercle)] */
// Positions PCB ; trou Ø 5,3 ; collier dessous (Ø ext., hauteur) : la collerette Ø 5,9 bute, le dôme dépasse ≈ 2,6
led_ir = [[16, 16], [6, 16]];
led_ir_trou = 5.3;
led_ir_collier = [8.5, 3];

/* [LED témoins D6 / D7 (couvercle)] */
// Positions PCB de D6 (bleue) et D7 (rouge) ; tube-guide sous le couvercle : la collerette bute au bas du tube,
// le dôme arrive 1,4 mm sous la face du couvercle. Pattes à laisser entre carte et collerette : témoin_pattes
led_temoin = [[11, 42], [32, 42]];
led_temoin_trou = 5.3;
led_temoin_tube = [8.5, 8];

/* [Fixation au mur] */
// 4 oreilles (2 en haut, 2 en bas) : position X des centres, saillie, largeur, épaisseur ; vis à bois Ø 4 fraisées
oreille_x = [12, 54];
oreille = [13, 14, 4];
oreille_trou = 4.5;
oreille_fraisure = 8.5;

$fn = 48;
L = interieur[0]; W = interieur[1]; H = interieur[2];
zc = plot_hauteur + carte[2];                       // dessus de la carte
colonnes = [[-colonne_decalage, -colonne_decalage], [L + colonne_decalage, -colonne_decalage],
            [-colonne_decalage, W + colonne_decalage], [L + colonne_decalage, W + colonne_decalage]];
// Point du PCB (repère EasyEDA) → point du boîtier (rotation de 180°)
function bp(p) = [carte_position[0] + carte[0] - p[0], carte_position[1] + carte[1] - p[1]];
usb_y = bp([0, 29])[1];
micro_c = bp(micro_capsule);
temoin_bas = H - led_temoin_tube[1];                // collerette des LED témoins
temoin_pattes = temoin_bas - zc;

// --- Contour extérieur, en 2D : rectangle (parois de 2 mm) + colonnes en saillie aux coins, raccords arrondis
module contour() {
    offset(r = -1) offset(r = 1) union() {
        translate([-paroi, -paroi]) square([L + 2 * paroi, W + 2 * paroi]);
        for (c = colonnes) translate(c) circle(d = colonne_diametre);
    }
}

module oreilles() {
    yt = -(oreille[0] - oreille[1] / 2);   // centre du trou, depuis la face extérieure de la paroi
    for (s = [0, 1], x = oreille_x) translate([x, s ? W + paroi : -paroi, -fond]) mirror([0, s, 0])
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
    translate([pont_x - pont[0] / 2, alim_y - pont[1] / 2, 0]) difference() {
        cube(pont);
        // passage du collier sous le dessus du pont, perpendiculaire au câble
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
        // fente du câble 12 V (paroi droite), ouverte vers le haut
        translate([L - 1, alim_y - alim_largeur / 2, alim_bas + alim_largeur / 2]) cube([paroi + 2, alim_largeur, H]);
        translate([L - 1, alim_y, alim_bas + alim_largeur / 2]) rotate([0, 90, 0]) cylinder(d = alim_largeur, h = paroi + 2);
        // avant-trous des vis du couvercle
        for (c = colonnes) translate([c[0], c[1], H - vis_colonne_profondeur]) cylinder(d = vis_pilote, h = vis_colonne_profondeur + 1);
    }
    // plots de la carte
    for (t = carte_trous) translate([carte_position[0] + t[0], carte_position[1] + t[1], 0]) difference() {
        cylinder(d = plot_diametre, h = plot_hauteur);
        translate([0, 0, plot_hauteur - vis_profondeur]) cylinder(d = vis_pilote, h = vis_profondeur + 1);
    }
    pont_serre_cable();
    // appui sous le bord de l'antenne (paroi basse)
    translate([antenne_x, 0, antenne_bas - appui[1]]) cube([antenne[0], appui[0], appui[1]]);
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
            // colliers des LED IR, tubes des LED témoins, plots M2 du micro
            for (p = led_ir) translate([bp(p)[0], bp(p)[1], H - led_ir_collier[1]]) cylinder(d = led_ir_collier[0], h = led_ir_collier[1]);
            for (p = led_temoin) translate([bp(p)[0], bp(p)[1], temoin_bas]) cylinder(d = led_temoin_tube[0], h = led_temoin_tube[1]);
            for (s = [-1, 1]) translate([micro_c[0] + s * micro_entraxe / 2, micro_c[1] + micro_trous_dy, H - micro_plot[1]])
                cylinder(d = micro_plot[0], h = micro_plot[1]);
        }
        for (c = colonnes) translate([c[0], c[1], H - 1]) cylinder(d = vis_passage, h = couvercle_epaisseur + 2);
        for (p = led_ir) translate([bp(p)[0], bp(p)[1], H - led_ir_collier[1] - 1]) cylinder(d = led_ir_trou, h = led_ir_collier[1] + couvercle_epaisseur + 2);
        for (p = led_temoin) translate([bp(p)[0], bp(p)[1], temoin_bas - 1]) cylinder(d = led_temoin_trou, h = led_temoin_tube[1] + couvercle_epaisseur + 2);
        for (s = [-1, 1]) translate([micro_c[0] + s * micro_entraxe / 2, micro_c[1] + micro_trous_dy, H - micro_plot[1] - 1])
            cylinder(d = micro_vis_pilote, h = micro_plot[1] - 0.5);
        // grille du micro
        translate([micro_c[0], micro_c[1], H - 1]) {
            cylinder(d = grille_trou, h = couvercle_epaisseur + 2);
            for (a = [0 : 60 : 300]) rotate(a) translate([grille_rayon, 0, 0]) cylinder(d = grille_trou, h = couvercle_epaisseur + 2);
        }
    }
}

// --- Composants (contrôle seulement) : carte, composants hauts, micro, LED, antenne
module piece_pcb(p, taille, couleur) {   // pavé centré sur une position PCB, posé sur la carte
    color(couleur) translate([bp(p)[0] - taille[0] / 2, bp(p)[1] - taille[1] / 2, zc]) cube(taille);
}
module composants() {
    color("green") translate([carte_position[0], carte_position[1], plot_hauteur]) cube(carte);
    // XIAO (A1, PCB 51 ; 29) : plaquette 21 × 17,8 dessus ≈ 18 mm au-dessus de la carte, prise USB-C 9 × 3,2
    color("dimgray") translate([bp([51.7, 29])[0] - 10.5, usb_y - 8.9, usb_hauteur - 3.5]) cube([21, 17.8, 5.1]);
    color("silver") translate([carte_position[0] - 1.5, usb_y - 4.5, usb_hauteur - 1.6]) cube([7.5, 9, 3.2]);
    piece_pcb([12.5, 5.2], [10.7, 7.9, 10], "royalblue");          // J1 bornier 12 V
    piece_pcb([39.5, 5.0], [10.3, 6.1, 13], "white");              // J5 micro (fiche JST-XH branchée)
    piece_pcb([26, 4.5], [7.6, 6.1, 13], "white");                 // J6 LED IR
    piece_pcb([13.8, 26], [11.5, 7.5, 10.5], "black");             // U1 R-78E5.0-1.0
    for (p = [[11.5, 13.5], [58.5, 10]]) piece_pcb(p, [5, 5, 12], "navy");        // C1, C8
    for (p = [[24, 29], [30.5, 16.5]]) piece_pcb(p, [6.3, 6.3, 12], "navy");      // C6, C10
    piece_pcb([23.5, 15.5], [5.1, 2.5, 11.5], "black");            // JP2 + cavalier
    piece_pcb([15, 36], [5.1, 2.5, 11.5], "black");                // JP1 + cavalier
    // LED témoins : pattes jusqu'à la collerette, dans le tube
    for (p = led_temoin) translate([bp(p)[0], bp(p)[1], 0]) {
        color("silver") translate([-1.5, -0.25, zc]) cube([3, 0.5, temoin_pattes]);
        color("red", 0.8) translate([0, 0, temoin_bas]) { cylinder(d = 5.9, h = 1); cylinder(d = 5, h = 6.1); translate([0, 0, 6.1]) sphere(d = 5); }
    }
    // LED IR : collerette sous le collier, dôme dans le couvercle
    for (p = led_ir) translate([bp(p)[0], bp(p)[1], H - led_ir_collier[1] - 1]) color("lavender") {
        cylinder(d = 5.9, h = 1); cylinder(d = 5, h = 6.1); translate([0, 0, 6.1]) sphere(d = 5);
    }
    // micro : capsule contre le couvercle (0,5 mm), plaquette, composants du dos (≈ 2,1)
    translate([micro_c[0], micro_c[1], H - micro_plot[1]]) {
        color("black") translate([0, 0, 0]) cylinder(d = 10, h = 5.3);
        color("purple") translate([-micro_plaquette[0] / 2, -6.7 - 0, -micro_plaquette[2]])
            mirror([0, 0, 0]) cube(micro_plaquette);
        color("gray") translate([-3, 2, -micro_plaquette[2] - 2.1]) cube([6, 6, 2.1]);
    }
    color("orange", 0.6) translate([antenne_x, 0.1, antenne_bas]) cube([antenne[0], 0.3, antenne[1]]);
}

if (piece == "boite") boite();
else if (piece == "couvercle") translate([0, 0, H + couvercle_epaisseur]) mirror([0, 0, 1]) couvercle();
else if (piece == "ensemble") { color("lightgray") boite(); composants(); color("gainsboro", 0.5) couvercle(); }
else if (piece == "eclate") { color("lightgray") boite(); composants(); translate([0, 0, 30]) color("gainsboro") couvercle(); }
else if (piece == "coupe") difference() {
    union() { color("lightgray") boite(); composants(); color("gainsboro") couvercle(); }
    translate([-50, micro_c[1], -50]) cube([200, 100, 100]);
}
