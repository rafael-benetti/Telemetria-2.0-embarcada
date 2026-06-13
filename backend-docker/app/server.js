const http = require("http");
const net = require("net");
const { randomUUID } = require("crypto");

const PORT = Number(process.env.PORT || 8080);
const MQTT_HOST = process.env.MQTT_HOST || "mqtt";
const MQTT_PORT = Number(process.env.MQTT_PORT || 1883);
const TELEMETRY_TOPIC = process.env.TELEMETRY_TOPIC || "dc/telemetry";
const LOCAL_TIME_ZONE = process.env.TZ || "America/Sao_Paulo";

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
  <style>
    :root { color-scheme: light; font-family: Arial, sans-serif; color: #17202a; background: #f4f6f8; }
    body { margin: 0; }
    header { background: #13315c; color: white; padding: 18px 24px; }
    main { padding: 24px; max-width: 1120px; margin: 0 auto; }
    h1 { margin: 0; font-size: 24px; }
    h2 { margin: 0 0 12px; font-size: 18px; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap: 16px; }
    .panel { background: white; border: 1px solid #d8dee6; border-radius: 6px; padding: 16px; }
    .status { display: inline-flex; align-items: center; gap: 8px; font-weight: 700; }
    .dot { width: 10px; height: 10px; border-radius: 50%; background: #a61b1b; }
    .ok .dot { background: #16794c; }
    table { width: 100%; border-collapse: collapse; background: white; border: 1px solid #d8dee6; }
    th, td { padding: 10px; border-bottom: 1px solid #e7ebf0; text-align: left; font-size: 14px; }
    th { background: #eef2f6; }
    input, select, button { height: 36px; border: 1px solid #bac4d0; border-radius: 4px; padding: 0 10px; }
    button { background: #1f6feb; color: white; border: 0; cursor: pointer; }
    form { display: flex; gap: 8px; flex-wrap: wrap; align-items: center; }
    code { background: #eef2f6; padding: 2px 5px; border-radius: 4px; }
  </style>
</head>
<body>
  <header><h1>DC Monitor</h1></header>
  <main>
    <div class="grid">
      <section class="panel">
        <h2>Broker MQTT</h2>
        <div id="mqttStatus" class="status"><span class="dot"></span><span>Carregando</span></div>
        <p>Assinando <code>${TELEMETRY_TOPIC}</code></p>
      </section>
      <section class="panel">
        <h2>Ações</h2>
        <form id="cmdForm">
          <input id="deviceId" placeholder="deviceId" required>
          <select id="cmdType">
            <option>RestartMachine</option>
            <option>update</option>
          </select>
          <button type="submit">Enviar</button>
        </form>
        <form id="clearForm" style="margin-top:10px">
          <button type="submit">Limpar dados recebidos</button>
        </form>
      </section>
    </div>
    <section class="panel" style="margin-top:16px">
      <h2>Dispositivos</h2>
      <table><thead><tr><th>ID</th><th>Ultimo contato</th><th>Rede</th><th>RSSI</th><th>Versao</th><th>Ultimo dado</th></tr></thead><tbody id="devices"></tbody></table>
    </section>
    <section class="panel" style="margin-top:16px">
      <h2>Eventos</h2>
      <table><thead><tr><th>Hora</th><th>Serial</th><th>Tipo</th><th>Pino</th><th>Contagem</th><th>Payload</th></tr></thead><tbody id="events"></tbody></table>
    </section>
  </main>
  <script>
    async function refresh() {
      const res = await fetch("/api/state");
      const data = await res.json();
      const mqttStatus = document.getElementById("mqttStatus");
      mqttStatus.className = "status " + (data.connected ? "ok" : "");
      mqttStatus.lastElementChild.textContent = data.connected ? "Conectado" : "Desconectado";
      devices.innerHTML = Object.entries(data.devices).map(([id, d]) => "<tr><td>" + id + "</td><td>" + (d.lastSeenAt || "-") + "</td><td>" + (d.network || "-") + "</td><td>" + (d.rssi ?? "-") + "</td><td>" + (d.version || "-") + "</td><td><code>" + escapeHtml(JSON.stringify(d)) + "</code></td></tr>").join("");
      events.innerHTML = data.events
        .filter((e) => e.type !== "ping")
        .map(e => "<tr><td>" + e.at + "</td><td>" + (e.serialNumber || "-") + "</td><td>" + (e.type || "-") + "</td><td>" + (e.pin ?? "-") + "</td><td>" + (e.count ?? "-") + "</td><td><code>" + escapeHtml(JSON.stringify(e)) + "</code></td></tr>")
        .join("");
    }
    function escapeHtml(value) {
      return String(value).replace(/[&<>"']/g, c => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
    }
    cmdForm.addEventListener("submit", async (event) => {
      event.preventDefault();
      const body = { type: cmdType.value };
      await fetch("/api/devices/" + encodeURIComponent(deviceId.value) + "/command", { method: "POST", headers: { "content-type": "application/json" }, body: JSON.stringify(body) });
    });
    clearForm.addEventListener("submit", async (event) => {
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
    const payload = await readBody(req);
    const topic = `dc/${deviceId}/cmd`;
    if (!mqttSocket || !state.connected) {
      sendJson(res, 503, { error: "mqtt disconnected" });
      return;
    }
    mqttSocket.write(publishPacket(topic, payload));
    sendJson(res, 202, { topic, payload: JSON.parse(payload) });
    return;
  }
  sendJson(res, 404, { error: "not found" });
});

startMqtt();
server.listen(PORT, () => console.log(`[http] listening on :${PORT}`));
