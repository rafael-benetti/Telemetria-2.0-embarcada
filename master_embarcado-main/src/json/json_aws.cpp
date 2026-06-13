#include "includes.h"

void JSON_getJson(PinInput_t data, char *buffer)
{
    StaticJsonDocument<256> jsonDoc;
    jsonDoc[KEY_SERIAL_NUMBER] = BOARD_deviceId;
    jsonDoc[KEY_RSSI] = REDE_getSignal();
    jsonDoc[KEY_NETWORK] = (REDE_conexao == WIFI) ? "wifi" : "gsm";
    jsonDoc[KEY_VERSION] = FW_VERSION;
    if (data.action == ePING)
    {
        jsonDoc[KEY_TYPE] = PING;
    }
    else
    {
        jsonDoc[KEY_TYPE] = PULSO;
        jsonDoc[KEY_COUNT] = data.qtd;
        jsonDoc[KEY_PIN] = data.pinNumber;
        jsonDoc[KEY_DATA_OFF] = data.isOff;
    }

    // serializeJson(jsonDoc, Serial);
    serializeJson(jsonDoc, buffer, 256);
}

void JSON_getJsonResponse(String action, bool response, char *buffer)
{
    StaticJsonDocument<192> jsonDoc;
    jsonDoc[KEY_SERIAL_NUMBER] = BOARD_deviceId;
    jsonDoc[KEY_TYPE] = COMMAND_ACK;
    jsonDoc[KEY_COMMAND] = action;
    jsonDoc[KEY_RESPONSE] = response;
    jsonDoc[KEY_VERSION] = FW_VERSION;

    // serializeJson(jsonDoc, Serial);
    serializeJson(jsonDoc, buffer, 192);
}

void JSON_getJsonUpdate(char *buffer)
{
    StaticJsonDocument<128> jsonDoc;
    jsonDoc[KEY_SERIAL_NUMBER] = BOARD_deviceId;
    jsonDoc[KEY_TYPE] = "update";
    jsonDoc[KEY_VERSION] = FW_VERSION;

    // serializeJson(jsonDoc, Serial);
    serializeJson(jsonDoc, buffer, 128);
}

// message = {
//     'telemetryBoardId': 1,
//     'fwVersion': '1.0.4',
//     'RSSI': 97,
//     'networkType': 'wifi',
//     'messageType': 'transaction',
//     'quantity': 10,
//     'pin': 3,
//     'isOffline': True
// }
