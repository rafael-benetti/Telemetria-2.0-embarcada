#ifndef AWS_MQTT_H_
#define AWS_MQTT_H_

#define KEY_SERIAL_NUMBER   "serialNumber"
#define KEY_RSSI            "rssi"
#define KEY_DATE            "date"
#define KEY_COUNT           "count"
#define KEY_TYPE            "type"
#define KEY_RESPONSE        "response"
#define KEY_FILE_NAME       "fileName"
#define KEY_PIN             "pin"
#define KEY_MODEL           "model"
#define KEY_VERSION         "version"
#define KEY_GIFT            "gift"
#define KEY_DATA_OFF        "data_off"
#define KEY_SLOT            "slot"
#define KEY_NETWORK         "network"

#define TIME_FOR_PING       1200        // 10 MIN

void MQTT_task(void *task);
bool MQTT_sendJsonToAws(char *data);
extern TaskHandle_t MQTT_handleTask;
extern char MQTT_buffer[128];


#endif