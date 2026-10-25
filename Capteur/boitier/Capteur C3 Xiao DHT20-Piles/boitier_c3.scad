// =============================================================================
//  boitier_c3.scad – Boîtier du Capteur C3 Xiao, variante DHT20 sur piles (XIAO ESP32-C3, 4 piles AA,
//  interrupteur, DHT20), posé sur une armoire ou une étagère
// -----------------------------------------------------------------------------
//  Pièces :
//    - BOÎTE (PETG), en haut : fond + parois, 4 plots de la carte ; sous le fond : 2 bossages des vis du support de
//      piles (côté carte) et trou des fils des piles ; paroi basse : trou de l'interrupteur MTS-102 ; paroi haute :
//      trou M3 du DHT20 (vissé dehors) et trou de son câble, appui de l'antenne Wi-Fi souple (dedans) ;
//      4 colonnes aux coins (à l'extérieur) : vis du couvercle en haut, vis du socle en bas ;
//    - COUVERCLE (PETG) : plaque vissée (4 vis M3), rebord de centrage ;
//    - SOCLE (PETG), en bas : cadre vissé sous la boîte (4 vis M3 par en dessous), le support de piles y pend sous
//      le fond de la boîte ; dessous : rebords et appuis de la trappe, fente de sortie dans la paroi gauche,
//      rainure du cran ;
//    - TRAPPE (PETG) : plaque qui coulisse sous le socle, vers la gauche, sans outil ; bossette du cran, stries.
//
//  Repère (boîtier posé à plat, vu de dessus, couvercle vers soi) : X vers la droite, Y vers le haut (paroi haute),
//  Z vers soi ; origine = coin bas gauche de l'INTÉRIEUR de la boîte, au ras de son fond intérieur.
//  La carte (46 × 65, même circuit imprimé que le Capteur S3) est vue composants vers soi, tournée de 180° par
//  rapport au dessin EasyEDA : borniers J3 (piles) / J4 (interrupteur) en BAS, J1 (DHT20) à DROITE, XIAO en haut
//  à gauche. Les positions des composants sont celles du PCB (repère EasyEDA, mm) ; bp() les passe dans le boîtier.
//  Pas de fente USB : on ouvre le couvercle pour programmer (interrupteur sur ARRÊT avant de brancher l'USB).
//
//  Réglages : menu Fenêtre > Customizer. Fiche : Capteur/boitier/Capteur C3 Xiao DHT20-Piles/Boîtier Capteur C3 Xiao DHT20-Piles.md
// =============================================================================

/* [Pièce à afficher] */
// ensemble, eclate, coupe : contrôle ; boite, couvercle, socle, trappe : pièces à exporter en STL
piece = "eclate"; // [ensemble, eclate, coupe, boite, couvercle, socle, trappe]

/* [Boîte (en haut)] */
// Intérieur : longueur (X), largeur (Y), hauteur (Z, du fond intérieur au dessous du couvercle)
interieur = [64, 83, 27];
paroi = 2;
fond = 2;
couvercle_epaisseur = 2;

/* [Circuit imprimé] */
carte = [46, 65, 1.6];
// Coin bas gauche de la carte dans la boîte : 2 mm de la paroi gauche, 15 mm de la paroi basse (fils des borniers,
// interrupteur)
carte_position = [2, 15];
// Trous M3 de la carte (symétriques : la rotation de 180° ne les change pas)
carte_trous = [[3.5, 3.5], [42.5, 3.5], [3.5, 61.5], [42.5, 61.5]];
plot_diametre = 7;
plot_hauteur = 4;
// Avant-trou pour une vis M3 vissée dans le PETG
vis_pilote = 2.5;
vis_profondeur = 6;

/* [XIAO ESP32-C3 (A1)] */
// Position PCB du centre du XIAO ; prise USB-C et dessus du XIAO au-dessus de la carte (sur barrettes femelles,
// estimés d'après une autre carte XIAO C3, à mesurer)
xiao_pcb = [33, 18];
xiao_usb = 16.5;
xiao_dessus = 18.6;

/* [Interrupteur MTS-102 (paroi basse)] */
// Trou Ø 6,2 ; corps 12,9 × 7,9, ≈ 18 mm derrière la paroi avec les cosses : à la place de J5 (non monté), loin de J4
interrupteur_x = 14;
interrupteur_hauteur = 20;
interrupteur_trou = 6.2;

/* [DHT20 (sur la paroi haute, dehors)] */
// Plaquette 28,5 × 13 posée à plat contre la paroi haute, capteur à GAUCHE, broches coudées à DROITE (dépassent
// de 7) ; trou M3 à 6,5 mm du bord côté broches ; trou du câble sous la pointe des broches (rebouché au montage).
// Bord côté broches (X) ; hauteur du centre (Z)
dht_broches_x = 50;
dht_hauteur = 13.5;
dht_module = [28.5, 13];
dht_trou_broches = 6.5;
dht_broches = 7;
dht_trou = 3.4;
dht_cable_trou = 6;

/* [Antenne Wi-Fi souple (paroi haute, dedans)] */
// Antenne 40 × 20 contre la paroi haute : X de début, bas
antenne = [40, 20];
antenne_x = 1;
antenne_bas = 5;
appui = [1, 1];

/* [Support 4 × AA (sous la boîte, dans le socle)] */
// 62 (X, 4 piles côte à côte) × 57,2 (Y, longueur d'une pile) × 16,5 ; vissé sous le fond de la boîte, piles vers
// le bas ; fils sur le grand côté BAS, à 6 mm du coin droit ; 2 trous Ø 3 au milieu, entraxe 44 (sens X)
piles = [62, 57.2, 16.5];
piles_coin = [1, 14];
piles_entraxe = 44;
// Bossages côté carte (vis M3 vissée par en dessous, depuis l'intérieur du support) : Ø, hauteur (sous la carte)
piles_bossage = [7, 3.5];
piles_vis_profondeur = 5;
// Trou des fils des piles dans le fond, sous J3 (X × Y) : coin bas gauche
fils_trou = [8, 8];
fils_trou_coin = [52, 3];

/* [Socle (en bas)] */
// Jeu sous les piles ; trappe : épaisseur, jeu ; rebords qui la retiennent (épaisseur, largeur sur les côtés, au
// bout droit) ; appuis au-dessus d'elle
socle_jeu = 1;
trappe_epaisseur = 2;
trappe_jeu = 0.3;
rebord_trappe = [1.5, 1.5, 6];
trappe_appui = 1.5;
// Cran : bossette sous la trappe (hauteur, largeur) qui tombe dans une rainure du rebord droit, à cran_x du bout
cran = [0.5, 1.2];
cran_x = 3;
// Stries pour le pouce (creuses), près du bout droit de l'ouverture
stries = 5;
stries_pas = 3;
stries_profondeur = 0.6;
// Vis du socle (par en dessous, dans les colonnes de la boîte) : passage, lamage (Ø, profondeur)
socle_lamage = [6.5, 3.5];

/* [Vis du couvercle et du socle] */
// Colonnes aux 4 coins, À L'EXTÉRIEUR de l'intérieur
colonne_diametre = 7;
colonne_decalage = 2.5;
vis_passage = 3.4;
vis_colonne_profondeur = 10;

/* [Couvercle] */
rebord_hauteur = 1.5;
rebord_epaisseur = 1.2;
jeu = 0.3;

$fn = 48;
L = interieur[0]; W = interieur[1]; H = interieur[2];
zc = plot_hauteur + carte[2];                       // dessus de la carte
colonnes = [[-colonne_decalage, -colonne_decalage], [L + colonne_decalage, -colonne_decalage],
            [-colonne_decalage, W + colonne_decalage], [L + colonne_decalage, W + colonne_decalage]];
// Point du PCB (repère EasyEDA) → point du boîtier (rotation de 180°)
function bp(p) = [carte_position[0] + carte[0] - p[0], carte_position[1] + carte[1] - p[1]];
xiao = bp(xiao_pcb);
usb_hauteur = zc + xiao_usb;
// Support de piles : trous ; dessous du fond de la boîte (= dessus du socle)
piles_trous = [for (s = [-1, 1]) [piles_coin[0] + piles[0] / 2 + s * piles_entraxe / 2, piles_coin[1] + piles[1] / 2]];
z_socle = -fond;
// Trappe : dessus, dessous ; rebords dessous ; bas du socle
trappe_haut = z_socle - piles[2] - socle_jeu;
trappe_bas = trappe_haut - trappe_epaisseur;
z_bas = trappe_bas - 0.1 - rebord_trappe[0];
// DHT20 : trou M3, trou du câble
dht_trou_x = dht_broches_x - dht_trou_broches;
dht_cable_x = dht_broches_x + dht_broches;

// --- Contour extérieur, en 2D : rectangle (parois de 2 mm) + colonnes en saillie aux coins, raccords arrondis
module contour() {
    offset(r = -1) offset(r = 1) union() {
        translate([-paroi, -paroi]) square([L + 2 * paroi, W + 2 * paroi]);
        for (c = colonnes) translate(c) circle(d = colonne_diametre);
    }
}

module boite() {
    difference() {
        translate([0, 0, -fond]) linear_extrude(fond + H, convexity = 10) contour();
        // intérieur
        cube([L, W, H + 1]);
        // trou des fils des piles dans le fond
        translate([fils_trou_coin[0], fils_trou_coin[1], -fond - 1]) cube([fils_trou[0], fils_trou[1], fond + 2]);
        // trou de l'interrupteur (paroi basse)
        translate([interrupteur_x, -paroi - 1, interrupteur_hauteur]) rotate([-90, 0, 0]) cylinder(d = interrupteur_trou, h = paroi + 2);
        // DHT20 (paroi haute) : trou M3 et trou du câble
        translate([dht_trou_x, W - 1, dht_hauteur]) rotate([-90, 0, 0]) cylinder(d = dht_trou, h = paroi + 2);
        translate([dht_cable_x, W - 1, dht_hauteur]) rotate([-90, 0, 0]) cylinder(d = dht_cable_trou, h = paroi + 2);
        // avant-trous des vis : couvercle en haut, socle en bas
        for (c = colonnes) {
            translate([c[0], c[1], H - vis_colonne_profondeur]) cylinder(d = vis_pilote, h = vis_colonne_profondeur + 1);
            translate([c[0], c[1], -fond - 1]) cylinder(d = vis_pilote, h = vis_colonne_profondeur + 1);
        }
    }
    // plots de la carte
    for (t = carte_trous) translate([carte_position[0] + t[0], carte_position[1] + t[1], 0]) difference() {
        cylinder(d = plot_diametre, h = plot_hauteur);
        translate([0, 0, plot_hauteur - vis_profondeur]) cylinder(d = vis_pilote, h = vis_profondeur + 1);
    }
    // bossages des vis du support de piles (vissées par en dessous, à travers le fond)
    for (t = piles_trous) translate([t[0], t[1], 0]) difference() {
        cylinder(d = piles_bossage[0], h = piles_bossage[1]);
        translate([0, 0, -fond - 1]) cylinder(d = vis_pilote, h = fond + 1 + piles_vis_profondeur - fond);
    }
    // appui sous le bord bas de l'antenne
    translate([antenne_x, W - appui[0], antenne_bas - appui[1]]) cube([antenne[0], appui[0], appui[1]]);
}

// avant-trous des vis du support traversant le fond (hors des bossages)
module trous_vis_piles() {
    for (t = piles_trous) translate([t[0], t[1], -fond - 1]) cylinder(d = vis_pilote, h = fond + 1);
}

module couvercle() {   // dessiné en place (dessous à Z = H) ; à imprimer retourné, face extérieure sur le plateau
    difference() {
        union() {
            translate([0, 0, H]) linear_extrude(couvercle_epaisseur, convexity = 10) contour();
            translate([0, 0, H - rebord_hauteur]) linear_extrude(rebord_hauteur, convexity = 10) difference() {
                offset(delta = -jeu) square([L, W]);
                offset(delta = -jeu - rebord_epaisseur) square([L, W]);
            }
        }
        for (c = colonnes) translate([c[0], c[1], H - 1]) cylinder(d = vis_passage, h = couvercle_epaisseur + 2);
    }
}

// Appui de la trappe le long d'une paroi (y0 : face de la paroi, s : +1 vers le haut, -1 vers le bas), chanfreiné
// dessus (il s'imprime socle retourné, rebords sur le plateau)
module appui_trappe(y0, s) {
    hull() {
        translate([0, s > 0 ? y0 : y0 - trappe_appui, trappe_haut + 0.1]) cube([L, trappe_appui, 0.8]);
        translate([0, s > 0 ? y0 : y0 - 0.01, trappe_haut + 0.1 + 0.8 + trappe_appui]) cube([L, 0.01, 0.01]);
    }
}

module socle() {   // dessiné en place ; à imprimer retourné, rebords de la trappe sur le plateau
    difference() {
        translate([0, 0, z_bas]) linear_extrude(z_socle - z_bas, convexity = 10) contour();
        // intérieur, au-dessus des rebords
        translate([0, 0, z_bas + rebord_trappe[0]]) cube([L, W, z_socle - z_bas]);
        // ouverture entre les rebords (le bout droit reste plein sur rebord_trappe[2])
        translate([0, rebord_trappe[1], z_bas - 1]) cube([L - rebord_trappe[2], W - 2 * rebord_trappe[1], rebord_trappe[0] + 2]);
        // fente de sortie de la trappe (paroi gauche)
        translate([-paroi - 1, 0, trappe_bas - trappe_jeu]) cube([paroi + 2, W, trappe_epaisseur + 2 * trappe_jeu]);
        // rainure du cran (dessus du rebord droit)
        translate([L - cran_x - (cran[1] + 0.2) / 2, rebord_trappe[1], z_bas + rebord_trappe[0] - cran[0] - 0.1])
            cube([cran[1] + 0.2, W - 2 * rebord_trappe[1], cran[0] + 0.2]);
        // vis du socle : passage et lamage par en dessous
        for (c = colonnes) translate([c[0], c[1], z_bas - 1]) {
            cylinder(d = vis_passage, h = z_socle - z_bas + 2);
            cylinder(d = socle_lamage[0], h = socle_lamage[1] + 1);
        }
    }
    appui_trappe(0, 1);
    appui_trappe(W, -1);
}

module trappe() {   // dessinée en place ; à imprimer à plat, stries vers le haut
    difference() {
        union() {
            translate([-paroi, trappe_jeu, trappe_bas]) cube([L - trappe_jeu + paroi, W - 2 * trappe_jeu, trappe_epaisseur]);
            // bossette du cran (dessous)
            translate([L - cran_x - cran[1] / 2, rebord_trappe[1] + 1, trappe_bas - cran[0] + 0.01])
                cube([cran[1], W - 2 * rebord_trappe[1] - 2, cran[0]]);
        }
        // stries pour le pouce (dessous), près du bout droit de l'ouverture
        for (i = [0 : stries - 1]) translate([L - rebord_trappe[2] - 6 - i * stries_pas, W / 2 - 12, trappe_bas - 1])
            cube([1.2, 24, 1 + stries_profondeur]);
    }
}

// --- Composants (contrôle seulement)
module piece_pcb(p, taille, couleur) {   // pavé centré sur une position PCB, posé sur la carte
    color(couleur) translate([bp(p)[0] - taille[0] / 2, bp(p)[1] - taille[1] / 2, zc]) cube(taille);
}
module composants() {
    color("green") translate([carte_position[0], carte_position[1], plot_hauteur]) cube(carte);
    // XIAO C3 sur barrettes femelles : plaquette 21 × 17,8, prise USB-C vers la paroi gauche
    color("dimgray") translate([xiao[0] - 10.5, xiao[1] - 8.9, usb_hauteur - 3.5]) cube([21, 17.8, xiao_dessus - xiao_usb + 3.5]);
    color("black") translate([xiao[0] - 10.5, xiao[1] - 8.9, zc]) cube([21, 2.5, usb_hauteur - 3.5 - zc]);
    color("black") translate([xiao[0] - 10.5, xiao[1] + 6.4, zc]) cube([21, 2.5, usb_hauteur - 3.5 - zc]);
    color("silver") translate([xiao[0] - 12, xiao[1] - 4.5, usb_hauteur - 1.6]) cube([7.5, 9, 3.2]);
    piece_pcb([3.2, 22], [6.1, 12.4, 13], "white");               // J1 DHT20 (fiche JST-XH branchée)
    piece_pcb([11, 60.6], [10, 7.6, 10], "royalblue");            // J3 piles (bornier XY301V)
    piece_pcb([21.5, 61.6], [7.4, 6.1, 13], "white");             // J4 interrupteur (fiche JST-XH)
    piece_pcb([39, 52], [6.3, 6.3, 11], "navy");                  // C5 470 µF
    piece_pcb([16.5, 52], [5, 5, 11], "navy");                    // C3 10 µF
    piece_pcb([37, 4.5], [5, 5, 11], "navy");                     // C2 10 µF
    piece_pcb([23.5, 51.5], [4.6, 3.6, 8], "black");              // U1 MCP1700
    piece_pcb([8, 52], [8, 2.5, 3], "black");                     // D2 1N5819
    for (x = [9.5, 13, 16.5, 20]) piece_pcb([x, 31], [2, 2, 8], "red");         // TP1–TP4
    // interrupteur : corps derrière la paroi basse, levier dehors
    translate([interrupteur_x, 0, interrupteur_hauteur]) {
        color("red") translate([-6.45, 0, -3.95]) cube([12.9, 18, 7.9]);
        color("silver") rotate([90, 0, 0]) cylinder(d = 6, h = paroi + 7);
    }
    // DHT20 contre la paroi haute : plaquette, capteur ajouré à gauche, broches coudées à droite
    translate([dht_broches_x - dht_module[0], W + paroi, dht_hauteur - dht_module[1] / 2]) {
        color("white") cube([dht_module[0], 1.6, dht_module[1]]);
        color("lightgray") translate([0, 1.6, 0.5]) cube([15, 5, 12]);
        color("gold") for (i = [0 : 3]) translate([dht_module[0], 0.5, dht_module[1] / 2 - 3.81 + i * 2.54 - 0.3]) cube([dht_broches, 0.6, 0.6]);
    }
    // antenne souple
    color("orange", 0.6) translate([antenne_x, W - 0.4, antenne_bas]) cube([antenne[0], 0.3, antenne[1]]);
    // support de piles sous le fond, ouvert vers le bas, 4 piles
    translate([piles_coin[0], piles_coin[1], z_socle - piles[2]]) {
        color("black") difference() { cube(piles); translate([1, 1, -1]) cube(piles - [2, 2, 2]); }
        for (i = [0 : 3]) color("orange") translate([1 + 7.5 + i * 15, 3.4, 7]) rotate([-90, 0, 0]) cylinder(d = 14.5, h = 50.5);
        color("red") translate([piles[0] - 6 - 1, -3, 10]) cube([2, 3, 2]);
    }
}

module boite_complete() { difference() { boite(); trous_vis_piles(); } }

if (piece == "boite") boite_complete();
else if (piece == "couvercle") translate([0, 0, H + couvercle_epaisseur]) mirror([0, 0, 1]) couvercle();
else if (piece == "socle") translate([0, 0, -z_bas]) socle();
else if (piece == "trappe") translate([0, 0, trappe_haut]) mirror([0, 0, 1]) trappe();
else if (piece == "ensemble") { color("lightgray") boite_complete(); composants(); color("gainsboro", 0.5) couvercle();
                                color("silver") socle(); color("tan") trappe(); }
else if (piece == "eclate") { color("lightgray") boite_complete(); composants(); translate([0, 0, 30]) color("gainsboro") couvercle();
                              translate([0, 0, -25]) color("silver") socle(); translate([-40, 0, -45]) color("tan") trappe(); }
else if (piece == "coupe") difference() {
    union() { color("lightgray") boite_complete(); composants(); color("gainsboro") couvercle(); color("silver") socle(); color("tan") trappe(); }
    // coupe par l'axe d'un trou du support de piles (plan Y-Z)
    translate([piles_trous[1][0], -50, -80]) cube([200, 200, 200]);
}
