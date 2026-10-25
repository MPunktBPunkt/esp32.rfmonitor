#pragma once
#include <Arduino.h>

const char PAGE_MAIN[] PROGMEM = R"RFUI(<!DOCTYPE html>
<html lang="de"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>RF Monitor</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500;600&family=Syne:wght@600;700;800&display=swap" rel="stylesheet">
<style>
:root{
  --bg:#0b1210;--bg2:#121a17;--panel:#16201c;--line:#24332c;
  --ink:#e8f0ea;--muted:#8aa094;--accent:#2dd4a8;--accent2:#f0b429;
  --bad:#ef5b5b;--ok:#3ecf8e;--warn:#f0b429;
}
*{box-sizing:border-box;margin:0;padding:0}
body{background:radial-gradient(1200px 600px at 10% -10%,#143528 0%,transparent 55%),
 radial-gradient(900px 500px at 100% 0%,#1a2a18 0%,transparent 50%),var(--bg);
 color:var(--ink);font:14px/1.45 "IBM Plex Mono",ui-monospace,monospace;min-height:100vh}
a{color:var(--accent)}
.shell{max-width:1180px;margin:0 auto;padding:0 16px 40px}
.brand{padding:28px 0 8px;display:grid;gap:10px}
.brand-mark{font-family:Syne,sans-serif;font-weight:800;font-size:clamp(2rem,5vw,3.2rem);
 letter-spacing:-.03em;line-height:.95;color:var(--ink)}
.brand-mark span{color:var(--accent)}
.brand-sub{color:var(--muted);max-width:42rem;font-size:13px}
.topline{display:flex;flex-wrap:wrap;gap:10px;align-items:center;margin-top:8px}
.chip{border:1px solid var(--line);background:rgba(22,32,28,.85);padding:5px 10px;font-size:11px;color:var(--muted)}
.chip b{color:var(--ink);font-weight:600}
.live{display:inline-flex;align-items:center;gap:7px;margin-left:auto}
.pulse{width:8px;height:8px;border-radius:50%;background:var(--bad)}
.pulse.on{background:var(--ok);box-shadow:0 0 0 0 rgba(62,207,142,.5);animation:ping 1.6s infinite}
@keyframes ping{0%{box-shadow:0 0 0 0 rgba(62,207,142,.45)}70%{box-shadow:0 0 0 8px rgba(62,207,142,0)}100%{box-shadow:0 0 0 0 rgba(62,207,142,0)}}
.spark{width:120px;height:28px;border:1px solid var(--line);background:#0a100e}
nav{display:flex;gap:2px;overflow:auto;border-bottom:1px solid var(--line);margin:18px 0 20px;padding-bottom:0}
.tab{appearance:none;border:0;background:transparent;color:var(--muted);padding:12px 14px;cursor:pointer;
 font:inherit;border-bottom:2px solid transparent;white-space:nowrap}
.tab:hover{color:var(--ink)}.tab.active{color:var(--accent);border-bottom-color:var(--accent)}
.pane{display:none}.pane.active{display:block}
.hero{display:grid;grid-template-columns:220px 1fr;gap:18px;margin-bottom:18px}
@media(max-width:820px){.hero{grid-template-columns:1fr}}
.gauge-wrap{background:linear-gradient(160deg,rgba(45,212,168,.08),transparent 40%),var(--panel);
 border:1px solid var(--line);padding:18px;display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:220px}
.gauge{width:180px;height:180px}
.gauge-lab{margin-top:8px;font-size:11px;color:var(--muted);letter-spacing:.08em;text-transform:uppercase}
.kpi-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(130px,1fr));gap:10px}
.kpi{background:var(--panel);border:1px solid var(--line);padding:12px}
.kpi .v{font-family:Syne,sans-serif;font-size:1.55rem;font-weight:700;color:var(--accent);line-height:1}
.kpi .k{margin-top:6px;font-size:11px;color:var(--muted);text-transform:uppercase;letter-spacing:.06em}
.panel{background:var(--panel);border:1px solid var(--line);padding:16px;margin-bottom:14px}
.panel h3{font-family:Syne,sans-serif;font-size:13px;font-weight:700;letter-spacing:.04em;
 text-transform:uppercase;color:var(--muted);margin-bottom:12px}
.grid2{display:grid;grid-template-columns:1fr 1fr;gap:14px}
@media(max-width:800px){.grid2{grid-template-columns:1fr}}
table{width:100%;border-collapse:collapse;font-size:12px}
th,td{padding:8px 6px;border-bottom:1px solid var(--line);text-align:left;vertical-align:top}
th{color:var(--muted);font-weight:500;text-transform:uppercase;font-size:10px;letter-spacing:.06em}
.mono{color:var(--accent)}
canvas{width:100%;height:150px;background:#0a100e;border:1px solid var(--line)}
.btn{appearance:none;border:1px solid var(--line);background:#1a2923;color:var(--ink);
 padding:8px 14px;cursor:pointer;font:inherit;margin:6px 6px 0 0}
.btn:hover{border-color:var(--accent);color:var(--accent)}
.btn-a{background:rgba(45,212,168,.12);border-color:rgba(45,212,168,.45);color:var(--accent)}
.btn-r{border-color:rgba(239,91,91,.45);color:var(--bad)}
input,select,textarea{width:100%;background:#0a100e;border:1px solid var(--line);color:var(--ink);
 padding:8px;font:inherit;border-radius:0}
label{display:block;color:var(--muted);font-size:11px;margin:10px 0 4px;text-transform:uppercase;letter-spacing:.05em}
.row{display:flex;gap:10px;flex-wrap:wrap;align-items:end}
.row>div{flex:1;min-width:120px}
.warn{border:1px solid rgba(240,180,41,.35);background:rgba(240,180,41,.08);color:var(--warn);padding:10px;margin-bottom:12px;font-size:12px}
.drop{border:1px dashed var(--line);padding:28px;text-align:center;cursor:pointer;color:var(--muted)}
.drop:hover{border-color:var(--accent);color:var(--ink)}
.bar{height:8px;background:#0a100e;border:1px solid var(--line);overflow:hidden}
.bar>i{display:block;height:100%;background:var(--accent)}
.heat{display:grid;grid-template-columns:repeat(14,1fr);gap:4px;align-items:end;height:120px;margin-top:8px}
.heat .col{display:flex;flex-direction:column;justify-content:flex-end;height:100%;gap:4px;position:relative;cursor:default}
.heat .stk{flex:1;background:#0a100e;border:1px solid var(--line);display:flex;align-items:flex-end}
.heat .stk i{display:block;width:100%;background:linear-gradient(180deg,var(--accent2),var(--accent));transition:height .35s ease}
.heat .n{text-align:center;font-size:10px;color:var(--muted)}
.heat .tip{display:none;position:absolute;bottom:calc(100% + 6px);left:50%;transform:translateX(-50%);
 background:#0a100e;border:1px solid var(--line);padding:8px 10px;font-size:11px;white-space:nowrap;z-index:5;color:var(--ink)}
.heat .col:hover .tip{display:block}
.dq{font-size:11px;color:var(--muted);margin-top:6px;letter-spacing:.06em;text-transform:uppercase}
.dq b{color:var(--accent);font-weight:600}
.break{display:grid;gap:8px}
.break .rowb{display:grid;grid-template-columns:90px 1fr 36px;gap:8px;align-items:center;font-size:12px}
.break .meter{height:10px;background:#0a100e;border:1px solid var(--line)}
.break .meter>i{display:block;height:100%;background:var(--accent2)}
.tag{font-size:10px;padding:2px 6px;border:1px solid var(--line);color:var(--muted);text-transform:uppercase}
.tag.k{color:var(--ok);border-color:rgba(62,207,142,.45)}
.tag.u{color:var(--warn);border-color:rgba(240,180,41,.45)}
.tag.b{color:var(--accent);border-color:rgba(45,212,168,.45)}
.term{background:#070b09;border:1px solid var(--line);padding:10px;max-height:360px;overflow:auto;
 font-size:12px}
.term .e{display:grid;grid-template-columns:56px 1fr;gap:8px;padding:4px 0;border-bottom:1px solid #122018}
.term .e:last-child{border:0}
.term .t{color:var(--muted)}
.term .lvl0{color:var(--bad)}.term .lvl1{color:var(--warn)}.term .lvl2{color:var(--muted)}
.meta{color:var(--muted);font-size:12px}
tr.own{background:rgba(45,212,168,.08)}
.tl-wrap{height:160px;background:#0a100e;border:1px solid var(--line);position:relative}
.tl-wrap canvas{width:100%;height:160px;border:0;display:block}
.seg{display:inline-flex;gap:4px;flex-wrap:wrap}
.seg button{appearance:none;border:1px solid var(--line);background:#0a100e;color:var(--muted);padding:6px 10px;font:inherit;cursor:pointer}
.seg button.on{border-color:var(--accent);color:var(--accent)}
.filterbar{display:flex;gap:8px;flex-wrap:wrap;align-items:end;margin:8px 0}
.scanline{pointer-events:none;position:fixed;inset:0;background:repeating-linear-gradient(
 transparent 0 2px,rgba(0,0,0,.035) 2px 3px);z-index:0;opacity:.35}
.shell>*{position:relative;z-index:1}
</style></head><body>
<div class="scanline"></div>
<div class="shell">
  <header class="brand">
    <div class="brand-mark">RF <span>Monitor</span></div>
    <p class="brand-sub">Stationärer RF-Observability-Node · WLAN/BLE-Scan, Score, Messqualität, History und Hub.</p>
    <div class="topline">
      <span class="chip" id="h-ver">v-</span>
      <span class="chip" id="h-board">-</span>
      <span class="chip" id="h-hw">-</span>
      <span class="chip"><b id="h-name">-</b></span>
      <span class="chip" id="h-loc">-</span>
      <span class="chip">CH <b id="h-ch">-</b></span>
      <div class="live">
        <canvas class="spark" id="spark" width="120" height="28"></canvas>
        <span class="pulse" id="sse-dot"></span>
        <span id="sse-lbl" class="meta">Verbinde…</span>
      </div>
    </div>
  </header>
  <nav>
    <button class="tab active" data-tab="status">Lage</button>
    <button class="tab" data-tab="rf">RF</button>
    <button class="tab" data-tab="wifi">WLAN</button>
    <button class="tab" data-tab="ble">BLE</button>
    <button class="tab" data-tab="history">History</button>
    <button class="tab" data-tab="events">Events</button>
    <button class="tab" data-tab="config">Config</button>
    <button class="tab" data-tab="ota">OTA</button>
  </nav>

  <div class="pane active" id="pane-status">
    <div class="hero">
      <div class="gauge-wrap">
        <canvas class="gauge" id="g-score" width="180" height="180"></canvas>
        <div class="gauge-lab">RF-Score <span id="g-sv"></span></div>
        <div class="dq">Data Quality <b id="g-dq">-</b> · Δ Loc <b id="g-dev">-</b> · Radio 5m <b id="g-duty">-</b>%</div>
      </div>
      <div>
        <div class="kpi-grid" id="kpi-grid"></div>
        <div class="panel" style="margin-top:14px">
          <h3>Geraet</h3>
          <table>
            <tr><td>Chip / Board</td><td id="d-board">-</td></tr>
            <tr><td>MAC</td><td class="mono" id="d-mac">-</td></tr>
            <tr><td>IP</td><td class="mono" id="d-ip">-</td></tr>
            <tr><td>SSID</td><td id="d-ssid">-</td></tr>
            <tr><td>Uptime</td><td id="d-uptime">-</td></tr>
            <tr><td>Hub</td><td id="d-hubok">-</td></tr>
          </table>
          <div style="margin-top:12px" class="meta">RAM <span id="d-heap">-</span></div>
          <div class="bar" style="margin-top:4px"><i id="b-heap"></i></div>
        </div>
      </div>
    </div>
    <div class="grid2">
      <div class="panel">
        <h3>Why this score?</h3>
        <div class="meta" id="why-sum">—</div>
        <div class="break" id="why-break" style="margin-top:8px"></div>
      </div>
      <div class="panel">
        <h3>Aktuelle Befunde / Alerts</h3>
        <div id="alert-conds" class="break" style="margin-bottom:8px"></div>
        <div id="findings" class="break"></div>
      </div>
    </div>
    <div class="panel">
      <h3>RF Timeline</h3>
      <div class="meta" id="tl-leg">Score · AP · BLE · Events</div>
      <div class="tl-wrap"><canvas id="c-timeline" width="800" height="160"></canvas></div>
    </div>
    <div class="panel">
      <h3>Kanal-Heatmap 2.4 GHz <span class="meta">(AP-Druck-Proxy, nicht Airtime)</span></h3>
      <div class="heat" id="ch-heat"></div>
    </div>
  </div>

  <div class="pane" id="pane-rf">
    <div class="grid2">
      <div class="panel"><h3>WLAN RSSI (dBm)</h3><canvas id="c-rssi"></canvas></div>
      <div class="panel"><h3>RF-Score</h3><canvas id="c-score"></canvas></div>
    </div>
    <div class="panel">
      <h3>Score-Breakdown (Abzuege) · Proxies markiert</h3>
      <div class="break" id="score-break"></div>
      <div id="rf-warn" class="warn" style="display:none;margin-top:12px"></div>
      <button class="btn btn-a" onclick="doWifiScan()">WLAN-Scan</button>
      <button class="btn btn-a" onclick="doBleScan()">BLE-Scan</button>
    </div>
    <div class="kpi-grid" id="rf-kpis" style="margin-bottom:14px"></div>
    <div class="panel">
      <h3>Messung / Radio-Budget</h3>
      <div class="kpi-grid" id="meas-kpis"></div>
    </div>
    <div class="grid2">
      <div class="panel">
        <h3>Standort-Baseline</h3>
        <div class="meta" id="loc-status">keine Baseline</div>
        <div class="bar" style="margin-top:8px"><i id="loc-bar" style="width:0%"></i></div>
        <div class="row" style="margin-top:10px">
          <div><label>Dauer (s)</label><input id="loc-dur" type="number" value="300" min="60" max="3600"></div>
        </div>
        <button class="btn btn-a" onclick="locCapture()">Baseline aufnehmen</button>
        <button class="btn" onclick="locStop()">Jetzt speichern</button>
        <button class="btn" onclick="locCancel()">Abbruch</button>
        <button class="btn btn-r" onclick="locClear()">Loeschen</button>
      </div>
      <div class="panel">
        <h3>Mess-Session</h3>
        <div class="meta" id="sess-status">inaktiv</div>
        <div class="row" style="margin-top:10px">
          <div><label>Profil</label>
            <select id="sess-profile">
              <option value="field">Field Session (10 min)</option>
              <option value="characterization">Score Characterization</option>
              <option value="quick">Quick Check (3 min)</option>
              <option value="diagnose">Diagnose</option>
            </select>
          </div>
          <div><label>Label</label><input id="sess-label" type="text" placeholder="technikraum-lauf1"></div>
        </div>
        <div class="row" id="sess-phases" style="margin-top:8px;display:none;flex-wrap:wrap;gap:6px">
          <span class="meta">Phase:</span>
          <button class="btn" onclick="sessPhase('A','RSSI -20dB')">A</button>
          <button class="btn" onclick="sessPhase('B','+5 APs')">B</button>
          <button class="btn" onclick="sessPhase('C','same channel')">C</button>
          <button class="btn" onclick="sessPhase('D','more channels')">D</button>
          <button class="btn" onclick="sessPhase('E','disconnect')">E</button>
          <button class="btn" onclick="sessPhase('F','stabilize')">F</button>
          <button class="btn" onclick="sessPhase('G','baseline')">G</button>
        </div>
        <button class="btn btn-a" onclick="sessStart()">Start</button>
        <button class="btn" onclick="sessStop()">Stop</button>
        <button class="btn" onclick="sessExport()">Export JSON</button>
        <button class="btn" onclick="snapExport()">Coexistence Snapshot</button>
      </div>
    </div>
  </div>

  <div class="pane" id="pane-wifi">
    <div class="panel">
      <h3>Access Points</h3>
      <div class="filterbar">
        <div><label>Filter</label>
          <select id="ap-filter" onchange="renderWifi()">
            <option value="all">alle</option>
            <option value="known">bekannt</option>
            <option value="unknown">unbekannt</option>
            <option value="own">eigenes</option>
            <option value="new">NEW</option>
            <option value="persistent">PERSISTENT</option>
            <option value="flapping">FLAPPING</option>
            <option value="disappeared">DISAPPEARED</option>
          </select>
        </div>
        <div><label>Sortierung</label>
          <select id="ap-sort" onchange="renderWifi()">
            <option value="rssi">RSSI</option>
            <option value="channel">Kanal</option>
            <option value="ssid">SSID</option>
          </select>
        </div>
        <div><label>RSSI ≥</label><input id="ap-rssi" type="number" value="-90" onchange="renderWifi()"></div>
        <div><label>Kanal</label><input id="ap-ch" type="number" min="0" max="14" placeholder="alle" onchange="renderWifi()"></div>
        <button class="btn btn-a" onclick="loadWifi()">Aktualisieren</button>
        <button class="btn" onclick="doWifiScan()">Scan jetzt</button>
      </div>
      <div class="meta" id="ap-life-sum" style="margin-top:8px"></div>
      <table style="margin-top:10px"><thead><tr><th>SSID</th><th>RSSI</th><th>CH</th><th>Life</th><th>Seen</th><th>BSSID</th><th></th></tr></thead>
      <tbody id="ap-body"></tbody></table>
    </div>
  </div>

  <div class="pane" id="pane-ble">
    <div class="panel">
      <h3>BLE-Geraete / Beacons</h3>
      <button class="btn btn-a" onclick="doBleScan()">Scan jetzt</button>
      <button class="btn" onclick="loadBle()">Aktualisieren</button>
      <table style="margin-top:10px"><thead><tr><th>Name</th><th>Adresse</th><th>RSSI</th><th>Typ</th><th>Hits</th><th></th></tr></thead>
      <tbody id="ble-body"></tbody></table>
    </div>
  </div>

  <div class="pane" id="pane-history">
    <div class="panel" style="margin-bottom:14px">
      <h3>Aufloesung / Zeitfenster</h3>
      <div class="seg" id="hist-res">
        <button type="button" data-r="raw" class="on" onclick="setHistRes('raw')">Raw</button>
        <button type="button" data-r="1m" onclick="setHistRes('1m')">1 min</button>
        <button type="button" data-r="15m" onclick="setHistRes('15m')">15 min</button>
      </div>
      <div class="seg" id="hist-win" style="margin-top:8px">
        <button type="button" data-n="30" onclick="setHistWin(30)">30</button>
        <button type="button" data-n="90" class="on" onclick="setHistWin(90)">90</button>
        <button type="button" data-n="180" onclick="setHistWin(180)">180</button>
        <button type="button" data-n="0" onclick="setHistWin(0)">alles</button>
      </div>
      <div class="meta" id="hist-meta" style="margin-top:8px"></div>
    </div>
    <div class="panel" style="margin-bottom:14px">
      <h3>Channel History <span class="meta">(AP-Druck über Zeit)</span></h3>
      <canvas id="h-chhist" width="800" height="160"></canvas>
      <div class="meta" id="chhist-meta" style="margin-top:6px"></div>
    </div>
    <div class="grid2">
      <div class="panel"><h3>RSSI</h3><canvas id="h-rssi"></canvas></div>
      <div class="panel"><h3>RF-Score</h3><canvas id="h-score"></canvas></div>
    </div>
    <div class="grid2">
      <div class="panel"><h3>APs</h3><canvas id="h-ap"></canvas></div>
      <div class="panel"><h3>Pressure*</h3><canvas id="h-press"></canvas></div>
    </div>
  </div>

  <div class="pane" id="pane-events">
    <div class="panel">
      <h3>Event-Terminal</h3>
      <div class="row">
        <div><label>Typ</label><input id="ev-type" placeholder="wifi.new_ap"></div>
        <div><label>Schwere</label>
          <select id="ev-lvl"><option value="">alle</option><option value="0">ERROR</option><option value="1">WARN</option><option value="2">INFO</option></select>
        </div>
      </div>
      <button class="btn" onclick="loadEvents()">Filtern</button>
      <button class="btn btn-r" onclick="clearEvents()">Leeren</button>
      <div class="term" id="ev-term" style="margin-top:12px"></div>
    </div>
  </div>

  <div class="pane" id="pane-config">
    <div class="panel"><h3>System</h3>
      <div class="row">
        <div><label>Geraetename</label><input id="c-name"></div>
        <div><label>Standort</label><input id="c-loc"></div>
      </div>
      <label>Betriebsmodus</label>
      <select id="c-mode"><option value="live">Live-Monitoring</option><option value="normal">Normalbetrieb</option><option value="diagnostic">Diagnose</option></select>
    </div>
    <div class="panel"><h3>Hub</h3>
      <label><input type="checkbox" id="c-enhub"> Hub aktiv</label>
      <div class="row">
        <div><label>Host</label><input id="c-hub"></div>
        <div><label>Port</label><input id="c-port" type="number"></div>
        <div><label>Heartbeat s</label><input id="c-hb" type="number"></div>
      </div>
    </div>
    <div class="panel"><h3>WLAN / RF</h3>
      <div class="row">
        <div><label>WLAN-Scan s</label><input id="c-scan" type="number"></div>
        <div><label>RSSI-Warnung dB</label><input id="c-drop" type="number"></div>
        <div><label>RF-Score-Grenze</label><input id="c-scorew" type="number"></div>
      </div>
      <label>Bekannte SSIDs (Komma)</label><input id="c-known">
    </div>
    <div class="panel"><h3>BLE</h3>
      <label><input type="checkbox" id="c-enble"> BLE-Scan aktiv</label>
      <div class="row"><div><label>BLE-Scan s</label><input id="c-bles" type="number"></div></div>
      <label>Bekannte BLE (Komma)</label><input id="c-kble">
    </div>
    <button class="btn btn-a" onclick="saveCfg()">Speichern</button>
    <button class="btn" onclick="testCfg()">Test</button>
    <button class="btn btn-r" onclick="restart()">Neustart</button>
    <div id="c-msg" class="meta" style="margin-top:8px"></div>
  </div>

  <div class="pane" id="pane-ota">
    <div class="panel">
      <h3>Firmware</h3>
      <div class="warn">Passendes Binary waehlen (Mini = esp32, S3 = esp32s3). Danach Neustart.</div>
      <div class="drop" id="drop">.bin hierher ziehen oder klicken</div>
      <input type="file" id="fw" accept=".bin" style="display:none">
      <div id="fname" class="meta" style="margin-top:8px;text-align:center"></div>
      <button class="btn btn-a" onclick="startUpload()">Flashen</button>
      <div class="bar" style="margin-top:12px;display:none" id="prog"><i id="obar"></i></div>
      <div id="omsg" class="meta" style="text-align:center;margin-top:8px"></div>
    </div>
  </div>
</div>
<script>
function $(id){return document.getElementById(id)}
function jget(u){return fetch(u).then(r=>r.json())}
function jpost(u,b){return fetch(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(b||{})}).then(r=>r.json())}
const ENC={0:'OPEN',1:'WEP',2:'WPA',3:'WPA2',4:'WPA/WPA2',5:'WPA2-ENT',6:'WPA3',7:'WPA2/WPA3',8:'WAPI'};
let hist={rssi:[],rf_score:[],ap_count:[],ble_count:[],channel_pressure:[]};
let histWin=90;
let histRes='raw';
let chHist=[];
let lastChannels=[];
let lastAps=[];
let ownSsid='';
let lastEvents=[];
try{const s=JSON.parse(localStorage.getItem('rfmon-ev')||'{}'); if(s.type!=null) setTimeout(()=>{if($('ev-type'))$('ev-type').value=s.type; if($('ev-lvl')&&s.lvl!=null)$('ev-lvl').value=s.lvl},0)}catch(e){}

function drawLine(id,series,color,min,max){
  const c=$(id); if(!c) return;
  const w=c.width=c.clientWidth||320, h=c.height=150;
  const ctx=c.getContext('2d'); ctx.clearRect(0,0,w,h);
  ctx.strokeStyle='#1c2a24'; ctx.lineWidth=1; ctx.beginPath();
  for(let i=0;i<5;i++){ctx.moveTo(0,h*i/4);ctx.lineTo(w,h*i/4)} ctx.stroke();
  if(!series||series.length<2){ctx.fillStyle='#8aa094';ctx.fillText('keine Daten',10,h/2);return}
  const ys=series.map(p=>Number(p.v)); let lo=min, hi=max;
  if(lo==null) lo=Math.min.apply(null,ys); if(hi==null) hi=Math.max.apply(null,ys); if(lo===hi){lo-=1;hi+=1}
  ctx.beginPath(); ctx.strokeStyle=color; ctx.lineWidth=2;
  series.forEach((p,i)=>{const x=i/(series.length-1)*w,y=h-(Number(p.v)-lo)/(hi-lo)*h; i?ctx.lineTo(x,y):ctx.moveTo(x,y)});
  ctx.stroke();
  ctx.globalAlpha=.14; ctx.lineTo(w,h); ctx.lineTo(0,h); ctx.closePath(); ctx.fillStyle=color; ctx.fill(); ctx.globalAlpha=1;
  ctx.fillStyle=color; ctx.fillText(Number(series[series.length-1].v).toFixed(1),8,16);
}
function drawSpark(series){
  const c=$('spark'); if(!c) return;
  const w=c.width, h=c.height, ctx=c.getContext('2d');
  ctx.clearRect(0,0,w,h);
  if(!series||series.length<2) return;
  const ys=series.map(p=>Number(p.v));
  let lo=Math.min.apply(null,ys), hi=Math.max.apply(null,ys); if(lo===hi){lo-=1;hi+=1}
  ctx.beginPath(); ctx.strokeStyle='#2dd4a8'; ctx.lineWidth=1.5;
  series.forEach((p,i)=>{const x=i/(series.length-1)*(w-2)+1,y=h-2-((Number(p.v)-lo)/(hi-lo))*(h-4); i?ctx.lineTo(x,y):ctx.moveTo(x,y)});
  ctx.stroke();
}
function drawGauge(score){
  const c=$('g-score'); if(!c) return;
  const ctx=c.getContext('2d'), w=c.width, h=c.height, cx=w/2, cy=h/2, r=72;
  ctx.clearRect(0,0,w,h);
  const start=Math.PI*0.75, end=Math.PI*2.25, span=end-start;
  ctx.lineWidth=12; ctx.strokeStyle='#1c2a24'; ctx.beginPath(); ctx.arc(cx,cy,r,start,end); ctx.stroke();
  const col=score>=70?'#3ecf8e':score>=40?'#f0b429':'#ef5b5b';
  ctx.strokeStyle=col; ctx.beginPath(); ctx.arc(cx,cy,r,start,start+span*(Math.max(0,Math.min(100,score))/100)); ctx.stroke();
  ctx.fillStyle='#e8f0ea'; ctx.font='700 42px Syne,sans-serif'; ctx.textAlign='center'; ctx.textBaseline='middle';
  ctx.fillText(String(Math.round(score)),cx,cy-4);
  ctx.fillStyle='#8aa094'; ctx.font='11px IBM Plex Mono,monospace'; ctx.fillText('/ 100',cx,cy+28);
}
function kpi(v,u,k){return '<div class="kpi"><div class="v">'+v+(u?'<small style="font-size:.45em;margin-left:4px">'+u+'</small>':'')+'</div><div class="k">'+k+'</div></div>'}
function renderBreak(parts){
  const p=parts||{};
  const rows=[['OwnLink',p.ownLink!=null?p.ownLink:p.rssi],['Jitter',p.jitter],['Pressure*',p.pressure!=null?p.pressure:p.congestion],['Density*',p.density],['Occupancy*',p.occupancy],['Stability',p.stability],['Disconnect',p.disconnect],['QualGate',p.qualityGate]];
  const max=Math.max(25,...rows.map(r=>Number(r[1]||0)));
  $('score-break').innerHTML=rows.map(r=>{
    const v=Number(r[1]||0); const pct=Math.min(100,v/max*100);
    return '<div class="rowb"><span>'+r[0]+'</span><div class="meter"><i style="width:'+pct+'%"></i></div><span>-'+v+'</span></div>';
  }).join('')+'<div class="meta" style="margin-top:8px">* Proxy · aktiv Score v'+(window._sv||2)+'</div>';
}
function renderWhy(why,loc){
  const el=$('why-break'), sum=$('why-sum');
  if(!el||!sum) return;
  if(!why){sum.textContent='—'; el.innerHTML=''; return}
  const str=why.strongestPenalty||'none';
  const strV=why.strongestPenaltyValue||0;
  const dq=why.measurementQuality!=null?why.measurementQuality:'-';
  const dev=why.locationDeviation!=null?why.locationDeviation:'—';
  sum.textContent='Score '+why.score+' · staerkster Abzug: '+str+(strV?' (−'+strV+')':'')+' · DQ '+dq+' · Loc Δ '+dev;
  const pens=(why.penalties||[]).slice().sort((a,b)=>(b.value||0)-(a.value||0));
  const max=Math.max(25,...pens.map(p=>Number(p.value||0)));
  let html=pens.map(p=>{
    const v=Number(p.value||0); const pct=Math.min(100,v/max*100);
    const lab=p.key+(p.proxy?'*':'');
    return '<div class="rowb"><span>'+lab+'</span><div class="meter"><i style="width:'+pct+'%"></i></div><span>-'+v+'</span></div>';
  }).join('');
  const dp=(loc&&loc.deviationParts)||null;
  if(dp){
    const parts=[['score',dp.score],['density',dp.density],['pressure',dp.pressure],['rssi',dp.rssi],['AP',dp.apCount],['BLE',dp.bleCount]];
    html+='<div class="meta" style="margin-top:10px">Location Δ Teile</div>';
    html+=parts.map(r=>{
      const v=Number(r[1]||0);
      const s=(v>0?'+':'')+v.toFixed(1);
      return '<div class="rowb"><span>'+r[0]+'</span><span></span><span>'+s+'</span></div>';
    }).join('');
  }
  el.innerHTML=html+'<div class="meta" style="margin-top:8px">* Proxy</div>';
}
function renderFindings(list){
  const el=$('findings'); if(!el) return;
  const rows=list||[];
  if(!rows.length){el.innerHTML='<div class="meta">keine Befunde</div>';return}
  el.innerHTML=rows.map(f=>{
    const col=f.level==='bad'?'var(--bad)':f.level==='warn'?'var(--warn)':f.level==='ok'?'var(--ok)':'var(--muted)';
    const mark=f.level==='ok'?'✓':f.level==='warn'?'⚠':f.level==='bad'?'✕':'·';
    return '<div class="rowb"><span style="color:'+col+'">'+mark+'</span><span style="grid-column:span 2">'+esc(f.text||f.code||'')+'</span></div>';
  }).join('');
}
function renderAlertConds(alert){
  const el=$('alert-conds'); if(!el) return;
  const cond=(alert&&alert.conditions)||{};
  const keys=Object.keys(cond);
  const active=keys.filter(k=>cond[k]&&cond[k].active);
  if(!active.length){el.innerHTML='<div class="meta">keine aktiven Alert-Conditions</div>';return}
  el.innerHTML=active.map(k=>{
    const c=cond[k]; const sec=Math.round((c.activeForMs||0)/1000);
    const st=c.state|| (c.acked?'acked':'active');
    return '<div class="rowb"><span>⚠</span><span>'+esc(k)+' · '+st+(sec?' · '+sec+'s':'')+'</span>'+
      (c.acked?'<span class="meta">ACK</span>':'<button class="btn" style="padding:2px 8px;font-size:11px" onclick="ackAlert(\''+k+'\')">ACK</button>')+'</div>';
  }).join('');
}
function ackAlert(code){jpost('/api/alerts/ack',code?{code:code}:{}).then(()=>jget('/api/status').then(applyStatus))}
function ackEvent(id){jpost('/api/events/ack',{id:id}).then(loadEvents)}
function pinEvent(id,on){jpost('/api/events/pin',{id:id,pinned:!!on}).then(loadEvents)}
function renderHeat(channels){
  lastChannels=channels||lastChannels||[];
  const max=Math.max(1,...lastChannels.map(c=>c.count||0));
  $('ch-heat').innerHTML=lastChannels.map(c=>{
    const pct=Math.round((c.count||0)/max*100);
    const tip='CH '+c.ch+' · APs '+(c.count||0)+
      (c.maxRssi!=null?' · max '+c.maxRssi+' dBm':'')+
      (c.avgRssi!=null?' · avg '+c.avgRssi+' dBm':'')+
      ' · known '+(c.known||0)+' / unk '+(c.unknown||0)+
      ' · '+(c.pressure||'-');
    return '<div class="col"><div class="tip">'+tip+'</div><div class="stk"><i style="height:'+(c.count?Math.max(8,pct):0)+'%"></i></div><div class="n">'+c.ch+'</div></div>';
  }).join('');
}
function applyStatus(s){
  ownSsid=s.ssid||ownSsid||'';
  $('h-name').textContent=s.name||'RF Monitor';
  $('h-ver').innerHTML='v<b>'+(s.version||'')+'</b>';
  $('h-board').textContent=s.boardLabel||s.board||'';
  const hw=s.hw||{};
  const al=(s.rf&&s.rf.alert)||{};
  $('h-hw').textContent=(hw.cores||'?')+'C'+(hw.psram?' · PSRAM':' · RAM')+(hw.workerRunning?' · RF@0':'')+(al.any?' · ALERT':'');
  $('h-loc').textContent=s.location||'';
  $('h-ch').textContent=s.channel!=null?s.channel:'-';
  $('d-board').textContent=(s.chip||'')+' · '+(s.boardLabel||s.board||'');
  $('d-mac').textContent=s.mac||'';
  $('d-ip').innerHTML=s.ip?'<a href="http://'+s.ip+'/">'+s.ip+'</a>':'-';
  $('d-ssid').textContent=s.ssid||'-';
  $('d-uptime').textContent=s.uptime||'-';
  $('d-hubok').textContent=s.hubEnabled?(s.hubOk?'online · '+(s.hub||''):'warte · '+(s.hub||'')):'deaktiviert';
  const hp=s.heap||0,hpct=Math.min(100,Math.round(hp/3276));
  $('d-heap').textContent=Math.round(hp/1024)+' KB'+(hw.psram?' · PSRAM '+Math.round((hw.psramFree||0)/1024)+' KB':'');
  $('b-heap').style.width=hpct+'%';
  $('b-heap').style.background=hpct>40?'var(--ok)':hpct>20?'var(--warn)':'var(--bad)';
  const rf=s.rf||{};
  drawGauge(Number(rf.rfScore||0));
  window._sv=rf.scoreVersion!=null?rf.scoreVersion:2;
  $('g-sv').textContent='v'+window._sv;
  $('g-dq').textContent=rf.measurementQuality!=null?rf.measurementQuality:'-';
  const loc=rf.location||{};
  const dev=rf.locationDeviation!=null?rf.locationDeviation:(loc.deviation!=null?loc.deviation:null);
  $('g-dev').textContent=dev!=null?(dev>0?'+'+dev:String(dev)):'—';
  const duty5=rf.radioDuty5mPct!=null?rf.radioDuty5mPct:rf.radioDutyPct;
  $('g-duty').textContent=Number(duty5||0).toFixed(1);
  renderBreak(rf.scoreParts);
  renderWhy(rf.why||((rf.assessment||{}).why),loc);
  renderAlertConds(rf.alert||{});
  renderFindings(rf.findings||[]);
  const life=rf.apLife||((rf.observation&&rf.observation.wifi&&rf.observation.wifi.apLife)||{});
  if($('ap-life-sum')&&life.tracked!=null){
    $('ap-life-sum').textContent='AP Life · visible '+life.visible+' · new '+life.new+' · persistent '+life.persistent+' · flap '+life.flapping+' · gone '+life.disappeared;
  }
  $('kpi-grid').innerHTML=
    kpi(rf.rfScore||0,'','Score v'+window._sv)+
    kpi(rf.scoreV1!=null?rf.scoreV1:'-','','Score v1')+
    kpi(rf.scoreV2!=null?rf.scoreV2:'-','','Score v2')+
    kpi(rf.measurementQuality!=null?rf.measurementQuality:'-','','Data Qual')+
    kpi(dev!=null?(dev>0?'+'+dev:dev):'—','','Loc Δ')+
    kpi(s.rssi||0,'dBm','RSSI')+
    kpi(rf.apCount||0,'','APs')+
    kpi(rf.bleCount||0,'','BLE');
  if(loc.capturing){
    $('loc-status').textContent='Aufnahme… '+loc.captureProgressPct+'% · '+loc.captureSamples+' Samples / '+loc.captureTargetS+'s';
    $('loc-bar').style.width=(loc.captureProgressPct||0)+'%';
  }else if(loc.valid){
    const b=loc.baseline||{};
    const dist=(loc.distribution&&loc.distribution.score)||{};
    const sd=dist.stddev!=null?(' ±'+Number(dist.stddev).toFixed(1)):'';
    const stale=loc.stale?' · STALE':'';
    const dr=loc.drift||{};
    const drift=dr.magnitude!=null?(' · drift '+Number(dr.magnitude).toFixed(0)+(dr.elevated?'!':'')):'';
    $('loc-status').textContent='Baseline v'+(loc.profileVersion||1)+' · Score≈'+Number(b.score||0).toFixed(0)+sd+' · APs≈'+Number(b.apCount||0).toFixed(1)+' · n='+loc.sampleCount+stale+drift;
    $('loc-bar').style.width='100%';
  }else{
    $('loc-status').textContent='keine Baseline';
    $('loc-bar').style.width='0%';
  }
  const sess=rf.session||{};
  const phBox=$('sess-phases');
  if(phBox) phBox.style.display=(sess.active&&sess.characterization)?'flex':'none';
  if(sess.active){
    const rem=sess.remainingS!=null?(' · rest '+sess.remainingS+'s'):'';
    const ph=sess.phase?(' · phase '+sess.phase):'';
    $('sess-status').textContent='aktiv · '+(sess.profile||'?')+' · '+sess.sampleCount+' Samples · avgScore '+Number(sess.avgScore||0).toFixed(0)+ph+rem;
  }else if(sess.sampleCount){
    $('sess-status').textContent='gestoppt · '+(sess.profile||'')+' · '+sess.sampleCount+' Samples · min/max '+sess.minScore+'/'+sess.maxScore;
  }else{
    $('sess-status').textContent='inaktiv';
  }
  $('rf-kpis').innerHTML=
    kpi(rf.wifiRssi||s.rssi||0,'dBm','RSSI')+
    kpi(Number(rf.wifiRssiAvg||0).toFixed(1),'dBm','Mittel')+
    kpi(rf.wifiRssiMin||0,'dBm','Min')+
    kpi(rf.wifiRssiMax||0,'dBm','Max')+
    kpi(Number(rf.wifiRssiJitter||0).toFixed(1),'dB','Jitter')+
    kpi(rf.channelOccupancyProxy!=null?rf.channelOccupancyProxy:(rf.channelLoad||0),'','Occupancy*')+
    kpi(rf.apDensity||0,'','Dichte*')+
    kpi(rf.unknownApCount||0,'','unbekannt')+
    kpi(rf.beaconCount||0,'','Beacons')+
    kpi(rf.disconnects||0,'','Drops');
  const obs=(rf.observation&&rf.observation.wifi)||{};
  const bleo=(rf.observation&&rf.observation.ble)||{};
  const radio=(rf.observation&&rf.observation.radio)||{};
  const d5=rf.radioDuty5mPct!=null?rf.radioDuty5mPct:(radio.duty5mPct!=null?radio.duty5mPct:rf.radioDutyPct);
  const d1h=rf.radioDuty1hPct!=null?rf.radioDuty1hPct:(radio.duty1hPct!=null?radio.duty1hPct:0);
  const dLife=rf.radioDutyPct!=null?rf.radioDutyPct:(radio.dutyLifetimePct!=null?radio.dutyLifetimePct:0);
  $('meas-kpis').innerHTML=
    kpi(obs.scanDurationMs!=null?obs.scanDurationMs:(rf.wifiScanDurationMs||0),'ms','WiFi Scan')+
    kpi(bleo.scanDurationMs!=null?bleo.scanDurationMs:(rf.bleScanDurationMs||0),'ms','BLE Scan')+
    kpi(Number(rf.wifiScanFailRate||obs.failRatePct||0).toFixed(1),'%','WiFi Fail')+
    kpi(Number(rf.bleScanFailRate||bleo.failRatePct||0).toFixed(1),'%','BLE Fail')+
    kpi(Number(d5||0).toFixed(1),'%','Duty 5m')+
    kpi(Number(d1h||0).toFixed(1),'%','Duty 1h')+
    kpi(Number(dLife||0).toFixed(1),'%','Duty boot')+
    kpi(hw.historyCapacity||'-','','Hist Cap');
  const warns=[];
  if((rf.rfScore||100)<40) warns.push('RF-Score niedrig');
  if((rf.measurementQuality||100)<50) warns.push('Messqualitaet niedrig');
  if((rf.channelCongestion||0)>=70) warns.push('Kanaldruck-Proxy hoch');
  if((d5||0)>45) warns.push('Radio-Zeitbudget 5m hoch');
  if(dev!=null && dev<=-25) warns.push('Abweichung vom Standortnormal');
  if(loc.stale) warns.push('Baseline veraltet');
  if(loc.drift&&loc.drift.elevated) warns.push('Baseline-Drift erhoeht');
  if((s.rssi||-30)<-75) warns.push('eigene Verbindung schwach');
  if(loc.capturing) warns.push('Baseline-Aufnahme laeuft');
  $('rf-warn').style.display=warns.length?'block':'none';
  $('rf-warn').textContent=warns.join(' · ');
  const t=s.time||{};
  const ts=(t.timeValid&&t.timestampEpochS)?t.timestampEpochS:(s.uptimeS||Math.floor(Date.now()/1000));
  if($('d-uptime')&&t.timeValid){
    $('d-uptime').textContent=(s.uptime||'-')+' · NTP ok';
  }
  hist.rssi.push({v:s.rssi||0,ts}); if(hist.rssi.length>180) hist.rssi.shift();
  hist.rf_score.push({v:Number(rf.rfScore||0),ts}); if(hist.rf_score.length>180) hist.rf_score.shift();
  hist.ap_count.push({v:Number(rf.apCount||0),ts}); if(hist.ap_count.length>180) hist.ap_count.shift();
  hist.ble_count.push({v:Number(rf.bleCount||0),ts}); if(hist.ble_count.length>180) hist.ble_count.shift();
  drawAll(); drawSpark(hist.rssi);
}
function sliceHist(series){
  if(!histWin||histWin<=0||series.length<=histWin) return series;
  return series.slice(series.length-histWin);
}
function setHistWin(n){
  histWin=n;
  document.querySelectorAll('#hist-win button').forEach(b=>b.classList.toggle('on',Number(b.dataset.n)===n));
  updateHistMeta();
  drawAll();
}
function setHistRes(r){
  histRes=r||'raw';
  document.querySelectorAll('#hist-res button').forEach(b=>b.classList.toggle('on',b.dataset.r===histRes));
  loadHistory();
}
function updateHistMeta(){
  const el=$('hist-meta'); if(!el) return;
  el.textContent='Res '+histRes+' · '+(histWin===0?'gesamter Ring':'letzte '+histWin+' Punkte');
}
function drawTimeline(){
  const c=$('c-timeline'); if(!c) return;
  const w=c.width=c.clientWidth||640, h=c.height=160;
  const ctx=c.getContext('2d'); ctx.clearRect(0,0,w,h);
  const sc=sliceHist(hist.rf_score), ap=sliceHist(hist.ap_count), bl=sliceHist(hist.ble_count);
  const n=Math.max(sc.length,ap.length,bl.length,2);
  const pad=8;
  const plot=(series,color,ymin,ymax)=>{
    if(!series.length) return;
    ctx.strokeStyle=color; ctx.lineWidth=1.5; ctx.beginPath();
    series.forEach((p,i)=>{
      const v=typeof p==='object'?Number(p.v):Number(p);
      const x=pad+(w-2*pad)*(i/(n-1||1));
      const y=h-pad-((Math.max(ymin,Math.min(ymax,v))-ymin)/(ymax-ymin||1))*(h-2*pad);
      i?ctx.lineTo(x,y):ctx.moveTo(x,y);
    });
    ctx.stroke();
  };
  plot(sc,'#f0b429',0,100);
  const apMax=Math.max(5,...ap.map(p=>Number(p.v||p||0)));
  plot(ap,'#2dd4a8',0,apMax);
  const blMax=Math.max(5,...bl.map(p=>Number(p.v||p||0)));
  plot(bl,'#8aa094',0,blMax);
  // event markers (relative to uptime seconds — approximate at right edge)
  if(lastEvents.length && sc.length){
    const t0=Number((sc[0]&&sc[0].ts)||0), t1=Number((sc[sc.length-1]&&sc[sc.length-1].ts)||t0+1);
    lastEvents.slice(0,24).forEach(e=>{
      const ts=Number(e.ts||0);
      if(t1<=t0||ts<t0||ts>t1) return;
      const x=pad+(w-2*pad)*((ts-t0)/(t1-t0));
      ctx.strokeStyle=e.level===0?'#ef5b5b':e.level===1?'#f0b429':'#8aa094';
      ctx.beginPath(); ctx.moveTo(x,pad); ctx.lineTo(x,h-pad); ctx.stroke();
    });
  }
  $('tl-leg').textContent='gelb=Score · gruen=APs · grau=BLE · Striche=Events';
}
function drawChannelHistory(){
  const c=$('h-chhist'); if(!c) return;
  const pts=chHist||[];
  const w=c.width=c.clientWidth||800, h=c.height=160;
  const ctx=c.getContext('2d'); ctx.clearRect(0,0,w,h);
  const meta=$('chhist-meta');
  if(!pts.length){
    ctx.fillStyle='#8aa094'; ctx.font='12px IBM Plex Mono,monospace';
    ctx.fillText('warte auf Channel-Snaps (alle 30s)…',12,24);
    if(meta) meta.textContent='keine Punkte';
    return;
  }
  const rows=14, cols=pts.length;
  const cellW=Math.max(2,(w-40)/cols), cellH=(h-20)/rows;
  let peak=1;
  pts.forEach(p=>{(p.ch||[]).forEach(v=>{if(v>peak)peak=v})});
  for(let r=0;r<rows;r++){
    const y=10+r*cellH;
    ctx.fillStyle='#8aa094'; ctx.font='10px IBM Plex Mono,monospace';
    if(r%2===0) ctx.fillText(String(r+1),2,y+cellH*0.75);
    for(let i=0;i<cols;i++){
      const v=(pts[i].ch&&pts[i].ch[r])||0;
      const t=v/peak;
      const g=Math.round(20+t*140), b=Math.round(30+t*80);
      ctx.fillStyle=v?('rgb('+(40+Math.round(t*180))+','+g+','+b+')'):'#121a17';
      ctx.fillRect(36+i*cellW, y, Math.max(1,cellW-0.5), Math.max(1,cellH-0.5));
    }
  }
  if(meta) meta.textContent=pts.length+' Snaps · peak max '+peak+' APs/Kanal · Interval ~30s';
}
function drawAll(){
  drawLine('c-rssi',sliceHist(hist.rssi),'#2dd4a8',-100,-20);
  drawLine('c-score',sliceHist(hist.rf_score),'#f0b429',0,100);
  drawLine('h-rssi',sliceHist(hist.rssi),'#2dd4a8',-100,-20);
  drawLine('h-score',sliceHist(hist.rf_score),'#f0b429',0,100);
  drawLine('h-ap',sliceHist(hist.ap_count),'#f0b429',0,null);
  drawLine('h-press',sliceHist(hist.channel_pressure),'#ef5b5b',0,100);
  drawChannelHistory();
  drawTimeline();
}
function loadHistory(){
  updateHistMeta();
  const q=histRes&&histRes!=='raw'?('?res='+encodeURIComponent(histRes)):'';
  Promise.all([
    jget('/api/rf/history'+q),
    jget('/api/rf/channels/history')
  ]).then(([h,ch])=>{
    hist.rssi=h.rssi||[]; hist.rf_score=h.rf_score||[]; hist.ap_count=h.ap_count||[];
    hist.ble_count=h.ble_count||[]; hist.channel_pressure=h.channel_pressure||[];
    chHist=ch.points||[];
    drawAll(); drawSpark(hist.rssi);
  }).catch(()=>{});
}
function renderWifi(){
  let rows=(lastAps||[]).slice();
  const f=$('ap-filter')?$('ap-filter').value:'all';
  const sort=$('ap-sort')?$('ap-sort').value:'rssi';
  const minR=Number($('ap-rssi')?$('ap-rssi').value:-999);
  const chF=$('ap-ch')&&$('ap-ch').value!==''?Number($('ap-ch').value):null;
  rows=rows.filter(a=>{
    if(f!=='disappeared' && a.life==='disappeared' && f!=='all') return false;
    if(a.life!=='disappeared' && a.rssi<minR) return false;
    if(chF!=null && a.channel!==chF) return false;
    if(f==='known' && !a.known) return false;
    if(f==='unknown' && a.known) return false;
    if(f==='own' && !(ownSsid && a.ssid && a.ssid.toLowerCase()===ownSsid.toLowerCase())) return false;
    if(['new','persistent','flapping','disappeared'].indexOf(f)>=0 && a.life!==f) return false;
    return true;
  });
  rows.sort((a,b)=>{
    if(sort==='channel') return (a.channel||0)-(b.channel||0) || (b.rssi-a.rssi);
    if(sort==='ssid') return String(a.ssid||'').localeCompare(String(b.ssid||''));
    return (b.rssi||0)-(a.rssi||0);
  });
  $('ap-body').innerHTML=rows.map(a=>{
    const isOwn=ownSsid && a.ssid && a.ssid.toLowerCase()===ownSsid.toLowerCase();
    const life=(a.life||'persistent').toUpperCase();
    return '<tr class="'+(isOwn?'own':'')+(a.life==='disappeared'?' meta':'')+'"><td>'+(a.hidden?'<i>versteckt</i>':esc(a.ssid))+(isOwn?' <span class="tag k">eigen</span>':'')+'</td><td>'+a.rssi+'</td><td>'+a.channel+'</td><td><span class="tag">'+(life)+'</span></td><td>'+(a.seenCount!=null?a.seenCount:'-')+'</td><td class="mono">'+a.bssid+'</td><td><span class="tag '+(a.known?'k':'u')+'">'+(a.known?'bekannt':'unk')+'</span></td></tr>';
  }).join('')||'<tr><td colspan="7">keine Daten</td></tr>';
}
function loadWifi(){
  jget('/api/wifi/list').then(d=>{ lastAps=d.aps||[]; renderWifi(); });
  jget('/api/wifi/channels').then(d=>renderHeat(d.channels||[]));
}
function loadBle(){
  jget('/api/ble/list').then(d=>{
    $('ble-body').innerHTML=(d.devices||[]).map(a=>'<tr><td>'+esc(a.name||'-')+'</td><td class="mono">'+a.addr+'</td><td>'+a.rssi+'</td><td><span class="tag '+(a.kind>0?'b':'')+'">'+(a.type||'')+'</span></td><td>'+a.hits+'</td><td>'+(a.known?'bekannt':'')+'</td></tr>').join('')||'<tr><td colspan="6">keine BLE-Geraete</td></tr>';
  });
}
function esc(t){return String(t||'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]))}
function loadEvents(){
  const t=$('ev-type').value, l=$('ev-lvl').value;
  try{localStorage.setItem('rfmon-ev',JSON.stringify({type:t,lvl:l}))}catch(e){}
  let u='/api/events/recent'; const q=[]; if(t) q.push('type='+encodeURIComponent(t)); if(l!=='') q.push('level='+l); if(q.length) u+='?'+q.join('&');
  jget(u).then(d=>{
    lastEvents=d.events||[];
    $('ev-term').innerHTML=lastEvents.map(e=>{
      const flags=(e.pinned?'📌 ':'')+(e.acked?'ACK ':'');
      return '<div class="e lvl'+e.level+'"><div class="t">'+e.ts+'s</div><div><b>'+esc(e.type)+'</b> — '+esc(e.title)+(e.details?' <span class="meta">'+esc(e.details)+'</span>':'')+
        ' <span class="meta">'+flags+'</span> '+
        '<button class="btn" style="padding:1px 6px;font-size:10px" onclick="ackEvent('+e.id+')">ACK</button> '+
        '<button class="btn" style="padding:1px 6px;font-size:10px" onclick="pinEvent('+e.id+','+(e.pinned?'false':'true')+')">'+(e.pinned?'Unpin':'Pin')+'</button>'+
        '</div></div>';
    }).join('')||'<div class="meta">keine Ereignisse</div>';
    drawTimeline();
  });
}
function clearEvents(){jpost('/api/events/clear',{}).then(loadEvents)}
function doWifiScan(){jget('/api/wifi/scan').then(()=>setTimeout(loadWifi,4000))}
function doBleScan(){jget('/api/ble/scan').then(()=>setTimeout(loadBle,5000))}
function locCapture(){jpost('/api/location/capture',{durationS:Number($('loc-dur').value)||300}).then(d=>{$('loc-status').textContent=d.ok?'Aufnahme gestartet ('+d.durationS+'s)':'Fehler'})}
function locStop(){jpost('/api/location/capture/stop',{}).then(d=>{$('loc-status').textContent=d.valid?'Baseline gespeichert':'zu wenig Samples'})}
function locCancel(){jpost('/api/location/capture/cancel',{}).then(()=>{$('loc-status').textContent='abgebrochen';$('loc-bar').style.width='0%'})}
function locClear(){if(confirm('Baseline loeschen?'))jpost('/api/location/clear',{}).then(()=>{$('loc-status').textContent='keine Baseline';$('loc-bar').style.width='0%'})}
function sessStart(){
  jpost('/api/session/start',{
    label:$('sess-label').value||'',
    profile:($('sess-profile')&&$('sess-profile').value)||'field'
  }).then(d=>{
    const s=d.session||{};
    $('sess-status').textContent=d.ok?('aktiv · '+(s.profile||'')):(d.error||'Fehler');
    if($('sess-phases')) $('sess-phases').style.display=(d.ok&&s.characterization)?'flex':'none';
  })
}
function sessPhase(id,note){jpost('/api/session/phase',{id:id,note:note||''}).then(d=>{$('sess-status').textContent=d.ok?('phase '+id):'Phase-Fehler'})}
function sessStop(){jpost('/api/session/stop',{}).then(d=>{const s=d.session||{};$('sess-status').textContent='gestoppt · '+(s.sampleCount||0)+' Samples'; if($('sess-phases'))$('sess-phases').style.display='none'})}
function downloadJson(name,obj){const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([JSON.stringify(obj,null,2)],{type:'application/json'}));a.download=name;a.click()}
function sessExport(){jget('/api/session/export').then(d=>downloadJson('rfmon-session.json',d))}
function snapExport(){jget('/api/rf/snapshot').then(d=>downloadJson('rfmon-coexistence-snapshot.json',d))}
function loadCfg(){
  jget('/api/config/get').then(c=>{
    $('c-name').value=c.deviceName||''; $('c-loc').value=c.locationName||''; $('c-mode').value=c.opMode||'normal';
    $('c-enhub').checked=!!c.enableHub; $('c-hub').value=c.hubHost||''; $('c-port').value=c.hubPort||8093; $('c-hb').value=c.heartbeatIntervalS||30;
    $('c-scan').value=c.wifiScanIntervalS||20; $('c-drop').value=c.rssiWarnDrop||12; $('c-scorew').value=c.rfScoreWarn||40;
    $('c-known').value=c.knownSsids||''; $('c-enble').checked=!!c.bleEnabled; $('c-bles').value=c.bleScanIntervalS||15; $('c-kble').value=c.knownBle||'';
  });
}
function saveCfg(){
  jpost('/api/config/save',{
    deviceName:$('c-name').value, locationName:$('c-loc').value, opMode:$('c-mode').value,
    enableHub:$('c-enhub').checked, hubHost:$('c-hub').value, hubPort:Number($('c-port').value),
    heartbeatIntervalS:Number($('c-hb').value), wifiScanIntervalS:Number($('c-scan').value),
    rssiWarnDrop:Number($('c-drop').value), rfScoreWarn:Number($('c-scorew').value),
    knownSsids:$('c-known').value, bleEnabled:$('c-enble').checked, bleScanIntervalS:Number($('c-bles').value),
    knownBle:$('c-kble').value, firstRunDone:true
  }).then(d=>{$('c-msg').textContent=d.ok?'Gespeichert.':'Fehler';});
}
function testCfg(){jpost('/api/config/test',{}).then(d=>{$('c-msg').textContent=JSON.stringify(d)})}
function restart(){jpost('/api/system/restart',{})}
document.querySelectorAll('.tab[data-tab]').forEach(t=>t.onclick=()=>{
  document.querySelectorAll('.tab').forEach(x=>x.classList.remove('active'));
  document.querySelectorAll('.pane').forEach(x=>x.classList.remove('active'));
  t.classList.add('active'); $('pane-'+t.dataset.tab).classList.add('active');
  location.hash=t.dataset.tab;
  if(t.dataset.tab==='rf'||t.dataset.tab==='wifi'||t.dataset.tab==='status') loadWifi();
  if(t.dataset.tab==='ble') loadBle();
  if(t.dataset.tab==='events') loadEvents();
  if(t.dataset.tab==='config') loadCfg();
  if(t.dataset.tab==='history') loadHistory();
});
function applyHash(){
  const h=(location.hash||'#status').slice(1);
  const tab=document.querySelector('.tab[data-tab="'+h+'"]'); if(tab) tab.click();
}
window.addEventListener('hashchange',applyHash);
const drop=$('drop'),inp=$('fw');
drop.onclick=()=>inp.click();
inp.onchange=()=>{if(inp.files[0])$('fname').textContent=inp.files[0].name+' ('+Math.round(inp.files[0].size/1024)+' KB)'};
drop.ondragover=e=>{e.preventDefault();drop.style.borderColor='#2dd4a8'};
drop.ondragleave=()=>drop.style.borderColor='#24332c';
drop.ondrop=e=>{e.preventDefault();drop.style.borderColor='#24332c';const f=e.dataTransfer.files[0];if(f&&f.name.endsWith('.bin')){const dt=new DataTransfer();dt.items.add(f);inp.files=dt.files;$('fname').textContent=f.name}};
function startUpload(){
  if(!inp.files[0]){alert('Bitte .bin waehlen');return}
  const fd=new FormData(); fd.append('firmware',inp.files[0]);
  const xhr=new XMLHttpRequest(); xhr.open('POST','/ota-upload');
  $('prog').style.display='block';
  xhr.upload.onprogress=e=>{if(e.lengthComputable){$('obar').style.width=Math.round(e.loaded/e.total*100)+'%';$('omsg').textContent='Hochladen '+Math.round(e.loaded/e.total*100)+'%'}};
  xhr.onload=()=>{$('omsg').textContent=xhr.status===200?'OK, Neustart...':'Fehler: '+xhr.responseText};
  xhr.send(fd);
}
const es=new EventSource('/events');
es.onopen=()=>{$('sse-dot').classList.add('on');$('sse-lbl').textContent='Live'};
es.onerror=()=>{$('sse-dot').classList.remove('on');$('sse-lbl').textContent='Getrennt'};
es.onmessage=e=>{try{applyStatus(JSON.parse(e.data))}catch(ex){}};
jget('/api/status').then(applyStatus); loadHistory(); loadWifi(); loadEvents(); setHistWin(90); applyHash();
</script></body></html>
)RFUI";
