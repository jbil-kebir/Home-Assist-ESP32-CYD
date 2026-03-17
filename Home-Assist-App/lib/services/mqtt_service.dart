//
// Fichier mqtt_service.dart
//
import 'dart:async';
import 'dart:convert';

import 'package:dom_new/providers/app_providers.dart';
import 'package:dom_new/services/preferences_service.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

class MqttService {
  late MqttServerClient client;
  late Ref ref;

  String broker = '51.77.244.19';
  int port = 1883;
  final String clientId = 'flutter_domotique_${DateTime.now().millisecondsSinceEpoch}';

  String username = '';
  String password = ''; 

  bool isConnected = false;

  // Création d'un stream pour envoyer des Map<String, dynamic> (topic, value) à tout le monde...
  // Il sera utilisé après réception d'un message MQTT et extraction des données
  // Spécifique aux capteurs...
  final _capteurMessagesController = StreamController<Map<String, dynamic>>.broadcast();
  // ...et d'un getter pour le lire
  Stream<Map<String, dynamic>> get capteurMessagesStream => _capteurMessagesController.stream;
  // Stream pour les retours de commande (ONR, OFFR, ENABLER, DISABLER)
  final _actionneurMessagesController = StreamController<Map<String, dynamic>>.broadcast();
  Stream<Map<String, dynamic>> get actionneurMessagesStream => _actionneurMessagesController.stream;

  Future<void> loadConfigAndConnect() async {
      final prefs = await PreferencesService.loadMqttConfig();

      broker = prefs['broker'] ?? broker;
      port = int.tryParse(prefs['port'] ?? '1883') ?? 1883;
      username = prefs['username'] ?? username;
      password = prefs['password'] ?? password;

      print('Config chargée : $broker:$port - $username');

      connect();
    }

  MqttService() {
    _init();
  }

  void setRef(Ref r) {
    ref = r;
  }

  void _init() {
    client = MqttServerClient(broker, clientId);
    client.port = port;
    client.logging(on: true); // pour voir les logs dans la console
    client.keepAlivePeriod = 60;
    client.onConnected = _onConnected;
    client.onDisconnected = _onDisconnected;
    client.onSubscribed = (topic) => print('Souscrit à $topic');

    final connMessage = MqttConnectMessage()
        .authenticateAs(username, password)
        .startClean();

    client.connectionMessage = connMessage;
  
    
  }

  Future<void> connect() async {
    if (isConnected) return;

    try {
      print('Tentative de connexion à $broker:$port avec $username');
      await client.connect(username, password);
    } catch (e) {
      print('Échec connexion : $e');
      client.disconnect();
      isConnected = false;
    }
  }

Future<void> reconnectWithNewConfig() async {
  print('Début de reconnectWithNewConfig()');

  // 1. Déconnexion propre
  if (client.connectionStatus?.state == MqttConnectionState.connected ||
      client.connectionStatus?.state == MqttConnectionState.connecting) {
    client.disconnect();
    isConnected = false;
    print('Client MQTT déconnecté avant recréation');
  }

  // 2. Charger les nouvelles valeurs
  final prefs = await PreferencesService.loadMqttConfig();
  print('Valeurs chargées depuis prefs : $prefs');

  broker = prefs['broker'] ?? broker;
  port = int.tryParse(prefs['port'] ?? '1883') ?? port;
  username = prefs['username'] ?? username;
  password = prefs['password'] ?? password;

  print('Nouvelles valeurs appliquées : broker=$broker, port=$port, user=$username');

  // 3. RECRÉER le client de zéro (c’est la partie cruciale)
  client = MqttServerClient(broker, clientId);
  client.port = port;
  client.logging(on: true);
  client.keepAlivePeriod = 60;
  client.onConnected = _onConnected;
  client.onDisconnected = _onDisconnected;
  client.onSubscribed = (topic) => print('Souscrit à $topic');

  // 4. Reconfigurer l’authentification
  final connMessage = MqttConnectMessage()
      .authenticateAs(username, password)
      .startClean();
  client.connectionMessage = connMessage;

  // 5. Lancer la connexion
  try {
    await connect();
    print('Reconnexion réussie avec les nouveaux identifiants');
  } catch (e) {
    print('Échec reconnexion : $e');
  }
}

  //=========================================================================================
  // void traiteMqttMessagesCapteurs()
  //
  // Traite les messages reçus par MQTT sur le topic home/thermometre/state
  // et met à jour le stream _capteurMessagesController. Il est accessible par le getter capteurMessagesStream.
  // Celui-ci est écouté par le provider mqttCapteurMessagesProvider, utilisé par ref.listen dans HomeScreen
  //
  // Concerne les mesures de température, d'humidité, de tension et d'état de la batterie
  //=========================================================================================
  void traiteMqttMessagesCapteurs(MqttReceivedMessage<MqttMessage> recMsg) {
    final publishMessage = recMsg.payload as MqttPublishMessage;
    // Payload en string (correction de l'erreur)
    final payload = utf8.decode(publishMessage.payload.message).trim();

    // Traitement des messages capteurs (format long)
    final parts = payload.split(' ');
    if (parts.length < 5) return;

    // ────────────────────────────────────────────────
    // NOM BRUT = tel qu'il arrive (pour l'affichage)
    final equipBrut = parts[0];               // "ThChBat" ou "thchbat " ou " THCHBAT"

    // NOM NORMALISÉ = pour les providers et comparaisons
    final equipNormalise = equipBrut.trim().toUpperCase();  // "THCHBAT"

    // TYPE NORMALISÉ (pour les if)
    final typeBrut = parts[1];
    final typeNormalise = typeBrut.trim().toUpperCase();    // "TEMP", "TENSION", etc.

    // ────────────────────────────────────────────────
    // Cas normal Temp / Hum
    if (typeNormalise == 'TEMP' || typeNormalise == 'HUM') {
      final date = parts[2];
      final heure = parts[3];
      final valeurStr = parts[4];
      final valeur = double.tryParse(valeurStr);
      if (valeur == null) return;

      final force = parts.length > 5 && parts[5].trim().toUpperCase() == 'FORCE';

      final update = <String, dynamic>{
        'equipement_brut': equipBrut,          // ← conservé pour l'affichage
        'equipement': equipNormalise,          // ← utilisé pour les providers
        'type': typeNormalise,
        'valeur': valeur,
        'unite': typeNormalise == 'TEMP' ? '°C' : '%',
        'dateHeure': '$date $heure',
        'force': force,
      };

      _capteurMessagesController.add(update);
    }
  // Cas Remote TempR / HumR
  else if (typeNormalise == 'TEMPR' || typeNormalise == 'HUMR') {
    // exactement le même code que pour TEMP / HUM
    final date = parts[2];
    final heure = parts[3];
    final valeurStr = parts[4];
    final valeur = double.tryParse(valeurStr);
    if (valeur == null) return;

    final force = parts.length > 5 && parts[5].trim().toUpperCase() == 'FORCE';

    final update = <String, dynamic>{
      'equipement': equipNormalise,
      'equipement_affiche': equipBrut,
      'type': typeNormalise.endsWith('R') ? typeNormalise.substring(0, typeNormalise.length - 1) : typeNormalise,
      'valeur': valeur,
      'unite': typeNormalise.startsWith('TEMP') ? '°C' : '%',
      'dateHeure': '$date $heure',
      'force': force,
    };

    _capteurMessagesController.add(update);
  } 
    // Cas batterie (Tension)
    else if (typeNormalise == 'TENSION' || typeNormalise == 'TENSIONR') {
      final etat = parts[2].trim().toUpperCase();  // CHARGEE ou DECHARGEE
      final date = parts[3];
      final heure = parts[4];
      final valeurStr = parts[5];
      final valeur = double.tryParse(valeurStr);
      if (valeur == null) return;

      final force = parts.length > 6 && parts[6].trim().toUpperCase() == 'FORCE';

      final update = <String, dynamic>{
        'equipement_brut': equipBrut,          // ← pour l'affichage
        'equipement': equipNormalise,          // ← pour les providers
        'type': 'TENSION',
        'etat': etat,
        'valeur': valeur,
        'dateHeure': '$date $heure',
        'force': force,
      };

      _capteurMessagesController.add(update);
    }
    // Traitement des flotteurs (ONOFF / ONOFFR)
    else if (typeNormalise == 'ONOFF' || typeNormalise == 'ONOFFR') {
      // exactement le même code que pour TEMP / HUM
      final date = parts[2];
      final heure = parts[3];
      final valeurStr = parts[4];
      final valeur = double.tryParse(valeurStr);
      if (valeur == null) return;

      final force = parts.length > 5 && parts[5].trim().toUpperCase() == 'FORCE';

      final update = <String, dynamic>{
        'equipement': equipNormalise,
        'equipement_affiche': equipBrut,
        'type': typeNormalise.endsWith('R') ? typeNormalise.substring(0, typeNormalise.length - 1) : typeNormalise,
        'valeur': valeur,
        'unite': '',
        'dateHeure': '$date $heure',
        'force': force,
      };

      _capteurMessagesController.add(update);
    } 
    // Cas Coul / CoulR :  Remontée des couleurs RGB et Luminosité du capteur RGB
    else if (typeNormalise == 'COUL' || typeNormalise == 'COULR') {
      final date = parts[2];
      final heure = parts[3];
      final valeurStrR = parts[4];
      final valeurStrG = parts[5];
      final valeurStrB = parts[6];
      final valeurStrLux = parts[7];
      final valeurStrLuxBrut = parts[8];
      final valeurR = double.tryParse(valeurStrR);
      final valeurG = double.tryParse(valeurStrG);
      final valeurB = double.tryParse(valeurStrB);
      final valeurLux = double.tryParse(valeurStrLux);
      final valeurLuxBrut = double.tryParse(valeurStrLuxBrut);
      if (valeurR == null || valeurG == null  || valeurB == null  || valeurLux == null  || valeurLuxBrut == null) return;

      final force = parts.length > 9 && parts[9].trim().toUpperCase() == 'FORCE';

      final update = <String, dynamic>{
        'equipement': equipNormalise,
        'equipement_affiche': equipBrut,
        'type': typeNormalise.endsWith('R') ? typeNormalise.substring(0, typeNormalise.length - 1) : typeNormalise,
        'valeurR': valeurR,
        'valeurG': valeurG,
        'valeurB': valeurB,
        'valeurLux': valeurLux,
        'valeurLuxBrut': valeurLuxBrut,
        'dateHeure': '$date $heure',
        'force': force,
      };

      _capteurMessagesController.add(update);
    }
    // Cas Del / Del : Capteur RGB. On remonté l'état d'une del rouge surveillée. 
    else if (typeNormalise == 'DEL' || typeNormalise == 'DELR') {
      final date = parts[2];
      final heure = parts[3];
      final delONStr = parts[4];
      final delON = double.tryParse(delONStr);
      if (delON == null) return;

      final force = parts.length > 5 && parts[5].trim().toUpperCase() == 'FORCE';

      final update = <String, dynamic>{
        'equipement': equipNormalise,
        'equipement_affiche': equipBrut,
        'type': typeNormalise.endsWith('R') ? typeNormalise.substring(0, typeNormalise.length - 1) : typeNormalise,
        'delON': delON,
        'dateHeure': '$date $heure',
        'force': force,
      };

      _capteurMessagesController.add(update);
    }

  }

  void traiteMqttMessagesActionneurs(MqttReceivedMessage<MqttMessage> recMsg) {
    final publishMessage = recMsg.payload as MqttPublishMessage;
    // Payload en string (correction de l'erreur)
    final payload = utf8.decode(publishMessage.payload.message).trim();

    final String sTopic = recMsg.topic;
    final String sPayload = payload;

    // Retour de commande (ONR, OFFR, ENABLER, DISABLER)
    final payloadUpper = payload.toUpperCase();

    final topicParts = recMsg.topic.split('/');
    if (topicParts.length != 3) return;
    final nomActionneur = topicParts[1];

    print('void traiteMqttMessagesActionneurs() - Reçu sur $sTopic : "$sPayload"');

    if (payloadUpper == 'ONR' || payloadUpper == 'OFFR' || payloadUpper == 'ENABLER' || payloadUpper == 'DISABLER') {
      final update = <String, dynamic>{
        'equipement_brut': nomActionneur,          // ← pour l'affichage
        'equipement': nomActionneur.trim().toUpperCase(),          // ← pour les providers (tout en majuscule)
        'valeur': payloadUpper,
      };

      _actionneurMessagesController.add(update);
      print('Retour commande ajouté au stream actionneur : $payloadUpper pour $nomActionneur');
    }    
    
    print('Mise à jour provider pour "$nomActionneur" (depuis topic $sTopic) avec message "$payloadUpper"');

  }

  void _onConnected() {
    print('Connecté !');
    isConnected = true;
    client.subscribe('home/thermometre/state', MqttQos.atLeastOnce);
    
    // Abonnement aux retours de commande pour chaque actionneur
    // Ce qui nous permettra d'afficher l'état des boutons : grisé, vert, rouge
    // Selon que c'est ENABLE / DISABLE / ENABLE et ON, ...
    // Reçoit un format simple : "ONR", "OFFR", "ENABLER", "DISABLER"
    const noms = ['chaudiere', 'projecteur', 'guirlande', 'chauffage'];
    for (final nom in noms) {
      final topic = 'home/$nom/command';
      client.subscribe(topic, MqttQos.atLeastOnce);
      print('Abonné au topic retour de commande : $topic');
    }
   
    // Mise à jour du provider d'état (connecté / déconnecté)
    ref.read(mqttConnectedProvider.notifier).state = true;

    // Lecture et traitement des messages
    client.updates!.listen((List<MqttReceivedMessage<MqttMessage>>? c) {

      if (c == null || c.isEmpty) return;

      final recMess = c[0];

      // Vérification de sécurité : on ne traite que les messages PUBLISH
      if (recMess.payload is! MqttPublishMessage) {
        print('Message reçu non-PUBLISH sur ${recMess.topic} → ignoré');
        return;
      }

      final publishMessage = recMess.payload as MqttPublishMessage;

      // Payload en string (correction de l'erreur)
      final payload = utf8.decode(publishMessage.payload.message).trim();

      final String sTopic = recMess.topic;
      final String sPayload = payload;
      print('Reçu sur $sTopic : "$sPayload"');

      // ────────────────────────────────────────────────
      // BRANCHEMENT selon le topic
      // ────────────────────────────────────────────────

      if (recMess.topic == 'home/thermometre/state') { // Remontée des mesures de température, humidité, tension et état batterie
        traiteMqttMessagesCapteurs(recMess);
      }
      else if (recMess.topic.startsWith('home/') && recMess.topic.endsWith('/command')) { // Etat des boutons : ONR, OFFR, ENABLER, DISABLER
        traiteMqttMessagesActionneurs(recMess);
      }
      else {
        print('Topic inconnu : ${recMess.topic} → $payload');
      }

    });

  }

  void _onDisconnected() {
    print('Déconnecté');
    isConnected = false;
    ref.read(mqttConnectedProvider.notifier).state = false;
    Future.delayed(const Duration(seconds: 8), connect); // retry
  }
//----------------------------------------------------------------------------------------
// void publishCommand()
//----------------------------------------------------------------------------------------
void publishCommand(String equip, dynamic command) {
  if (!isConnected) {
    print('Not connected');
    return;
  }

  String payload;
  if (command is bool) {
    payload = command ? 'ON' : 'OFF';  // garde la compatibilité avec les anciens boutons
  } else if (command is String) {
    payload = command;  // ENABLE ou DISABLE
  } else {
    print('Commande invalide');
    return;
  }

  final topic = 'home/$equip/command';
  final builder = MqttClientPayloadBuilder();
  builder.addString(payload);
  client.publishMessage(topic, MqttQos.atLeastOnce, builder.payload!);

  print('Publié: "$payload" sur $topic');
}

//----------------------------------------------------------------------------------------
// void publishMessage(String topic, String payload)
//----------------------------------------------------------------------------------------
void publishMessage(String topic, String payload) {
  if (!isConnected) {
    print('MQTT non connecté - impossible d’envoyer : $payload');
    return;
  }

  final builder = MqttClientPayloadBuilder();
  builder.addString(payload);
  client.publishMessage(topic, MqttQos.atLeastOnce, builder.payload!);

  print('Message publié sur $topic : "$payload"');
}


  void dispose() {
    client.disconnect();
    _capteurMessagesController.close();
    _actionneurMessagesController.close();
  }



  
}


