//
// Fichier main.dart
//
import 'package:dom_new/providers/app_providers.dart';
import 'package:dom_new/screens/config_screen.dart';
import 'package:dom_new/services/preferences_service.dart';
import 'package:dom_new/widgets/actionneur_card_2_Btn.dart';
import 'package:dom_new/widgets/appBar.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'widgets/capteur_card.dart';
import 'widgets/actionneur_card.dart';
import 'package:device_info_plus/device_info_plus.dart';
import 'package:intl/intl.dart';
import 'package:network_info_plus/network_info_plus.dart';

void main() {
  runApp(const ProviderScope(child: MyApp()));
}

class MyApp extends ConsumerWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {

    Future.microtask(() async {
      final hasConfig = await PreferencesService.hasConfig();
      if (!hasConfig) {
        Navigator.pushReplacement(
          context,
          MaterialPageRoute(builder: (context) => const ConfigScreen()),
        );
      } else {
        ref.read(mqttServiceProvider).loadConfigAndConnect();
      }
    });    

    return MaterialApp(
      title: 'Domotique MQTT',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const LifecycleWrapper(child: HomeScreen()),
    );
  }
}

// Wrapper qui écoute le cycle de vie et envoie le message à chaque reprise
class LifecycleWrapper extends ConsumerStatefulWidget {
  final Widget child;

  const LifecycleWrapper({super.key, required this.child});

  @override
  ConsumerState<LifecycleWrapper> createState() => _LifecycleWrapperState();
}

class _LifecycleWrapperState extends ConsumerState<LifecycleWrapper> with WidgetsBindingObserver {
  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addObserver(this);

  }

  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    super.dispose();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    super.didChangeAppLifecycleState(state);
    if (state == AppLifecycleState.resumed) {
      print('App resumed → envoi message');
      sendStartupMessage(ref);
    }
  }

  @override
  Widget build(BuildContext context) {
    return widget.child;
  }
}

// Fonction d’envoi du message (utilisée au démarrage ET à chaque reprise)
Future<void> sendStartupMessage(WidgetRef ref) async {
  try {
    // Nom du téléphone
    final deviceInfo = DeviceInfoPlugin();
    final androidInfo = await deviceInfo.androidInfo;
    final nomTelephone = androidInfo.model ?? 'Android inconnu';

    // Date et heure actuelles
    final now = DateTime.now();
    final formatter = DateFormat('dd/MM/yyyy HH:mm:ss');
    final dateTimeStr = formatter.format(now);

    // IP
    final info = NetworkInfo();
    String ip = await info.getWifiIP() ?? 'IP inconnue';

    // Message
    final message = "DEVICE APP $nomTelephone $dateTimeStr 0 $ip";

    // Envoi
    ref.read(mqttServiceProvider).publishMessage('home/configuration/command', message);
  } catch (e) {
    print('Erreur envoi message : $e');
  }
}

// HomeScreen reste exactement comme avant (ConsumerWidget)
class HomeScreen extends ConsumerWidget {
  const HomeScreen({super.key});

  final Map<String, String> capteurs = const {
    'nomade': 'ThNomade',
    'cave': 'ThCave',
    'chambre 1er': 'ThCh1er',
    'chambre RDC': 'ThChRDC',
    'sdb': 'ThSdb',
  };

  final Map<String, String> actionneurs = const {
    'chaudiere': 'chaudiere',
    'projecteur': 'projecteur',
    'guirlande': 'guirlande',
    'chauffage': 'chauffage',
  };

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final isConnected = ref.watch(mqttConnectedProvider);

    // Écoute des messages MQTT
    ref.listen(mqttServiceProvider, (previous, next) {
      ref.read(mqttConnectedProvider.notifier).state = next.isConnected;
    });

    ref.listen(mqttConnectedProvider, (previous, next) {
          if (next == true && (previous == false || previous == null)) {
            print('Connexion stable → envoi message de démarrage');
            sendStartupMessage(ref);
          }
        });   

    // Listener pour les capteurs 
    ref.listen(mqttCapteurMessagesProvider, (previous, next) {
      if (next.value == null) return;
      final update = next.value!;
      String? nom = update['equipement'] as String?;
      if (nom == null) return;

      // Batteries liées
      if (nom == 'BATSDB') nom = 'THSDB';
      else if (nom == 'BATNOMADE') nom = 'THNOMADE';
      else if (nom == 'BATCH1ER') nom = 'THCH1ER';
      else if (nom == 'BATCAVE') nom = 'THCAVE';


      if (update['type'] == 'TEMP') {
        ref.read(temperatureProvider(nom).notifier).state = update['valeur'];
      } else if (update['type'] == 'HUM') {
        ref.read(humiditeProvider(nom).notifier).state = update['valeur'];
      } else if (update['type'] == 'TENSION') {
        final etat = update['etat'] as String?;
        final tension = update['valeur'] as double?;
        if (tension != null) {
          ref.read(batterieTensionProvider(nom).notifier).state = tension;
          if (etat != null) {
            ref.read(batterieEtatProvider(nom).notifier).state = etat;
          }
        }
      }
      else if (update['type'] == 'ONOFF' || update['type'] == 'ONOFFR') {
        final valeur = update['valeur'] as double?;
        if (valeur != null) {
          final estHaut = valeur == 1.0 || valeur == 1;
          ref.read(flotteurProvider(nom).notifier).state = estHaut;
        }
      }

      ref.read(derniereMesureProvider(nom).notifier).state = update['dateHeure'];
    });

    // Listener dédié aux retours de commande
    ref.listen(mqttActionneurMessagesProvider, (previous, next) {
      if (next.value == null) return;
      final update = next.value!;
      final nom = update['equipement'] as String?;
      if (nom == null) return;

      final valeur = update['valeur'] as String;

      if (valeur == 'ONR') {
        ref.read(actionneurOnOffProvider(nom).notifier).state = true;
      } else if (valeur == 'OFFR') {
        ref.read(actionneurOnOffProvider(nom).notifier).state = false;
      } else if (valeur == 'ENABLER') {
        ref.read(actionneurEnabledProvider(nom).notifier).state = true;
      } else if (valeur == 'DISABLER') {
        ref.read(actionneurEnabledProvider(nom).notifier).state = false;
      }

    });

    return Scaffold(
      appBar: CustomAppBar(),
      body: ListView(
        padding: const EdgeInsets.all(6),
        children: [
          // -----------------------------------------------------------------------
          // Liste des capteurs
          Container(
            color: const Color.fromARGB(255, 177, 219, 178),
            child: Column(
              children: [
                Text(
                  'Capteurs',
                  style: Theme.of(context).textTheme.titleMedium?.copyWith(
                        fontWeight: FontWeight.bold,
                      ),
                ),
              ],
            ),
          ),
          Container(
            child: GridView.count(
              crossAxisCount: 2,
              shrinkWrap: true,
              physics: const NeverScrollableScrollPhysics(),
              mainAxisSpacing: 12,
              crossAxisSpacing: 12,
              childAspectRatio: 1.7,
              children: [
                CapteurCard(
                  nomCapteurNormalise: capteurs['nomade']!.trim().toUpperCase(),
                  nomCapteurAffiche: capteurs['nomade']!,
                  bDeuxMesures: true,
                  bFlotteur: true,
                ),
                CapteurCard(
                  nomCapteurNormalise: capteurs['cave']!.trim().toUpperCase(),
                  nomCapteurAffiche: capteurs['cave']!,
                  bDeuxMesures: true,
                  bFlotteur: false,
                ),
                CapteurCard(
                  nomCapteurNormalise: capteurs['chambre 1er']!.trim().toUpperCase(),
                  nomCapteurAffiche: capteurs['chambre 1er']!,
                  bDeuxMesures: false,
                  bFlotteur: false,
                ),
                CapteurCard(
                  nomCapteurNormalise: capteurs['chambre RDC']!.trim().toUpperCase(),
                  nomCapteurAffiche: capteurs['chambre RDC']!,
                  bDeuxMesures: false,
                  bFlotteur: false,
                ),
                CapteurCard(
                  nomCapteurNormalise: capteurs['sdb']!.trim().toUpperCase(),
                  nomCapteurAffiche: capteurs['sdb']!,
                  bDeuxMesures: false,
                  bFlotteur: false,
                ),
              ],
            ),
          ),

          // -----------------------------------------------------------------------
          // Liste des actionneurs
          Container(
            color: const Color.fromARGB(255, 177, 219, 178),
            child: Column(
              children: [
                Text(
                  'Actionneurs',
                  style: Theme.of(context).textTheme.titleMedium?.copyWith(
                        fontWeight: FontWeight.bold,
                      ),
                ),
              ],
            ),
          ),
          Container(
            child: GridView.count(
              crossAxisCount: 2,
              shrinkWrap: true,
              physics: const NeverScrollableScrollPhysics(),
              mainAxisSpacing: 12,
              crossAxisSpacing: 12,
              childAspectRatio: 1.3,
              children: [
                ActionneurCard(
                  nomActionneurNormalise: actionneurs['projecteur']!.trim().toUpperCase(),
                  nomActionneurAffiche: actionneurs['projecteur']!,
                ),
                ActionneurCard(
                  nomActionneurNormalise: actionneurs['guirlande']!.trim().toUpperCase(),
                  nomActionneurAffiche: actionneurs['guirlande']!,
                ),
                ActionneurCard(
                  nomActionneurNormalise: actionneurs['chauffage']!.trim().toUpperCase(),
                  nomActionneurAffiche: actionneurs['chauffage']!,
                ),
                ActionneurCard2Btn(
                  nomActionneurNormalise: actionneurs['chaudiere']!.trim().toUpperCase(),
                  nomActionneurAffiche: actionneurs['chaudiere']!,
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

