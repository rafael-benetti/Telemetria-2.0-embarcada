# Projeto futuro: DC Monitor

## Objetivo

O DC Monitor sera um sistema para monitoramento geral do datacenter, combinando sensores fisicos instalados no ambiente com monitoramento de rede e servidores feito pelo backend.

A placa ESP32 de telemetria ficara responsavel pelo ambiente fisico. O backend ficara responsavel por receber os dados, registrar historico, exibir dashboard e disparar alertas.

## Escopo inicial

Monitorar o estado geral do datacenter e indicar rapidamente quando houver risco operacional, falha de infraestrutura ou condicao fora do normal.

## Itens monitorados

- Temperatura.
- Energia.
- Nobreak.
- Internet.
- Gerador ligado.
- Porta do rack aberta.
- Porta do datacenter aberta.
- Fumaca/incendio.
- Umidade.
- Ruido.
- Camera interna.

## Papel da placa ESP32

A placa ESP32 ficara instalada no rack ou em ponto estrategico da sala. Ela devera ler sensores fisicos e enviar os dados para o backend por MQTT.

Possiveis entradas/sensores:

- Sensor de temperatura.
- Sensor de umidade.
- Sensor magnetico da porta do rack.
- Sensor magnetico da porta da sala.
- Contato seco do nobreak indicando rede/bateria/falha.
- Contato seco ou rele indicando gerador ligado.
- Sensor de falta de energia.
- Sensor de vazamento ou alarme auxiliar, se necessario.
- Sensor de fumaca/incendio com saida por rele ou contato seco.
- Sensor de ruido, se houver necessidade de detectar alarme sonoro ou anomalia acustica.

## Papel do backend

O backend DC Monitor devera:

- Receber mensagens MQTT da placa.
- Gravar historico de leituras e eventos.
- Monitorar disponibilidade da internet.
- Monitorar servidores e servicos pela rede.
- Exibir dashboard operacional.
- Gerar alertas por WhatsApp, e-mail ou outro canal.
- Permitir configuracao de limites e regras de alerta.

## Dashboard esperado

O dashboard deve mostrar:

- Status geral: `Datacenter OK` ou `Alerta`.
- Temperatura atual.
- Umidade atual.
- Estado da energia.
- Estado do nobreak.
- Estado da internet.
- Estado do gerador.
- Porta do rack aberta/fechada.
- Porta do datacenter aberta/fechada.
- Estado de fumaca/incendio.
- Nivel ou status de ruido.
- Camera interna ou link/preview da camera.
- Historico recente de eventos.

## Alertas esperados

O sistema deve alertar quando:

- Temperatura passar do limite configurado.
- Umidade passar do limite configurado.
- Energia cair.
- Nobreak entrar em bateria.
- Nobreak reportar falha.
- Internet ficar inativa.
- Gerador ligar.
- Porta do rack abrir fora do horario permitido.
- Porta do datacenter abrir fora do horario permitido.
- Sensor de fumaca/incendio disparar.
- Ruido anormal for detectado.
- A placa ficar offline.

## Arquitetura proposta

```text
Sensores fisicos
  -> Placa ESP32 no rack
  -> MQTT
  -> Backend DC Monitor
  -> Banco de dados
  -> Dashboard
  -> Alertas
```

Para os servidores, o backend pode monitorar diretamente pela rede:

```text
Backend DC Monitor
  -> ping nos servidores
  -> checagem de portas
  -> checagem de servicos
  -> agentes/scripts opcionais
  -> historico e alertas
```

## MVP sugerido

Primeira versao realista:

- ESP32 envia temperatura.
- ESP32 envia umidade.
- ESP32 envia porta do rack aberta/fechada.
- ESP32 envia energia presente/ausente.
- Backend monitora internet ativa/inativa.
- Backend exibe dashboard simples.
- Backend envia alerta quando houver temperatura alta, porta aberta fora do horario ou queda de energia.

Depois do MVP:

- Adicionar nobreak.
- Adicionar gerador.
- Adicionar porta do datacenter.
- Adicionar fumaca/incendio.
- Adicionar ruido.
- Integrar camera interna.
- Adicionar monitoramento dos servidores.

## Observacoes de hardware

A placa atual e adequada para monitorar sinais digitais e enviar telemetria por MQTT. Para sensores analogicos ou digitais especificos, pode ser necessario adaptar a leitura no firmware ou usar sensores/modulos com saida por contato seco/rele.

Para ambiente de datacenter, sempre que possivel preferir sensores com contato seco, normalmente fechado, para que rompimento de cabo ou desconexao tambem seja detectado como falha.

## Nome do projeto

Nome sugerido:

`DC Monitor`

Descricao curta:

Sistema de monitoramento fisico e operacional para datacenter.
