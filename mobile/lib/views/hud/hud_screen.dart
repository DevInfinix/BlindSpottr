import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:provider/provider.dart';
import '../../services/telemetry_service.dart';

class HudScreen extends StatefulWidget {
  const HudScreen({super.key});

  @override
  State<HudScreen> createState() => _HudScreenState();
}

class _HudScreenState extends State<HudScreen> {
  InAppWebViewController? _webViewController;
  bool _isWebViewReady = false;

  void _pushTelemetryToThreeJs(dynamic telemetryData) {
    if (_webViewController != null && _isWebViewReady) {
      final jsonString = jsonEncode(telemetryData.toJson());
      _webViewController!.evaluateJavascript(
        source: "if (window.updateTelemetry) { window.updateTelemetry($jsonString); }",
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    final telemetry = context.watch<TelemetryService>();
    _pushTelemetryToThreeJs(telemetry.currentData);

    return Scaffold(
      backgroundColor: const Color(0xFF08090C),
      body: Stack(
        children: [
          // 1. Embedded WebGL Three.js Tesla-style Canvas
          InAppWebView(
            initialFile: "web/threejs/index.html",
            initialSettings: InAppWebViewSettings(
              isElementFullscreenEnabled: false,
              supportZoom: false,
              transparentBackground: true,
              useWideViewPort: true,
            ),
            onWebViewCreated: (controller) {
              _webViewController = controller;
            },
            onLoadStop: (controller, url) {
              setState(() {
                _isWebViewReady = true;
              });
              _pushTelemetryToThreeJs(telemetry.currentData);
            },
          ),

          // 2. Overlay Top Bar: Connection & Latency Telemetry
          SafeArea(
            child: Padding(
              padding: const EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Container(
                    padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
                    decoration: BoxDecoration(
                      color: const Color(0xCC12161F),
                      borderRadius: BorderRadius.circular(20),
                      border: Border.all(color: Colors.white10),
                    ),
                    child: Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        Container(
                          width: 8,
                          height: 8,
                          decoration: BoxDecoration(
                            shape: BoxShape.circle,
                            color: telemetry.status == ConnectionStatus.connected
                                ? const Color(0xFF00E676)
                                : const Color(0xFFFF1744),
                          ),
                        ),
                        const SizedBox(width: 8),
                        Text(
                          telemetry.status == ConnectionStatus.connected
                              ? "LIVE • ${telemetry.settings.esp32Ip}"
                              : (telemetry.status == ConnectionStatus.connecting
                                  ? "CONNECTING..."
                                  : "OFFLINE"),
                          style: const TextStyle(
                            color: Colors.white,
                            fontSize: 12,
                            fontWeight: FontWeight.w600,
                          ),
                        ),
                      ],
                    ),
                  ),

                  // Packets & Latency Metric
                  Container(
                    padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
                    decoration: BoxDecoration(
                      color: const Color(0xCC12161F),
                      borderRadius: BorderRadius.circular(20),
                      border: Border.all(color: Colors.white10),
                    ),
                    child: Text(
                      "${telemetry.packetsReceived} pkts • ${telemetry.latencyMs}ms",
                      style: const TextStyle(
                        color: Color(0xFFA0AEC0),
                        fontSize: 12,
                        fontWeight: FontWeight.w500,
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}
