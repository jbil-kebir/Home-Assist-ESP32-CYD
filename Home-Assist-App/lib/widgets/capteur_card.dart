//
// Fichier capteur_card.dart
//
import 'package:dom_new/providers/app_providers.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

class CapteurCard extends ConsumerWidget {
  final bool bDeuxMesures;
  final String nomCapteurNormalise; // pour les providers
  final String nomCapteurAffiche;   // pour l'écran
  final bool bFlotteur; // true s'il faut afficher un flotteur
  final bool bBatterie; // true s'il faut afficher un niveau batterie
  final bool bDEL;
  //final bool bCouleurs;

  const CapteurCard({
    super.key,
    required this.nomCapteurNormalise,
    required this.nomCapteurAffiche,
    required this.bDeuxMesures,
    required this.bFlotteur,
    required this.bBatterie,
    required this.bDEL,
    //required this.bCouleurs,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    // On surveille les providers associés à ce capteur
    final temp = ref.watch(temperatureProvider(nomCapteurNormalise));
    final hum = ref.watch(humiditeProvider(nomCapteurNormalise));
    final dh = ref.watch(derniereMesureProvider(nomCapteurNormalise));
    final tension = ref.watch(batterieTensionProvider(nomCapteurNormalise));
    final etat = ref.watch(batterieEtatProvider(nomCapteurNormalise));
  
    // État du flotteur (seulement si bFlotteur == true)
    final flotteurHaut = bFlotteur ? ref.watch(flotteurProvider('FLOTTEURREMISE')) : null;
    // État DEL surveillée par le capteur RGB
    //final bAfficherLedRGB = bDEL ? ref.watch(delOnOffProvider('THSDB')) : null;

    // Providers couleurs et état LED (seulement si bCouleurs ou bDEL)
    //final couleurR = bCouleurs ? ref.watch(couleurRProvider(nomCapteurNormalise)) : null;
    //final couleurG = bCouleurs ? ref.watch(couleurGProvider(nomCapteurNormalise)) : null;
    //final couleurB = bCouleurs ? ref.watch(couleurBProvider(nomCapteurNormalise)) : null;
    //final couleurLux = bCouleurs ? ref.watch(couleurLuxProvider(nomCapteurNormalise)) : null;
    //final couleurLuxBrut = bCouleurs ? ref.watch(couleurLuxBrutProvider(nomCapteurNormalise)) : null;
    final etatLed = bDEL ? ref.watch(delOnOffProvider("COULSDB")) : null;


    return Card(
      elevation: 2,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
      child: Padding(
        padding: const EdgeInsets.all(10),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Ligne nom + batterie
            Row(
              children: [
                Expanded(
                  child: Text( // ================================================================= Nom du capteur
                    nomCapteurAffiche,
                    style: Theme.of(context).textTheme.titleMedium?.copyWith(
                          fontWeight: FontWeight.bold,
                        ),
                    overflow: TextOverflow.ellipsis,
                  ),
                ),
                if (bBatterie) ...[ // ================================================================= Icône de la batterie
                const Icon(
                  Icons.battery_4_bar,
                  color: Colors.blueAccent,
                  size: 18,
                ),
                const SizedBox(width: 4),
                if (tension != null)
                  Text( // ================================================================= Tension de la batterie
                    tension < 0 ? '-- V' : '${tension.toStringAsFixed(1)} V',
                    style: TextStyle(
                      fontSize: 14,
                      fontWeight: FontWeight.w600,
                      color: etat == "CHARGEE"
                          ? Colors.green
                          : etat == "DECHARGEE"
                              ? Colors.red
                              : null,
                    ),
                  )
                else
                  Text( // ================================================================= Unité (V)
                    '-- V',
                    style: TextStyle(
                      fontSize: 14,
                      fontWeight: FontWeight.w600,
                      color: Colors.grey,
                    ),
                  ),          
                ],

              ],
            ),
            const SizedBox(height: 10),

            // Température + Humidité
            Row(
              mainAxisAlignment: MainAxisAlignment.start,
              children: [
                const Icon( // ================================================================= Icône de la température
                  Icons.device_thermostat,
                  color: Colors.blueAccent,
                  size: 18,
                ),
                const SizedBox(width: 4),
                if (temp != null)
                  Text( // ================================================================= Valeur de la température et unité
                    temp == -127 ? '-- °C' : '${temp.toStringAsFixed(1)} °C',
                    style: TextStyle(
                      fontSize: 14,
                      fontWeight: FontWeight.w600,
                      color: temp > 25 ? Colors.red : temp < 18 ? Colors.blue : null,
                    ),
                  )
                else
                  Text(
                    '-- °C',
                    style: TextStyle(
                      fontSize: 14,
                      fontWeight: FontWeight.w600,
                      color: Colors.grey,
                    ),
                  ),

                if (bDeuxMesures) ...[ // ================================================================= Cas où il y a une deuxième mesure (Humidité)
                  const SizedBox(width: 14),
                  const Icon( // ================================================================= Icône humiité
                    Icons.water_drop,
                    color: Colors.blueAccent,
                    size: 18,
                  ),
                  const SizedBox(width: 4),
                  if (hum != null)
                    Text( // ================================================================= Valeur de l'humidité
                      '${hum.toStringAsFixed(1)} %',
                      style: TextStyle(
                        fontSize: 14,
                        fontWeight: FontWeight.w600,
                        color: hum > 60 ? Colors.red : hum < 40 ? Colors.blue : null,
                      ),
                    )
                  else
                    Text(
                      '-- %',
                      style: TextStyle(
                        fontSize: 14,
                        fontWeight: FontWeight.w600,
                        color: Colors.grey,
                      ),
                    ),
                ],
              ],
            ),

            // Affichage des couleurs brutes 16 bits + état LED
            if (etatLed != null) ...[
              //const SizedBox(height: 12),
              Row(
                //mainAxisAlignment: MainAxisAlignment.spaceBetween,
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  // Valeurs brutes 16 bits
                  /*Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        'R: ${couleurR.toInt()}   G: ${couleurG.toInt()}',
                        style: const TextStyle(fontSize: 12, fontWeight: FontWeight.w500),
                      ),
                      Text(
                        'B: ${couleurB.toInt()}   C: ${couleurLuxBrut.toInt()}',
                        style: const TextStyle(fontSize: 12, fontWeight: FontWeight.w500),
                      ),
                    ],
                  ),*/

                  // État LED (si bDEL activé)
                  if (bDEL && etatLed != null) ...[
                    Icon(
                      etatLed ? Icons.power : Icons.power_off,
                      color: etatLed ? Colors.green : Colors.red,
                      size: 20,
                    ),
                    const SizedBox(width: 6),
                    Text(
                      etatLed ? 'LED ON' : 'LED OFF',
                      style: TextStyle(
                        fontSize: 13,
                        fontWeight: FontWeight.w600,
                        color: etatLed ? Colors.green : Colors.red,
                      ),
                    ),
                  ],
                ],
              ),
            ],

            // Date + flotteur
            Row(
              mainAxisAlignment: MainAxisAlignment.start,
              children: [
                if (dh != null)
                  Padding(
                    padding: const EdgeInsets.only(top: 4),
                    child: Text(
                      dh,
                      style: const TextStyle(fontSize: 11, color: Colors.grey),
                    ),
                  ),
                if (temp == null && hum == null)
                  const Text(
                    'En attente...',
                    style: TextStyle(fontSize: 14, color: Colors.grey),
                  ),

                // Affichage du flotteur (seulement si bFlotteur == true)
                if (bFlotteur) ...[
                  const SizedBox(width: 2),
                  Icon(
                    // Non-const : pas de const ici
                    flotteurHaut == true
                        ? Icons.arrow_downward
                        : flotteurHaut == false
                            ? Icons.arrow_upward
                            : Icons.remove_circle_outline, // cas null
                    color: flotteurHaut == true
                        ? Colors.green[700]
                        : flotteurHaut == false
                            ? Colors.red[700]
                            : Colors.grey,
                    size: 18,
                  ),
                  //const SizedBox(width: 4),
                  Text(
                    flotteurHaut == true
                        ? 'Bas'
                        : flotteurHaut == false
                            ? 'Haut'
                            : '--',
                    style: TextStyle(
                      fontSize: 11,
                      fontWeight: FontWeight.w600,
                      color: flotteurHaut == true
                          ? Colors.green[800]
                          : flotteurHaut == false
                              ? Colors.red[800]
                              : Colors.grey,
                    ),
                  ),
                ],
              ],
            ),
          ],
        ),
      ),
    );
  }
}