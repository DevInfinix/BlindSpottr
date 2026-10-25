import 'dart:async';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:web_socket_channel/web_socket_channel.dart';
import '../models/telemetry.dart';

enum ConnectionStatus { disconnected, connecting, connected }

class TelemetryService extends ChangeNotifier {
  final AppSettings settings = AppSettings();
  WebSocketChannel? _channel;
  StreamSubscription? _subscription;
  Timer? _reconnectTimer;

  ConnectionStatus _status = ConnectionStatus.disconnected;
  ConnectionStatus get status => _status;

  TelemetryData _currentData = TelemetryData.initial();
  TelemetryData get currentData => _currentData;

  int _packetsReceived = 0;
  int get packetsReceived => _packetsReceived;

  DateTime? _lastPacketTime;
  int _latencyMs = 0;
  int get latencyMs => _latencyMs;

  void connect() {
    if (_status == ConnectionStatus.connected || _status == ConnectionStatus.connecting) {
      return;
    }

    _status = ConnectionStatus.connecting;
    notifyListeners();

    final uri = Uri.parse("ws://${settings.esp32Ip}:${settings.wsPort}/ws");

    try {
      _channel = WebSocketChannel.connect(uri);
      _subscription = _channel!.stream.listen(
        (message) {
          final now = DateTime.now();
          if (_lastPacketTime != null) {
            _latencyMs = now.difference(_lastPacketTime!).inMilliseconds;
          }
          _lastPacketTime = now;
          _packetsReceived++;

          try {
            final Map<String, dynamic> parsed = jsonDecode(message as String);
            _currentData = TelemetryData.fromJson(parsed);
            if (_status != ConnectionStatus.connected) {
              _status = ConnectionStatus.connected;
            }
            notifyListeners();
          } catch (e) {
            debugPrint("JSON Parse Error: $e");
          }
        },
        onError: (error) {
          _handleDisconnect();
        },
        onDone: () {
          _handleDisconnect();
        },
      );
    } catch (e) {
      _handleDisconnect();
    }
  }

  void _handleDisconnect() {
    _status = ConnectionStatus.disconnected;
    _currentData = TelemetryData.initial();
    notifyListeners();

    if (settings.autoReconnect) {
      _reconnectTimer?.cancel();
      _reconnectTimer = Timer(const Duration(seconds: 3), () {
        connect();
      });
    }
  }

  void disconnect() {
    _reconnectTimer?.cancel();
    _subscription?.cancel();
    _channel?.sink.close();
    _status = ConnectionStatus.disconnected;
    notifyListeners();
  }

  @override
  void dispose() {
    _reconnectTimer?.cancel();
    _subscription?.cancel();
    _channel?.sink.close();
    super.dispose();
  }
}
