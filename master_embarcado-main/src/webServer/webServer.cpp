#include "includes.h"

//DEFINES
#define FORMAT_SPIFFS_IF_FAILED true
#define AP_SSID "Telemetria"
#define AP_PSW ""

//Variaveis Globais
TaskHandle_t WEBSERVER_handleTask;
AsyncWebServer HTTP_server(80);


//Prototipos de funções
void getHandlers(void);
void WEBSERVER_task(void *pvt);

void WEBSERVER_task(void *pvt)
{
    esp_task_wdt_delete(MQTT_handleTask);
    vTaskDelete(MQTT_handleTask);
    for (size_t i = 0; i < MAX_PINS_INPUT; i++)
    {
        detachInterrupt(PINS_ENTRY[i]);
    }

    eREDE_STATUS connectionStatus = SETUP_REDE;
    xQueueSend(xIHMQueue, &connectionStatus, portMAX_DELAY);

    //Sistema de Arquivos
    if (!SPIFFS.begin())
    {
        DBG_PRINTLN("An Error has occurred while mounting SPIFFS");
        ESP.restart();
    }

    // Configura WIFI
    WiFi.disconnect();
    WiFi.mode(WIFI_AP_STA);
    vTaskDelay(pdMS_TO_TICKS(300));
    WiFi.softAPConfig(IPAddress(10, 10, 10, 10), IPAddress(10, 10, 10, 10), IPAddress(255, 255, 255, 0));
    vTaskDelay(pdMS_TO_TICKS(100));
    WiFi.softAP(AP_SSID, AP_PSW);
    vTaskDelay(pdMS_TO_TICKS(100));
    DBG_PRINTLN("");
    DBG_PRINTLN("WiFi Criado com o SSID: ");
    DBG_PRINTLN(AP_SSID);
    DBG_PRINTLN("IP address: ");
    DBG_PRINTLN(WiFi.softAPIP());

    // Configura Servidor HTTP
    getHandlers();
    HTTP_server.begin();
    DBG_PRINTLN("Server iniciado...");
    while (true)
    {
        esp_task_wdt_reset();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
};

void getHandlers(void)
{
    HTTP_server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/bootstrap.min.css", "text/css");
    });

    HTTP_server.on("/customs.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/customs.css", "text/css");
    });

    HTTP_server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/jquery.min.js", "text/javascript");
    });

    HTTP_server.on("/black.png", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/black.png", "image");
    });

    HTTP_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    HTTP_server.on("/getValues", HTTP_GET, [](AsyncWebServerRequest *request) {
        const size_t capacity = JSON_ARRAY_SIZE(13) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 14 * JSON_OBJECT_SIZE(4);
        DynamicJsonDocument doc(capacity);

        doc["version"] = FW_VERSION;
        doc["deviceName"] = BOARD_deviceName;

        doc["type"] = MEM_readChar(ADR_REDE);

        JsonObject gsm = doc.createNestedObject("gsm");
        gsm["apn"] = MEM_readString(ADR_GSM_APN);
        gsm["login"] = MEM_readString(ADR_GSM_LOGIN);
        gsm["pass"] = MEM_readString(ADR_GSM_PASS);
        gsm["pin"] = MEM_readString(ADR_GSM_PIN);

        JsonObject wifi = doc.createNestedObject("wifi");
        wifi["login"] = MEM_readString(ADR_WIFI_SSID);
        wifi["pass"] = MEM_readString(ADR_WIFI_PSW);

        uint16_t countBytes = measureJson(doc) + 1;
        char WEBSERVER_response[countBytes];
        serializeJson(doc, WEBSERVER_response, countBytes);
        request->send(200, "application/json", WEBSERVER_response);
    });

    HTTP_server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
        const String response = request->getParam(static_cast<size_t>(0))->value();

        const size_t capacity = JSON_ARRAY_SIZE(13) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 14 * JSON_OBJECT_SIZE(4) + 180;
        DynamicJsonDocument jsonDoc(capacity);
        deserializeJson(jsonDoc, response);
        if (jsonDoc["gsm"])
        {
            const char *gsm_apn = jsonDoc["gsm"]["apn"];
            const char *gsm_login = jsonDoc["gsm"]["login"];
            const char *gsm_pass = jsonDoc["gsm"]["pass"];
            const char *gsm_pin = jsonDoc["gsm"]["pin"];
            MEM_writeString(ADR_GSM_APN, gsm_apn);
            MEM_writeString(ADR_GSM_LOGIN, gsm_login);
            MEM_writeString(ADR_GSM_PASS, gsm_pass);
            MEM_writeString(ADR_GSM_PIN, gsm_pin);
            MEM_writeChar(ADR_REDE, 0);
        }
        if (jsonDoc["wifi"])
        {
            const char *wifi_login = jsonDoc["wifi"]["login"];
            const char *wifi_pass = jsonDoc["wifi"]["pass"];
            MEM_writeString(ADR_WIFI_SSID, wifi_login);
            MEM_writeString(ADR_WIFI_PSW, wifi_pass);
            MEM_writeChar(ADR_REDE, 1);
        }

        request->send(200, "application/json", "{\"message\":\"Welcome\"}");

        vTaskDelay(3000 / portTICK_PERIOD_MS);

        ESP.restart();
    });
};
