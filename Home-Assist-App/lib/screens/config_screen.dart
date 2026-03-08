import 'package:dom_new/providers/app_providers.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import '../services/preferences_service.dart';
//import '../services/mqtt_service.dart';

class ConfigScreen extends ConsumerStatefulWidget {
  const ConfigScreen({super.key});

  @override
  ConsumerState<ConfigScreen> createState() => _ConfigScreenState();
}

class _ConfigScreenState extends ConsumerState<ConfigScreen> {
  final _formKey = GlobalKey<FormState>();

  late TextEditingController _brokerController;
  late TextEditingController _portController;
  late TextEditingController _usernameController;
  late TextEditingController _passwordController;

  @override
  void initState() {
    super.initState();
    _brokerController = TextEditingController();
    _portController = TextEditingController(text: '1883');
    _usernameController = TextEditingController();
    _passwordController = TextEditingController();

    // Charger les valeurs existantes au démarrage
    _loadConfig();
  }

  Future<void> _loadConfig() async {
    final config = await PreferencesService.loadMqttConfig();
    setState(() {
      _brokerController.text = config['broker'] ?? '51.77.244.19';
      _portController.text = config['port'] ?? '1883';
      _usernameController.text = config['username'] ?? '';
      _passwordController.text = config['password'] ?? '';
    });
  }

Future<void> _saveAndConnect() async {
  if (!_formKey.currentState!.validate()) return;

  final broker = _brokerController.text.trim();
  final port = _portController.text.trim();
  final username = _usernameController.text.trim();
  final password = _passwordController.text.trim();

  print('========================= Sauvegarde des valeurs : broker=$broker, port=$port, user=$username ========================================');

  await PreferencesService.saveMqttConfig(
    broker: broker,
    port: port,
    username: username,
    password: password,
  );

  // Force la reconnexion complète
  await ref.read(mqttServiceProvider).reconnectWithNewConfig();

  if (mounted) {
    Navigator.pop(context);
    ScaffoldMessenger.of(context).showSnackBar(
      const SnackBar(content: Text('Configuration sauvegardée – reconnexion en cours')),
    );
  }
}


  @override
  void dispose() {
    _brokerController.dispose();
    _portController.dispose();
    _usernameController.dispose();
    _passwordController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Configuration MQTT'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Form(
          key: _formKey,
          child: ListView(
            children: [
              TextFormField(
                controller: _brokerController,
                decoration: const InputDecoration(labelText: 'Broker (IP ou domaine)'),
                keyboardType: TextInputType.url,
                validator: (value) => value?.isEmpty ?? true ? 'Requis' : null,
              ),
              const SizedBox(height: 16),
              TextFormField(
                controller: _portController,
                decoration: const InputDecoration(labelText: 'Port'),
                keyboardType: TextInputType.number,
                validator: (value) {
                  if (value?.isEmpty ?? true) return 'Requis';
                  if (int.tryParse(value!) == null) return 'Numéro invalide';
                  return null;
                },
              ),
              const SizedBox(height: 16),
              TextFormField(
                controller: _usernameController,
                decoration: const InputDecoration(labelText: 'Nom d’utilisateur'),
                validator: (value) => value?.isEmpty ?? true ? 'Requis' : null,
              ),
              const SizedBox(height: 16),
              TextFormField(
                controller: _passwordController,
                decoration: const InputDecoration(labelText: 'Mot de passe'),
                obscureText: true,
                validator: (value) => value?.isEmpty ?? true ? 'Requis' : null,
              ),
              const SizedBox(height: 32),
              ElevatedButton(
                onPressed: _saveAndConnect,
                child: const Text('Sauvegarder et se connecter'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}