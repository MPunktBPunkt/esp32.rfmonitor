#pragma once
#include <Arduino.h>

const char PAGE_MAIN[] PROGMEM = R"RFUI(<!DOCTYPE html>
<html lang="de"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>esp32.rfmonitor</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#0d1117;color:#e6edf3;font-family:Segoe UI,sans-serif;font-size:14px}
header{background:#161b22;border-bottom:1px solid #30363d;padding:14px 20px;display:flex;align-items:center;gap:12px;flex-wrap:wrap}
h1{font-size:18px;color:#58a6ff}
.badge{background:rgba(88,166,255,.15);color:#58a6ff;border:1px solid rgba(88,166,255,.3);padding:2px 8px;border-radius:10px;font-size:11px}
.dot{width:8px;height:8px;border-radius:50%;display:inline-block;margin-right:6px}
.dot-on{background:#3fb950;box-shadow:0 0 6px #3fb950}.dot-off{background:#f85149}
.tabs{display:flex;background:#161b22;border-bottom:1px solid #30363d;padding:0 12px;overflow:auto}
.tab{padding:10px 16px;border-bottom:2px solid transparent;color:#8b949e;text-decoration:none;font-weight:500;white-space:nowrap;cursor:pointer}
.tab.active,.tab:hover{color:#58a6ff;border-bottom-color:#58a6ff}
.content{padding:18px;max-width:1100px;margin:0 auto}
.pane{display:none}.pane.active{display:block}
.card{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:16px;margin-bottom:14px}
.card h3{font-size:11px;color:#8b949e;text-transform:uppercase;letter-spacing:.5px;margin-bottom:12px}
table{width:100%;border-collapse:collapse;font-size:13px}
td,th{padding:7px 8px;border-bottom:1px solid #21262d;text-align:left}
td:first-child,th:first-child{color:#8b949e}
.mono{font-family:ui-monospace,Consolas,monospace;color:#58a6ff}
.io-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(150px,1fr));gap:8px}
.io-card{background:#1c2128;border:1px solid #30363d;border-radius:6px;padding:10px;text-align:center}
.io-val{font-size:22px;font-weight:700;color:#58a6ff}
.io-key{font-size:11px;color:#8b949e;margin-top:4px}
.bar{height:8px;background:#21262d;border-radius:4px;overflow:hidden}
.bar-fill{height:100%;background:#58a6ff}
.green{background:#3fb950}.yellow{background:#e3b341}.red{background:#f85149}
.grid2{display:grid;grid-template-columns:1fr 1fr;gap:14px}
@media(max-width:800px){.grid2{grid-template-columns:1fr}}
canvas{width:100%;height:160px;background:#0d1117;border:1px solid #21262d;border-radius:6px}
input,select,textarea{background:#0d1117;border:1px solid #30363d;color:#e6edf3;border-radius:6px;padding:8px;width:100%;font:inherit}
label{display:block;color:#8b949e;font-size:12px;margin:10px 0 4px}
.btn{background:#238636;color:#fff;border:none;padding:8px 16px;border-radius:6px;cursor:pointer;font-weight:600;margin:6px 6px 0 0}
.btn:hover{background:#2ea043}
.btn-b{background:#1f6feb}.btn-b:hover{background:#388bfd}
.btn-r{background:#da3633}
.row{display:flex;gap:8px;flex-wrap:wrap;align-items:end}
.row>div{flex:1;min-width:120px}
.warn{background:rgba(227,179,65,.1);border:1px solid rgba(227,179,65,.3);border-radius:6px;padding:10px;color:#e3b341;font-size:13px;margin-bottom:12px}
.drop{border:2px dashed #30363d;border-radius:8px;padding:28px;text-align:center;cursor:pointer;color:#8b949e}
.drop:hover{border-color:#58a6ff;color:#e6edf3}
.ch-row{display:flex;align-items:center;gap:8px;margin:4px 0}
.ch-row span{width:28px;color:#8b949e;font-size:12px}
.lvl0{color:#f85149}.lvl1{color:#e3b341}.lvl2{color:#8b949e}
.tag{font-size:11px;padding:1px 6px;border-radius:8px;border:1px solid #30363d;color:#8b949e}
.tag.k{color:#3fb950;border-color:#3fb950}.tag.u{color:#e3b341;border-color:#e3b341}
.tag.b{color:#a371f7;border-color:#a371f7}
</style></head><body>
<header>
  <h1>&#128225; <span id="h-name">RF Monitor</span></h1>
  <span class="badge" id="h-ver"></span>
  <span class="badge" id="h-board"></span>
  <span class="badge" id="h-loc"></span>
  <span style="margin-left:auto;font-size:12px;color:#8b949e">
    <span class="dot dot-off" id="sse-dot"></span><span id="sse-lbl">Verbinde...</span>
  </span>
</header>
<nav class="tabs">
  <a class="tab active" data-tab="status">Status</a>
  <a class="tab" data-tab="rf">RF</a>
  <a class="tab" data-tab="wifi">WLAN</a>
  <a class="tab" data-tab="ble">BLE</a>
  <a class="tab" data-tab="history">History</a>
  <a class="tab" data-tab="events">Events</a>
  <a class="tab" data-tab="config">Config</a>
  <a class="tab" href="/wizard">Wizard</a>
  <a class="tab" data-tab="ota">OTA</a>
</nav>
<div class="content">
<div class="pane active" id="pane-status">
  <div class="card"><h3>Geraet</h3><table>
    <tr><td>Name</td><td id="d-name">-</td></tr>
    <tr><td>Standort</td><td id="d-loc">-</td></tr>
    <tr><td>Board</td><td id="d-board">-</td></tr>
    <tr><td>Chip</td><td id="d-chip">-</td></tr>
    <tr><td>MAC</td><td class="mono" id="d-mac">-</td></tr>
    <tr><td>IP</td><td class="mono" id="d-ip">-</td></tr>
    <tr><td>WLAN RSSI</td><td id="d-rssi">-</td></tr>
    <tr><td>Uptime</td><td id="d-uptime">-</td></tr>
    <tr><td>Firmware</td><td id="d-ver">-</td></tr>
  </table></div>
  <div class="io-grid" id="kpi-grid" style="margin-bottom:14px"></div>
  <div class="card"><h3>Speicher</h3>
    <div>RAM <span id="d-heap">-</span><div class="bar"><div class="bar-fill" id="b-heap"></div></div></div>
    <div style="margin-top:10px">Flash (OTA) <span id="d-sketch">-</span><div class="bar"><div class="bar-fill" id="b-sketch"></div></div></div>
  </div>
  <div class="card"><h3>ESP-Hub</h3><table>
    <tr><td>Adresse</td><td class="mono" id="d-hub">-</td></tr>
    <tr><td>Heartbeat</td><td id="d-interval">-</td></tr>
    <tr><td>Status</td><td id="d-hubok">-</td></tr>
  </table></div>
</div>
<div class="pane" id="pane-rf">
  <div class="grid2">
    <div class="card"><h3>WLAN RSSI (dBm)</h3><canvas id="c-rssi"></canvas></div>
    <div class="card"><h3>RF-Score (0-100)</h3><canvas id="c-score"></canvas></div>
  </div>
  <div class="card"><h3>Funklage</h3>
    <div class="io-grid" id="rf-kpis"></div>
    <div id="rf-warn" class="warn" style="display:none;margin-top:12px"></div>
    <button class="btn btn-b" onclick="doWifiScan()">WLAN-Scan</button>
    <button class="btn btn-b" onclick="doBleScan()">BLE-Scan</button>
  </div>
  <div class="card"><h3>Kanalbelegung 2.4 GHz</h3><div id="ch-bars"></div></div>
</div>
<div class="pane" id="pane-wifi">
  <div class="card">
    <h3>Access Points</h3>
    <button class="btn btn-b" onclick="loadWifi()">Aktualisieren</button>
    <table><thead><tr><th>SSID</th><th>RSSI</th><th>CH</th><th>Auth</th><th>BSSID</th><th></th></tr></thead>
    <tbody id="ap-body"></tbody></table>
  </div>
</div>
<div class="pane" id="pane-ble">
  <div class="card">
    <h3>BLE-Geraete und Beacons</h3>
    <button class="btn btn-b" onclick="doBleScan()">Scan jetzt</button>
    <button class="btn" onclick="loadBle()">Aktualisieren</button>
    <table><thead><tr><th>Name</th><th>Adresse</th><th>RSSI</th><th>Typ</th><th>Hits</th><th></th></tr></thead>
    <tbody id="ble-body"></tbody></table>
  </div>
</div>
<div class="pane" id="pane-history">
  <div class="grid2">
    <div class="card"><h3>RSSI Verlauf</h3><canvas id="h-rssi"></canvas></div>
    <div class="card"><h3>RF-Score Verlauf</h3><canvas id="h-score"></canvas></div>
  </div>
  <div class="grid2">
    <div class="card"><h3>AP-Anzahl</h3><canvas id="h-ap"></canvas></div>
    <div class="card"><h3>BLE-Anzahl</h3><canvas id="h-ble"></canvas></div>
  </div>
</div>
<div class="pane" id="pane-events">
  <div class="card">
    <h3>Ereignisse</h3>
    <div class="row">
      <div><label>Typ</label><input id="ev-type" placeholder="z.B. wifi.new_ap"></div>
      <div><label>Schwere</label>
        <select id="ev-lvl"><option value="">alle</option><option value="0">ERROR</option><option value="1">WARN</option><option value="2">INFO</option></select>
      </div>
    </div>
    <button class="btn" onclick="loadEvents()">Filtern</button>
    <button class="btn btn-r" onclick="clearEvents()">Leeren</button>
    <table style="margin-top:10px"><thead><tr><th>Zeit</th><th>Typ</th><th>Titel</th><th>Details</th></tr></thead>
    <tbody id="ev-body"></tbody></table>
  </div>
</div>
<div class="pane" id="pane-config">
  <div class="card"><h3>System</h3>
    <div class="row">
      <div><label>Geraetename</label><input id="c-name"></div>
      <div><label>Standort</label><input id="c-loc"></div>
    </div>
    <label>Betriebsmodus</label>
    <select id="c-mode"><option value="live">Live-Monitoring</option><option value="normal">Normalbetrieb</option><option value="diagnostic">Diagnose</option></select>
  </div>
  <div class="card"><h3>Hub</h3>
    <label><input type="checkbox" id="c-enhub"> Hub aktiv</label>
    <div class="row">
      <div><label>Host</label><input id="c-hub"></div>
      <div><label>Port</label><input id="c-port" type="number"></div>
      <div><label>Heartbeat s</label><input id="c-hb" type="number"></div>
    </div>
  </div>
  <div class="card"><h3>WLAN / RF</h3>
    <div class="row">
      <div><label>WLAN-Scanintervall s</label><input id="c-scan" type="number"></div>
      <div><label>RSSI-Warnung dB</label><input id="c-drop" type="number"></div>
      <div><label>RF-Score-Grenze</label><input id="c-scorew" type="number"></div>
    </div>
    <label>Bekannte SSIDs (Komma)</label><input id="c-known">
  </div>
  <div class="card"><h3>BLE</h3>
    <label><input type="checkbox" id="c-enble"> BLE-Scan aktiv</label>
    <div class="row">
      <div><label>BLE-Scanintervall s</label><input id="c-bles" type="number"></div>
    </div>
    <label>Bekannte BLE-Adressen/Namen (Komma)</label><input id="c-kble">
  </div>
  <button class="btn" onclick="saveCfg()">Speichern</button>
  <button class="btn btn-b" onclick="testCfg()">Test</button>
  <button class="btn btn-r" onclick="restart()">Neustart</button>
  <div id="c-msg" style="margin-top:8px;color:#8b949e"></div>
</div>
<div class="pane" id="pane-ota">
  <div class="card">
    <h3>Firmware hochladen</h3>
    <div class="warn">Nach dem Hochladen startet der ESP neu. Bitte das passende Binary (Mini oder S3) waehlen.</div>
    <div class="drop" id="drop">.bin Datei hierher ziehen oder klicken</div>
    <input type="file" id="fw" accept=".bin" style="display:none">
    <div id="fname" style="margin-top:8px;color:#8b949e;text-align:center"></div>
    <button class="btn" onclick="startUpload()">Flashen</button>
    <div class="bar" style="margin-top:12px;display:none" id="prog"><div class="bar-fill" id="obar"></div></div>
    <div id="omsg" style="text-align:center;margin-top:8px;color:#8b949e"></div>
  </div>
</div>
</div>
<script>
function $(id){return document.getElementById(id)}
function jget(u){return fetch(u).then(r=>r.json())}
function jpost(u,b){return fetch(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(b||{})}).then(r=>r.json())}
const ENC={0:'OPEN',1:'WEP',2:'WPA',3:'WPA2',4:'WPA/WPA2',5:'WPA2-ENT',6:'WPA3',7:'WPA2/WPA3',8:'WAPI'};
let hist={rssi:[],rf_score:[],ap_count:[],ble_count:[]};
function drawLine(id,series,color,min,max){
  const c=$(id); if(!c) return;
  const w=c.width=c.clientWidth||320, h=c.height=160;
  const ctx=c.getContext('2d'); ctx.clearRect(0,0,w,h);
  ctx.strokeStyle='#21262d'; ctx.beginPath();
  for(let i=0;i<5;i++){ctx.moveTo(0,h*i/4);ctx.lineTo(w,h*i/4)} ctx.stroke();
  if(!series||series.length<2){ctx.fillStyle='#8b949e';ctx.fillText('keine Daten',10,h/2);return}
  const ys=series.map(p=>p.v); let lo=min, hi=max;
  if(lo==null) lo=Math.min.apply(null,ys); if(hi==null) hi=Math.max.apply(null,ys); if(lo===hi){lo-=1;hi+=1}
  ctx.beginPath(); ctx.strokeStyle=color; ctx.lineWidth=2;
  series.forEach((p,i)=>{const x=i/(series.length-1)*w,y=h-(p.v-lo)/(hi-lo)*h; i?ctx.lineTo(x,y):ctx.moveTo(x,y)});
  ctx.stroke();
  ctx.globalAlpha=.18; ctx.lineTo(w,h); ctx.lineTo(0,h); ctx.closePath(); ctx.fillStyle=color; ctx.fill(); ctx.globalAlpha=1;
  const last=series[series.length-1].v;
  ctx.fillStyle=color; ctx.fillText(Number(last).toFixed(1),8,16);
}
function kpi(v,u,k){return '<div class="io-card"><div class="io-val">'+v+(u?' '+u:'')+'</div><div class="io-key">'+k+'</div></div>'}
function applyStatus(s){
  $('h-name').textContent=s.name||'RF Monitor';
  $('h-ver').textContent='v'+(s.version||'');
  $('h-board').textContent=s.boardLabel||s.board||'';
  $('h-loc').textContent=s.location||'';
  $('d-name').textContent=s.name||'';
  $('d-loc').textContent=s.location||'';
  $('d-board').textContent=s.boardLabel||s.board||'';
  $('d-chip').textContent=s.chip||'';
  $('d-mac').textContent=s.mac||'';
  $('d-ip').innerHTML='<a class="mono" href="http://'+s.ip+'">'+s.ip+'</a>';
  $('d-rssi').textContent=(s.rssi||0)+' dBm';
  $('d-uptime').textContent=s.uptime||'';
  $('d-ver').textContent='v'+(s.version||'');
  const hp=s.heap||0,hpct=Math.min(100,Math.round(hp/3276));
  $('d-heap').textContent=Math.round(hp/1024)+' KB';
  $('b-heap').style.width=hpct+'%';
  $('b-heap').className='bar-fill '+(hpct>40?'green':hpct>20?'yellow':'red');
  const sk=s.sketch||0,spct=Math.min(100,Math.round(sk/19660));
  $('d-sketch').textContent=Math.round(sk/1024)+' KB';
  $('b-sketch').style.width=spct+'%';
  $('d-hub').textContent=s.hub||'';
  $('d-interval').textContent='alle '+(s.interval||30)+' s';
  $('d-hubok').textContent=s.hubEnabled?(s.hubOk?'online':'warte...'):'deaktiviert';
  const rf=s.rf||{};
  $('kpi-grid').innerHTML=
    kpi(rf.rfScore||0,'','RF-Score')+
    kpi(s.rssi||0,'dBm','WLAN RSSI')+
    kpi(rf.apCount||0,'','APs')+
    kpi(rf.bleCount||0,'','BLE');
  $('rf-kpis').innerHTML=
    kpi(rf.wifiRssi||s.rssi||0,'dBm','RSSI')+
    kpi(Number(rf.wifiRssiAvg||0).toFixed(1),'dBm','Mittel')+
    kpi(rf.wifiRssiMin||0,'dBm','Min')+
    kpi(rf.wifiRssiMax||0,'dBm','Max')+
    kpi(Number(rf.wifiRssiJitter||0).toFixed(1),'dB','Jitter')+
    kpi(rf.rfScore||0,'','Score')+
    kpi(rf.channelLoad||0,'','Kanal-Last')+
    kpi(rf.apCount||0,'','APs')+
    kpi(rf.unknownApCount||0,'','unbekannt')+
    kpi(rf.bleCount||0,'','BLE')+
    kpi(rf.beaconCount||0,'','Beacons');
  const warns=[];
  if((rf.rfScore||100)<40) warns.push('RF-Score niedrig');
  if((rf.channelCongestion||0)>=70) warns.push('Kanalueberlastung');
  if((s.rssi||-30)<-75) warns.push('eigene Verbindung schwach');
  $('rf-warn').style.display=warns.length?'block':'none';
  $('rf-warn').textContent=warns.join(' · ');
  hist.rssi.push({v:s.rssi||0}); if(hist.rssi.length>180) hist.rssi.shift();
  hist.rf_score.push({v:Number(rf.rfScore||0)}); if(hist.rf_score.length>180) hist.rf_score.shift();
  hist.ap_count.push({v:Number(rf.apCount||0)}); if(hist.ap_count.length>180) hist.ap_count.shift();
  hist.ble_count.push({v:Number(rf.bleCount||0)}); if(hist.ble_count.length>180) hist.ble_count.shift();
  drawAll();
}
function drawAll(){
  drawLine('c-rssi',hist.rssi,'#58a6ff',-100,-20);
  drawLine('c-score',hist.rf_score,'#3fb950',0,100);
  drawLine('h-rssi',hist.rssi,'#58a6ff',-100,-20);
  drawLine('h-score',hist.rf_score,'#3fb950',0,100);
  drawLine('h-ap',hist.ap_count,'#e3b341',0,null);
  drawLine('h-ble',hist.ble_count,'#a371f7',0,null);
}
function loadHistory(){
  jget('/api/rf/history').then(h=>{
    hist.rssi=h.rssi||[]; hist.rf_score=h.rf_score||[]; hist.ap_count=h.ap_count||[]; hist.ble_count=h.ble_count||[];
    drawAll();
  }).catch(()=>{});
}
function loadWifi(){
  jget('/api/wifi/list').then(d=>{
    $('ap-body').innerHTML=(d.aps||[]).map(a=>'<tr><td>'+(a.hidden?'<i>versteckt</i>':a.ssid)+'</td><td>'+a.rssi+'</td><td>'+a.channel+'</td><td>'+(ENC[a.enc]||a.enc)+'</td><td class="mono">'+a.bssid+'</td><td><span class="tag '+(a.known?'k':'u')+'">'+(a.known?'bekannt':'neu')+'</span></td></tr>').join('')||'<tr><td colspan="6">keine Daten</td></tr>';
  });
  jget('/api/wifi/channels').then(d=>{
    const max=Math.max(1,...(d.channels||[]).map(c=>c.count));
    $('ch-bars').innerHTML=(d.channels||[]).map(c=>'<div class="ch-row"><span>'+c.ch+'</span><div class="bar" style="flex:1"><div class="bar-fill '+(c.count>4?'red':c.count>2?'yellow':'green')+'" style="width:'+(c.count/max*100)+'%"></div></div><span>'+c.count+'</span></div>').join('');
  });
}
function loadBle(){
  jget('/api/ble/list').then(d=>{
    $('ble-body').innerHTML=(d.devices||[]).map(a=>'<tr><td>'+(a.name||'-')+'</td><td class="mono">'+a.addr+'</td><td>'+a.rssi+'</td><td><span class="tag '+(a.kind>0?'b':'')+'">'+(a.type||'')+'</span></td><td>'+a.hits+'</td><td>'+(a.known?'bekannt':'')+'</td></tr>').join('')||'<tr><td colspan="6">keine BLE-Geraete (Scan laeuft zyklisch)</td></tr>';
  });
}
function loadEvents(){
  const t=$('ev-type').value, l=$('ev-lvl').value;
  let u='/api/events/recent'; const q=[]; if(t) q.push('type='+encodeURIComponent(t)); if(l!=='') q.push('level='+l); if(q.length) u+='?'+q.join('&');
  jget(u).then(d=>{
    $('ev-body').innerHTML=(d.events||[]).map(e=>'<tr class="lvl'+e.level+'"><td>'+e.ts+'s</td><td class="mono">'+e.type+'</td><td>'+e.title+'</td><td>'+e.details+'</td></tr>').join('')||'<tr><td colspan="4">keine Ereignisse</td></tr>';
  });
}
function clearEvents(){jpost('/api/events/clear',{}).then(loadEvents)}
function doWifiScan(){jget('/api/wifi/scan').then(()=>setTimeout(loadWifi,4000))}
function doBleScan(){jget('/api/ble/scan').then(()=>setTimeout(loadBle,5000))}
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
  if(t.dataset.tab==='rf'||t.dataset.tab==='wifi') loadWifi();
  if(t.dataset.tab==='ble') loadBle();
  if(t.dataset.tab==='events') loadEvents();
  if(t.dataset.tab==='config') loadCfg();
  if(t.dataset.tab==='history') loadHistory();
});
function applyHash(){
  const p=location.pathname.replace('/',''), map={rf:'rf',wifi:'wifi',ble:'ble',history:'history',eventlog:'events',config:'config',ota:'ota'};
  const h=map[p]||(location.hash||'#status').slice(1);
  const tab=document.querySelector('.tab[data-tab="'+h+'"]'); if(tab) tab.click();
}
window.addEventListener('hashchange',applyHash);
const drop=$('drop'),inp=$('fw');
drop.onclick=()=>inp.click();
inp.onchange=()=>{if(inp.files[0])$('fname').textContent=inp.files[0].name+' ('+Math.round(inp.files[0].size/1024)+' KB)'};
drop.ondragover=e=>{e.preventDefault();drop.style.borderColor='#58a6ff'};
drop.ondragleave=()=>drop.style.borderColor='#30363d';
drop.ondrop=e=>{e.preventDefault();drop.style.borderColor='#30363d';const f=e.dataTransfer.files[0];if(f&&f.name.endsWith('.bin')){const dt=new DataTransfer();dt.items.add(f);inp.files=dt.files;$('fname').textContent=f.name}};
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
es.onopen=()=>{$('sse-dot').className='dot dot-on';$('sse-lbl').textContent='Live'};
es.onerror=()=>{$('sse-dot').className='dot dot-off';$('sse-lbl').textContent='Getrennt'};
es.onmessage=e=>{try{applyStatus(JSON.parse(e.data))}catch(ex){}};
jget('/api/status').then(applyStatus); loadHistory(); applyHash();
</script></body></html>
)RFUI";
