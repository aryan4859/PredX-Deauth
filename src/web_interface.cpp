#include <WebServer.h>
#include <string.h>
#include "monitor.h"
#include "web_interface.h"

WebServer server(80);

namespace {
String json_string(const char *value) {
  String escaped;
  escaped.reserve(strlen(value) + 8);
  escaped += '"';
  for (const char *cursor = value; *cursor; ++cursor) {
    const uint8_t character = static_cast<uint8_t>(*cursor);
    if (character == '"' || character == '\\') {
      escaped += '\\';
      escaped += static_cast<char>(character);
    } else if (character >= 0x20) {
      escaped += static_cast<char>(character);
    }
  }
  escaped += '"';
  return escaped;
}

void handle_root() {
  static const char page[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#101827"><title>PredX Guardian</title>
<style>
:root{color-scheme:dark;--bg:#101827;--panel:#192438;--line:#2a3952;--text:#e7edf7;--muted:#9aa9bd;--accent:#65d6ad;--warn:#ffbd69}
*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at top,#1b2b42,var(--bg) 60%);color:var(--text);font:15px/1.5 system-ui,sans-serif}
main{max-width:1100px;margin:auto;padding:28px 20px 50px}header{display:flex;justify-content:space-between;align-items:center;gap:15px;flex-wrap:wrap}
h1{font-size:clamp(1.7rem,4vw,2.4rem);margin:0}.subtitle,.muted{color:var(--muted)}.badge{border:1px solid #26684f;color:var(--accent);border-radius:99px;padding:5px 12px;font-size:.85rem}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:14px;margin:24px 0}
.card,section{background:var(--panel);border:1px solid var(--line);border-radius:14px;padding:18px}.metric{font-size:1.9rem;font-weight:700;margin-top:3px}
.row{display:flex;justify-content:space-between;align-items:center;gap:12px;flex-wrap:wrap}h2{font-size:1.15rem;margin:0 0 12px}
button{border:0;border-radius:8px;padding:10px 14px;background:var(--accent);color:#082218;font-weight:700;cursor:pointer}button.secondary{background:#33445e;color:var(--text)}
.table-wrap{overflow:auto}table{width:100%;border-collapse:collapse;min-width:620px}th,td{text-align:left;padding:11px 9px;border-bottom:1px solid var(--line)}th{color:var(--muted);font-size:.82rem;font-weight:600}
.spacer{height:16px}.events{display:grid;gap:9px}.event{border:1px solid var(--line);border-left:3px solid var(--warn);padding:11px 13px;border-radius:8px;background:#141f31}
.event.info{border-left-color:var(--accent)}.event strong{margin-right:8px}.empty{color:var(--muted);padding:12px 0}.note{font-size:.88rem;color:var(--muted);margin:12px 0 0}
</style></head>
<body><main>
<header><div><h1>PredX Guardian</h1><div class="subtitle">Passive Wi-Fi security monitor</div></div><span class="badge" id="status">Starting…</span></header>
<div class="cards">
<div class="card"><div class="muted">Monitor channel</div><div class="metric" id="channel">—</div></div>
<div class="card"><div class="muted">Access points observed</div><div class="metric" id="apCount">0</div></div>
<div class="card"><div class="muted">Deauth / disassoc frames</div><div class="metric" id="frameCount">0</div></div>
<div class="card"><div class="muted">Security events</div><div class="metric" id="eventCount">0</div></div>
</div>
<section><div class="row"><h2>Observed access points</h2><span class="muted">Beacon and probe-response frames on this channel</span></div>
<div class="table-wrap"><table><thead><tr><th>SSID</th><th>BSSID</th><th>Security</th><th>Signal</th><th>Channel</th></tr></thead><tbody id="aps"></tbody></table></div>
<div class="empty" id="noAps">Waiting for nearby access point beacons…</div>
<p class="note">A matching SSID with weaker security is a review signal, not proof of a rogue AP. Confirm against your known network inventory.</p></section>
<div class="spacer"></div>
<section><div class="row"><div><h2>Security event log</h2><div class="muted">Recent deauthentication, disassociation, and SSID security mismatch alerts</div></div>
<div class="row"><button id="simulate" class="secondary">Add simulated event</button><button id="clear" class="secondary">Clear log</button></div></div>
<div class="events" id="events"></div><div class="empty" id="noEvents">No events recorded.</div>
<p class="note">Simulation only adds a labeled example to this log; it sends no radio frames. Data is held in RAM and resets on reboot.</p></section>
<p class="note">Passive monitor is fixed to channel <span id="channelNote">—</span>; other channels are not observed. This tool does not transmit deauthentication frames.</p>
</main>
<script>
const text=(id,value)=>document.getElementById(id).textContent=value;
function cell(row,value){const td=document.createElement("td");td.textContent=value;row.appendChild(td)}
function render(data){
 text("status",data.active?"MONITORING":"MONITOR OFF");text("channel",data.channel);text("channelNote",data.channel);
 text("apCount",data.aps.length);text("frameCount",data.deauthCount);text("eventCount",data.events.length);
 const aps=document.getElementById("aps");aps.replaceChildren();
 data.aps.forEach(ap=>{const row=document.createElement("tr");cell(row,ap.ssid);cell(row,ap.bssid);cell(row,ap.security);cell(row,ap.rssi+" dBm");cell(row,String(ap.channel));aps.appendChild(row)});
 document.getElementById("noAps").hidden=data.aps.length!==0;
 const events=document.getElementById("events");events.replaceChildren();
 data.events.slice().reverse().forEach(event=>{
  const item=document.createElement("div");item.className="event "+(event.severity==="info"?"info":"");
  const heading=document.createElement("div");const title=document.createElement("strong");title.textContent=event.type+(event.simulated?" · SIMULATED":"");heading.appendChild(title);
  const severity=document.createElement("span");severity.textContent=event.severity;severity.className="muted";heading.appendChild(severity);item.appendChild(heading);
  const detail=document.createElement("div");detail.textContent=(event.ssid||"")+" · "+event.bssid+" · count "+event.count;item.appendChild(detail);
  const time=document.createElement("small");time.className="muted";time.textContent="Uptime "+Math.floor(event.timestamp/1000)+"s";item.appendChild(time);events.appendChild(item);
 });
 document.getElementById("noEvents").hidden=data.events.length!==0;
}
async function refresh(){try{const response=await fetch("/api/state",{cache:"no-store"});if(!response.ok)throw new Error("state request failed");render(await response.json())}catch(e){text("status","OFFLINE")}}
document.getElementById("simulate").addEventListener("click",async()=>{await fetch("/api/simulate",{method:"POST"});refresh()});
document.getElementById("clear").addEventListener("click",async()=>{await fetch("/api/clear",{method:"POST"});refresh()});
refresh();setInterval(refresh,2000);
</script></body></html>)HTML";
  server.send_P(200, "text/html", page);
}

void handle_state() {
  static ObservedAccessPoint aps[32];
  static SecurityEvent events[32];
  const size_t ap_count = get_access_points(aps, 32);
  const size_t event_count = get_security_events(events, 32);

  String json;
  json.reserve(128 + ap_count * 110 + event_count * 150);
  json += "{\"active\":";
  json += monitor_is_active() ? "true" : "false";
  json += ",\"channel\":";
  json += String(monitor_channel());
  json += ",\"deauthCount\":";
  json += String(deauth_frame_count());
  json += ",\"aps\":[";
  for (size_t i = 0; i < ap_count; ++i) {
    if (i) json += ',';
    json += "{\"ssid\":" + json_string(aps[i].ssid) +
            ",\"bssid\":" + json_string(aps[i].bssid) +
            ",\"security\":" + json_string(aps[i].security) +
            ",\"rssi\":" + String(aps[i].rssi) +
            ",\"channel\":" + String(aps[i].channel) + "}";
  }
  json += "],\"events\":[";
  for (size_t i = 0; i < event_count; ++i) {
    if (i) json += ',';
    json += "{\"timestamp\":" + String(events[i].timestamp_ms) +
            ",\"type\":" + json_string(events[i].type) +
            ",\"severity\":" + json_string(events[i].severity) +
            ",\"ssid\":" + json_string(events[i].ssid) +
            ",\"bssid\":" + json_string(events[i].bssid) +
            ",\"count\":" + String(events[i].count) +
            ",\"simulated\":" + (events[i].simulated ? "true" : "false") + "}";
  }
  json += "]}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void handle_simulate() {
  add_simulated_event();
  server.send(202, "application/json", "{\"ok\":true,\"simulated\":true}");
}

void handle_clear() {
  clear_security_events();
  server.send(200, "application/json", "{\"ok\":true}");
}
}

void start_web_interface() {
  server.on("/", HTTP_GET, handle_root);
  server.on("/api/state", HTTP_GET, handle_state);
  server.on("/api/simulate", HTTP_POST, handle_simulate);
  server.on("/api/clear", HTTP_POST, handle_clear);
  server.onNotFound([]() {
    server.send(404, "application/json", "{\"error\":\"not found\"}");
  });
  server.begin();
}

void web_interface_handle_client() {
  server.handleClient();
}
