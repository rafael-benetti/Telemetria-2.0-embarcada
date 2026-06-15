
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
    return MQTT_client.publish(MQTT_TOPIC_PUBLISH, data);
}

bool MQTT_sendRelayConfirmation(const char *command, uint32_t durationMs, const char *status)
{
    JSON_getRelayConfirmation(command, status, durationMs, MQTT_buffer);
    return MQTT_sendJsonToAws(MQTT_buffer);
}

bool MQTT_connect()
{
    MQTT_client.setServer(MQTT_BROKER_HOST, MQTT_PORT);

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
    DeserializationError error = deserializeJson(jsonDoc, payload, lenght);
    if (error)
    {
        DBG_PRINT("Invalid MQTT payload: ");
        DBG_PRINTLN(error.c_str());
        return;
    }

    String action = jsonDoc[KEY_COMMAND] | jsonDoc[KEY_TYPE] | "";
    DBG_PRINT("MQTT command received: ");
    DBG_PRINTLN(action);

    if (action.length() == 0)
    {
        DBG_PRINTLN("Invalid command");
        return;
    }

    if (action == "RestartMachine")
    {
        GPIO_restartPIN(PIN_RESTART_MACHINE);
        JSON_getJsonResponse(action, true, MQTT_buffer);
        MQTT_sendJsonToAws(MQTT_buffer);
    }
    else if (action == "update")
    {
        JSON_getJsonResponse(action, true, MQTT_buffer);
        MQTT_sendJsonToAws(MQTT_buffer);
        String otaFileName = jsonDoc[KEY_FILE_NAME] | "firmware.bin";
        otaFileName.trim();
        if (otaFileName.length() == 0 || otaFileName == "null")
        {
            otaFileName = "firmware.bin";
        }
        if (!otaFileName.startsWith("/"))
        {
            otaFileName = "/" + otaFileName;
        }
        REDE_ota(otaFileName);
        ESP.restart();
    }
    else if (action == COMMAND_ACTION_J8)
    {
        uint32_t durationMs = jsonDoc[KEY_DURATION_MS] | 1000;
        RELAY_requestPulseOnPin(23, false, durationMs, action.c_str());
        MQTT_sendRelayConfirmation(action.c_str(), durationMs, "executed");
    }
    else if (action == COMMAND_ACTION_J11)
    {
        uint32_t durationMs = jsonDoc[KEY_DURATION_MS] | 1000;
        RELAY_requestPulseOnPin(22, false, durationMs, action.c_str());
        MQTT_sendRelayConfirmation(action.c_str(), durationMs, "executed");
    }
    else if (action == COMMAND_ACTION_J8_PULSE)
    {
        uint32_t durationMs = jsonDoc[KEY_DURATION_MS] | 1000;
        RELAY_requestPulseOnPin(23, false, durationMs, action.c_str());
        MQTT_sendRelayConfirmation(action.c_str(), durationMs, "executed");
    }
    else if (action == COMMAND_ACTION_J11_PULSE)
    {
        uint32_t durationMs = jsonDoc[KEY_DURATION_MS] | 1000;
        RELAY_requestPulseOnPin(22, false, durationMs, action.c_str());
        MQTT_sendRelayConfirmation(action.c_str(), durationMs, "executed");
    }
    else if (action == COMMAND_ACTION_J8_ON)
    {
        RELAY_requestStateOnPin(23, false, true, action.c_str());
        MQTT_sendRelayConfirmation(action.c_str(), 0, "executed");
    }
    else if (action == COMMAND_ACTION_J8_OFF)
    {
        RELAY_requestStateOnPin(23, false, false, action.c_str());
        MQTT_sendRelayConfirmation(action.c_str(), 0, "executed");
    }
    else if (action == COMMAND_ACTION_J11_ON)
    {
        RELAY_requestStateOnPin(22, false, true, action.c_str());
        MQTT_sendRelayConfirmation(action.c_str(), 0, "executed");
    }
    else if (action == COMMAND_ACTION_J11_OFF)
    {
        RELAY_requestStateOnPin(22, false, false, action.c_str());
        MQTT_sendRelayConfirmation(action.c_str(), 0, "executed");
    }
    else
    {
        DBG_PRINTLN("Invalid command");
    }
}

void MQTT_start()
{
    eREDE_STATUS redeStatus;
    REDE_getClient(&MQTT_client);

    while (true)
    {
        MQTT_client.loop();
        RELAY_taskUpdate();
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
