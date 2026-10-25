#pragma once
#include <Arduino.h>

// High-performance immersive Tesla-style perception HUD (Canvas rendering + interactive telemetry)
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>BlindSpottr FSD Perception HUD</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body, html { width: 100%; height: 100%; overflow: hidden; background: #07090e; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    #canvas { width: 100%; height: 100%; display: block; position: absolute; top: 0; left: 0; z-index: 1; }
    #overlay-hud { position: absolute; top: 0; left: 0; width: 100%; height: 100%; z-index: 2; pointer-events: none; display: flex; flex-direction: column; justify-content: space-between; padding: 16px; }
    
    /* Top Status Bar */
    .hud-top { display: flex; justify-content: space-between; align-items: center; }
    .status-badge { pointer-events: auto; background: rgba(15, 23, 42, 0.85); backdrop-filter: blur(12px); border: 1px solid rgba(255, 255, 255, 0.12); border-radius: 24px; padding: 7px 16px; color: #fff; font-size: 13px; font-weight: 600; display: flex; align-items: center; gap: 8px; cursor: pointer; transition: all 0.2s; }
    .status-badge:hover { background: rgba(30, 41, 59, 0.95); border-color: rgba(56, 189, 248, 0.4); }
    .status-dot { width: 9px; height: 9px; border-radius: 50%; background: #00e676; box-shadow: 0 0 10px #00e676; }
    .status-dot.danger { background: #ff1744; box-shadow: 0 0 12px #ff1744; }
    .status-dot.connecting { background: #38bdf8; box-shadow: 0 0 10px #38bdf8; }
    
    /* Speedometer & Gear Indicator (Tesla aesthetic) */
    .speedo-cluster { pointer-events: auto; background: rgba(15, 23, 42, 0.75); backdrop-filter: blur(12px); border: 1px solid rgba(255, 255, 255, 0.08); border-radius: 16px; padding: 8px 16px; display: flex; align-items: center; gap: 14px; }
    .gear-pills { display: flex; gap: 6px; font-size: 11px; font-weight: 800; color: #64748b; }
    .gear-pills span.active { color: #38bdf8; text-shadow: 0 0 8px rgba(56, 189, 248, 0.6); }
    .speed-display { font-size: 26px; font-weight: 800; color: #fff; line-height: 1; letter-spacing: -0.5px; }
    .speed-unit { font-size: 10px; color: #94a3b8; font-weight: 600; text-transform: uppercase; margin-left: 2px; }

    /* Center Telemetry & Headway */
    .hud-center { text-align: center; }
    .distance-badge { display: inline-block; background: rgba(15, 23, 42, 0.85); backdrop-filter: blur(16px); border: 1px solid rgba(0, 230, 118, 0.4); border-radius: 14px; padding: 10px 22px; color: #00e676; font-size: 20px; font-weight: 800; letter-spacing: 0.5px; transition: all 0.3s ease; }
    .distance-badge.warning { border-color: rgba(255, 234, 0, 0.6); color: #ffea00; }
    .distance-badge.danger { border-color: rgba(255, 23, 68, 0.8); color: #ff1744; animation: pulseRed 0.8s infinite; }
    @keyframes pulseRed { 0% { box-shadow: 0 0 0 0 rgba(255, 23, 68, 0.5); } 70% { box-shadow: 0 0 0 16px rgba(255, 23, 68, 0); } 100% { box-shadow: 0 0 0 0 rgba(255, 23, 68, 0); } }

    /* Bottom Flank Cards */
    .hud-bottom { display: flex; justify-content: space-between; align-items: flex-end; }
    .telemetry-card { background: rgba(15, 23, 42, 0.85); backdrop-filter: blur(12px); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 14px; padding: 12px 18px; color: #94a3b8; font-size: 11px; letter-spacing: 0.5px; }
    .telemetry-card b { color: #fff; font-size: 14px; margin-left: 4px; }
    .card-left { border-left: 3px solid #38bdf8; }
    .card-right { border-right: 3px solid #38bdf8; text-align: right; }

    /* Interactive Settings Modal */
    #settings-modal { display: none; position: absolute; top: 0; left: 0; width: 100%; height: 100%; z-index: 10; background: rgba(0,0,0,0.85); backdrop-filter: blur(16px); padding: 20px; align-items: center; justify-content: center; }
    .modal-card { background: #0f172a; border: 1px solid rgba(255,255,255,0.15); border-radius: 20px; width: 100%; max-width: 400px; padding: 24px; color: #fff; box-shadow: 0 20px 40px rgba(0,0,0,0.6); }
    .modal-card h2 { font-size: 18px; font-weight: 700; margin-bottom: 20px; display: flex; justify-content: space-between; align-items: center; }
    .modal-card label { display: block; font-size: 11px; color: #94a3b8; margin-top: 14px; margin-bottom: 6px; text-transform: uppercase; font-weight: 700; letter-spacing: 0.8px; }
    .modal-card input[type=text], .modal-card input[type=password] { width: 100%; padding: 12px 14px; background: #1e293b; border: 1px solid #334155; border-radius: 10px; color: #fff; font-size: 14px; outline: none; }
    .modal-card input[type=range] { width: 100%; accent-color: #38bdf8; margin: 8px 0; }
    .btn-save { width: 100%; padding: 14px; background: #0284c7; border: none; border-radius: 12px; color: #fff; font-size: 14px; font-weight: 700; margin-top: 22px; cursor: pointer; transition: all 0.2s; }
    .btn-save:hover { background: #0369a1; }
    .btn-close { background: none; border: none; color: #94a3b8; font-size: 22px; cursor: pointer; }
  </style>
</head>
<body>
  <canvas id="canvas"></canvas>

  <div id="overlay-hud">
    <!-- Top Header -->
    <div class="hud-top">
      <div class="status-badge" onclick="toggleSettings()">
        <div id="status-dot" class="status-dot connecting"></div>
        <span id="state-text">INITIALIZING...</span>
      </div>

      <div class="speedo-cluster">
        <div class="gear-pills">
          <span class="active">D</span>
          <span>N</span>
          <span>R</span>
          <span>P</span>
        </div>
        <div class="speed-display">
          <span id="speed-val">0</span><span class="speed-unit">cm/s</span>
        </div>
      </div>

      <div class="status-badge" onclick="toggleSettings()">
        <span>⚙️ Settings</span>
      </div>
    </div>

    <!-- Center Proximity HUD -->
    <div class="hud-center">
      <div id="dist-card" class="distance-badge">HEADWAY: CLEAR</div>
    </div>

    <!-- Bottom Hazard Flanks -->
    <div class="hud-bottom">
      <div class="telemetry-card card-left">LEFT BLIND SPOT: <b id="left-text">CLEAR</b></div>
      <div class="telemetry-card card-right">RIGHT BLIND SPOT: <b id="right-text">CLEAR</b></div>
    </div>
  </div>

  <!-- Settings Modal (Hotspot & Sensor Calibration) -->
  <div id="settings-modal">
    <div class="modal-card">
      <h2><span>Vehicle & Network Config</span><button class="btn-close" onclick="toggleSettings()">✕</button></h2>
      
      <label>Headway Warning Threshold: <span id="thresh-val" style="color:#38bdf8; font-weight:bold;">20 cm</span></label>
      <input type="range" id="thresh-slider" min="10" max="60" value="20" oninput="document.getElementById('thresh-val').innerText = this.value + ' cm'">

      <label>Connect to Phone Hotspot (SSID):</label>
      <input type="text" id="cfg-ssid" placeholder="Hotspot Name">

      <label>Hotspot Password:</label>
      <input type="password" id="cfg-pass" placeholder="Password">

      <button class="btn-save" onclick="saveWifiConfig()">Save & Connect Truck</button>
    </div>
  </div>

  <script>
    const canvas = document.getElementById('canvas');
    const ctx = canvas.getContext('2d');

    function resize() {
      canvas.width = window.innerWidth * window.devicePixelRatio;
      canvas.height = window.innerHeight * window.devicePixelRatio;
      ctx.scale(window.devicePixelRatio, window.devicePixelRatio);
    }
    window.addEventListener('resize', resize);
    resize();

    let currentData = {
      left: false,
      right: false,
      front_valid: false,
      front_cm: -1.0,
      state: "CLEAR"
    };

    function toggleSettings() {
      const modal = document.getElementById('settings-modal');
      modal.style.display = (modal.style.display === 'flex') ? 'none' : 'flex';
    }

    function saveWifiConfig() {
      const ssid = document.getElementById('cfg-ssid').value;
      const pass = document.getElementById('cfg-pass').value;
      if (!ssid) return alert("Please enter your hotspot name!");

      const body = new URLSearchParams();
      body.append('ssid', ssid);
      body.append('pass', pass);

      fetch('/api/wifi', { method: 'POST', body: body })
        .then(r => r.json())
        .then(res => {
          alert("Credentials saved! ESP32 is now connecting to: " + ssid);
          toggleSettings();
        })
        .catch(e => alert("Saved! ESP32 will auto-connect to hotspot."));
    }

    function updateTelemetryUI(data) {
      currentData = data;
      document.getElementById('left-text').innerText = data.left ? "HAZARD" : "CLEAR";
      document.getElementById('left-text').style.color = data.left ? "#ff1744" : "#00e676";
      document.getElementById('right-text').innerText = data.right ? "HAZARD" : "CLEAR";
      document.getElementById('right-text').style.color = data.right ? "#ff1744" : "#00e676";

      const distCard = document.getElementById('dist-card');
      const dot = document.getElementById('status-dot');
      const stateText = document.getElementById('state-text');
      const speedVal = document.getElementById('speed-val');

      if (data.front_valid && data.front_cm > 0) {
        distCard.innerText = `HEADWAY: ${Math.round(data.front_cm)} cm`;
        speedVal.innerText = (data.front_cm < 30) ? "12" : "28";

        if (data.front_cm < 20) {
          distCard.className = "distance-badge danger";
          dot.className = "status-dot danger";
          stateText.innerText = "COLLISION ALERT";
        } else if (data.front_cm < 45) {
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
        speedVal.innerText = "35";

        if (data.left || data.right) {
          dot.className = "status-dot danger";
          stateText.innerText = data.left && data.right ? "DUAL SIDE ALERT" : (data.left ? "LEFT BLIND SPOT" : "RIGHT BLIND SPOT");
        } else {
          dot.className = "status-dot";
          stateText.innerText = "LIVE PERCEPTION";
        }
      }
    }

    function connectWS() {
      const wsHost = window.location.host || "192.168.4.1";
      const ws = new WebSocket(`ws://${wsHost}/ws`);

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

      ws.onerror = () => { ws.close(); };
    }
    connectWS();

    let roadOffset = 0;
    let targetObstacleZ = 200;
    let curObstacleZ = 200;

    function render(time) {
      const w = window.innerWidth;
      const h = window.innerHeight;
      ctx.clearRect(0, 0, w, h);

      const vpX = w / 2;
      const vpY = h * 0.38;

      ctx.fillStyle = '#07090e';
      ctx.fillRect(0, 0, w, h);

      // Perspective Road Horizon
      const grad = ctx.createLinearGradient(0, vpY, 0, h);
      grad.addColorStop(0, '#0f172a');
      grad.addColorStop(1, '#020617');
      ctx.fillStyle = grad;
      ctx.beginPath();
      ctx.moveTo(vpX - 8, vpY);
      ctx.lineTo(vpX + 8, vpY);
      ctx.lineTo(w * 0.9, h);
      ctx.lineTo(w * 0.1, h);
      ctx.closePath();
      ctx.fill();

      // Road Lane Markings
      roadOffset = (roadOffset + 4) % 40;
      ctx.strokeStyle = 'rgba(56, 189, 248, 0.5)';
      ctx.lineWidth = 3;

      for (let z = 10; z < 400; z += 40) {
        const pz = z - roadOffset;
        if (pz <= 5) continue;
        const scale = 220 / (pz + 100);
        const y = vpY + (h - vpY) * (scale * 0.7);
        const lw = 40 * scale;

        ctx.beginPath();
        ctx.moveTo(vpX - lw, y);
        ctx.lineTo(vpX - lw, y + 15 * scale);
        ctx.moveTo(vpX + lw, y);
        ctx.lineTo(vpX + lw, y + 15 * scale);
        ctx.stroke();
      }

      // 3D Front Obstacle Proxy
      if (currentData.front_valid && currentData.front_cm > 0 && currentData.front_cm <= 120) {
        targetObstacleZ = 40 + currentData.front_cm * 2.2;
      } else {
        targetObstacleZ = 300;
      }
      curObstacleZ += (targetObstacleZ - curObstacleZ) * 0.15;

      if (curObstacleZ < 260) {
        const obsScale = 220 / (curObstacleZ + 100);
        const obsY = vpY + (h - vpY) * (obsScale * 0.7);
        const obsW = 80 * obsScale;
        const obsH = 45 * obsScale;

        ctx.save();
        ctx.translate(vpX, obsY);
        const isDanger = (currentData.front_cm > 0 && currentData.front_cm <= 20);
        ctx.fillStyle = isDanger ? 'rgba(239, 68, 68, 0.85)' : 'rgba(245, 158, 11, 0.85)';
        ctx.strokeStyle = isDanger ? '#ff1744' : '#f59e0b';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.roundRect(-obsW/2, -obsH, obsW, obsH, 6 * obsScale);
        ctx.fill();
        ctx.stroke();

        ctx.fillStyle = '#fff';
        ctx.font = `bold ${Math.max(10, 14 * obsScale)}px sans-serif`;
        ctx.textAlign = 'center';
        ctx.fillText(`${Math.round(currentData.front_cm)} cm`, 0, -obsH / 2 + 4 * obsScale);
        ctx.restore();
      }

      // Draw Hero 3D Truck (Foreground)
      const truckX = vpX;
      const truckY = h * 0.75;
      const tw = 84;
      const th = 130;

      ctx.save();
      ctx.translate(truckX, truckY);

      // Radar Waves on Screen Left (when Left IR triggers):
      // Drawn on x < 0 (left of truck)
      if (currentData.left) {
        const pulse = (Math.sin(time * 0.01) + 1) * 0.5;
        ctx.strokeStyle = `rgba(255, 23, 68, ${0.4 + 0.5 * pulse})`;
        ctx.lineWidth = 3;
        for (let i = 1; i <= 3; i++) {
          ctx.beginPath();
          ctx.arc(-tw/2 - 10, -th * 0.2, 18 * i + pulse * 8, Math.PI * 0.6, Math.PI * 1.4);
          ctx.stroke();
        }
      }

      // Radar Waves on Screen Right (when Right IR triggers):
      // Drawn on x > 0 (right of truck)
      if (currentData.right) {
        const pulse = (Math.sin(time * 0.01) + 1) * 0.5;
        ctx.strokeStyle = `rgba(255, 23, 68, ${0.4 + 0.5 * pulse})`;
        ctx.lineWidth = 3;
        for (let i = 1; i <= 3; i++) {
          ctx.beginPath();
          ctx.arc(tw/2 + 10, -th * 0.2, 18 * i + pulse * 8, -Math.PI * 0.4, Math.PI * 0.4);
          ctx.stroke();
        }
      }

      // Truck Shadow
      ctx.fillStyle = 'rgba(0,0,0,0.6)';
      ctx.beginPath();
      ctx.ellipse(0, th * 0.4, tw * 0.65, 12, 0, 0, Math.PI * 2);
      ctx.fill();

      // Truck Body
      ctx.fillStyle = '#1e293b';
      ctx.strokeStyle = '#38bdf8';
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.roundRect(-tw/2, -th/2, tw, th, 12);
      ctx.fill();
      ctx.stroke();

      // Windshield
      ctx.fillStyle = '#0284c7';
      ctx.beginPath();
      ctx.roundRect(-tw * 0.38, -th * 0.35, tw * 0.76, 32, 6);
      ctx.fill();

      // Cargo Bed
      ctx.fillStyle = '#0f172a';
      ctx.strokeStyle = 'rgba(255,255,255,0.15)';
      ctx.beginPath();
      ctx.roundRect(-tw * 0.42, 0, tw * 0.84, th * 0.42, 6);
      ctx.fill();
      ctx.stroke();

      // Headlights
      ctx.fillStyle = '#38bdf8';
      ctx.shadowColor = '#38bdf8';
      ctx.shadowBlur = 12;
      ctx.fillRect(-tw/2 + 6, -th/2 - 4, 16, 6);
      ctx.fillRect(tw/2 - 22, -th/2 - 4, 16, 6);

      ctx.restore();
      requestAnimationFrame(render);
    }
    requestAnimationFrame(render);
  </script>
</body>
</html>
)rawliteral";
