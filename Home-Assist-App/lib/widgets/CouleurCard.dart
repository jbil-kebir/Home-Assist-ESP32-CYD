// Fichier CouleurCard.dart

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:dom_new/providers/app_providers.dart';

class CouleurCard extends ConsumerWidget {
  final String nomCapteurNormalise; // ex: "THSDB"
  final String nomCapteurAffiche;   // ex: "ThSdb - Couleur"

  const CouleurCard({
    super.key,
    required this.nomCapteurNormalise,
    required this.nomCapteurAffiche,
  });

@override
Widget build(BuildContext context, WidgetRef ref) {
  // Providers couleurs (16 bits bruts)
  final couleurR     = ref.watch(couleurRProvider(nomCapteurNormalise));
  final couleurG     = ref.watch(couleurGProvider(nomCapteurNormalise));
  final couleurB     = ref.watch(couleurBProvider(nomCapteurNormalise));
  final couleurClear = ref.watch(couleurLuxBrutProvider(nomCapteurNormalise));

  // État LED
  final etatLed      = ref.watch(delOnOffProvider(nomCapteurNormalise));

  // Dernière mesure (date/heure)
  final dh = ref.watch(derniereMesureProvider(nomCapteurNormalise));

  return Card(
    elevation: 2,
    shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
    child: Padding(
      padding: const EdgeInsets.all(12.0),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // Titre
          Text(
            nomCapteurAffiche,
            style: Theme.of(context).textTheme.titleMedium?.copyWith(
                  fontWeight: FontWeight.bold,
                ),
          ),
          //const SizedBox(height: 12),

          // Bloc couleurs + état LED
          if (couleurR != null && couleurG != null && couleurB != null && couleurClear != null) ...[
            Row(
              mainAxisAlignment: MainAxisAlignment.start,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Cercle de couleur approximative (conversion 16 → 8 bits)
                Container(
                  width: 20, //40,
                  height: 20, //40,
                  decoration: BoxDecoration(
                    shape: BoxShape.circle,
                    color: etatLed ? Colors.green : Colors.red,/*Color.fromRGBO(
                      ((couleurR ?? 0) / 256).clamp(0, 255).toInt(),
                      ((couleurG ?? 0) / 256).clamp(0, 255).toInt(),
                      ((couleurB ?? 0) / 256).clamp(0, 255).toInt(),
                      1.0,
                    ),*/
                    border: Border.all(color: Colors.grey.shade400, width: 2),
                    boxShadow: [
                      BoxShadow(
                        color: Colors.black.withOpacity(0.15),
                        blurRadius: 4,
                        offset: const Offset(0, 2),
                      ),
                    ],
                  ),
                ),
                const SizedBox(width: 16),

                // Valeurs brutes 16 bits
                Column(
                  //crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      'R: ${(couleurR ?? 0).toInt().toString().padLeft(5, ' ')}',
                      style: const TextStyle(fontSize: 13),
                    ),
                    Text(
                      'G: ${(couleurG ?? 0).toInt().toString().padLeft(5, ' ')}',
                      style: const TextStyle(fontSize: 13),
                    ),
                    Text(
                      'B: ${(couleurB ?? 0).toInt().toString().padLeft(5, ' ')}',
                      style: const TextStyle(fontSize: 13),
                    ),
                  ],
                ),
                const SizedBox(width: 18),
                Column(
                  mainAxisAlignment: MainAxisAlignment.start,
                  //crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      'C: ${(couleurClear ?? 0).toInt().toString().padLeft(5, ' ')}',
                      style: const TextStyle(fontSize: 13),
                    ),
                  ],
                ),
              ],
            ),

            //const SizedBox(height: 12),

            // État LED
            /*if (etatLed != null)
              Row(
                children: [
                  Icon(
                    etatLed ? Icons.power : Icons.power_off,
                    color: etatLed ? Colors.red : Colors.grey,
                    size: 20,
                  ),
                  const SizedBox(width: 8),
                  Text(
                    etatLed ? 'LED Rouge ALLUMÉE' : 'LED Rouge éteinte',
                    style: TextStyle(
                      fontSize: 14,
                      fontWeight: FontWeight.w600,
                      color: etatLed ? Colors.red : Colors.grey,
                    ),
                  ),
                ],
              ),*/

            //const SizedBox(height: 8),
          ],

          // Date/heure (inchangée)
          if (dh != null)
            Text(
              dh,
              style: const TextStyle(fontSize: 11, color: Colors.grey),
            ),
        ],
      ),
    ),
  );
}

}