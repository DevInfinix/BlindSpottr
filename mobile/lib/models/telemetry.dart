class TelemetryData {
  final int id;
  final bool leftObstacle;
  final bool rightObstacle;
  final bool frontValid;
  final double frontDistanceCm;
  final String state;
  final int uptimeMs;

  TelemetryData({
    required this.id,
    required this.leftObstacle,
    required this.rightObstacle,
    required this.frontValid,
    required this.frontDistanceCm,
    required this.state,
    required this.uptimeMs,
  });

  factory TelemetryData.initial() {
    return TelemetryData(
      id: 0,
      leftObstacle: false,
      rightObstacle: false,
      frontValid: false,
      frontDistanceCm: -1.0,
      state: "DISCONNECTED",
      uptimeMs: 0,
    );
  }

  factory TelemetryData.fromJson(Map<String, dynamic> json) {
    return TelemetryData(
      id: json['id'] as int? ?? 0,
      leftObstacle: json['left'] as bool? ?? false,
      rightObstacle: json['right'] as bool? ?? false,
      frontValid: json['front_valid'] as bool? ?? false,
      frontDistanceCm: (json['front_cm'] as num?)?.toDouble() ?? -1.0,
      state: json['state'] as String? ?? "CLEAR",
      uptimeMs: json['uptime_ms'] as int? ?? 0,
    );
  }

  Map<String, dynamic> toJson() => {
    'id': id,
    'left': leftObstacle,
    'right': rightObstacle,
    'front_valid': frontValid,
    'front_cm': frontDistanceCm,
    'state': state,
    'uptime_ms': uptimeMs,
  };
}

class AppSettings {
  double warningDistanceCm;
  bool enableAcousticAlerts;
  bool autoReconnect;
  String esp32Ip;
  int wsPort;

  AppSettings({
    this.warningDistanceCm = 20.0,
    this.enableAcousticAlerts = true,
    this.autoReconnect = true,
    this.esp32Ip = "192.168.4.1",
    this.wsPort = 80,
  });
}
