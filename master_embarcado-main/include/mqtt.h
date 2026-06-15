#ifndef AWS_MQTT_H_
#define AWS_MQTT_H_

#define KEY_SERIAL_NUMBER   "serialNumber"
#define KEY_RSSI            "rssi"
#define KEY_DATE            "date"
#define KEY_COUNT           "count"
#define KEY_TYPE            "type"
#define KEY_COMMAND         "command"
#define KEY_RESPONSE        "response"
#define KEY_FILE_NAME       "fileName"
#define KEY_PIN             "pin"
#define KEY_MODEL           "model"
#define KEY_VERSION         "version"
#define KEY_GIFT            "gift"
#define KEY_DATA_OFF        "data_off"
#define KEY_SLOT            "slot"
#define KEY_NETWORK         "network"
#define KEY_DURATION_MS     "duration_ms"

#define COMMAND_ACTION_J8    "actionj8"
#define COMMAND_ACTION_J11   "actionj11"
#define COMMAND_ACTION_J8_PULSE    "actionj8_pulse"
#define COMMAND_ACTION_J11_PULSE   "actionj11_pulse"
#define COMMAND_ACTION_J8_ON       "actionj8_on"
#define COMMAND_ACTION_J8_OFF      "actionj8_off"
#define COMMAND_ACTION_J11_ON      "actionj11_on"
#define COMMAND_ACTION_J11_OFF     "actionj11_off"

#define TIME_FOR_PING       120         // 1 MIN

void MQTT_task(void *task);
bool MQTT_sendJsonToAws(char *data);
bool MQTT_sendRelayConfirmation(const char *command, uint32_t durationMs, const char *status);
extern TaskHandle_t MQTT_handleTask;
extern char MQTT_buffer[128];


#endif
