const http = require("http");
const net = require("net");
const fs = require("fs");
const path = require("path");
const { randomUUID } = require("crypto");

const PORT = Number(process.env.PORT || 8080);
const MQTT_HOST = process.env.MQTT_HOST || "mqtt";
const MQTT_PORT = Number(process.env.MQTT_PORT || 1883);
const TELEMETRY_TOPIC = process.env.TELEMETRY_TOPIC || "dc/telemetry";
const LOCAL_TIME_ZONE = process.env.TZ || "America/Sao_Paulo";
const FIRMWARE_FILE = process.env.FIRMWARE_FILE || "firmware.bin";
const FIRMWARE_PATH = path.join(__dirname, FIRMWARE_FILE);

const state = {
  connected: false,
  lastMessageAt: null,
  devices: {},
  events: [],
};

let mqttSocket = null;
let mqttPacketId = 1;

function nowLocalIso() {
  const parts = new Intl.DateTimeFormat("sv-SE", {
    timeZone: LOCAL_TIME_ZONE,
    year: "numeric",
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    fractionalSecondDigits: 3,
  }).formatToParts(new Date());

  const lookup = Object.fromEntries(parts.map((part) => [part.type, part.value]));
  return `${lookup.year}-${lookup.month}-${lookup.day}T${lookup.hour}:${lookup.minute}:${lookup.second}.${lookup.fractionalSecond}`;
}

function encodeString(value) {
  const body = Buffer.from(String(value));
  const header = Buffer.alloc(2);
  header.writeUInt16BE(body.length, 0);
  return Buffer.concat([header, body]);
}

function encodeLength(length) {
  const bytes = [];
  do {
    let digit = length % 128;
    length = Math.floor(length / 128);
    if (length > 0) digit |= 128;
    bytes.push(digit);
  } while (length > 0);
  return Buffer.from(bytes);
}

function packet(type, flags, body) {
  return Buffer.concat([Buffer.from([(type << 4) | flags]), encodeLength(body.length), body]);
}

function connectPacket() {
  const clientId = `dc-monitor-${randomUUID().slice(0, 8)}`;
  const variableHeader = Buffer.concat([
    encodeString("MQTT"),
    Buffer.from([4, 2, 0, 30]),
  ]);
  return packet(1, 0, Buffer.concat([variableHeader, encodeString(clientId)]));
}

function subscribePacket(topic) {
  const id = Buffer.alloc(2);
  id.writeUInt16BE(mqttPacketId++ || 1, 0);
  return packet(8, 2, Buffer.concat([id, encodeString(topic), Buffer.from([0])]));
}

function publishPacket(topic, payload) {
  return packet(3, 0, Buffer.concat([encodeString(topic), Buffer.from(payload)]));
}

function parseRemainingLength(buffer, offset) {
  let multiplier = 1;
  let value = 0;
  let bytes = 0;
  let digit;
  do {
    digit = buffer[offset + bytes];
    value += (digit & 127) * multiplier;
    multiplier *= 128;
    bytes += 1;
  } while ((digit & 128) !== 0);
  return { value, bytes };
}

function readMqttPackets(buffer, onPacket) {
  let offset = 0;
  while (offset + 2 <= buffer.length) {
    const first = buffer[offset];
    const remaining = parseRemainingLength(buffer, offset + 1);
    const start = offset + 1 + remaining.bytes;
    const end = start + remaining.value;
    if (end > buffer.length) break;
    onPacket(first >> 4, buffer.subarray(start, end));
    offset = end;
  }
}

function handlePublish(body) {
  const topicLength = body.readUInt16BE(0);
  const topic = body.subarray(2, 2 + topicLength).toString();
  const payload = body.subarray(2 + topicLength).toString();
  if (topic !== TELEMETRY_TOPIC) return;

  let message;
  try {
    message = JSON.parse(payload);
  } catch {
    message = { raw: payload };
  }

  const now = nowLocalIso();
  const deviceId = String(message.serialNumber || "unknown");
  const event = { at: now, topic, ...message };
  state.lastMessageAt = now;
  state.devices[deviceId] = { ...state.devices[deviceId], ...message, lastSeenAt: now };
  state.events.unshift(event);
  state.events = state.events.slice(0, 100);
  console.log(`[mqtt] ${topic} ${payload}`);
}

function startMqtt() {
  mqttSocket = net.createConnection({ host: MQTT_HOST, port: MQTT_PORT }, () => {
    state.connected = true;
    mqttSocket.write(connectPacket());
    mqttSocket.write(subscribePacket(TELEMETRY_TOPIC));
    console.log(`[mqtt] connected to ${MQTT_HOST}:${MQTT_PORT}`);
  });

  mqttSocket.on("data", (data) => {
    readMqttPackets(data, (type, body) => {
      if (type === 3) handlePublish(body);
    });
  });

  mqttSocket.on("close", () => {
    state.connected = false;
    console.log("[mqtt] disconnected, retrying in 3s");
    setTimeout(startMqtt, 3000);
  });

  mqttSocket.on("error", (error) => {
    state.connected = false;
    console.error(`[mqtt] ${error.message}`);
  });
}

function sendJson(res, status, value) {
  const body = JSON.stringify(value, null, 2);
  res.writeHead(status, {
    "content-type": "application/json; charset=utf-8",
    "cache-control": "no-store",
  });
  res.end(body);
}

function readBody(req) {
  return new Promise((resolve, reject) => {
    const chunks = [];
    req.on("data", (chunk) => chunks.push(chunk));
    req.on("end", () => resolve(Buffer.concat(chunks).toString()));
    req.on("error", reject);
  });
}

function renderDashboard() {
  return `<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>DC Monitor</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
  <style>
    *, *::before, *::after { box-sizing: border-box; }

    :root {
      color-scheme: light;
      font-family: 'Inter', system-ui, -apple-system, sans-serif;
      color: #1e293b;
      background: #eef2f7;
      --shadow-sm: 0 1px 2px rgba(15,23,42,.05);
      --shadow-md: 0 8px 24px rgba(15,23,42,.07);
      --shadow-lg: 0 18px 42px rgba(15,23,42,.10);
      --radius: 12px;
      --radius-sm: 8px;
      --blue-50: #eff6ff;
      --blue-100: #dbeafe;
      --blue-500: #3b82f6;
      --blue-600: #2563eb;
      --blue-700: #1d4ed8;
      --blue-900: #1e3a5f;
      --green-50: #f0fdf4;
      --green-500: #22c55e;
      --green-600: #16a34a;
      --red-50: #fef2f2;
      --red-500: #ef4444;
      --red-600: #dc2626;
      --slate-50: #f8fafc;
      --slate-100: #f1f5f9;
      --slate-200: #e2e8f0;
      --slate-300: #cbd5e1;
      --slate-400: #94a3b8;
      --slate-500: #64748b;
      --slate-600: #475569;
      --slate-700: #334155;
      --slate-800: #1e293b;
      --slate-900: #0f172a;
    }

    body {
      margin: 0;
      min-height: 100vh;
      background:
        linear-gradient(180deg, rgba(255,255,255,.72), rgba(255,255,255,0) 280px),
        #eef2f7;
    }

    header {
      background: rgba(248,250,252,.92);
      color: var(--slate-900);
      padding: 20px 32px;
      border-bottom: 1px solid rgba(203,213,225,.72);
      box-shadow: var(--shadow-sm);
      backdrop-filter: blur(14px);
      position: sticky;
      top: 0;
      z-index: 100;
    }
    header h1 {
      margin: 0;
      font-size: 22px;
      font-weight: 700;
      letter-spacing: -0.3px;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    header h1::before {
      content: '';
      display: inline-block;
      width: 8px;
      height: 28px;
      background: var(--blue-600);
      border-radius: 4px;
    }

    main {
      padding: 28px 32px;
      max-width: 1360px;
      margin: 0 auto;
      animation: fadeIn .4s ease;
    }

    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(8px); }
      to { opacity: 1; transform: translateY(0); }
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
    }

    .panel {
      background: rgba(255,255,255,.94);
      border: 1px solid rgba(203,213,225,.82);
      border-radius: var(--radius);
      padding: 22px 24px;
      box-shadow: var(--shadow-md);
      transition: box-shadow .2s ease, transform .2s ease, border-color .2s ease;
    }
    .panel:hover {
      border-color: rgba(148,163,184,.8);
      box-shadow: var(--shadow-lg);
      transform: translateY(-1px);
    }

    h2 {
      margin: 0 0 16px;
      font-size: 13px;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.8px;
      color: var(--slate-500);
    }

    .status {
      display: inline-flex;
      align-items: center;
      gap: 10px;
      font-weight: 600;
      font-size: 15px;
      color: var(--red-600);
    }
    .status .dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: var(--red-500);
      box-shadow: 0 0 0 3px var(--red-50);
      transition: all .3s ease;
    }
    .status.ok { color: var(--green-600); }
    .status.ok .dot {
      background: var(--green-500);
      box-shadow: 0 0 0 3px var(--green-50), 0 0 8px rgba(34,197,94,.4);
      animation: pulse 2s ease-in-out infinite;
    }

    @keyframes pulse {
      0%, 100% { box-shadow: 0 0 0 3px var(--green-50), 0 0 8px rgba(34,197,94,.3); }
      50% { box-shadow: 0 0 0 6px var(--green-50), 0 0 12px rgba(34,197,94,.5); }
    }

    .topic-badge {
      display: inline-block;
      margin-top: 12px;
      padding: 5px 12px;
      background: var(--slate-100);
      border: 1px solid var(--slate-200);
      border-radius: 6px;
      font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
      font-size: 13px;
      color: var(--slate-600);
    }

    .stat-row {
      display: flex;
      gap: 20px;
      margin-top: 14px;
      flex-wrap: wrap;
    }
    .stat-item {
      display: flex;
      flex-direction: column;
      gap: 2px;
    }
    .stat-label {
      font-size: 11px;
      font-weight: 500;
      text-transform: uppercase;
      letter-spacing: 0.5px;
      color: var(--slate-400);
    }
    .stat-value {
      font-size: 20px;
      font-weight: 700;
      color: var(--slate-800);
    }

    .table-wrap {
      overflow-x: auto;
      border: 1px solid var(--slate-200);
      border-radius: var(--radius-sm);
      background: white;
    }

    table {
      width: 100%;
      border-collapse: separate;
      border-spacing: 0;
      min-width: 980px;
    }
    th {
      background: var(--slate-50);
      padding: 11px 14px;
      text-align: left;
      font-size: 11px;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.6px;
      color: var(--slate-500);
      border-bottom: 1px solid var(--slate-200);
    }
    td {
      padding: 11px 14px;
      border-bottom: 1px solid var(--slate-100);
      font-size: 13px;
      color: var(--slate-700);
      vertical-align: top;
    }
    tr:last-child td { border-bottom: none; }
    tr:hover td { background: var(--slate-50); }
    .payload-cell { min-width: 420px; }
    .event-detail-cell {
      padding-top: 0;
      background: #fbfcff;
    }
    .event-detail-row:hover td {
      background: #fbfcff;
    }
    .event-payload-layout {
      display: grid;
      grid-template-columns: minmax(260px, .75fr) minmax(360px, 1fr);
      gap: 12px;
      align-items: stretch;
      min-width: 660px;
    }
    .payload-summary-card {
      display: grid;
      gap: 10px;
      min-width: 260px;
      padding: 12px;
      border: 1px solid var(--slate-200);
      border-radius: 10px;
      background: linear-gradient(180deg, #ffffff, var(--slate-50));
      box-shadow: var(--shadow-sm);
    }
    .summary-grid {
      display: grid;
      grid-template-columns: repeat(6, minmax(0, 1fr));
      gap: 8px;
    }
    .summary-row.span-2 { grid-column: span 2; }
    .summary-row.span-3 { grid-column: span 3; }
    .payload-summary-title {
      color: var(--slate-800);
      font-size: 12px;
      font-weight: 800;
      letter-spacing: .2px;
    }
    .summary-row {
      display: grid;
      grid-template-columns: 28px 1fr;
      gap: 10px;
      align-items: center;
      padding: 8px;
      border: 1px solid var(--slate-100);
      border-radius: 8px;
      background: white;
    }
    .summary-content {
      min-width: 0;
    }
    .summary-icon {
      width: 28px;
      height: 28px;
      display: grid;
      place-items: center;
      border-radius: 8px;
      background: var(--blue-50);
      color: var(--blue-700);
    }
    .summary-icon svg {
      width: 16px;
      height: 16px;
      stroke: currentColor;
      stroke-width: 2;
      fill: none;
      stroke-linecap: round;
      stroke-linejoin: round;
    }
    .summary-label {
      display: block;
      color: var(--slate-400);
      font-size: 10px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: .5px;
      margin-bottom: 2px;
    }
    .summary-value {
      display: block;
      color: var(--slate-800);
      font-size: 13px;
      font-weight: 700;
      line-height: 1.25;
      overflow-wrap: anywhere;
      word-break: break-word;
    }
    .payload-card {
      display: grid;
      gap: 8px;
      min-width: 360px;
      max-width: 560px;
      padding: 10px;
      border: 1px solid var(--slate-200);
      border-radius: 10px;
      background: #ffffff;
      box-shadow: var(--shadow-sm);
    }
    .payload-toolbar {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 10px;
    }
    .payload-title {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      color: var(--slate-600);
      font-size: 12px;
      font-weight: 700;
    }
    .payload-title::before {
      content: '';
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--green-500);
      box-shadow: 0 0 0 3px var(--green-50);
    }
    .copy-btn {
      height: 28px;
      padding: 0 10px;
      border-radius: 999px;
      border: 1px solid var(--slate-200);
      background: white;
      color: var(--slate-600);
      font-size: 12px;
      font-weight: 700;
      box-shadow: none;
    }
    .copy-btn:hover {
      background: var(--blue-50);
      border-color: var(--blue-100);
      color: var(--blue-700);
      transform: none;
      box-shadow: none;
    }
    .copy-btn.copied {
      background: var(--green-50);
      border-color: rgba(34,197,94,.28);
      color: var(--green-600);
    }
    .payload-box {
      margin: 0;
      max-height: 190px;
      overflow: auto;
      white-space: pre-wrap;
      word-break: break-word;
      background: #0f172a;
      color: #e2e8f0;
      border: 1px solid #1e293b;
      border-radius: 8px;
      padding: 12px;
      font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
      font-size: 12px;
      line-height: 1.55;
      box-shadow: inset 0 1px 0 rgba(255,255,255,.04);
    }
    .rssi-badge {
      display: inline-flex;
      align-items: center;
      gap: 4px;
      padding: 3px 10px;
      border-radius: 20px;
      font-size: 12px;
      font-weight: 600;
    }
    .rssi-good { background: var(--green-50); color: var(--green-600); }
    .rssi-mid { background: #fffbeb; color: #d97706; }
    .rssi-bad { background: var(--red-50); color: var(--red-600); }

    form {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      align-items: center;
    }

    input, select {
      height: 40px;
      border: 1px solid var(--slate-300);
      border-radius: var(--radius-sm);
      padding: 0 14px;
      font-family: inherit;
      font-size: 14px;
      color: var(--slate-700);
      background: white;
      outline: none;
      transition: border-color .2s, box-shadow .2s;
    }
    input:focus, select:focus {
      border-color: var(--blue-500);
      box-shadow: 0 0 0 3px rgba(59,130,246,.15);
    }
    input::placeholder { color: var(--slate-400); }

    button {
      height: 40px;
      padding: 0 20px;
      border: none;
      border-radius: var(--radius-sm);
      font-family: inherit;
      font-size: 14px;
      font-weight: 600;
      cursor: pointer;
      transition: all .15s ease;
      display: inline-flex;
      align-items: center;
      gap: 6px;
    }
    .btn-primary {
      background: linear-gradient(135deg, var(--blue-600) 0%, var(--blue-500) 100%);
      color: white;
      box-shadow: 0 1px 3px rgba(37,99,235,.3);
    }
    .btn-primary:hover {
      background: linear-gradient(135deg, var(--blue-700) 0%, var(--blue-600) 100%);
      box-shadow: 0 4px 12px rgba(37,99,235,.35);
      transform: translateY(-1px);
    }
    .btn-primary:active { transform: translateY(0); }

    .btn-danger {
      background: white;
      color: var(--red-600);
      border: 1px solid var(--slate-200);
    }
    .btn-danger:hover {
      background: var(--red-50);
      border-color: var(--red-500);
    }

    .actions-form { margin-top: 12px; }

    .empty-state {
      text-align: center;
      padding: 32px 16px;
      color: var(--slate-400);
      font-size: 14px;
    }
    .empty-state::before {
      content: '';
      display: block;
      width: 40px;
      height: 40px;
      margin: 0 auto 12px;
      background: var(--slate-100);
      border-radius: 50%;
    }

    @media (max-width: 768px) {
      header { padding: 16px 20px; }
      main { padding: 20px 16px; }
      .grid { grid-template-columns: 1fr; }
      .event-payload-layout {
        grid-template-columns: 1fr;
        min-width: 360px;
      }
      .summary-row.span-2,
      .summary-row.span-3 {
        grid-column: span 6;
      }
    }
  </style>
</head>
<body>
  <header><h1>DC Monitor</h1></header>
  <main>
    <div class="grid">
      <section class="panel">
        <h2>Broker MQTT</h2>
        <div id="mqttStatus" class="status"><span class="dot"></span><span>Carregando</span></div>
        <div class="topic-badge">${TELEMETRY_TOPIC}</div>
        <div class="stat-row">
          <div class="stat-item"><span class="stat-label">Dispositivos</span><span class="stat-value" id="statDevices">0</span></div>
          <div class="stat-item"><span class="stat-label">Eventos</span><span class="stat-value" id="statEvents">0</span></div>
          <div class="stat-item"><span class="stat-label">Ultimo contato</span><span class="stat-value" id="statLast" style="font-size:13px;font-weight:500;color:var(--slate-500)">-</span></div>
        </div>
      </section>
      <section class="panel">
        <h2>Comandos</h2>
        <form id="cmdForm">
          <input id="deviceId" placeholder="ID do dispositivo" required style="flex:1;min-width:140px">
          <select id="cmdType">
            <option>RestartMachine</option>
            <option>update</option>
          </select>
          <input id="fileName" placeholder="firmware.bin" value="firmware.bin" style="flex:1;min-width:150px">
          <button type="submit" class="btn-primary">Enviar</button>
        </form>
        <form id="clearForm" class="actions-form">
          <button type="submit" class="btn-danger">Limpar dados recebidos</button>
        </form>
      </section>
    </div>
    <section class="panel" style="margin-top:20px">
      <h2>Dispositivos</h2>
      <div class="table-wrap"><table><thead><tr><th>ID</th><th>Ultimo contato</th><th>Rede</th><th>RSSI</th><th>Versao</th><th>Ultimo dado</th></tr></thead><tbody id="devices"></tbody></table></div>
    </section>
    <section class="panel" style="margin-top:20px">
      <h2>Eventos</h2>
      <div class="table-wrap"><table><tbody id="events"></tbody></table></div>
    </section>
  </main>
  <script>
    function rssiBadge(value) {
      if (value == null || value === '-') return '<td>-</td>';
      var n = Number(value);
      var cls = n >= -50 ? 'rssi-good' : n >= -70 ? 'rssi-mid' : 'rssi-bad';
      var bars = n >= -50 ? '&#9646;&#9646;&#9646;&#9646;' : n >= -70 ? '&#9646;&#9646;&#9646;' : '&#9646;';
      return '<td><span class="rssi-badge ' + cls + '">' + bars + ' ' + n + ' dBm</span></td>';
    }
    function formatPtBrDate(value) {
      if (!value) return "-";
      var parts = String(value).match(/^(\\d{4})-(\\d{2})-(\\d{2})T(\\d{2}):(\\d{2}):(\\d{2})/);
      if (!parts) return value;
      return parts[3] + "/" + parts[2] + "/" + parts[1] + " " + parts[4] + ":" + parts[5] + ":" + parts[6];
    }
    function payloadBox(value) {
      var displayValue = Object.assign({}, value);
      if (displayValue.at) displayValue.at = formatPtBrDate(displayValue.at);
      if (displayValue.lastSeenAt) displayValue.lastSeenAt = formatPtBrDate(displayValue.lastSeenAt);
      var text = JSON.stringify(displayValue, null, 2);
      return '<div class="payload-card"><div class="payload-toolbar"><span class="payload-title">Payload recebido</span><button type="button" class="copy-btn" data-copy="' + escapeHtml(text) + '">Copiar</button></div><pre class="payload-box">' + escapeHtml(text) + '</pre></div>';
    }
    function summaryIcon(name) {
      var icons = {
        time: '<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"></circle><path d="M12 7v5l3 2"></path></svg>',
        serial: '<svg viewBox="0 0 24 24"><rect x="4" y="5" width="16" height="14" rx="2"></rect><path d="M8 9h8M8 13h5"></path></svg>',
        type: '<svg viewBox="0 0 24 24"><path d="M4 7h16M4 12h16M4 17h10"></path></svg>',
        pin: '<svg viewBox="0 0 24 24"><path d="M9 3v5M15 3v5M8 8h8v7a4 4 0 0 1-8 0V8z"></path><path d="M12 19v2"></path></svg>',
        count: '<svg viewBox="0 0 24 24"><path d="M5 7h14M5 12h14M5 17h14"></path><path d="M8 5v14M16 5v14"></path></svg>'
      };
      return icons[name] || icons.type;
    }
    function summaryRow(icon, label, value, span) {
      return '<div class="summary-row span-' + span + '"><span class="summary-icon">' + summaryIcon(icon) + '</span><span class="summary-content"><span class="summary-label">' + label + '</span><span class="summary-value">' + escapeHtml(value ?? "-") + '</span></span></div>';
    }
    function eventSummaryBox(event, eventId) {
      return '<div class="payload-summary-card"><div class="payload-summary-title"><span style="color:var(--blue-700);">ID:' + escapeHtml(eventId) + '</span> <span style="color:var(--slate-300);margin:0 6px;">|</span> Dados do payload <span style="color:var(--slate-300);margin:0 6px;">|</span> <span style="color:var(--slate-400);font-weight:700;">' + escapeHtml(formatPtBrDate(event.at)) + '</span></div>'
        + '<div class="summary-grid">'
        + summaryRow("serial", "Serial", event.serialNumber || "-", 2)
        + summaryRow("type", "Tipo", event.type || "-", 2)
        + summaryRow("pin", "Pino", event.pin ?? "-", 2)
        + summaryRow("count", "Contagem", event.count ?? "-", 2)
        + summaryRow("type", "RSSI", event.rssi ?? "-", 2)
        + summaryRow("type", "Rede", event.network || "-", 2)
        + summaryRow("type", "Versao", event.version || "-", 2)
        + summaryRow("type", "Offline", event.data_off === undefined ? "-" : String(event.data_off), 2)
        + summaryRow("type", "Topico", event.topic || "-", 6)
        + '</div></div>';
    }
    function bindCopyButtons() {
      document.querySelectorAll(".copy-btn").forEach(function(button) {
        button.addEventListener("click", async function() {
          var value = button.getAttribute("data-copy") || "";
          try {
            await navigator.clipboard.writeText(value);
            button.textContent = "Copiado";
            button.classList.add("copied");
            setTimeout(function() {
              button.textContent = "Copiar";
              button.classList.remove("copied");
            }, 1200);
          } catch {
            button.textContent = "Falha";
            setTimeout(function() { button.textContent = "Copiar"; }, 1200);
          }
        });
      });
    }
    async function refresh() {
      var res = await fetch("/api/state");
      var data = await res.json();
      var mqttStatus = document.getElementById("mqttStatus");
      mqttStatus.className = "status " + (data.connected ? "ok" : "");
      mqttStatus.lastElementChild.textContent = data.connected ? "Conectado" : "Desconectado";
      var devCount = Object.keys(data.devices).length;
      var evtCount = data.events.filter(function(e){ return e.type !== "ping"; }).length;
      document.getElementById("statDevices").textContent = devCount;
      document.getElementById("statEvents").textContent = evtCount;
      document.getElementById("statLast").textContent = formatPtBrDate(data.lastMessageAt);
      devices.innerHTML = Object.entries(data.devices).map(function(entry) {
        var id = entry[0], d = entry[1];
        return "<tr><td><strong>" + id + "</strong></td><td>" + formatPtBrDate(d.lastSeenAt) + "</td><td>" + (d.network || "-") + "</td>" + rssiBadge(d.rssi) + "<td>" + (d.version || "-") + '</td><td class="payload-cell">' + payloadBox(d) + "</td></tr>";
      }).join("") || '<tr><td colspan="6" class="empty-state">Nenhum dispositivo conectado</td></tr>';
      var filtered = data.events.filter(function(e){ return e.type !== "ping"; });
      var total = filtered.length;
      events.innerHTML = filtered
        .map(function(e, index) {
          var eventId = String(total - index).padStart(4, "0");
          return '<tr class="event-detail-row"><td colspan="6" class="event-detail-cell">'
            + '<div class="event-payload-layout">' + eventSummaryBox(e, eventId) + payloadBox(e) + '</div>'
            + '</td></tr>';
        })
        .join("") || '<tr><td colspan="6" class="empty-state">Nenhum evento registrado</td></tr>';
      bindCopyButtons();
    }
    function escapeHtml(value) {
      return String(value).replace(/[&<>"']/g, function(c) { return ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" })[c]; });
    }
    cmdForm.addEventListener("submit", async function(event) {
      event.preventDefault();
      var body = { type: cmdType.value };
      if (cmdType.value === "update") {
        body.fileName = fileName.value || "firmware.bin";
      }
      await fetch("/api/devices/" + encodeURIComponent(deviceId.value) + "/command", { method: "POST", headers: { "content-type": "application/json" }, body: JSON.stringify(body) });
    });
    function syncCommandFields() {
      fileName.style.display = cmdType.value === "update" ? "" : "none";
    }
    cmdType.addEventListener("change", syncCommandFields);
    syncCommandFields();
    clearForm.addEventListener("submit", async function(event) {
      event.preventDefault();
      await fetch("/api/state/clear", { method: "POST" });
      await refresh();
    });
    refresh();
    setInterval(refresh, 2000);
  </script>
</body>
</html>`;
}

const server = http.createServer(async (req, res) => {
  const url = new URL(req.url, `http://${req.headers.host}`);
  if (req.method === "GET" && url.pathname === "/") {
    res.writeHead(200, { "content-type": "text/html; charset=utf-8" });
    res.end(renderDashboard());
    return;
  }
  if ((req.method === "GET" || req.method === "HEAD") && url.pathname === `/${FIRMWARE_FILE}`) {
    if (!fs.existsSync(FIRMWARE_PATH)) {
      sendJson(res, 404, { error: `${FIRMWARE_FILE} not found` });
      return;
    }
    res.writeHead(200, {
      "content-type": "application/octet-stream",
      "content-length": fs.statSync(FIRMWARE_PATH).size,
      "cache-control": "no-store",
    });
    if (req.method === "HEAD") {
      res.end();
      return;
    }
    fs.createReadStream(FIRMWARE_PATH).pipe(res);
    return;
  }
  if (req.method === "GET" && url.pathname === "/api/state") {
    sendJson(res, 200, state);
    return;
  }
  if (req.method === "POST" && url.pathname === "/api/state/clear") {
    state.lastMessageAt = null;
    state.devices = {};
    state.events = [];
    sendJson(res, 200, { ok: true });
    return;
  }
  const commandMatch = url.pathname.match(/^\/api\/devices\/([^/]+)\/command$/);
  if (req.method === "POST" && commandMatch) {
    const deviceId = decodeURIComponent(commandMatch[1]);
    let command;
    try {
      command = JSON.parse(await readBody(req));
    } catch {
      sendJson(res, 400, { error: "invalid json" });
      return;
    }
    if (command.type === "update" && !command.fileName) {
      command.fileName = FIRMWARE_FILE;
    }
    const payload = JSON.stringify(command);
    const topic = `dc/${deviceId}/cmd`;
    if (!mqttSocket || !state.connected) {
      sendJson(res, 503, { error: "mqtt disconnected" });
      return;
    }
    mqttSocket.write(publishPacket(topic, payload));
    sendJson(res, 202, { topic, payload: command });
    return;
  }
  sendJson(res, 404, { error: "not found" });
});

startMqtt();
server.listen(PORT, () => console.log(`[http] listening on :${PORT}`));
