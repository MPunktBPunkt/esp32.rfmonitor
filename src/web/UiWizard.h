#pragma once
#include <Arduino.h>

const char PAGE_WIZARD[] PROGMEM = R"RFWIZ(<!DOCTYPE html>
<html lang="de"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>RF Monitor Setup</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#0d1117;color:#e6edf3;font-family:Segoe UI,sans-serif;font-size:14px}
header{background:#161b22;border-bottom:1px solid #30363d;padding:14px 20px}
h1{font-size:18px;color:#58a6ff}
.wrap{max-width:760px;margin:0 auto;padding:18px}
.steps{display:flex;gap:6px;flex-wrap:wrap;margin-bottom:16px}
.steps span{padding:6px 10px;border-radius:12px;border:1px solid #30363d;color:#8b949e;font-size:12px}
.steps span.on{border-color:#58a6ff;color:#58a6ff}
.card{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:16px;margin-bottom:14px}
label{display:block;color:#8b949e;font-size:12px;margin:8px 0 4px}
input,select{width:100%;background:#0d1117;border:1px solid #30363d;color:#e6edf3;border-radius:6px;padding:8px;font:inherit}
.btn{background:#238636;color:#fff;border:none;padding:9px 16px;border-radius:6px;cursor:pointer;font-weight:600;margin:6px 6px 0 0}
.btn-b{background:#1f6feb}.btn-r{background:#6e7681}
.hint{color:#8b949e;font-size:12px;margin-top:6px}
.ok{color:#3fb950}
</style></head><body>
<header><h1>esp32.rfmonitor Setup</h1>
<div style="margin-top:8px"><a href="/" style="color:#8b949e;font-size:12px">Zur UI</a></div>
</header>
<div class="wrap">
<div class="steps" id="steps"></div>
<div id="view"></div>
<div>
  <button class="btn-r btn" id="back">Zurueck</button>
  <button class="btn btn-b" id="next">Weiter</button>
</div>
</div>
<script>
const STEPS=['Willkommen','Geraet','Hub','Scans','Grenzwerte','Fertig'];
let step=0, cfg={}, draft={};
const $=s=>document.getElementById(s);
function jget(u){return fetch(u).then(r=>r.json())}
function jpost(u,b){return fetch(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(b||{})}).then(r=>r.json())}
function renderSteps(){$('steps').innerHTML=STEPS.map((s,i)=>'<span class="'+(i===step?'on':'')+'">'+(i+1)+'. '+s+'</span>').join('')}
function v(){
  if(step===0) $('view').innerHTML='<div class="card"><h3>RF Environment Monitor</h3><p>Dieser Knoten beobachtet WLAN und BLE an einem festen Standort. WLAN ist bereits konfiguriert. Hier setzt du Name, Hub und Scan-Intervalle.</p><p class="hint">Kein Spektrumanalysator und kein ToolNode — ein spezialisierter Monitor.</p></div>';
  if(step===1) $('view').innerHTML='<div class="card"><label>Geraetename</label><input id="name" value="'+(draft.deviceName||cfg.deviceName||'RF Monitor')+'"><label>Standort</label><input id="loc" value="'+(draft.locationName||cfg.locationName||'')+'"><p class="hint">z.B. Technikraum, Flur, Wohnzimmer</p></div>';
  if(step===2) $('view').innerHTML='<div class="card"><label><input type="checkbox" id="enhub" '+(draft.enableHub!==false && cfg.enableHub!==false?'checked':'')+'> Hub aktiv</label><label>Hub-IP</label><input id="hub" value="'+(draft.hubHost||cfg.hubHost||'')+'"><label>Port</label><input id="port" type="number" value="'+(draft.hubPort||cfg.hubPort||8093)+'"></div>';
  if(step===3) $('view').innerHTML='<div class="card"><label>Modus</label><select id="mode"><option value="normal">Normalbetrieb</option><option value="live">Live-Monitoring</option><option value="diagnostic">Diagnose</option></select><label>WLAN-Scanintervall (s)</label><input id="scan" type="number" value="'+(draft.wifiScanIntervalS||cfg.wifiScanIntervalS||20)+'"><label><input type="checkbox" id="enble" '+(draft.bleEnabled!==false && cfg.bleEnabled!==false?'checked':'')+'> BLE-Scan aktiv</label><label>BLE-Scanintervall (s)</label><input id="bles" type="number" value="'+(draft.bleScanIntervalS||cfg.bleScanIntervalS||15)+'"><label>Bekannte SSIDs (Komma)</label><input id="known" value="'+(draft.knownSsids||cfg.knownSsids||'')+'"></div>';
  if(step===4) $('view').innerHTML='<div class="card"><label>RSSI-Abfall Warnung (dB)</label><input id="drop" type="number" value="'+(draft.rssiWarnDrop||cfg.rssiWarnDrop||12)+'"><label>RF-Score Grenze</label><input id="score" type="number" value="'+(draft.rfScoreWarn||cfg.rfScoreWarn||40)+'"><label>Bekannte BLE-Geraete optional</label><input id="kble" value="'+(draft.knownBle||cfg.knownBle||'')+'"></div>';
  if(step===5) $('view').innerHTML='<div class="card"><p class="ok">Setup wird gespeichert. Danach oeffnet sich die Monitor-UI.</p></div>';
  if(step===3) $('mode').value=draft.opMode||cfg.opMode||'normal';
}
function collect(){
  if($('name')){draft.deviceName=$('name').value; draft.locationName=$('loc').value}
  if($('hub')){draft.enableHub=$('enhub').checked; draft.hubHost=$('hub').value; draft.hubPort=Number($('port').value)}
  if($('scan')){draft.opMode=$('mode').value; draft.wifiScanIntervalS=Number($('scan').value); draft.bleEnabled=$('enble').checked; draft.bleScanIntervalS=Number($('bles').value); draft.knownSsids=$('known').value}
  if($('drop')){draft.rssiWarnDrop=Number($('drop').value); draft.rfScoreWarn=Number($('score').value); draft.knownBle=$('kble').value}
}
function finish(){
  collect();
  const p=Object.assign({},cfg,draft,{firstRunDone:true});
  jpost('/api/config/save',p).then(()=>{location.href='/'});
}
$('next').onclick=()=>{collect(); if(step>=STEPS.length-1) finish(); else {step++; renderSteps(); v();}};
$('back').onclick=()=>{if(step>0){collect(); step--; renderSteps(); v();}};
jget('/api/config/get').then(c=>{cfg=c; draft=Object.assign({},c); renderSteps(); v();});
</script></body></html>
)RFWIZ";
