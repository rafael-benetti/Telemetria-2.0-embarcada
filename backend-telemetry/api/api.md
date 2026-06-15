# DC Monitor - API Reference

Base URL: `http://localhost:8080`

---

## Sumário

| # | Método | Rota | Descrição |
|---|--------|------|-----------|
| 1 | `GET` | `/api/state` | Status completo do sistema |
| 2 | `GET` | `/api/events` | Lista de eventos de telemetria |
| 3 | `POST` | `/api/state/clear` | Limpar dados de dispositivos e eventos |
| 4 | `POST` | `/api/firmware` | Upload de firmware para VPS |
| 5 | `GET`/`HEAD` | `/firmware.bin` | Download do firmware para OTA |
| 6 | `POST` | `/api/devices/{id}/command` | Enviar comando MQTT para placa |

---

## 1. GET /api/state

Retorna o estado completo do sistema: conexão MQTT, dispositivos, eventos e firmware.

### Request

```bash
curl -X GET http://localhost:8080/api/state \
  -H "Content-Type: application/json"
```

### Response `200`

```json
{
  "connected": false,
  "lastMessageAt": null,
  "devices": {},
  "events": [],
  "firmware": {
    "name": "firmware.bin",
    "uploadedAt": null,
    "size": 0
  }
}
```

### Campos

| Campo | Tipo | Descrição |
|-------|------|-----------|
| `connected` | boolean | Status da conexão com broker MQTT |
| `lastMessageAt` | string \| null | ISO timestamp do último evento recebido |
| `devices` | object | Mapa de dispositivos (key = serialNumber) |
| `events` | array | Últimos 100 eventos de telemetria |
| `firmware.name` | string | Nome do firmware armazenado |
| `firmware.uploadedAt` | string \| null | Data do último upload |
| `firmware.size` | number | Tamanho em bytes |

---

## 2. GET /api/events

Retorna eventos de telemetria em tempo real via **SSE** (Server-Sent Events).

### JSON (modo legado / polling)

Sem header especial, retorna o array dos últimos 100 eventos.

```bash
curl -X GET http://localhost:8080/api/events \
  -H "Content-Type: application/json"
```

```json
[
  {
    "at": "2026-06-14T21:48:03.123",
    "topic": "dc/telemetry",
    "serialNumber": "1234",
    "type": "relay",
    "pin": 8,
    "count": 42,
    "rssi": -55,
    "network": "4G",
    "version": "v2.1"
  }
]
```

---

### SSE (Server-Sent Events) — tempo real

Enviando `Accept: text/event-stream`, o servidor mantém a conexão aberta e empurra eventos em tempo real conforme chegam do MQTT.

```bash
curl -N http://localhost:8080/api/events \
  -H "Accept: text/event-stream"
```

### Eventos SSE

#### `connected` (primeiro evento)
```json
{
  "count": 42
}
```
Indica quantos eventos existem no buffer inicial.

#### `telemetry` (a cada mensagem MQTT)
```json
{
  "event": {
    "at": "2026-06-14T21:48:03.123",
    "topic": "dc/telemetry",
    "serialNumber": "1234",
    "type": "relay",
    "pin": 8,
    "count": 42,
    "rssi": -55,
    "network": "4G",
    "version": "v2.1"
  },
  "deviceId": "1234"
}
```

#### `clear` (quando dados são limpos)
```json
{}
```

### Keepalive

O servidor envia comentários (`:keepalive`) a cada 15s para manter a conexão ativa.

### Exemplo de consumo no frontend

```js
var source = new EventSource('/api/events');

source.addEventListener('connected', function (e) {
  console.log('Conectado,', JSON.parse(e.data).count, 'eventos iniciais');
});

source.addEventListener('telemetry', function (e) {
  var data = JSON.parse(e.data);
  console.log('Novo evento:', data.event);
});

source.addEventListener('clear', function () {
  console.log('Eventos limpos');
});
```

### Campos do evento (`telemetry`)

| Campo | Tipo | Descrição |
|-------|------|-----------|
| `at` | string | ISO timestamp do evento |
| `topic` | string | Tópico MQTT de origem |
| `serialNumber` | string | ID do dispositivo |
| `type` | string | Tipo do evento (`ping`, `relay`, etc.) |
| `pin` | number \| null | Pino do relé |
| `count` | number \| null | Contagem de acionamentos |
| `rssi` | number \| null | Intensidade do sinal (dBm) |
| `network` | string \| null | Tipo de rede |
| `version` | string \| null | Versão do firmware da placa |

---

## 3. POST /api/state/clear

Limpa todos os dispositivos e eventos armazenados na memória.

### Request

```bash
curl -X POST http://localhost:8080/api/state/clear \
  -H "Content-Type: application/json"
```

### Response `200`

```json
{
  "ok": true
}
```

---

## 4. POST /api/firmware

Faz upload de um arquivo `.bin` de firmware para a VPS.

### Request

```bash
curl -X POST http://localhost:8080/api/firmware \
  -H "Content-Type: application/octet-stream" \
  -H "x-firmware-name: firmware.bin" \
  --data-binary "@caminho/para/seu/firmware.bin"
```

### Response `200`

```json
{
  "ok": true,
  "name": "firmware.bin",
  "size": 524288,
  "path": "/app/uploads/firmware.bin"
}
```

### Response `500`

```json
{
  "error": "upload failed: ..."
}
```

### Observações

- O header `x-firmware-name` define o nome do arquivo salvo.
- O conteúdo deve ser enviado como `application/octet-stream` (raw binary).
- O arquivo é sobrescrito a cada novo upload.
- Serve de fonte para o comando OTA `update`.

---

## 5. GET /firmware.bin

Download do firmware armazenado (usado pelas placas no update OTA).

### Request

```bash
curl -O http://localhost:8080/firmware.bin
```

### HEAD (verificar existência)

```bash
curl -I http://localhost:8080/firmware.bin
```

### Response `200` (GET)

Retorna o arquivo binário com headers:
```
content-type: application/octet-stream
content-length: <tamanho>
```

### Response `404` (inexistente)

```json
{
  "error": "firmware.bin not found"
}
```

---

## 6. POST /api/devices/{id}/command

Envia um comando MQTT para uma placa específica.

O dispositivo deve estar online e o broker MQTT conectado.

### Request base

```
POST /api/devices/{deviceId}/command
Content-Type: application/json
```

O `deviceId` é o identificador (serialNumber) da placa.

---

### 5.1 RestartMachine

Reinicia a placa remotamente.

```bash
curl -X POST http://localhost:8080/api/devices/1/command \
  -H "Content-Type: application/json" \
  -d '{"type": "RestartMachine"}'
```

---

### 5.2 Update (OTA)

Dispara atualização OTA: a placa baixa o firmware da VPS e reinicia.

```bash
curl -X POST http://localhost:8080/api/devices/1/command \
  -H "Content-Type: application/json" \
  -d '{"type": "update", "fileName": "firmware.bin"}'
```

> Nota: o backend força `fileName` para o valor de `FIRMWARE_FILE`, independente do enviado.

---

### 5.3 Pulso Azul (actionj8_pulse)

Dispara um pulso no relé azul (pino J8).

```bash
curl -X POST http://localhost:8080/api/devices/1/command \
  -H "Content-Type: application/json" \
  -d '{"command": "actionj8_pulse", "duration_ms": 1000}'
```

---

### 5.4 Pulso Preto (actionj11_pulse)

Dispara um pulso no relé preto (pino J11).

```bash
curl -X POST http://localhost:8080/api/devices/1/command \
  -H "Content-Type: application/json" \
  -d '{"command": "actionj11_pulse", "duration_ms": 1000}'
```

---

### 5.5 Ligar Azul (actionj8_on)

Liga o relé azul (pino J8).

```bash
curl -X POST http://localhost:8080/api/devices/1/command \
  -H "Content-Type: application/json" \
  -d '{"command": "actionj8_on", "duration_ms": 1000}'
```

---

### 5.6 Desligar Azul (actionj8_off)

Desliga o relé azul (pino J8).

```bash
curl -X POST http://localhost:8080/api/devices/1/command \
  -H "Content-Type: application/json" \
  -d '{"command": "actionj8_off", "duration_ms": 1000}'
```

---

### 5.7 Ligar Preto (actionj11_on)

Liga o relé preto (pino J11).

```bash
curl -X POST http://localhost:8080/api/devices/1/command \
  -H "Content-Type: application/json" \
  -d '{"command": "actionj11_on", "duration_ms": 1000}'
```

---

### 5.8 Desligar Preto (actionj11_off)

Desliga o relé preto (pino J11).

```bash
curl -X POST http://localhost:8080/api/devices/1/command \
  -H "Content-Type: application/json" \
  -d '{"command": "actionj11_off", "duration_ms": 1000}'
```

---

### Response `202` (sucesso)

```json
{
  "topic": "dc/1/cmd",
  "payload": {
    "type": "RestartMachine"
  }
}
```

Payload reflete exatamente o que foi publicado no MQTT.

### Response `400` (json inválido)

```json
{
  "error": "invalid json"
}
```

### Response `503` (mqtt desconectado)

```json
{
  "error": "mqtt disconnected"
}
```

---

## Resumo dos comandos de relé

| Nome | `command` | Pino | Cor |
|------|-----------|------|-----|
| Pulso Azul | `actionj8_pulse` | J8 | Azul |
| Ligar Azul | `actionj8_on` | J8 | Azul |
| Desligar Azul | `actionj8_off` | J8 | Azul |
| Pulso Preto | `actionj11_pulse` | J11 | Preto |
| Ligar Preto | `actionj11_on` | J11 | Preto |
| Desligar Preto | `actionj11_off` | J11 | Preto |

---

## MongoDB

Os dados de **dispositivos** e **eventos** são persistidos automaticamente no MongoDB.

### Collections

| Collection | Descrição | Índices |
|-----------|-----------|---------|
| `devices` | Um documento por dispositivo, atualizado a cada telemetria | `{ lastSeenAt: -1 }` |
| `events` | Cada evento de telemetria é inserido como documento | `{ createdAt: 1 }` (TTL 7d), `{ serialNumber: 1 }` |

### Fluxo de persistência

```
MQTT → handlePublish() → upsertDevice() + insertEvent() → MongoDB
                         → broadcastSSE() → frontend em tempo real
```

### Startup

Ao iniciar, o servidor carrega do MongoDB para a memória:
```js
const persisted = await db.loadState();
state.devices = persisted.devices;
state.events = persisted.events;
```

### Sem MongoDB

Se o MongoDB não estiver disponível, o servidor funciona em **memória apenas** (modo legado), com um warning no log:

```
[db] not available, running in memory-only mode: getaddrinfo ENOTFOUND mongo
```

### Mongo Express

Acesse a UI do MongoDB em: [http://localhost:8081](http://localhost:8081)

Credenciais: `admin` / `admin`

---

## Variáveis de ambiente

| Variável | Padrão | Descrição |
|----------|--------|-----------|
| `PORT` | `8080` | Porta do servidor HTTP |
| `MQTT_HOST` | `mqtt` | Host do broker MQTT |
| `MQTT_PORT` | `1883` | Porta do broker MQTT |
| `TELEMETRY_TOPIC` | `dc/telemetry` | Tópico MQTT de telemetria |
| `FIRMWARE_FILE` | `firmware.bin` | Nome do arquivo de firmware |
| `UPLOAD_DIR` | `./uploads` | Diretório de uploads |
| `TZ` | `America/Sao_Paulo` | Fuso horário |
| `MONGO_URI` | `mongodb://mongo:27017` | URI de conexão MongoDB |
| `MONGO_DB` | `dc-monitor` | Nome do banco MongoDB |
| `EVENTS_TTL_SECONDS` | `604800` | TTL dos eventos em segundos (7 dias) |

---

## Collection Postman

Importe o JSON abaixo no Postman para ter todos os endpoints prontos:

```json
{
  "info": {
    "name": "DC Monitor API",
    "schema": "https://schema.getpostman.com/json/collection/v2.1.0/collection.json"
  },
  "item": [
    {
      "name": "GET /api/state",
      "request": {
        "method": "GET",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "url": {"raw": "{{base_url}}/api/state", "host": ["{{base_url}}"], "path": ["api", "state"]}
      }
    },
    {
      "name": "GET /api/events (JSON)",
      "request": {
        "method": "GET",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "url": {"raw": "{{base_url}}/api/events", "host": ["{{base_url}}"], "path": ["api", "events"]}
      }
    },
    {
      "name": "GET /api/events (SSE stream)",
      "request": {
        "method": "GET",
        "header": [{"key": "Accept", "value": "text/event-stream"}],
        "url": {"raw": "{{base_url}}/api/events", "host": ["{{base_url}}"], "path": ["api", "events"]}
      }
    },
    {
      "name": "POST /api/state/clear",
      "request": {
        "method": "POST",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "url": {"raw": "{{base_url}}/api/state/clear", "host": ["{{base_url}}"], "path": ["api", "state", "clear"]}
      }
    },
    {
      "name": "POST /api/firmware",
      "request": {
        "method": "POST",
        "header": [
          {"key": "Content-Type", "value": "application/octet-stream"},
          {"key": "x-firmware-name", "value": "firmware.bin"}
        ],
        "body": {"mode": "file", "file": {}},
        "url": {"raw": "{{base_url}}/api/firmware", "host": ["{{base_url}}"], "path": ["api", "firmware"]}
      }
    },
    {
      "name": "GET /firmware.bin",
      "request": {
        "method": "GET",
        "url": {"raw": "{{base_url}}/firmware.bin", "host": ["{{base_url}}"], "path": ["firmware.bin"]}
      }
    },
    {
      "name": "POST RestartMachine",
      "request": {
        "method": "POST",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "body": {"mode": "raw", "raw": "{\n  \"type\": \"RestartMachine\"\n}"},
        "url": {"raw": "{{base_url}}/api/devices/1/command", "host": ["{{base_url}}"], "path": ["api", "devices", "1", "command"]}
      }
    },
    {
      "name": "POST Update OTA",
      "request": {
        "method": "POST",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "body": {"mode": "raw", "raw": "{\n  \"type\": \"update\",\n  \"fileName\": \"firmware.bin\"\n}"},
        "url": {"raw": "{{base_url}}/api/devices/1/command", "host": ["{{base_url}}"], "path": ["api", "devices", "1", "command"]}
      }
    },
    {
      "name": "POST Pulso Azul",
      "request": {
        "method": "POST",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "body": {"mode": "raw", "raw": "{\n  \"command\": \"actionj8_pulse\",\n  \"duration_ms\": 1000\n}"},
        "url": {"raw": "{{base_url}}/api/devices/1/command", "host": ["{{base_url}}"], "path": ["api", "devices", "1", "command"]}
      }
    },
    {
      "name": "POST Pulso Preto",
      "request": {
        "method": "POST",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "body": {"mode": "raw", "raw": "{\n  \"command\": \"actionj11_pulse\",\n  \"duration_ms\": 1000\n}"},
        "url": {"raw": "{{base_url}}/api/devices/1/command", "host": ["{{base_url}}"], "path": ["api", "devices", "1", "command"]}
      }
    },
    {
      "name": "POST Ligar Azul",
      "request": {
        "method": "POST",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "body": {"mode": "raw", "raw": "{\n  \"command\": \"actionj8_on\",\n  \"duration_ms\": 1000\n}"},
        "url": {"raw": "{{base_url}}/api/devices/1/command", "host": ["{{base_url}}"], "path": ["api", "devices", "1", "command"]}
      }
    },
    {
      "name": "POST Desligar Azul",
      "request": {
        "method": "POST",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "body": {"mode": "raw", "raw": "{\n  \"command\": \"actionj8_off\",\n  \"duration_ms\": 1000\n}"},
        "url": {"raw": "{{base_url}}/api/devices/1/command", "host": ["{{base_url}}"], "path": ["api", "devices", "1", "command"]}
      }
    },
    {
      "name": "POST Ligar Preto",
      "request": {
        "method": "POST",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "body": {"mode": "raw", "raw": "{\n  \"command\": \"actionj11_on\",\n  \"duration_ms\": 1000\n}"},
        "url": {"raw": "{{base_url}}/api/devices/1/command", "host": ["{{base_url}}"], "path": ["api", "devices", "1", "command"]}
      }
    },
    {
      "name": "POST Desligar Preto",
      "request": {
        "method": "POST",
        "header": [{"key": "Content-Type", "value": "application/json"}],
        "body": {"mode": "raw", "raw": "{\n  \"command\": \"actionj11_off\",\n  \"duration_ms\": 1000\n}"},
        "url": {"raw": "{{base_url}}/api/devices/1/command", "host": ["{{base_url}}"], "path": ["api", "devices", "1", "command"]}
      }
    }
  ],
  "variable": [{"key": "base_url", "value": "http://localhost:8080"}]
}
```

Crie uma variável `base_url` no Postman com o valor `http://localhost:8080` para facilitar.
