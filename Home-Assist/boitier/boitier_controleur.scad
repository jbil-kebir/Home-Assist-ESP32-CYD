// =============================================================================
//  boitier_controleur.scad – Boîtier du contrôleur HA (version sans CYD), posé sur une étagère
// -----------------------------------------------------------------------------
//  Pièces :
//    - BOÎTE (PETG) : fond + parois, 4 plots de la carte, 2 oreilles de fixation (vis à bois, au besoin),
//      façade AVANT : interrupteur MTS-102 + LED 5 mm ; ARRIÈRE : jack 12 V (J1) + prise SMA du CC1101 ;
//      côté DROIT : fentes USB-C du DevKit et du XIAO, aération, antenne LoRa souple (en haut, à l'intérieur) ;
//      côté GAUCHE : DHT20 vissé à l'extérieur (vis M3 + écrou), trou du câble, aération, antenne Wi-Fi souple
//      du XIAO (à l'intérieur) ; 4 colonnes de vis du couvercle aux coins (à l'extérieur) ;
//    - COUVERCLE (PETG) : plaque vissée (4 vis M3) avec rebord de centrage.
//
//  Repère (vu de dessus, avant en bas) : X vers la droite, Y vers l'arrière, Z vers le haut ;
//  origine = coin avant gauche de l'INTÉRIEUR, au ras du fond intérieur.
//  Carte 57 × 84 (dessin EasyEDA, origine au coin avant gauche, Y vers l'arrière) posée telle quelle,
//  à 2 mm de la paroi gauche (antenne Wi-Fi), 1 mm de la paroi droite (prises USB-C) et 1 mm de l'avant.
//
//  Réglages : menu Fenêtre > Customizer. Fiche : Home-Assist/boitier/Boîtier contrôleur HA.md
//  ÉTAT 2026-10-24 : PREMIER JET. Hauteurs du DevKit, de l'empilement XIAO + Wio, de la SMA du CC1101 et du jack
//  ESTIMÉES → à vérifier sur les pièces avant d'imprimer (voir la fiche § 5).
// =============================================================================

/* [Pièce à afficher] */
// ensemble, eclate, coupe : contrôle ; boite, couvercle : pièces à exporter en STL
piece = "eclate"; // [ensemble, eclate, coupe, boite, couvercle]

/* [Boîte] */
// Intérieur : largeur (X), profondeur (Y), hauteur (Z, du fond intérieur au dessous du couvercle)
interieur = [60, 86, 36];
paroi = 2;
fond = 2;
couvercle_epaisseur = 2;

/* [Circuit imprimé] */
carte = [57, 84, 1.6];
carte_position = [2, 1];
// Trous M3 de la carte (coordonnées sur la carte, origine au coin avant gauche)
carte_trous = [[3.5, 3.5], [53.5, 3.5], [53.5, 80.5], [3.5, 47]];
plot_diametre = 7;
plot_hauteur = 4;
vis_pilote = 2.5;
vis_profondeur = 6;

/* [Hauteurs au-dessus de la carte (ESTIMÉES, à mesurer)] */
// DevKit sur barrettes femelles 1 × 19 : milieu de la prise USB-C ; dessus du module WROOM
devkit_usb = 14.6;
devkit_dessus = 15.7;
// XIAO S3 + Wio-SX1262 sur barrettes femelles 1 × 7 : milieu de la prise USB-C ; dessus de l'empilement
xiao_usb = 16.5;
xiao_dessus = 25;
// CC1101 sur barrette femelle 2 × 4 : axe de la prise SMA
sma_hauteur = 13;
// Jack DC-005-20 : axe de la fiche
jack_hauteur = 6.5;

/* [Prises sur la carte (coordonnées sur la carte)] */
// Y des prises USB-C (paroi droite) : DevKit, XIAO
devkit_usb_y = 24.075;
xiao_usb_y = 49.095;
usb_fente = [13, 8];
// X de l'axe de la SMA du CC1101 et du jack (paroi arrière)
sma_x = 8.3;
sma_trou = 8;
jack_x = 21.6;
jack_trou = 11;

/* [Façade avant : interrupteur et LED] */
// X sur la façade (intérieur), hauteur au-dessus du fond intérieur : au-dessus du DevKit (le corps de
// l'interrupteur s'enfonce d'environ 18 mm derrière la façade)
interrupteur_x = 24;
led_x = 36;
facade_hauteur = 28;
interrupteur_trou = 6.2;
led_trou = 5.1;
led_collier = [8.5, 3];

/* [Côté gauche : DHT20 et aération] */
// Module 28,5 × 13, capteur vers l'avant ; trou de fixation (vis M3 + écrou à l'intérieur) : Y, hauteur
dht_module = [28.5, 13];
dht_trou_y = 28;
dht_hauteur = 28;
dht_trou = 3.4;
// Trou du câble (fils soudés ou Dupont sur les broches, JST-XH dans le boîtier), derrière les broches
dht_cable_y = 42;
dht_cable_trou = 6;
// Fentes d'aération (paroi gauche et paroi droite) : Y des centres, taille (Y × Z), hauteur du bas
aeration_gauche_y = [52, 58, 64, 70, 76];
aeration_droite_y = [62, 68, 74, 80];
aeration_fente = [3, 8];
aeration_bas = 4;

/* [Antennes souples (à l'intérieur, contre les parois)] */
// Wi-Fi du XIAO 40 × 20 (paroi gauche), LoRa du Wio 40 × 10 (paroi droite, en haut) : Y de début, bas de l'antenne
antenne_wifi = [40, 20];
antenne_wifi_y = 45;
antenne_wifi_bas = 14;
antenne_lora = [40, 10];
antenne_lora_y = 1;
antenne_lora_bas = 25;
appui = [1, 1];

/* [Vis du couvercle] */
colonne_diametre = 7;
colonne_decalage = 2.5;
vis_passage = 3.4;
vis_colonne_profondeur = 10;

/* [Couvercle] */
rebord_hauteur = 1.5;
rebord_epaisseur = 1.2;
jeu = 0.3;

/* [Fixation] */
// 2 oreilles (gauche et droite, au milieu, au niveau du fond) : saillie, largeur, épaisseur ; vis à bois Ø 4 fraisée
oreille = [13, 14, 4];
oreille_trou = 4.5;
oreille_fraisure = 8.5;

$fn = 48;
L = interieur[0]; W = interieur[1]; H = interieur[2];
z_carte = plot_hauteur + carte[2];             // dessus de la carte
function bx(x) = carte_position[0] + x;        // carte → boîte
function by(y) = carte_position[1] + y;
colonnes = [[-colonne_decalage, -colonne_decalage], [L + colonne_decalage, -colonne_decalage],
            [-colonne_decalage, W + colonne_decalage], [L + colonne_decalage, W + colonne_decalage]];

// --- Contour extérieur, en 2D : rectangle (parois de 2 mm) + colonnes en saillie aux coins, raccords arrondis
module contour() {
    offset(r = -1) offset(r = 1) union() {
        translate([-paroi, -paroi]) square([L + 2 * paroi, W + 2 * paroi]);
        for (c = colonnes) translate(c) circle(d = colonne_diametre);
    }
}

module oreilles() {
    xt = -(oreille[0] - oreille[1] / 2);   // centre du trou, depuis la face extérieure de la paroi
    for (s = [0, 1]) translate([s ? L + paroi : -paroi, W / 2, -fond]) mirror([s, 0, 0])
        difference() {
            hull() {
                translate([-0.01, -oreille[1] / 2, 0]) cube([0.01, oreille[1], oreille[2]]);
                translate([xt, 0, 0]) cylinder(d = oreille[1], h = oreille[2]);
            }
            translate([xt, 0, -1]) {
                cylinder(d = oreille_trou, h = oreille[2] + 2);
                translate([0, 0, 1 + oreille[2] - oreille_fraisure / 2 + oreille_trou / 2])
                    cylinder(d1 = oreille_trou, d2 = oreille_fraisure + 0.2, h = (oreille_fraisure - oreille_trou) / 2 + 0.1);
            }
        }
}

// fente oblongue (Y × Z) dans la paroi gauche (cote = 0) ou droite (cote = 1)
module fente_x(cote, y, z, t) translate([cote ? L - 1 : -paroi - 1, y, z]) rotate([0, 90, 0])
    linear_extrude(paroi + 2) offset(r = min(t[0], t[1]) / 2 - 0.01) square([t[1] - min(t[0], t[1]) + 0.02, t[0] - min(t[0], t[1]) + 0.02], center = true);

module boite() {
    difference() {
        union() {
            translate([0, 0, -fond]) linear_extrude(fond + H) contour();
            oreilles();
        }
        cube([L, W, H + 1]);
        // paroi droite : fentes USB-C du DevKit et du XIAO, aération
        fente_x(1, by(devkit_usb_y), z_carte + devkit_usb, usb_fente);
        fente_x(1, by(xiao_usb_y), z_carte + xiao_usb, usb_fente);
        for (y = aeration_droite_y) fente_x(1, y, aeration_bas + aeration_fente[1] / 2, aeration_fente);
        // paroi gauche : DHT20 (vis M3), câble, aération
        translate([-paroi - 1, dht_trou_y, dht_hauteur]) rotate([0, 90, 0]) cylinder(d = dht_trou, h = paroi + 2);
        translate([-paroi - 1, dht_cable_y, dht_hauteur]) rotate([0, 90, 0]) cylinder(d = dht_cable_trou, h = paroi + 2);
        for (y = aeration_gauche_y) fente_x(0, y, aeration_bas + aeration_fente[1] / 2, aeration_fente);
        // façade avant : interrupteur, LED
        translate([interrupteur_x, -paroi - 1, facade_hauteur]) rotate([-90, 0, 0]) cylinder(d = interrupteur_trou, h = paroi + 2);
        translate([led_x, -paroi - 1, facade_hauteur]) rotate([-90, 0, 0]) cylinder(d = led_trou, h = paroi + led_collier[1] + 2);
        // paroi arrière : SMA du CC1101, jack 12 V
        translate([bx(sma_x), W - 1, z_carte + sma_hauteur]) rotate([-90, 0, 0]) cylinder(d = sma_trou, h = paroi + 2);
        translate([bx(jack_x), W - 1, z_carte + jack_hauteur]) rotate([-90, 0, 0]) cylinder(d = jack_trou, h = paroi + 2);
        // avant-trous des vis du couvercle
        for (c = colonnes) translate([c[0], c[1], H - vis_colonne_profondeur]) cylinder(d = vis_pilote, h = vis_colonne_profondeur + 1);
    }
    // plots de la carte
    for (t = carte_trous) translate([bx(t[0]), by(t[1]), 0]) difference() {
        cylinder(d = plot_diametre, h = plot_hauteur);
        translate([0, 0, plot_hauteur - vis_profondeur]) cylinder(d = vis_pilote, h = vis_profondeur + 1);
    }
    // collier de la LED derrière la façade (la collerette Ø 5,8 bute contre lui)
    translate([led_x, 0, facade_hauteur]) rotate([-90, 0, 0]) difference() {
        cylinder(d = led_collier[0], h = led_collier[1]);
        translate([0, 0, -1]) cylinder(d = led_trou, h = led_collier[1] + 2);
    }
    // appuis sous le bord bas des antennes
    translate([0, antenne_wifi_y, antenne_wifi_bas - appui[1]]) cube([appui[0], antenne_wifi[0], appui[1]]);
    translate([L - appui[0], antenne_lora_y, antenne_lora_bas - appui[1]]) cube([appui[0], antenne_lora[0], appui[1]]);
}

module couvercle() {   // dessiné en place (dessous à Z = H) ; à imprimer retourné, face extérieure sur le plateau
    difference() {
        union() {
            translate([0, 0, H]) linear_extrude(couvercle_epaisseur) contour();
            translate([0, 0, H - rebord_hauteur]) linear_extrude(rebord_hauteur) difference() {
                offset(delta = -jeu) square([L, W]);
                offset(delta = -jeu - rebord_epaisseur) square([L, W]);
            }
        }
        for (c = colonnes) translate([c[0], c[1], H - 1]) cylinder(d = vis_passage, h = couvercle_epaisseur + 2);
    }
}

// --- Composants (contrôle seulement)
module composants() {
    color("green") translate([bx(0), by(0), plot_hauteur]) cube(carte);
    // DevKit 55 × 28, prise USB-C au bord droit (+1,7)
    color("dimgray") translate([bx(1.9), by(devkit_usb_y - 14), z_carte + devkit_dessus - 4.5]) cube([55, 28, 4.5]);
    color("silver") translate([bx(57) - 6, by(devkit_usb_y) - 4.5, z_carte + devkit_usb - 1.6]) cube([7.7, 9, 3.2]);
    // XIAO S3 + Wio (21 × 17,8), USB-C au bord droit (+1,5)
    color("dimgray") translate([bx(46.45 - 10.5), by(xiao_usb_y) - 8.9, z_carte + xiao_usb - 3.5]) cube([21, 17.8, xiao_dessus - xiao_usb + 3.5]);
    color("silver") translate([bx(57) - 6, by(xiao_usb_y) - 4.5, z_carte + xiao_usb - 1.6]) cube([7.5, 9, 3.2]);
    // CC1101 (15 × 28) au ras du bord arrière, SMA dans l'axe
    color("royalblue") translate([bx(0.8), by(56), z_carte + sma_hauteur - 1.5]) cube([15, 28, 1.6]);
    color("gold") translate([bx(sma_x), by(84), z_carte + sma_hauteur]) rotate([-90, 0, 0]) cylinder(d = 6.35, h = 10);
    // jack, MP1584EN
    color("black") translate([bx(jack_x) - 4.5, by(70), z_carte]) cube([9, 15, 11]);
    color("blue") translate([bx(38.7) - 11, by(74.8) - 8.5, z_carte + 9]) cube([22, 17, 4]);
    // interrupteur (corps 12,9 × 7,9, 18 mm derrière la façade avec les cosses), LED
    color("red") translate([interrupteur_x - 6.45, 0, facade_hauteur - 3.95]) cube([12.9, 18, 7.9]);
    color("lime") translate([led_x, -paroi - 2.5, facade_hauteur]) rotate([-90, 0, 0]) cylinder(d = 5, h = 8.6);
    // DHT20 à l'extérieur de la paroi gauche (capteur vers l'avant)
    color("white") translate([-paroi - 1.6, dht_trou_y + 6.5 - dht_module[0], dht_hauteur - dht_module[1] / 2]) cube([1.6, dht_module[0], dht_module[1]]);
    // antennes souples
    color("orange", 0.6) translate([0.1, antenne_wifi_y, antenne_wifi_bas]) cube([0.3, antenne_wifi[0], antenne_wifi[1]]);
    color("orange", 0.6) translate([L - 0.4, antenne_lora_y, antenne_lora_bas]) cube([0.3, antenne_lora[0], antenne_lora[1]]);
}

if (piece == "boite") boite();
else if (piece == "couvercle") translate([0, 0, H + couvercle_epaisseur]) mirror([0, 0, 1]) couvercle();
else if (piece == "ensemble") { color("lightgray") boite(); composants(); color("gainsboro", 0.5) couvercle(); }
else if (piece == "eclate") { color("lightgray") boite(); composants(); translate([0, 0, 30]) color("gainsboro") couvercle(); }
else if (piece == "coupe") difference() {
    union() { color("lightgray") boite(); composants(); color("gainsboro") couvercle(); }
    translate([-50, -50, -50]) cube([200, by(devkit_usb_y) + 50, 100]);
}
