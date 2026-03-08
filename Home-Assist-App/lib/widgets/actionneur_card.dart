//
// Fichier actionneur_card.dart
//
import 'package:dom_new/providers/app_providers.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

class ActionneurCard extends ConsumerWidget {
  final String nomActionneurNormalise;  // pour les providers
  final String nomActionneurAffiche;   // pour l'écran

  const ActionneurCard({
    super.key,
    required this.nomActionneurNormalise,
    required this.nomActionneurAffiche,
  });

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final dateTime = 'toto_date';
    final isEnabled = ref.watch(actionneurEnabledProvider(nomActionneurNormalise));
    final isOn = ref.watch(actionneurOnOffProvider(nomActionneurNormalise));

    return Card(
      elevation: 2,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
      child: Padding(
        padding: const EdgeInsets.fromLTRB(2, 2, 2, 2), // (12, 8, 12, 12), // haut réduit à 8 au lieu de 12
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            // Ligne nom + bouton ENABLE/DISABLE (espace réduit)
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                Expanded(
                  child: Text(
                    //nomActionneurNormalise.toUpperCase(),
                    nomActionneurNormalise,
                    style: Theme.of(context).textTheme.titleSmall?.copyWith(
                      fontWeight: FontWeight.bold,
                    ),
                    overflow: TextOverflow.ellipsis,
                  ),
                ),
                IconButton(
                  onPressed: () {
                    final command = isEnabled ? 'DISABLE' : 'ENABLE';
                    ref.read(mqttServiceProvider).publishCommand(nomActionneurAffiche, command);
                  },
                  icon: Icon(
                    isEnabled ? Icons.power_settings_new : Icons.power_off,
                    color: isEnabled ? Colors.green : Colors.grey,
                    size: 32, // un peu plus petit
                  ),
                  padding: EdgeInsets.zero, // supprime le padding autour de l'icône
                  constraints: const BoxConstraints(), // enlève l'espace minimum
                ),
              ],
            ),

            // Date (si présente) – très proche du nom
            if (dateTime.isNotEmpty)
              Padding(
                padding: const EdgeInsets.only(top: 2), // réduit de 4 à 2
                child: Text(
                  dateTime,
                  style: const TextStyle(fontSize: 10, color: Colors.grey), // encore plus petit
                  textAlign: TextAlign.center,
                ),
              ),

            const SizedBox(height: 6), // espace entre nom/date et boutons → réduit de 12 à 6

            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                Expanded(
                  child: ElevatedButton(
                     onPressed: isEnabled
                      ? () { // Si actif, on rend disponible la commande ON
                          ref.read(mqttServiceProvider).publishCommand(nomActionneurAffiche, true);
                          print('Commande envoyée : ON $nomActionneurNormalise');
                      }
                      : null,
                    style: ElevatedButton.styleFrom(
                      padding: const EdgeInsets.symmetric(vertical: 6), // boutons plus fins verticalement
                      backgroundColor: isEnabled ? 
                        isOn ? Colors.green[700] : Colors.red
                        : Colors.grey,
                      foregroundColor: Colors.white,
                      textStyle: const TextStyle(fontSize: 12), // texte plus petit
                    ),
                    child: isOn ? const Text('OFF') : const Text('ON'),
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}