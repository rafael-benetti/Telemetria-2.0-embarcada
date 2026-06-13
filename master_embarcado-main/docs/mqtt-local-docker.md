# MQTT local com Docker

O firmware foi ajustado para deixar de usar o broker remoto antigo e publicar em um broker local, pensado para rodar junto do backend DC Monitor.

## Broker local

Foi adicionado um diretorio proprio para infraestrutura local:

```text
backend-docker/
```

Dentro dele ha um `docker-compose` simples com Mosquitto:

```powershell
docker compose -f backend-docker/docker-compose.yml up -d
```

Ele sobe um broker MQTT em:

```text
porta 1883
```

## Configuracao no firmware

O host MQTT fica em `include/mqtt_config.h`:

```cpp
#define MQTT_BROKER_HOST "192.168.0.20"
```

Esse IP deve ser o IP da maquina que esta rodando o Docker na mesma rede da placa ESP32. Nao use `localhost` ou `127.0.0.1`, porque para a placa isso apontaria para ela mesma, nao para o computador.

Topicos configurados:

```text
publicacao: dc/telemetry
comandos:   dc/<deviceId>/cmd
```

## Teste local

Para ver mensagens chegando no broker:

```powershell
docker exec -it dc-monitor-mqtt mosquitto_sub -t "#" -v
```

Para enviar comando para uma placa, substitua `<deviceId>` pelo ID real:

```powershell
docker exec -it dc-monitor-mqtt mosquitto_pub -t "dc/<deviceId>/cmd" -m "{\"type\":\"RestartMachine\"}"
```

## OTA local

O OTA tambem foi redirecionado para host local:

```cpp
#define OTA_HOST MQTT_BROKER_HOST
#define OTA_PORT 8080
```

Para usar OTA, algum servico HTTP local deve servir o arquivo `.bin` na porta configurada.
