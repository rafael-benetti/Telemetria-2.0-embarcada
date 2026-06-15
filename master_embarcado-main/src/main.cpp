#include "includes.h"
#include "ble.h"

uint16_t MAIN_timePing = TIME_FOR_PING;
volatile bool MAIN_config = false;
volatile bool MAIN_cli = false;

void MAIN_decode(PinInput_t *data, bool isPing)
{
  static PinInput_t sendMsg;
  if (isPing)
    sendMsg.action = ePING;
  else
  {
    sendMsg.action = eCREDIT;
    sendMsg.pinNumber = data->pinNumber;

#ifdef MACHINE_MODEL_RL
    if (data->pinNumber > 2)
      sendMsg.qtd = 1;
    else
#endif

      sendMsg.qtd = data->qtd;
    data->qtd = 0;
  }

  xQueueSend(xMessageQueue, &sendMsg, portMAX_DELAY);
}

void setup()
{
#ifdef configENABLE_DEBUG
  Serial.begin(9600);
#endif
  esp_task_wdt_init(120, true);
  GPIO_init();
  RELAY_init();
  FAST_init();
  MEM_init();
  DATAOFF_init();
  BOARD_init();
  REDE_init();

  vTaskDelay(pdMS_TO_TICKS(500));

  xTaskCreatePinnedToCore(IHM_task, "IHM_task", 1024, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(MQTT_task, "MQTT_task", 10000, NULL, 1, &MQTT_handleTask, 0);
  xTaskCreatePinnedToCore(BLE_task, "BLE_task", 20000, NULL, 1, NULL, 1);

  vTaskDelay(10000 / portTICK_RATE_MS);

  GPIO_interruptStart();
}
void loop()
{
  while (1)
  {
    esp_task_wdt_reset();
    MAIN_timePing++;

    if (!digitalRead(PIN_CONFIG) && !MAIN_config)
    {
      MAIN_config = true;
      xTaskCreatePinnedToCore(WEBSERVER_task, "WebServer_task", 8192, NULL, 1, &WEBSERVER_handleTask, 1);
    }
#ifdef configSERIAL_CLI
    if (!digitalRead(PIN_BOOT) && !MAIN_cli)
    {
      MAIN_cli = true;
      xTaskCreatePinnedToCore(CLI_task, "CLI_task", 8192, NULL, 1, &CLI_handleTask, 1);
    }
#endif

    if (MAIN_timePing > TIME_FOR_PING)
    {
      MAIN_timePing = 0;
      MAIN_decode(NULL, true);
    }

    if (FAST_getFlagCredito())
    {
      FAST_setFlagCredito(false);

      for (uint8_t i = 0; i < 13; i++)
      {
        if (GPIO_inputs[i].qtd)
        {
          GPIO_inputs[i].pinNumber = i + 1;
          MAIN_decode(&GPIO_inputs[i], false);
        }
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void MAIN_setConfig(bool isConfig)
{
  MAIN_config = isConfig;
}

void MAIN_setCLI(bool isCLI)
{
  MAIN_cli = isCLI;
}
