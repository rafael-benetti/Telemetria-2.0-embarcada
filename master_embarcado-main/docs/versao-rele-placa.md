# Versao com rele da placa

Esta documentacao descreve a versao atual do firmware testada na placa com rele onboard.

## Resumo

- ESP32 publica telemetria por MQTT.
- A placa continua lendo pulsos normalmente.
- O firmware aceita comandos MQTT para acionar os reles da placa.
- A versao validada em campo usa:
  - `actionj8` para o rele azul
  - `actionj11` para o rele preto

## Comunicacao MQTT

### Telemetria

Topico de publicacao:

```text
dc/telemetry
```

Exemplo de payload:

```json
{
  "serialNumber": 4,
  "rssi": 86,
  "network": "wifi",
  "version": "1.0.0",
  "type": "pulso",
  "count": 2,
  "pin": 12,
  "data_off": false
}
```

### Comandos

Topico de comando:

```text
dc/<serialNumber>/cmd
```

Exemplo para a placa `4`:

```text
dc/4/cmd
```

## Comandos suportados

### `actionj8`

Aciona o rele azul.

Exemplo:

```json
{ "command": "actionj8", "duration_ms": 1000 }
```

### `actionj11`

Aciona o rele preto.

Exemplo:

```json
{ "command": "actionj11", "duration_ms": 1000 }
```

## Confirmacao de comando

Quando um comando de rele e executado, o firmware publica uma confirmacao no topico de telemetria com um formato parecido com:

```json
{
  "type": "relay",
  "command": "actionj8",
  "status": "executed",
  "duration_ms": 1000,
  "serialNumber": 4,
  "rssi": 86,
  "network": "wifi",
  "version": "1.0.0"
}
```

## Mapeamento encontrado

O que foi validado no hardware:

- `GPIO 22` aciona o rele azul
- `GPIO 23` aciona o rele preto

O firmware foi ajustado para usar os comandos `actionj8` e `actionj11` como contrato final da placa.

## Fluxo de boot

Ao ligar:

1. O ESP32 sobe configuracao de rede.
2. O firmware tenta Wi-Fi ou GSM conforme gravado na EEPROM.
3. O MQTT conecta no broker configurado.
4. A placa volta a publicar telemetria normalmente.

## Leituras de pulso

A versao de rele nao remove a logica anterior:

- leitura de pulsos continua ativa
- publicacao de eventos `ping`
- publicacao de eventos `pulso`
- armazenamento offline continua disponivel

## Comandos de manutencao

### Compilar

```powershell
pio run
```

### Gerar SPIFFS

```powershell
pio run -t buildfs
```

### Upload do firmware

```powershell
pio run -t upload
```

### Upload do SPIFFS

```powershell
pio run -t uploadfs
```

### Monitor serial

```powershell
pio device monitor --baud 9600
```

## Observacoes importantes

- Nao use 220V direto no ESP32.
- Use o rele como contato seco quando for acionar botoeira, porta, motor ou outro equipamento.
- Se o rele responder invertido, o problema costuma estar no hardware da placa, nao no MQTT.
