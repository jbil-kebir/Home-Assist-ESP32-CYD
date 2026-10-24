// =============================================================================
//  boitier_chaudiere.scad – Boîtier du Capteur chaudière Sdb (XIAO ESP32-S3 + Wio-SX1262, DHT20, TCS34725),
//  vissé sur le coffrage en bois, le TCS34725 dessous face au voyant de la chaudière
// -----------------------------------------------------------------------------
//  Pièces :
//    - BOÎTE (PETG) : fond + parois, 4 plots de la carte, fente de la prise USB-C et trou de la LED bleue (paroi
//      gauche), fixation du TCS34725 sous la paroi basse (2 trous M2, logements d’écrous, fenêtre des fils au ras du fond),
//      appuis des antennes souples (Wi-Fi paroi haute, LoRa paroi droite), 4 colonnes de vis du couvercle aux coins
//      (à l'extérieur), 4 oreilles de fixation ;
//    - COUVERCLE (PETG), face tournée vers la pièce : plaque vissée (4 vis M3), rebord de centrage, trou M3 du
//      DHT20 (vissé dehors) et trou de son câble.
//
//  Repère (boîtier posé à plat sur le coffrage, vu de face, couvercle vers soi) : X vers la droite, Y vers le haut,
//  Z vers soi ; origine = coin bas gauche de l'INTÉRIEUR, au ras du fond intérieur.
//  La carte (46 × 41) est vue composants vers soi, tournée de 180° par rapport au dessin EasyEDA :
//  prise USB-C du XIAO et LED bleue à GAUCHE, J1 (DHT20) à DROITE, J2 (TCS34725) en HAUT.
//  Les positions des composants sont celles du PCB (repère EasyEDA, mm) ; bp() les passe dans le boîtier.
//
//  Réglages : menu Fenêtre > Customizer. Fiche : Capteur/boitier/Capteur chaudiere Sdb/Boîtier Capteur chaudiere Sdb.md
// =============================================================================

/* [Pièce à afficher] */
// ensemble, eclate, coupe : contrôle ; boite, couvercle : pièces à exporter en STL
piece = "eclate"; // [ensemble, eclate, coupe, boite, couvercle]

/* [Boîte] */
// Intérieur : longueur (X), largeur (Y), hauteur (Z, du fond intérieur au dessous du couvercle)
interieur = [57, 55, 33];
paroi = 2;
fond = 2;
couvercle_epaisseur = 2;

/* [Circuit imprimé] */
carte = [46, 41, 1.6];
// Coin bas gauche de la carte dans le boîtier : 1 mm de la paroi gauche (USB-C), 4 mm de la paroi basse
carte_position = [1, 4];
// Trous M3 de la carte (symétriques : la rotation de 180° ne les change pas)
carte_trous = [[3.5, 3.5], [42.5, 3.5], [3.5, 37.5], [42.5, 37.5]];
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
// Fente de la prise USB-C dans la paroi gauche (Y × Z) : la fiche du câble y entre
usb_fente = [13, 8];

/* [LED bleue D1 (paroi gauche)] */
// Position PCB de la LED ; pattes pliées à 90°, la collerette bute contre l'intérieur de la paroi
led_pcb = [42.2, 31.5];
led_hauteur = 12;
led_trou = 5.3;

/* [TCS34725 (sous la paroi basse)] */
// Plaquette 20 × 20, face SANS composants contre la paroi, capteur vers le bas ; broches côté FOND,
// trous côté COUVERCLE. Centre X du module
tcs_x = 28.5;
// Distance face arrière du boîtier → centre du capteur (centre de la LED du voyant à 15 du coffrage :
// 4 mm de marge, rattrapés par des cales sous les oreilles)
tcs_centre = 11;
tcs_plaquette = [20, 20, 1.6];
tcs_entraxe = 15;
// Trous : 2 mm du bord côté couvercle ; axe des broches : 16,4 mm des trous (Mesures.md)
tcs_trous_bord = 2;
tcs_trous_broches = 16.4;
tcs_trou = 2.4;
// Fenêtre des fils sur la rangée de broches (X × Z), cachée par le module ; elle peut entailler le fond
// intérieur de tcs_entaille au plus (la rangée de broches est presque au niveau du fond)
tcs_fenetre = [19, 3];
tcs_entaille = 0.5;
// Logement de l'écrou M2 (4 mm sur plats) dans un bossage intérieur : bossage (X, Y, Z), écrou (sur plats, épaisseur)
tcs_bossage = [8, 3, 8];
tcs_ecrou = [4.3, 1.8];

/* [DHT20 (sur le couvercle, dehors)] */
// Plaquette 28,5 × 13 posée verticalement, capteur vers le HAUT, broches coudées vers le BAS (dépassent de 7) ;
// trou M3 à 6,5 mm du bord côté broches ; trou du câble sous la pointe des broches (rebouché au montage)
dht_x = 47;
dht_module = [13, 28.5];
dht_trou_y = 34.5;
dht_trou_broches = 6.5;
dht_broches = 7;
dht_trou = 3.4;
dht_cable_trou = 6;

/* [Antennes souples (à l'intérieur, contre les parois)] */
// Wi-Fi du XIAO 40 × 20 (paroi haute) : X de début, bas ; LoRa du Wio 40 × 10 (paroi droite) : Y de début, bas
antenne_wifi = [40, 20];
antenne_wifi_x = 1;
antenne_wifi_bas = 12;
antenne_lora = [40, 10];
antenne_lora_y = 7;
antenne_lora_bas = 20;
appui = [1, 1];

/* [Vis du couvercle] */
// Colonnes aux 4 coins, À L'EXTÉRIEUR de l'intérieur
colonne_diametre = 7;
colonne_decalage = 2.5;
vis_passage = 3.4;
vis_colonne_profondeur = 10;

/* [Couvercle] */
rebord_hauteur = 1.5;
rebord_epaisseur = 1.2;
jeu = 0.3;

/* [Fixation sur le coffrage] */
// 4 oreilles (2 en haut, 2 en bas, hors de la zone du TCS) : position X des centres, saillie, largeur, épaisseur ;
// vis à bois Ø 4 fraisées
oreille_x = [8, 49];
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
xiao = bp(xiao_pcb);
usb_y = xiao[1];
usb_hauteur = zc + xiao_usb;
led_y = bp(led_pcb)[1];
// TCS34725 : centre à tcs_centre de la face arrière (Z = -fond)
tcs_z0 = -fond + tcs_centre - tcs_plaquette[1] / 2;   // bord côté fond
tcs_trous_z = tcs_z0 + tcs_plaquette[1] - tcs_trous_bord;
tcs_broches_z = tcs_trous_z - tcs_trous_broches;
tcs_fenetre_z = max(tcs_broches_z, tcs_fenetre[1] / 2 - tcs_entaille);
// DHT20 : bord côté broches, trou du câble
dht_bas = dht_trou_y - dht_trou_broches;
dht_cable_y = dht_bas - dht_broches;

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

module boite() {
    difference() {
        union() {
            translate([0, 0, -fond]) linear_extrude(fond + H, convexity = 10) contour();
            oreilles();
            // bossages des écrous M2 du TCS34725 (intérieur de la paroi basse)
            for (s = [-1, 1]) translate([tcs_x + s * tcs_entraxe / 2 - tcs_bossage[0] / 2, 0, tcs_trous_z - tcs_bossage[2] / 2])
                cube(tcs_bossage);
        }
        // intérieur (sans les bossages)
        difference() {
            cube([L, W, H + 1]);
            for (s = [-1, 1]) translate([tcs_x + s * tcs_entraxe / 2 - tcs_bossage[0] / 2, -1, tcs_trous_z - tcs_bossage[2] / 2])
                cube(tcs_bossage + [0, 1, 0]);
        }
        // fente USB-C (paroi gauche)
        translate([-paroi - 1, usb_y, usb_hauteur]) rotate([0, 90, 0])
            linear_extrude(paroi + 2) offset(r = 1.5) square([usb_fente[1] - 3, usb_fente[0] - 3], center = true);
        // trou de la LED bleue (paroi gauche)
        translate([-paroi - 1, led_y, led_hauteur]) rotate([0, 90, 0]) cylinder(d = led_trou, h = paroi + 2);
        // TCS34725 : 2 trous M2, logements hexagonaux des écrous (ouverts vers l'intérieur), fenêtre des fils
        for (s = [-1, 1]) translate([tcs_x + s * tcs_entraxe / 2, 0, tcs_trous_z]) rotate([-90, 0, 0]) {
            translate([0, 0, -paroi - 1]) cylinder(d = tcs_trou, h = paroi + tcs_bossage[1] + 2);
            translate([0, 0, tcs_bossage[1] - tcs_ecrou[1]]) rotate(30)
                cylinder(d = (tcs_ecrou[0] + 0.3) / cos(30), h = tcs_ecrou[1] + 1, $fn = 6);
        }
        translate([tcs_x - tcs_fenetre[0] / 2, -paroi - 1, tcs_fenetre_z - tcs_fenetre[1] / 2]) cube([tcs_fenetre[0], paroi + 2, tcs_fenetre[1]]);
        // avant-trous des vis du couvercle
        for (c = colonnes) translate([c[0], c[1], H - vis_colonne_profondeur]) cylinder(d = vis_pilote, h = vis_colonne_profondeur + 1);
    }
    // plots de la carte
    for (t = carte_trous) translate([carte_position[0] + t[0], carte_position[1] + t[1], 0]) difference() {
        cylinder(d = plot_diametre, h = plot_hauteur);
        translate([0, 0, plot_hauteur - vis_profondeur]) cylinder(d = vis_pilote, h = vis_profondeur + 1);
    }
    // appuis sous le bord bas des antennes
    translate([antenne_wifi_x, W - appui[0], antenne_wifi_bas - appui[1]]) cube([antenne_wifi[0], appui[0], appui[1]]);
    translate([L - appui[0], antenne_lora_y, antenne_lora_bas - appui[1]]) cube([appui[0], antenne_lora[0], appui[1]]);
}

module couvercle() {   // dessiné en place (dessous à Z = H) ; à imprimer retourné, face extérieure sur le plateau
    difference() {
        union() {
            translate([0, 0, H]) linear_extrude(couvercle_epaisseur, convexity = 10) contour();
            // rebord de centrage, à l'intérieur des parois
            translate([0, 0, H - rebord_hauteur]) linear_extrude(rebord_hauteur, convexity = 10) difference() {
                offset(delta = -jeu) square([L, W]);
                offset(delta = -jeu - rebord_epaisseur) square([L, W]);
            }
        }
        for (c = colonnes) translate([c[0], c[1], H - 1]) cylinder(d = vis_passage, h = couvercle_epaisseur + 2);
        // DHT20 : trou M3 et trou du câble
        translate([dht_x, dht_trou_y, H - 1]) cylinder(d = dht_trou, h = couvercle_epaisseur + 2);
        translate([dht_x, dht_cable_y, H - 1]) cylinder(d = dht_cable_trou, h = couvercle_epaisseur + 2);
    }
}

// --- Composants (contrôle seulement) : carte, composants hauts, LED, TCS34725, DHT20, antennes
module piece_pcb(p, taille, couleur) {   // pavé centré sur une position PCB, posé sur la carte
    color(couleur) translate([bp(p)[0] - taille[0] / 2, bp(p)[1] - taille[1] / 2, zc]) cube(taille);
}
module composants() {
    color("green") translate([carte_position[0], carte_position[1], plot_hauteur]) cube(carte);
    // XIAO + Wio empilés sur barrettes femelles : plaquettes 21 × 17,8, prise USB-C 9 × 3,2 vers la paroi gauche
    color("dimgray") translate([xiao[0] - 10.5, usb_y - 8.9, usb_hauteur - 3.5]) cube([21, 17.8, xiao_dessus - xiao_usb + 3.5]);
    color("black") translate([xiao[0] - 10.5, usb_y - 8.9, zc]) cube([21, 2.5, usb_hauteur - 3.5 - zc]);
    color("black") translate([xiao[0] - 10.5, usb_y + 6.4, zc]) cube([21, 2.5, usb_hauteur - 3.5 - zc]);
    color("silver") translate([xiao[0] - 12, usb_y - 4.5, usb_hauteur - 1.6]) cube([7.5, 9, 3.2]);
    piece_pcb([3.2, 22], [6.1, 12.4, 13], "white");               // J1 DHT20 (fiche JST-XH branchée)
    piece_pcb([13.5, 3.2], [14.9, 6.1, 13], "white");             // J2 TCS34725
    piece_pcb([37, 4.5], [5, 5, 11], "navy");                     // C2 10 µF
    piece_pcb([31, 4.5], [5, 3, 6], "orange");                    // C1 100 nF
    piece_pcb([31, 34], [6.5, 2.5, 3], "tan");                    // R1
    for (x = [9.5, 13, 16.5, 20]) piece_pcb([x, 31], [2, 2, 8], "red");   // TP1–TP4
    // LED bleue : pattes montées puis pliées vers la paroi, collerette contre l'intérieur de la paroi
    lx = bp(led_pcb)[0];
    color("silver") {
        translate([lx - 0.25, led_y - 0.25, zc]) cube([0.5, 0.5, led_hauteur - zc]);
        translate([0, led_y - 0.25, led_hauteur - 0.25]) cube([lx, 0.5, 0.5]);
    }
    color("blue", 0.8) translate([0, led_y, led_hauteur]) rotate([0, -90, 0]) {
        cylinder(d = 5.9, h = 1); cylinder(d = 5, h = 6.1); translate([0, 0, 6.1]) sphere(d = 5);
    }
    // TCS34725 sous la paroi basse : plaquette, capteur vers le bas
    translate([tcs_x - tcs_plaquette[0] / 2, -paroi - tcs_plaquette[2], tcs_z0]) {
        color("purple") cube([tcs_plaquette[0], tcs_plaquette[2], tcs_plaquette[1]]);
        color("white") translate([tcs_plaquette[0] / 2 - 2, -2, tcs_plaquette[1] / 2 - 2]) cube([4, 2, 4]);
    }
    // DHT20 sur le couvercle : plaquette, capteur ajouré en haut, broches coudées vers le bas
    translate([dht_x - dht_module[0] / 2, dht_bas, H + couvercle_epaisseur]) {
        color("white") cube([dht_module[0], dht_module[1], 1.6]);
        color("lightgray") translate([0.5, dht_module[1] - 15, 1.6]) cube([12, 15, 5]);
        color("gold") for (i = [0 : 3]) translate([dht_module[0] / 2 - 3.81 + i * 2.54 - 0.3, -dht_broches, 0.5]) cube([0.6, dht_broches, 0.6]);
    }
    // antennes souples
    color("orange", 0.6) translate([antenne_wifi_x, W - 0.4, antenne_wifi_bas]) cube([antenne_wifi[0], 0.3, antenne_wifi[1]]);
    color("orange", 0.6) translate([L - 0.4, antenne_lora_y, antenne_lora_bas]) cube([0.3, antenne_lora[0], antenne_lora[1]]);
}

if (piece == "boite") boite();
else if (piece == "couvercle") translate([0, 0, H + couvercle_epaisseur]) mirror([0, 0, 1]) couvercle();
else if (piece == "ensemble") { color("lightgray") boite(); composants(); color("gainsboro", 0.5) couvercle(); }
else if (piece == "eclate") { color("lightgray") boite(); composants(); translate([0, 0, 30]) color("gainsboro") couvercle(); }
else if (piece == "coupe") difference() {
    union() { color("lightgray") boite(); composants(); color("gainsboro") couvercle(); }
    // coupe par l'axe d'un trou du TCS34725 (plan Y-Z)
    translate([tcs_x + tcs_entraxe / 2, -50, -50]) cube([200, 200, 200]);
}
