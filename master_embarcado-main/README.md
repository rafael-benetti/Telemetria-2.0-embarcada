# Telemetria

Firmware ESP32/Arduino para placa de telemetria com leitura de pulsos, envio MQTT, configuracao por Wi-Fi/BLE/serial, OTA e armazenamento offline.

Documentacao:

- [Visao geral do firmware](docs/visao-geral-firmware.md)
- [Hardware e possibilidade de rastreamento](docs/hardware-e-rastreamento.md)
- [Projeto futuro: DC Monitor](docs/projeto-dc-monitor.md)
- [MQTT local com Docker](docs/mqtt-local-docker.md)
- [Versao com rele da placa](docs/versao-rele-placa.md)
- [Teste da placa com rele](RELAY_BOARD_TEST.md)

Comandos principais:

```powershell
pio run
pio run --target buildfs
pio run --target upload
pio run --target uploadfs
pio device monitor --baud 9600
```
