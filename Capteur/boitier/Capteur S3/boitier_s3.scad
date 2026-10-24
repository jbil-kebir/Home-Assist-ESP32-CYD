// =============================================================================
//  boitier_s3.scad – Boîtier du Capteur S3, variante cuve (XIAO ESP32-S3 + Wio-SX1262, 4 piles AA, interrupteur,
//  flotteur, DHT20), vissé sur du bois
// -----------------------------------------------------------------------------
//  Pièces :
//    - BOÎTE (PETG) : deux compartiments côte à côte séparés par une cloison (carte à gauche, piles à droite) ;
//      4 plots de la carte, 2 plots du support de piles, trou des fils des piles dans la cloison ; paroi basse :
//      fente du fil du flotteur (ouverte vers le couvercle) + pont de serre-câble, trou de l'interrupteur MTS-102 ;
//      appuis des antennes souples (Wi-Fi paroi haute, LoRa paroi gauche) ; glissières de la trappe (appuis sur
//      la cloison et la paroi droite, fente de sortie dans la paroi haute) ; 6 colonnes de vis du couvercle
//      (à l'extérieur) ; 4 oreilles de fixation ;
//    - COUVERCLE (PETG), face vers soi : plaque vissée (6 vis M3) sur toute la boîte, rebord de centrage autour du
//      compartiment de la carte, trou M3 du DHT20 (vissé dehors) et trou de son câble, ouverture au-dessus de la
//      trappe (ses bords retiennent la trappe), rainure du cran ;
//    - TRAPPE (PETG) : plaque qui coulisse sous le couvercle, vers le haut, sans outil ; bossette du cran, stries
//      pour le pouce.
//
//  Repère (boîtier posé à plat sur le bois, vu de face, couvercle vers soi) : X vers la droite, Y vers le haut,
//  Z vers soi ; origine = coin bas gauche de l'INTÉRIEUR, au ras du fond intérieur.
//  La carte (46 × 65) est vue composants vers soi, tournée de 180° par rapport au dessin EasyEDA :
//  borniers J3 (piles) / J4 (interrupteur) / J5 (flotteur) en BAS, J1 (DHT20) à DROITE, XIAO en haut à gauche.
//  Les positions des composants sont celles du PCB (repère EasyEDA, mm) ; bp() les passe dans le boîtier.
//  Pas de fente USB : on ouvre le couvercle pour programmer (interrupteur sur ARRÊT avant de brancher l'USB).
//
//  Réglages : menu Fenêtre > Customizer. Fiche : Capteur/boitier/Capteur S3/Boîtier Capteur S3.md
// =============================================================================

/* [Pièce à afficher] */
// ensemble, eclate, coupe : contrôle ; boite, couvercle, trappe : pièces à exporter en STL
piece = "eclate"; // [ensemble, eclate, coupe, boite, couvercle, trappe]

/* [Boîte] */
// Intérieur total : longueur (X), largeur (Y), hauteur (Z, du fond intérieur au dessous du couvercle)
interieur = [121, 83, 33];
paroi = 2;
fond = 2;
couvercle_epaisseur = 2;
// Cloison entre la carte (à gauche) et les piles (à droite) : X de sa face gauche, épaisseur
cloison_x = 57;
cloison = 2;
// Trou des fils des piles dans la cloison (Y × Z) : coin bas
cloison_trou = [10, 10];
cloison_trou_bas = [3, 4];

/* [Circuit imprimé] */
carte = [46, 65, 1.6];
// Coin bas gauche de la carte dans le boîtier : 2 mm de la paroi gauche (antenne LoRa), 15 mm de la paroi basse
// (fils des borniers, serre-câble, interrupteur)
carte_position = [2, 15];
// Trous M3 de la carte (symétriques : la rotation de 180° ne les change pas)
carte_trous = [[3.5, 3.5], [42.5, 3.5], [3.5, 61.5], [42.5, 61.5]];
plot_diametre = 7;
plot_hauteur = 4;
// Avant-trou pour une vis M3 vissée dans le PETG
vis_pilote = 2.5;
vis_profondeur = 6;

/* [XIAO S3 + Wio (A1)] */
// Position PCB du centre du XIAO ; prise USB-C et dessus du Wio au-dessus de la carte (estimés, à mesurer)
xiao_pcb = [33, 18];
xiao_usb = 16.5;
xiao_dessus = 25;

/* [Support 4 × AA (compartiment de droite)] */
// 62 (Y, 4 piles côte à côte) × 57,2 (X, longueur d'une pile) × 16,5 ; ouvert vers le couvercle ; fils sur le grand
// côté gauche (vers la cloison), à 6 mm du coin bas ; 2 trous Ø 3 au milieu, entraxe 44 (sens Y)
piles = [57.2, 62, 16.5];
piles_centre = [91, 33];
piles_entraxe = 44;
piles_plot = [7, 3];

/* [Fil du flotteur (paroi basse)] */
// Fente ouverte vers le couvercle (il la referme) : centre X (sous J5), largeur (diamètre du câble), bas de la fente
flotteur_x = 14.5;
flotteur_largeur = 5;
flotteur_bas = 8;
// Pont de serre-câble (collier de 2,5 mm) entre la fente et J5 : centre Y, taille, passage du collier
pont_y = 6;
pont = [9, 6, 7];
pont_passage = [2, 3.5];
pont_passage_z = 3;

/* [Interrupteur MTS-102 (paroi basse)] */
// Trou Ø 6,2 ; corps 12,9 × 7,9, ≈ 18 mm derrière la paroi avec les cosses : au-dessus des borniers
interrupteur_x = 30;
interrupteur_hauteur = 25;
interrupteur_trou = 6.2;

/* [DHT20 (sur le couvercle, dehors)] */
// Plaquette 28,5 × 13 posée verticalement, capteur vers le HAUT, broches coudées vers le BAS (dépassent de 7) ;
// trou M3 à 6,5 mm du bord côté broches ; trou du câble sous la pointe des broches (rebouché au montage)
dht_x = 49.5;
dht_module = [13, 28.5];
dht_trou_y = 58.5;
dht_trou_broches = 6.5;
dht_broches = 7;
dht_trou = 3.4;
dht_cable_trou = 6;

/* [Antennes souples (à l'intérieur, contre les parois)] */
// Wi-Fi du XIAO 40 × 20 (paroi haute) : X de début, bas ; LoRa du Wio 40 × 10 (paroi gauche) : Y de début, bas
antenne_wifi = [40, 20];
antenne_wifi_x = 2;
antenne_wifi_bas = 12;
antenne_lora = [40, 10];
antenne_lora_y = 20;
antenne_lora_bas = 20;
appui = [1, 1];

/* [Trappe des piles (glissière)] */
// Plaque sous le couvercle, posée sur 2 appuis (cloison, paroi droite) ; elle sort vers le haut par une fente de la
// paroi haute. Épaisseur, jeu, largeur des appuis et des bords du couvercle qui la retiennent
trappe_epaisseur = 2;
trappe_jeu = 0.3;
trappe_appui = 1.5;
trappe_rebord = 1.5;
// Ouverture du couvercle au-dessus de la trappe : marges basse et haute (le couvercle recouvre la trappe)
trappe_marge_bas = 6;
trappe_marge_haut = 3;
// Cran : bossette sur la trappe (hauteur, largeur) qui tombe dans une rainure du couvercle, à cran_y du bas
cran = [0.5, 1.2];
cran_y = 3;
// Stries pour le pouce (creuses : elles passent sous le couvercle)
stries = 5;
stries_pas = 3;
stries_profondeur = 0.6;

/* [Vis du couvercle] */
// 6 colonnes à l'extérieur de l'intérieur : 4 coins + 2 au milieu des grands côtés (couvercle de 125 mm)
colonne_diametre = 7;
colonne_decalage = 2.5;
colonne_milieu_x = 56;
vis_passage = 3.4;
vis_colonne_profondeur = 10;

/* [Couvercle] */
rebord_hauteur = 1.5;
rebord_epaisseur = 1.2;
jeu = 0.3;

/* [Fixation sur le bois] */
// 4 oreilles (2 en haut, 2 en bas, loin du fil du flotteur et de l'interrupteur) : position X des centres, saillie,
// largeur, épaisseur ; vis à bois Ø 4 fraisées
oreille_x = [45, 105];
oreille = [13, 14, 4];
oreille_trou = 4.5;
oreille_fraisure = 8.5;

$fn = 48;
L = interieur[0]; W = interieur[1]; H = interieur[2];
zc = plot_hauteur + carte[2];                       // dessus de la carte
px0 = cloison_x + cloison;                          // face gauche du compartiment des piles
colonnes = [[-colonne_decalage, -colonne_decalage], [L + colonne_decalage, -colonne_decalage],
            [-colonne_decalage, W + colonne_decalage], [L + colonne_decalage, W + colonne_decalage],
            [colonne_milieu_x, -colonne_decalage], [colonne_milieu_x, W + colonne_decalage]];
// Point du PCB (repère EasyEDA) → point du boîtier (rotation de 180°)
function bp(p) = [carte_position[0] + carte[0] - p[0], carte_position[1] + carte[1] - p[1]];
xiao = bp(xiao_pcb);
usb_hauteur = zc + xiao_usb;
// Trappe : dessous, dessus ; appuis juste dessous
trappe_z = H - 0.1 - trappe_epaisseur;
appui_z = trappe_z - 0.1;
trappe_x = [px0 + trappe_jeu, L - trappe_jeu];
// DHT20 : bord côté broches, trou du câble
dht_bas = dht_trou_y - dht_trou_broches;
dht_cable_y = dht_bas - dht_broches;

// --- Contour extérieur, en 2D : rectangle (parois de 2 mm) + colonnes en saillie, raccords arrondis
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
    translate([flotteur_x - pont[0] / 2, pont_y - pont[1] / 2, 0]) difference() {
        cube(pont);
        // passage du collier sous le dessus du pont, perpendiculaire au câble
        translate([-1, (pont[1] - pont_passage[1]) / 2, pont_passage_z]) cube([pont[0] + 2, pont_passage[1], pont_passage[0]]);
    }
}

// Appui de la trappe le long d'une paroi (x0 : face de la paroi, s : +1 vers la droite, -1 vers la gauche),
// chanfreiné dessous pour s'imprimer sans support
module appui_trappe(x0, s) {
    hull() {
        translate([s > 0 ? x0 : x0 - trappe_appui, 0, appui_z - 0.8]) cube([trappe_appui, W, 0.8]);
        translate([s > 0 ? x0 : x0 - 0.01, 0, appui_z - 0.8 - trappe_appui]) cube([0.01, W, 0.01]);
    }
}

module boite() {
    difference() {
        union() {
            translate([0, 0, -fond]) linear_extrude(fond + H, convexity = 10) contour();
            oreilles();
        }
        // les deux compartiments (la cloison reste entre eux)
        cube([cloison_x, W, H + 1]);
        translate([px0, 0, 0]) cube([L - px0, W, H + 1]);
        // trou des fils des piles dans la cloison
        translate([cloison_x - 1, cloison_trou_bas[0], cloison_trou_bas[1]]) cube([cloison + 2, cloison_trou[0], cloison_trou[1]]);
        // fente du fil du flotteur (paroi basse), ouverte vers le couvercle
        translate([flotteur_x - flotteur_largeur / 2, -paroi - 1, flotteur_bas + flotteur_largeur / 2]) cube([flotteur_largeur, paroi + 2, H]);
        translate([flotteur_x, -paroi - 1, flotteur_bas + flotteur_largeur / 2]) rotate([-90, 0, 0]) cylinder(d = flotteur_largeur, h = paroi + 2);
        // trou de l'interrupteur (paroi basse)
        translate([interrupteur_x, -paroi - 1, interrupteur_hauteur]) rotate([-90, 0, 0]) cylinder(d = interrupteur_trou, h = paroi + 2);
        // fente de sortie de la trappe (paroi haute, au-dessus du compartiment des piles)
        translate([px0, W - 1, trappe_z - trappe_jeu]) cube([L - px0, paroi + 2, H]);
        // avant-trous des vis du couvercle
        for (c = colonnes) translate([c[0], c[1], H - vis_colonne_profondeur]) cylinder(d = vis_pilote, h = vis_colonne_profondeur + 1);
    }
    // plots de la carte
    for (t = carte_trous) translate([carte_position[0] + t[0], carte_position[1] + t[1], 0]) difference() {
        cylinder(d = plot_diametre, h = plot_hauteur);
        translate([0, 0, plot_hauteur - vis_profondeur]) cylinder(d = vis_pilote, h = vis_profondeur + 1);
    }
    // plots du support de piles
    for (s = [-1, 1]) translate([piles_centre[0], piles_centre[1] + s * piles_entraxe / 2, 0]) difference() {
        cylinder(d = piles_plot[0], h = piles_plot[1]);
        translate([0, 0, piles_plot[1] - vis_profondeur]) cylinder(d = vis_pilote, h = vis_profondeur + 1);
    }
    pont_serre_cable();
    // appuis de la trappe (cloison, paroi droite)
    appui_trappe(px0, 1);
    appui_trappe(L, -1);
    // appuis sous le bord bas des antennes
    translate([antenne_wifi_x, W - appui[0], antenne_wifi_bas - appui[1]]) cube([antenne_wifi[0], appui[0], appui[1]]);
    translate([0, antenne_lora_y, antenne_lora_bas - appui[1]]) cube([appui[0], antenne_lora[0], appui[1]]);
}

module couvercle() {   // dessiné en place (dessous à Z = H) ; à imprimer retourné, face extérieure sur le plateau
    difference() {
        union() {
            translate([0, 0, H]) linear_extrude(couvercle_epaisseur, convexity = 10) contour();
            // rebord de centrage, autour du compartiment de la carte
            translate([0, 0, H - rebord_hauteur]) linear_extrude(rebord_hauteur, convexity = 10) difference() {
                offset(delta = -jeu) square([cloison_x, W]);
                offset(delta = -jeu - rebord_epaisseur) square([cloison_x, W]);
            }
        }
        for (c = colonnes) translate([c[0], c[1], H - 1]) cylinder(d = vis_passage, h = couvercle_epaisseur + 2);
        // DHT20 : trou M3 et trou du câble
        translate([dht_x, dht_trou_y, H - 1]) cylinder(d = dht_trou, h = couvercle_epaisseur + 2);
        translate([dht_x, dht_cable_y, H - 1]) cylinder(d = dht_cable_trou, h = couvercle_epaisseur + 2);
        // ouverture au-dessus de la trappe (les bords du couvercle la retiennent)
        translate([trappe_x[0] + trappe_rebord, trappe_marge_bas, H - 1])
            cube([trappe_x[1] - trappe_x[0] - 2 * trappe_rebord, W - trappe_marge_bas - trappe_marge_haut, couvercle_epaisseur + 2]);
        // rainure du cran (sous le couvercle)
        translate([trappe_x[0], cran_y - (cran[1] + 0.2) / 2, H - 0.01]) cube([trappe_x[1] - trappe_x[0], cran[1] + 0.2, cran[0] + 0.1]);
    }
}

module trappe() {   // dessinée en place ; à imprimer à plat, stries vers le haut
    tl = W + paroi - trappe_jeu;   // du bas du compartiment au ras extérieur de la paroi haute
    difference() {
        union() {
            translate([trappe_x[0], trappe_jeu, trappe_z]) cube([trappe_x[1] - trappe_x[0], tl, trappe_epaisseur]);
            // bossette du cran
            translate([trappe_x[0] + trappe_rebord + 1, cran_y - cran[1] / 2, trappe_z + trappe_epaisseur - 0.01])
                cube([trappe_x[1] - trappe_x[0] - 2 * trappe_rebord - 2, cran[1], cran[0]]);
        }
        // stries pour le pouce, dans l'ouverture du couvercle, près du bas
        for (i = [0 : stries - 1]) translate([(trappe_x[0] + trappe_x[1]) / 2 - 12, trappe_marge_bas + 4 + i * stries_pas, trappe_z + trappe_epaisseur - stries_profondeur])
            cube([24, 1.2, 1]);
    }
}

// --- Composants (contrôle seulement) : carte, composants hauts, piles, interrupteur, DHT20, antennes
module piece_pcb(p, taille, couleur) {   // pavé centré sur une position PCB, posé sur la carte
    color(couleur) translate([bp(p)[0] - taille[0] / 2, bp(p)[1] - taille[1] / 2, zc]) cube(taille);
}
module composants() {
    color("green") translate([carte_position[0], carte_position[1], plot_hauteur]) cube(carte);
    // XIAO + Wio empilés sur barrettes femelles : plaquettes 21 × 17,8, prise USB-C vers la paroi gauche
    color("dimgray") translate([xiao[0] - 10.5, xiao[1] - 8.9, usb_hauteur - 3.5]) cube([21, 17.8, xiao_dessus - xiao_usb + 3.5]);
    color("black") translate([xiao[0] - 10.5, xiao[1] - 8.9, zc]) cube([21, 2.5, usb_hauteur - 3.5 - zc]);
    color("black") translate([xiao[0] - 10.5, xiao[1] + 6.4, zc]) cube([21, 2.5, usb_hauteur - 3.5 - zc]);
    color("silver") translate([xiao[0] - 12, xiao[1] - 4.5, usb_hauteur - 1.6]) cube([7.5, 9, 3.2]);
    piece_pcb([3.2, 22], [6.1, 12.4, 13], "white");               // J1 DHT20 (fiche JST-XH branchée)
    piece_pcb([11, 60.6], [10, 7.6, 10], "royalblue");            // J3 piles (bornier XY301V)
    piece_pcb([33.5, 60.6], [10, 7.6, 10], "royalblue");          // J5 flotteur
    piece_pcb([21.5, 61.6], [7.4, 6.1, 13], "white");             // J4 interrupteur (fiche JST-XH)
    piece_pcb([39, 52], [6.3, 6.3, 11], "navy");                  // C5 470 µF
    piece_pcb([16.5, 52], [5, 5, 11], "navy");                    // C3 10 µF
    piece_pcb([37, 4.5], [5, 5, 11], "navy");                     // C2 10 µF
    piece_pcb([23.5, 51.5], [4.6, 3.6, 8], "black");              // U1 MCP1700
    piece_pcb([8, 52], [8, 2.5, 3], "black");                     // D2 1N5819
    for (p = [[39.8, 41.5], [43.3, 41.5]]) piece_pcb(p, [2.5, 2.5, 9], "tan");   // R4, R5 debout
    for (x = [9.5, 13, 16.5, 20]) piece_pcb([x, 31], [2, 2, 8], "red");         // TP1–TP4
    // support de piles sur ses plots, 4 piles
    translate([piles_centre[0] - piles[0] / 2, piles_centre[1] - piles[1] / 2, piles_plot[1]]) {
        color("black") difference() { cube(piles); translate([1, 1, 2]) cube(piles - [2, 2, 0]); }
        for (i = [0 : 3]) color("orange") translate([1.5, 1 + 7.5 + i * 15, 9.5]) rotate([0, 90, 0]) cylinder(d = 14.5, h = 50.5);
        color("red") translate([-3, 6 - 1, 4]) cube([3, 2, 2]);
    }
    // interrupteur : corps derrière la paroi basse, levier dehors
    translate([interrupteur_x, 0, interrupteur_hauteur]) {
        color("red") translate([-6.45, 0, -3.95]) cube([12.9, 18, 7.9]);
        color("silver") rotate([90, 0, 0]) cylinder(d = 6, h = paroi + 7);
    }
    // DHT20 sur le couvercle : plaquette, capteur ajouré en haut, broches coudées vers le bas
    translate([dht_x - dht_module[0] / 2, dht_bas, H + couvercle_epaisseur]) {
        color("white") cube([dht_module[0], dht_module[1], 1.6]);
        color("lightgray") translate([0.5, dht_module[1] - 15, 1.6]) cube([12, 15, 5]);
        color("gold") for (i = [0 : 3]) translate([dht_module[0] / 2 - 3.81 + i * 2.54 - 0.3, -dht_broches, 0.5]) cube([0.6, dht_broches, 0.6]);
    }
    // antennes souples
    color("orange", 0.6) translate([antenne_wifi_x, W - 0.4, antenne_wifi_bas]) cube([antenne_wifi[0], 0.3, antenne_wifi[1]]);
    color("orange", 0.6) translate([0.1, antenne_lora_y, antenne_lora_bas]) cube([0.3, antenne_lora[0], antenne_lora[1]]);
}

if (piece == "boite") boite();
else if (piece == "couvercle") translate([0, 0, H + couvercle_epaisseur]) mirror([0, 0, 1]) couvercle();
else if (piece == "trappe") translate([0, 0, -trappe_z]) trappe();
else if (piece == "ensemble") { color("lightgray") boite(); composants(); color("gainsboro", 0.5) couvercle(); color("tan") trappe(); }
else if (piece == "eclate") { color("lightgray") boite(); composants(); translate([0, 0, 30]) color("gainsboro") couvercle();
                              translate([0, 40, 15]) color("tan") trappe(); }
else if (piece == "coupe") difference() {
    union() { color("lightgray") boite(); composants(); color("gainsboro") couvercle(); color("tan") trappe(); }
    // coupe par le milieu du support de piles (plan Y-Z)
    translate([piles_centre[0], -50, -50]) cube([200, 200, 200]);
}
