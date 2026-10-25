import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../services/telemetry_service.dart';

class SettingsScreen extends StatelessWidget {
  const SettingsScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final telemetry = context.watch<TelemetryService>();
    final settings = telemetry.settings;

    return Scaffold(
      backgroundColor: const Color(0xFF08090C),
      appBar: AppBar(
        backgroundColor: const Color(0xFF0F172A),
        elevation: 0,
        title: const Text(
          "Vehicle & Sensor Settings",
          style: TextStyle(color: Colors.white, fontSize: 18, fontWeight: FontWeight.w600),
        ),
      ),
      body: ListView(
        padding: const EdgeInsets.all(16.0),
        children: [
          // Section 1: Headway & Safety Calibration
          _buildSectionHeader("SAFETY CALIBRATION"),
          _buildCard([
            ListTile(
              title: const Text("Front Hazard Threshold", style: TextStyle(color: Colors.white, fontSize: 15)),
              subtitle: Text(
                "Trigger critical headway collision alert below ${settings.warningDistanceCm.round()} cm",
                style: const TextStyle(color: Color(0xFFA0AEC0), fontSize: 12),
              ),
              trailing: Text(
                "${settings.warningDistanceCm.round()} cm",
                style: const TextStyle(color: Color(0xFF38BDF8), fontWeight: FontWeight.bold, fontSize: 16),
              ),
            ),
            Slider(
              value: settings.warningDistanceCm,
              min: 10,
              max: 100,
              divisions: 18,
              activeColor: const Color(0xFF38BDF8),
              inactiveColor: const Color(0xFF1E293B),
              onChanged: (val) {
                settings.warningDistanceCm = val;
                telemetry.notifyListeners();
              },
            ),
          ]),

          const SizedBox(height: 20),

          // Section 2: Audio & Alert Preferences
          _buildSectionHeader("ACOUSTIC ALERTS"),
          _buildCard([
            SwitchListTile(
              activeColor: const Color(0xFF00E676),
              title: const Text("Cockpit Multi-Tone Buzzer", style: TextStyle(color: Colors.white, fontSize: 15)),
              subtitle: const Text("Directional 2200Hz / 2900Hz pitch feedback", style: TextStyle(color: Color(0xFFA0AEC0), fontSize: 12)),
              value: settings.enableAcousticAlerts,
              onChanged: (val) {
                settings.enableAcousticAlerts = val;
                telemetry.notifyListeners();
              },
            ),
          ]),

          const SizedBox(height: 20),

          // Section 3: Telemetry Stream Connection
          _buildSectionHeader("TELEMETRY STREAM CONNECTION"),
          _buildCard([
            ListTile(
              title: const Text("ESP32 SoftAP Host", style: TextStyle(color: Colors.white, fontSize: 15)),
              subtitle: Text(settings.esp32Ip, style: const TextStyle(color: Color(0xFFA0AEC0), fontSize: 12)),
              trailing: const Icon(Icons.wifi, color: Color(0xFF38BDF8)),
            ),
            const Divider(color: Colors.white10),
            SwitchListTile(
              activeColor: const Color(0xFF00E676),
              title: const Text("Auto-Reconnect", style: TextStyle(color: Colors.white, fontSize: 15)),
              subtitle: const Text("Continuously re-establish WebSocket link if truck powers off", style: TextStyle(color: Color(0xFFA0AEC0), fontSize: 12)),
              value: settings.autoReconnect,
              onChanged: (val) {
                settings.autoReconnect = val;
                telemetry.notifyListeners();
              },
            ),
          ]),

          const SizedBox(height: 30),

          // Reconnect / Connect Button
          ElevatedButton.icon(
            style: ElevatedButton.styleFrom(
              backgroundColor: const Color(0xFF0284C7),
              foregroundColor: Colors.white,
              padding: const EdgeInsets.symmetric(vertical: 16),
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
            ),
            icon: Icon(telemetry.status == ConnectionStatus.connected ? Icons.sync : Icons.power),
            label: Text(
              telemetry.status == ConnectionStatus.connected ? "RECONNECT TELEMETRY" : "CONNECT TO BLINDSPOTTR",
              style: const TextStyle(fontWeight: FontWeight.bold, letterSpacing: 0.5),
            ),
            onPressed: () {
              telemetry.disconnect();
              telemetry.connect();
            },
          ),
        ],
      ),
    );
  }

  static Widget _buildSectionHeader(String title) {
    return Padding(
      padding: const EdgeInsets.only(left: 4.0, bottom: 8.0),
      child: Text(
        title,
        style: const TextStyle(
          color: Color(0xFF64748B),
          fontSize: 11,
          fontWeight: FontWeight.bold,
          letterSpacing: 1.2,
        ),
      ),
    );
  }

  static Widget _buildCard(List<Widget> children) {
    return Container(
      decoration: BoxDecoration(
        color: const Color(0xFF0F172A),
        borderRadius: BorderRadius.circular(14),
        border: Border.all(color: Colors.white10),
      ),
      child: Column(children: children),
    );
  }
}
