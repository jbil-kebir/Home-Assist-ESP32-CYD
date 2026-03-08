
//
// Fichier app_providers.dart
//

import 'package:dom_new/services/mqtt_service.dart';
import 'package:flutter_riverpod/legacy.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:riverpod/legacy.dart';  



//----------------------------------------------------------------------------------------
// Provider d'état MQTT : connecté / non connecté
//----------------------------------------------------------------------------------------
// 
final mqttConnectedProvider = StateProvider<bool>((ref) => false);

//----------------------------------------------------------------------------------------
// Providers capteurs
//----------------------------------------------------------------------------------------
// Providers persistants pour chaque capteur (StateProvider.family)
final temperatureProvider = StateProvider.family<double?, String>((ref, nom) => null); // Température
final humiditeProvider = StateProvider.family<double?, String>((ref, nom) => null); // Humidité
final derniereMesureProvider = StateProvider.family<String?, String>((ref, nom) => null); // Horodatage
// Batterie - état texte ("CHARGEE" / "DECHARGEE")
final batterieEtatProvider = StateProvider.family<String?, String>((ref, nom) => null);
// Batterie - tension (ex. 7.5 V)
final batterieTensionProvider = StateProvider.family<double?, String>((ref, nom) => null);
// État réel du flotteur (true = haut/ON, false = bas/OFF)
final flotteurProvider = StateProvider.family<bool, String>((ref, nom) => false);  // bas par défaut

//----------------------------------------------------------------------------------------
// Providers actionneurs
//----------------------------------------------------------------------------------------
// État ENABLED/DISABLED par actionneur (true = activé)
final actionneurEnabledProvider = StateProvider.family<bool, String>((ref, nomActionneur) => false,); // désactivé par défaut
// État réel ON/OFF (true = ON, false = OFF) – renvoyé par le contrôleur
final actionneurOnOffProvider = StateProvider.family<bool, String>((ref, nom) => false);  // OFF par défaut

//----------------------------------------------------------------------------------------
// Providers
//----------------------------------------------------------------------------------------

// Provider global
final mqttServiceProvider = Provider<MqttService>((ref) {
  final service = MqttService();
  service.setRef(ref);  // ← on passe ref au service
  service.connect(); // lance la connexion au démarrage
  ref.onDispose(service.dispose);
  return service;
});

final mqttCapteurMessagesProvider = StreamProvider<Map<String, dynamic>>((ref) {
  final service = ref.watch(mqttServiceProvider);
  return service.capteurMessagesStream;
});

// Provider pour les messages actionneurs (retours de commande)
final mqttActionneurMessagesProvider = StreamProvider<Map<String, dynamic>>((ref) {
  final service = ref.watch(mqttServiceProvider);
  return service.actionneurMessagesStream;
});
