import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'services/telemetry_service.dart';
import 'views/hud/hud_screen.dart';
import 'views/settings/settings_screen.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(
    MultiProvider(
      providers: [
        ChangeNotifierProvider(create: (_) => TelemetryService()..connect()),
      ],
      child: const BlindSpottrApp(),
    ),
  );
}

class BlindSpottrApp extends StatelessWidget {
  const BlindSpottrApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'BlindSpottr FSD',
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark().copyWith(
        scaffoldBackgroundColor: const Color(0xFF08090C),
        primaryColor: const Color(0xFF0284C7),
      ),
      home: const MainNavigationShell(),
    );
  }
}

class MainNavigationShell extends StatefulWidget {
  const MainNavigationShell({super.key});

  @override
  State<MainNavigationShell> createState() => _MainNavigationShellState();
}

class _MainNavigationShellState extends State<MainNavigationShell> {
  int _currentIndex = 0;

  final List<Widget> _screens = const [
    HudScreen(),
    SettingsScreen(),
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: IndexedStack(
        index: _currentIndex,
        children: _screens,
      ),
      bottomNavigationBar: NavigationBarTheme(
        data: NavigationBarThemeData(
          backgroundColor: const Color(0xFF0F172A),
          indicatorColor: const Color(0xFF0284C7).withOpacity(0.25),
          labelTextStyle: WidgetStateProperty.all(
            const TextStyle(fontSize: 12, fontWeight: FontWeight.w600, color: Colors.white),
          ),
        ),
        child: NavigationBar(
          selectedIndex: _currentIndex,
          onDestinationSelected: (index) {
            setState(() {
              _currentIndex = index;
            });
          },
          destinations: const [
            NavigationDestination(
              icon: Icon(Icons.navigation_outlined, color: Color(0xFFA0AEC0)),
              selectedIcon: Icon(Icons.navigation, color: Color(0xFF38BDF8)),
              label: '3D Telemetry',
            ),
            NavigationDestination(
              icon: Icon(Icons.settings_outlined, color: Color(0xFFA0AEC0)),
              selectedIcon: Icon(Icons.settings, color: Color(0xFF38BDF8)),
              label: 'Settings',
            ),
          ],
        ),
      ),
    );
  }
}
