const http = require("http");
const net = require("net");
const fs = require("fs");
const path = require("path");
const { randomUUID } = require("crypto");
const db = require("./db");

const PORT = Number(process.env.PORT || 8080);
const MQTT_HOST = process.env.MQTT_HOST || "mqtt";
const MQTT_PORT = Number(process.env.MQTT_PORT || 1883);
const TELEMETRY_TOPIC = process.env.TELEMETRY_TOPIC || "dc/telemetry";
const LOCAL_TIME_ZONE = process.env.TZ || "America/Sao_Paulo";
const FIRMWARE_FILE = process.env.FIRMWARE_FILE || "firmware.bin";
const UPLOAD_DIR = process.env.UPLOAD_DIR || path.join(__dirname, "uploads");
const FIRMWARE_PATH = path.join(UPLOAD_DIR, FIRMWARE_FILE);

const state = {
  connected: false,
  lastMessageAt: null,
  devices: {},
  events: [],
  firmware: { name: FIRMWARE_FILE, uploadedAt: null, size: 0 },
};

const sseClients = [];

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

function broadcastSSE(event, deviceId) {
  const data = JSON.stringify({ event, deviceId });
  const message = `event: telemetry\ndata: ${data}\n\n`;
  for (let i = sseClients.length - 1; i >= 0; i--) {
    try {
      sseClients[i].write(message);
    } catch {
      sseClients.splice(i, 1);
    }
  }
}

function broadcastSSEClear() {
  const message = `event: clear\ndata: {}\n\n`;
  for (let i = sseClients.length - 1; i >= 0; i--) {
    try {
      sseClients[i].write(message);
    } catch {
      sseClients.splice(i, 1);
    }
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
  db.upsertDevice(deviceId, state.devices[deviceId]);
  db.insertEvent(event);
  broadcastSSE(event, deviceId);
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

function ensureUploadDir() {
  fs.mkdirSync(UPLOAD_DIR, { recursive: true });
}

const PUBLIC_DIR = path.join(__dirname, "public");
const DASHBOARD_HTML = fs.readFileSync(path.join(PUBLIC_DIR, "dashboard.html"), "utf-8");

function renderDashboard() {
  return DASHBOARD_HTML.replace(/__TELEMETRY_TOPIC__/g, TELEMETRY_TOPIC);
}

const MIME_TYPES = {
  ".html": "text/html; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".js": "application/javascript; charset=utf-8",
  ".json": "application/json",
  ".png": "image/png",
  ".svg": "image/svg+xml",
  ".ico": "image/x-icon",
};

function serveStatic(urlPath, res) {
  const filePath = path.join(PUBLIC_DIR, urlPath.replace(/^\/public\//, ""));
  if (!filePath.startsWith(PUBLIC_DIR)) {
    sendJson(res, 404, { error: "not found" });
    return true;
  }
  try {
    const ext = path.extname(filePath);
    const contentType = MIME_TYPES[ext] || "application/octet-stream";
    const content = fs.readFileSync(filePath);
    res.writeHead(200, { "content-type": contentType, "cache-control": "no-cache" });
    res.end(content);
    return true;
  } catch {
    return false;
  }
}

const server = http.createServer(async (req, res) => {
  const url = new URL(req.url, `http://${req.headers.host}`);
  const pathname = url.pathname;
  if (req.method === "GET" && pathname.startsWith("/public/")) {
    if (serveStatic(pathname, res)) return;
    sendJson(res, 404, { error: "not found" });
    return;
  }
  if (req.method === "GET" && pathname === "/") {
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
  if (req.method === "GET" && url.pathname === "/api/events") {
    const wantsSSE = (req.headers.accept || "").includes("text/event-stream");
    if (!wantsSSE) {
      sendJson(res, 200, state.events);
      return;
    }
    res.writeHead(200, {
      "content-type": "text/event-stream",
      "cache-control": "no-store",
      "connection": "keep-alive",
      "access-control-allow-origin": "*",
    });
    res.write(`event: connected\ndata: ${JSON.stringify({ count: state.events.length })}\n\n`);
    for (const evt of state.events) {
      const data = JSON.stringify({ event: evt, deviceId: evt.serialNumber || "unknown" });
      res.write(`event: telemetry\ndata: ${data}\n\n`);
    }
    sseClients.push(res);
    const keepAlive = setInterval(() => {
      try { res.write(":keepalive\n\n"); } catch { clearInterval(keepAlive); }
    }, 15000);
    req.on("close", () => {
      clearInterval(keepAlive);
      const idx = sseClients.indexOf(res);
      if (idx !== -1) sseClients.splice(idx, 1);
    });
    return;
  }
  if (req.method === "POST" && url.pathname === "/api/firmware") {
    const chunks = [];
    req.on("data", (chunk) => chunks.push(chunk));
    req.on("end", () => {
      const buffer = Buffer.concat(chunks);
      const uploadedName = String(req.headers["x-firmware-name"] || FIRMWARE_FILE).split(/[\\/]/).pop();
      try {
        ensureUploadDir();
        fs.writeFileSync(FIRMWARE_PATH, buffer);
        state.firmware = {
          name: uploadedName || FIRMWARE_FILE,
          uploadedAt: nowLocalIso(),
          size: buffer.length,
        };
        sendJson(res, 200, { ok: true, name: state.firmware.name, size: buffer.length, path: FIRMWARE_PATH });
      } catch (error) {
        sendJson(res, 500, { error: `upload failed: ${error.message}` });
      }
    });
    req.on("error", () => sendJson(res, 500, { error: "upload failed" }));
    return;
  }
  if (req.method === "POST" && url.pathname === "/api/state/clear") {
    state.lastMessageAt = null;
    state.devices = {};
    state.events = [];
    db.clearAll();
    sendJson(res, 200, { ok: true });
    broadcastSSEClear();
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
    if (command.type === "update") {
      command.fileName = FIRMWARE_FILE;
    }
    const payload = JSON.stringify(command);
    const topic = `dc/${deviceId}/cmd`;
    if (!mqttSocket || !state.connected) {
      sendJson(res, 503, { error: "mqtt disconnected" });
      return;
    }
    console.log(`[cmd] ${topic} ${payload}`);
    mqttSocket.write(publishPacket(topic, payload));
    sendJson(res, 202, { topic, payload: command });
    return;
  }
  sendJson(res, 404, { error: "not found" });
});

async function start() {
  try {
    await db.connect();
    const persisted = await db.loadState();
    if (persisted.devices) state.devices = persisted.devices;
    if (persisted.events) state.events = persisted.events;
  } catch (err) {
    console.warn(`[db] not available, running in memory-only mode: ${err.message}`);
  }
  startMqtt();
  server.listen(PORT, () => console.log(`[http] listening on :${PORT}`));
}

start();
