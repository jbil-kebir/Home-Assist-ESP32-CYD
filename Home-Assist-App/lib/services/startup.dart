import 'package:device_info_plus/device_info_plus.dart';
import 'package:dom_new/providers/app_providers.dart';
import 'package:dom_new/services/mqtt_service.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:intl/intl.dart';
import 'package:network_info_plus/network_info_plus.dart';

Future<void> sendStartupMessage(WidgetRef ref) async {
  try {
    // 1. Nom du téléphone
    final deviceInfo = DeviceInfoPlugin();
    final androidInfo = await deviceInfo.androidInfo;
    final nomTelephone = androidInfo.model ?? 'Android inconnu';

    // 2. Date et heure actuelles
    final now = DateTime.now();
    final formatter = DateFormat('dd/MM/yyyy HH:mm:ss');
    final dateTimeStr = formatter.format(now);

    // 3. Adresse IP (Wi-Fi de préférence)
    final info = NetworkInfo();
    String ip = await info.getWifiIP() ?? 'IP inconnue';

    // 4. Construction de la trame
    final message = "DEVICE APP $nomTelephone $dateTimeStr 0 $ip";

    // 5. Envoi via MQTT
    ref.read(mqttServiceProvider).publishMessage('home/configuration/command', message);

  } catch (e) {
    print('Erreur lors de l’envoi du message de démarrage : $e');
  }
}