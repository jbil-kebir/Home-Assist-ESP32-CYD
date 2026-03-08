//
// Fichier preferences_service.dart
//
import 'package:shared_preferences/shared_preferences.dart';

class PreferencesService {
  static const String _brokerKey = 'mqtt_broker';
  static const String _portKey = 'mqtt_port';
  static const String _usernameKey = 'mqtt_username';
  static const String _passwordKey = 'mqtt_password';

  static Future<Map<String, String?>> loadMqttConfig() async {
    final prefs = await SharedPreferences.getInstance();
    return {
      'broker': prefs.getString(_brokerKey),
      'port': prefs.getString(_portKey),
      'username': prefs.getString(_usernameKey),
      'password': prefs.getString(_passwordKey),
    };
  }

  static Future<void> saveMqttConfig({
    required String broker,
    required String port,
    required String username,
    required String password,
  }) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString(_brokerKey, broker);
    await prefs.setString(_portKey, port);
    await prefs.setString(_usernameKey, username);
    await prefs.setString(_passwordKey, password);
  }

  static Future<bool> hasConfig() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.containsKey(_brokerKey) &&
           prefs.containsKey(_portKey) &&
           prefs.containsKey(_usernameKey) &&
           prefs.containsKey(_passwordKey);
  }
}