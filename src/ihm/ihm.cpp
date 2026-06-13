#include "includes.h"

QueueHandle_t xIHMQueue;

eREDE_STATUS redeStatus = CONNECTING_WIFI;

void IHM_task(void *task)
{
    esp_task_wdt_add(NULL);
    xIHMQueue = xQueueCreate(5, sizeof(eREDE_STATUS));
    if (xIHMQueue == NULL)
    {
        DBG_PRINTLN("Fail to create the IHM queue.");
        ESP.restart();
    }
    static uint32_t counter = 0;

    while (true)
    {
        esp_task_wdt_reset();
        vTaskDelay(50 / portTICK_RATE_MS);
        xQueueReceive(xIHMQueue, &redeStatus, (TickType_t)100);
        counter++;

        if (redeStatus == SETUP_REDE)
        {
            digitalWrite(PIN_LED_CONEXAO, HIGH);
            digitalWrite(PIN_LED_PULSO, HIGH);
        }
        else
        {
            digitalWrite(PIN_LED_PULSO, LOW);
            
            if (redeStatus == CONNECTED)
            {
                digitalWrite(PIN_LED_CONEXAO, HIGH);
            }

            else if (redeStatus == CONNECTING_WIFI and counter % 10 == 0)
            {
                digitalWrite(PIN_LED_CONEXAO, !digitalRead(PIN_LED_CONEXAO));
            }

            else if (redeStatus == CONNECTING_AWS and counter % 5 == 0)
            {

                digitalWrite(PIN_LED_CONEXAO, !digitalRead(PIN_LED_CONEXAO));
            }

            if (counter == 500)
                counter = 0;
        }
    }
}
