#pragma once
#include <Arduino.h>

// Self-contained, production-grade 3D Tesla-style HUD served directly from ESP32 flash memory
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>BlindSpottr Tesla 3D HUD</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body, html { width: 100%; height: 100%; overflow: hidden; background: #07090e; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    #canvas-container { width: 100%; height: 100%; position: absolute; top: 0; left: 0; z-index: 1; }
    #overlay-hud { position: absolute; top: 0; left: 0; width: 100%; height: 100%; z-index: 2; pointer-events: none; display: flex; flex-direction: column; justify-content: space-between; padding: 16px; }
    .hud-top { display: flex; justify-content: space-between; align-items: center; }
    .status-badge { background: rgba(18, 22, 31, 0.85); backdrop-filter: blur(8px); border: 1px solid rgba(255, 255, 255, 0.12); border-radius: 20px; padding: 7px 14px; color: #fff; font-size: 13px; font-weight: 600; display: flex; align-items: center; gap: 8px; }
    .status-dot { width: 8px; height: 8px; border-radius: 50%; background: #00e676; box-shadow: 0 0 8px #00e676; }
    .status-dot.danger { background: #ff1744; box-shadow: 0 0 10px #ff1744; }
    .status-dot.warning { background: #ffea00; box-shadow: 0 0 10px #ffea00; }
    .status-dot.connecting { background: #38bdf8; box-shadow: 0 0 10px #38bdf8; }
    .hud-center { text-align: center; }
    .distance-badge { display: inline-block; background: rgba(10, 14, 22, 0.85); backdrop-filter: blur(12px); border: 1px solid rgba(0, 230, 118, 0.4); border-radius: 12px; padding: 8px 18px; color: #00e676; font-size: 19px; font-weight: 700; letter-spacing: 0.5px; transition: all 0.3s ease; }
    .distance-badge.warning { border-color: rgba(255, 234, 0, 0.6); color: #ffea00; }
    .distance-badge.danger { border-color: rgba(255, 23, 68, 0.8); color: #ff1744; animation: pulseRed 0.8s infinite; }
    @keyframes pulseRed { 0% { box-shadow: 0 0 0 0 rgba(255, 23, 68, 0.4); } 70% { box-shadow: 0 0 0 14px rgba(255, 23, 68, 0); } 100% { box-shadow: 0 0 0 0 rgba(255, 23, 68, 0); } }
    .hud-bottom { display: flex; justify-content: space-between; align-items: flex-end; }
    .telemetry-card { background: rgba(18, 22, 31, 0.85); backdrop-filter: blur(8px); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 12px; padding: 10px 14px; color: #a0aec0; font-size: 11px; }
    .telemetry-card b { color: #fff; font-size: 13px; }
  </style>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>
</head>
<body>
  <div id="canvas-container"></div>
  <div id="overlay-hud">
    <div class="hud-top">
      <div class="status-badge">
        <div id="status-dot" class="status-dot connecting"></div>
        <span id="state-text">CONNECTING TO TRUCK...</span>
      </div>
      <div class="status-badge">
        <span id="rate-text">ws://192.168.4.1/ws</span>
      </div>
    </div>
    <div class="hud-center">
      <div id="dist-card" class="distance-badge">HEADWAY: CLEAR</div>
    </div>
    <div class="hud-bottom">
      <div class="telemetry-card">LEFT BLIND SPOT: <b id="left-text">CLEAR</b></div>
      <div class="telemetry-card">RIGHT BLIND SPOT: <b id="right-text">CLEAR</b></div>
    </div>
  </div>

  <script>
    // 1. Three.js Tesla-Style 3D Scene Initialization
    const container = document.getElementById('canvas-container');
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x07090e);
    scene.fog = new THREE.FogExp2(0x07090e, 0.025);

    const camera = new THREE.PerspectiveCamera(55, window.innerWidth / window.innerHeight, 0.1, 1000);
    camera.position.set(0, 7.5, 12);
    camera.lookAt(0, 1.2, -2);

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: false });
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.toneMapping = THREE.ACESFilmicToneMapping;
    renderer.toneMappingExposure = 1.1;
    container.appendChild(renderer.domElement);

    // Dynamic Lighting
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.4);
    scene.add(ambientLight);

    const dirLight = new THREE.DirectionalLight(0xa5c9ff, 1.2);
    dirLight.position.set(8, 16, 10);
    scene.add(dirLight);

    const hemiLight = new THREE.HemisphereLight(0x3b82f6, 0x050505, 0.6);
    scene.add(hemiLight);

    // 2. Procedural Moving Road & Grid
    const roadGroup = new THREE.Group();
    scene.add(roadGroup);

    const groundGeo = new THREE.PlaneGeometry(80, 200);
    const groundMat = new THREE.MeshStandardMaterial({ color: 0x090c14, roughness: 0.9, metalness: 0.1 });
    const ground = new THREE.Mesh(groundGeo, groundMat);
    ground.rotation.x = -Math.PI / 2;
    ground.position.y = -0.05;
    roadGroup.add(ground);

    const gridHelper = new THREE.GridHelper(100, 50, 0x1f293d, 0x111827);
    gridHelper.position.y = 0;
    scene.add(gridHelper);

    const laneMarkers = [];
    const laneGeo = new THREE.PlaneGeometry(0.35, 3.5);
    const laneMat = new THREE.MeshBasicMaterial({ color: 0x3b82f6, transparent: true, opacity: 0.7 });
    for (let i = 0; i < 20; i++) {
      const leftMarker = new THREE.Mesh(laneGeo, laneMat);
      leftMarker.rotation.x = -Math.PI / 2;
      leftMarker.position.set(-2.8, 0.01, (i * 8) - 60);
      roadGroup.add(leftMarker);
      laneMarkers.push(leftMarker);

      const rightMarker = new THREE.Mesh(laneGeo, laneMat);
      rightMarker.rotation.x = -Math.PI / 2;
      rightMarker.position.set(2.8, 0.01, (i * 8) - 60);
      roadGroup.add(rightMarker);
      laneMarkers.push(rightMarker);
    }

    // 3. Authored 3D Scale Truck Model
    const truckGroup = new THREE.Group();
    scene.add(truckGroup);

    const cabMat = new THREE.MeshStandardMaterial({ color: 0x1e293b, roughness: 0.35, metalness: 0.6 });
    const cabGeo = new THREE.BoxGeometry(2.2, 1.6, 2.8);
    const cab = new THREE.Mesh(cabGeo, cabMat);
    cab.position.set(0, 1.2, 0.8);
    truckGroup.add(cab);

    const glassMat = new THREE.MeshPhysicalMaterial({ color: 0x0284c7, transmission: 0.8, opacity: 0.9, transparent: true, roughness: 0.1 });
    const windGeo = new THREE.BoxGeometry(2.0, 0.7, 0.05);
    const wind = new THREE.Mesh(windGeo, glassMat);
    wind.position.set(0, 1.5, -0.62);
    wind.rotation.x = 0.25;
    truckGroup.add(wind);

    const bedMat = new THREE.MeshStandardMaterial({ color: 0x0f172a, roughness: 0.7, metalness: 0.4 });
    const bedGeo = new THREE.BoxGeometry(2.1, 1.1, 3.2);
    const bed = new THREE.Mesh(bedGeo, bedMat);
    bed.position.set(0, 1.05, 3.4);
    truckGroup.add(bed);

    const wheelMat = new THREE.MeshStandardMaterial({ color: 0x0a0a0a, roughness: 0.8 });
    const wheelGeo = new THREE.CylinderGeometry(0.45, 0.45, 0.4, 24);
    const wheelPositions = [
      [-1.15, 0.45, 0.8], [1.15, 0.45, 0.8],
      [-1.15, 0.45, 3.2], [1.15, 0.45, 3.2],
      [-1.15, 0.45, 4.2], [1.15, 0.45, 4.2]
    ];
    wheelPositions.forEach(pos => {
      const wheel = new THREE.Mesh(wheelGeo, wheelMat);
      wheel.rotation.z = Math.PI / 2;
      wheel.position.set(...pos);
      truckGroup.add(wheel);
    });

    const lightGlowMat = new THREE.MeshBasicMaterial({ color: 0x38bdf8 });
    const headGeo = new THREE.BoxGeometry(0.4, 0.2, 0.1);
    const headL = new THREE.Mesh(headGeo, lightGlowMat);
    headL.position.set(-0.8, 0.9, -0.6);
    truckGroup.add(headL);
    const headR = new THREE.Mesh(headGeo, lightGlowMat);
    headR.position.set(0.8, 0.9, -0.6);
    truckGroup.add(headR);

    // 4. Volumetric Blind-Spot Radar Wave Shaders
    function createWaveMesh(isLeft) {
      const waveGroup = new THREE.Group();
      for (let i = 0; i < 3; i++) {
        const ringGeo = new THREE.RingGeometry(0.8 + i * 0.9, 1.0 + i * 0.9, 32, 1, Math.PI * 0.2, Math.PI * 0.6);
        const ringMat = new THREE.MeshBasicMaterial({
          color: 0xef4444,
          transparent: true,
          opacity: 0.0,
          side: THREE.DoubleSide
        });
        const ring = new THREE.Mesh(ringGeo, ringMat);
        ring.rotation.x = -Math.PI / 2;
        ring.position.y = 0.4;
        if (!isLeft) ring.rotation.z = Math.PI;
        waveGroup.add(ring);
      }
      waveGroup.position.set(isLeft ? -1.5 : 1.5, 0.2, 1.2);
      scene.add(waveGroup);
      return waveGroup;
    }

    const leftRadarWaves = createWaveMesh(true);
    const rightRadarWaves = createWaveMesh(false);

    // 5. Dynamic 3D Obstacle Proxy (Front Vehicle)
    const obstacleGeo = new THREE.BoxGeometry(2.0, 1.4, 3.2);
    const obstacleMat = new THREE.MeshStandardMaterial({
      color: 0xd97706,
      roughness: 0.4,
      metalness: 0.5,
      transparent: true,
      opacity: 0.0
    });
    const obstacleVehicle = new THREE.Mesh(obstacleGeo, obstacleMat);
    obstacleVehicle.position.set(0, 1.1, -12);
    scene.add(obstacleVehicle);

    const wireGeo = new THREE.EdgesGeometry(obstacleGeo);
    const wireMat = new THREE.LineBasicMaterial({ color: 0xf59e0b, linewidth: 2, transparent: true, opacity: 0.0 });
    const wireframeBox = new THREE.LineSegments(wireGeo, wireMat);
    obstacleVehicle.add(wireframeBox);

    // 6. Live Telemetry Processing & Native WebSocket Client
    let currentData = {
      left: false,
      right: false,
      front_valid: false,
      front_cm: -1.0,
      state: "CLEAR"
    };

    function updateTelemetryUI(data) {
      currentData = data;

      document.getElementById('left-text').innerText = data.left ? "HAZARD" : "CLEAR";
      document.getElementById('left-text').style.color = data.left ? "#ff1744" : "#00e676";
      document.getElementById('right-text').innerText = data.right ? "HAZARD" : "CLEAR";
      document.getElementById('right-text').style.color = data.right ? "#ff1744" : "#00e676";

      const distCard = document.getElementById('dist-card');
      const dot = document.getElementById('status-dot');
      const stateText = document.getElementById('state-text');

      if (data.front_valid && data.front_cm > 0) {
        distCard.innerText = `HEADWAY: ${Math.round(data.front_cm)} cm`;
        if (data.front_cm < 20) {
          distCard.className = "distance-badge danger";
          dot.className = "status-dot danger";
          stateText.innerText = "COLLISION WARNING";
        } else if (data.front_cm < 40) {
          distCard.className = "distance-badge warning";
          dot.className = "status-dot warning";
          stateText.innerText = "CAUTION DISTANCE";
        } else {
          distCard.className = "distance-badge";
          dot.className = "status-dot";
          stateText.innerText = "CLEAR CORRIDOR";
        }
      } else {
        distCard.innerText = "HEADWAY: CLEAR";
        distCard.className = "distance-badge";
        if (data.left || data.right) {
          dot.className = "status-dot danger";
          stateText.innerText = data.left && data.right ? "DUAL BLIND SPOT ALERT" : (data.left ? "LEFT BLIND SPOT" : "RIGHT BLIND SPOT");
        } else {
          dot.className = "status-dot";
          stateText.innerText = "SYSTEM ACTIVE";
        }
      }
    }

    // Connect to ESP32 WebSocket
    function connectWS() {
      const wsUrl = `ws://${window.location.hostname || "192.168.4.1"}/ws`;
      const ws = new WebSocket(wsUrl);

      ws.onopen = () => {
        document.getElementById('status-dot').className = "status-dot";
        document.getElementById('state-text').innerText = "LIVE PERCEPTION";
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          if (data && data.state) {
            updateTelemetryUI(data);
          }
        } catch (e) {}
      };

      ws.onclose = () => {
        document.getElementById('status-dot').className = "status-dot connecting";
        document.getElementById('state-text').innerText = "RECONNECTING...";
        setTimeout(connectWS, 2000);
      };

      ws.onerror = () => {
        ws.close();
      };
    }
    connectWS();

    // 7. Render Animation Loop
    let clock = new THREE.Clock();
    function animate() {
      requestAnimationFrame(animate);
      const delta = clock.getDelta();
      const elapsed = clock.getElapsedTime();

      // Animate road stripes backward
      laneMarkers.forEach(m => {
        m.position.z += 18 * delta;
        if (m.position.z > 25) {
          m.position.z = -55;
        }
      });

      // Chassis oscillation
      truckGroup.position.y = Math.sin(elapsed * 4) * 0.02;
      truckGroup.rotation.z = Math.sin(elapsed * 2) * 0.005;

      // Animate Left Radar Waves
      leftRadarWaves.children.forEach((mesh, idx) => {
        if (currentData.left) {
          mesh.material.opacity = 0.3 + 0.4 * Math.sin(elapsed * 8 + idx * 1.5);
          mesh.material.color.setHex(0xff1744);
        } else {
          mesh.material.opacity = Math.max(0, mesh.material.opacity - delta * 4);
        }
      });

      // Animate Right Radar Waves
      rightRadarWaves.children.forEach((mesh, idx) => {
        if (currentData.right) {
          mesh.material.opacity = 0.3 + 0.4 * Math.sin(elapsed * 8 + idx * 1.5);
          mesh.material.color.setHex(0xff1744);
        } else {
          mesh.material.opacity = Math.max(0, mesh.material.opacity - delta * 4);
        }
      });

      // Animate Dynamic Front 3D Obstacle Proxy
      if (currentData.front_valid && currentData.front_cm > 0 && currentData.front_cm <= 150) {
        const targetZ = -2.8 - (currentData.front_cm / 6.0);
        obstacleVehicle.position.z += (targetZ - obstacleVehicle.position.z) * 0.15;
        obstacleVehicle.material.opacity = THREE.MathUtils.lerp(obstacleVehicle.material.opacity, 0.85, 0.1);
        wireMat.opacity = THREE.MathUtils.lerp(wireMat.opacity, 1.0, 0.1);

        if (currentData.front_cm <= 20) {
          obstacleMat.color.setHex(0xef4444);
          wireMat.color.setHex(0xff1744);
        } else {
          obstacleMat.color.setHex(0xd97706);
          wireMat.color.setHex(0xf59e0b);
        }
      } else {
        obstacleVehicle.material.opacity = Math.max(0, obstacleVehicle.material.opacity - delta * 3);
        wireMat.opacity = Math.max(0, wireMat.opacity - delta * 3);
      }

      renderer.render(scene, camera);
    }
    animate();

    window.addEventListener('resize', () => {
      camera.aspect = window.innerWidth / window.innerHeight;
      camera.updateProjectionMatrix();
      renderer.setSize(window.innerWidth, window.innerHeight);
    });
  </script>
</body>
</html>
)rawliteral";
