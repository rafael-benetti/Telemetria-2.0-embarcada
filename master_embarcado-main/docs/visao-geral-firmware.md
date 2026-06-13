# Visao geral do firmware

Este projeto e um firmware ESP32/Arduino para uma placa de telemetria embarcada. A placa le sinais fisicos de pulso, envia eventos para um broker MQTT, aceita comandos remotos e possui modos locais de configuracao por Wi-Fi, BLE e serial.

## Capacidades da placa

- Ler ate 13 entradas digitais de pulso.
- Contar creditos/eventos por entrada.
- Acionar saidas digitais para reiniciar maquina, reiniciar modulo de pagamento e gerar credito remoto.
- Indicar estado por LEDs de conexao e pulso.
- Usar rede Wi-Fi do ESP32.
- Usar modem GSM via serial.
- Criar ponto de acesso Wi-Fi local para configuracao.
- Rodar servidor web interno com arquivos armazenados em SPIFFS.
- Expor configuracao por BLE.
- Expor configuracao e diagnostico por CLI serial.
- Fazer atualizacao OTA.
- Salvar configuracoes e dados offline na EEPROM.

## Inicializacao

No boot, o firmware inicializa:

- GPIOs e interrupcoes das entradas.
- Timer usado para agrupar pulsos.
- EEPROM.
- Armazenamento offline.
- Identificacao da placa.
- Rede Wi-Fi/GSM.
- Tarefa MQTT.
- Tarefa BLE.
- Tarefa de interface/LEDs.

Depois disso, o loop principal reseta o watchdog, envia ping periodico, verifica entrada em modo de configuracao, verifica CLI serial e processa pulsos acumulados.

## Telemetria

Quando uma entrada recebe pulsos, o firmware incrementa um contador. Depois de um intervalo sem novos pulsos, ele enfileira uma mensagem para envio MQTT.

A mensagem enviada pode conter:

- ID da placa.
- Nivel de sinal/RSSI.
- Tipo de rede: `wifi` ou `gsm`.
- Versao do firmware.
- Quantidade de pulsos.
- Numero do pino.
- Indicacao se o dado veio do armazenamento offline.

Configuracao atual de MQTT local:

- Broker padrao: `192.168.0.20`
- Porta: `1883`
- Topico de publicacao: `dc/telemetry`
- Topico de comandos: `dc/<deviceId>/cmd`

O broker padrao deve ser o IP da maquina na rede local que roda o Docker/backend. Esse valor fica em `include/mqtt_config.h` e pode ser alterado antes do build.

## Comandos remotos

Via MQTT, o firmware aceita comandos para:

- `RestartMachine`: acionar o pino de restart da maquina.
- `RestartPayments`: acionar o pino de restart do modulo de pagamento.
- `remoteCredit`: gerar pulsos de credito remoto.
- `update`: baixar firmware OTA e reiniciar a placa.

## Configuracao local por Wi-Fi

Ao entrar no modo de configuracao, a placa cria um ponto de acesso:

- SSID: `Telemetria`
- IP: `http://10.10.10.10`

Pela interface web, e possivel configurar:

- SSID Wi-Fi.
- Senha Wi-Fi.
- APN GSM.
- Login GSM.
- Senha GSM.
- PIN GSM.
- Tipo de rede utilizada.

Os arquivos dessa interface ficam em `data/` e sao gravados no SPIFFS.

## Configuracao por BLE

O BLE expoe caracteristicas para ler e escrever:

- SSID Wi-Fi.
- Senha Wi-Fi.
- Tipo de rede.
- APN GSM.
- Login GSM.
- Senha GSM.
- PIN GSM.

Tambem expoe informacoes do dispositivo, como fabricante, modelo, revisao de hardware e versao de firmware.

## CLI serial

Com `configSERIAL_CLI` ativo, o firmware disponibiliza uma CLI serial em `9600 baud`. Ela permite consultar informacoes da placa e gravar configuracoes padrao.

## Armazenamento offline

Se um evento nao puder ser publicado no MQTT, o firmware salva a contagem por pino na EEPROM. Quando a conexao volta, ele tenta reenviar esses dados marcando a mensagem como offline.

## OTA

O firmware possui rotina de atualizacao OTA. Por padrao, o host de OTA foi apontado para o mesmo host local do MQTT:

- Host: mesmo valor de `MQTT_BROKER_HOST`
- Porta: `8080`

Isso remove a dependencia do host remoto antigo. Para usar OTA localmente, o backend ou algum servidor HTTP local deve servir o arquivo de firmware.

## Pontos fortes

- Modulos separados por responsabilidade: MQTT, GSM, Wi-Fi, BLE, GPIO, EEPROM, webserver, OTA e IHM.
- Uso de tarefas FreeRTOS.
- Suporte a dados offline.
- Multiplas formas de configuracao local.
- Watchdog habilitado para recuperar travamentos.
- Build validado com PlatformIO para o ambiente `esp32dev`.

## Pontos de atencao

- MQTT local esta configurado sem TLS na porta `1883`.
- OTA local esta configurado por HTTP na porta `8080`.
- O AP de configuracao nao possui senha.
- A interface web retorna senhas e PINs salvos.
- O JSON recebido por MQTT e webserver nao tem validacao rigorosa.
- O reenvio offline apaga dados da EEPROM antes de confirmar nova publicacao MQTT.
- A entrada 0 possui tratamento diferente: nao e anexada no loop normal de interrupcoes e e desativada durante credito remoto.
- A documentacao operacional ainda deve ser expandida com pinagem, protocolo MQTT e processo de provisionamento.

## Comandos uteis

Build do firmware:

```powershell
pio run
```

Build da imagem SPIFFS:

```powershell
pio run --target buildfs
```

Upload do firmware:

```powershell
pio run --target upload
```

Upload do SPIFFS:

```powershell
pio run --target uploadfs
```

Monitor serial:

```powershell
pio device monitor --baud 9600
```
