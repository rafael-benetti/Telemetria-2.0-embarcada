
#include "includes.h"
#include "PubSubClient.h"

// =============  PROTOTIPOS  =================
void MQTT_callback(char *topic, uint8_t *payload, uint32_t lenght);
void MQTT_start();
void MQTT_task(void *task);
bool MQTT_connect();
// ============================================

// =============== VARIAVEIS GLOBAIS ===================
TaskHandle_t MQTT_handleTask;
PubSubClient MQTT_client;
PinInput_t receiveMsg;
char MQTT_buffer[128];

// =====================================================

// ============= DEFINIÇOES  =================

bool MQTT_sendJsonToAws(char *data)
{
    return MQTT_client.publish(AWS_IOT_TOPIC_PUBLISH, data);
}

bool MQTT_connect()
{
    MQTT_client.setServer(AWS_IOT_ENDPOINT, AWS_PORT);

    vTaskDelay(500 / portTICK_PERIOD_MS);

    MQTT_client.setCallback(MQTT_callback);

    MQTT_client.setKeepAlive(MQTT_KEEP_ALIVE);

    DBG_PRINT("Connecting to Broker with ID: ");
    DBG_PRINTLN(BOARD_deviceName);
    if (!MQTT_client.connect(BOARD_deviceName.c_str()))
    {
        DBG_PRINTLN("Fail to connect on broker.");
        return false;
    }
    else if (!MQTT_client.subscribe(BOARD_topicSub.c_str(), MQTT_QOS))
    {
        DBG_PRINT("Fail to subscriber on topic: ");
        DBG_PRINTLN(BOARD_topicSub);
        return true;
    }

    DBG_PRINTLN("Conected.");
    return true;
}
void MQTT_callback(char *topic, uint8_t *payload, uint32_t lenght)
{
    StaticJsonDocument<128> jsonDoc;
    deserializeJson(jsonDoc, payload);
    String action = jsonDoc[KEY_TYPE];
    DBG_PRINTLN(action);
    if (action == "RestartMachine")
    {
        GPIO_restartPIN(PIN_RESTART_MACHINE);
    }
    else if (action == "RestartPayments")
    {
        GPIO_restartPIN(PIN_RESTART_PAY);
    }
    else if (action == "remoteCredit")
    {
        GPIO_remoteCredit(jsonDoc["credit"]);
    }
    else if (action == "update")
    {
        String otaFileName = "/" + jsonDoc[KEY_FILE_NAME].as<String>();
        REDE_ota(otaFileName);
        ESP.restart();
    }
}

void MQTT_start()
{
    eREDE_STATUS redeStatus;
    REDE_getClient(&MQTT_client);

    while (true)
    {
        MQTT_client.loop();
        vTaskDelay(2000 / portTICK_RATE_MS);

        if (xQueueReceive(xMessageQueue, &receiveMsg, (TickType_t)100))
        {
            JSON_getJson(receiveMsg, MQTT_buffer);
            DBG_PRINTLN("Enviando dado: " + String(MQTT_buffer));
            if (!MQTT_sendJsonToAws(MQTT_buffer) and receiveMsg.action != ePING)
            {
                DBG_PRINTLN("Falha ao enviar. Salvando na memória...");
                DATAOFF_save(receiveMsg);
            }
        }

        if (!REDE_isConected())
        {
            redeStatus = CONNECTING_WIFI;
            xQueueSend(xIHMQueue, &redeStatus, portMAX_DELAY);
            DBG_PRINTLN("Conectando na rede...");
            if (REDE_connect())
            {
                redeStatus = CONNECTED;
                xQueueSend(xIHMQueue, &redeStatus, portMAX_DELAY);
            }
        }
        else if (!MQTT_client.connected())
        {
            redeStatus = CONNECTING_AWS;
            xQueueSend(xIHMQueue, &redeStatus, portMAX_DELAY);
            if (MQTT_connect())
            {
                redeStatus = CONNECTED;
                MAIN_timePing = TIME_FOR_PING;
                xQueueSend(xIHMQueue, &redeStatus, portMAX_DELAY);
            }
        }
        else if (DATAOFF_get())
        {
            DATAOFF_sendData();
        }
    }
}

void MQTT_task(void *tsk)
{
    REDE_connect();
    MQTT_start();
}
