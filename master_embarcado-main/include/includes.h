#ifndef INCLUDES_H_
#define INCLUDES_H_

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define configENABLE_DEBUG
#define configSERIAL_CLI
//#define MACHINE_MODEL_RL // Roullete

#ifdef MACHINE_MODEL_RL
#define FW_VERSION          "RL-1.0.0"
#else
#define FW_VERSION          "1.0.0"
#endif

#define BOARD_MODEL         "telemetry"
#define MANUFACTOR_NAME     "Black Telemetry"
#define HW_REVISION         "1.3.1"




#define ESP_INTR_FLAG_DEFAULT 0

#define EEPROM_SIZE 4096

#include "Arduino.h"
#include "debug.h"
#include "EEPROM.h"
#include "SPIFFS.h"
#include <esp_task_wdt.h>
#include "input.h"
#include "webserver.h"
#include "mem_eeprom.h"
#include "ESPAsyncWebServer.h"
#include "file.h"
#include "wifi_sta.h"
#include "gpio.h"
#include "relay_control.h"
#include "address.h"
#include "mqtt.h"
#include <ArduinoJson.h>
#include "PubSubClient.h"
#include "mqtt_config.h"
#include "json_aws.h"
#include "fast.h"
#include "ota.h"
#include "ihm.h"
#include "gsm.h"
#include "rede.h"
#include "dataoff.h"
#include "interruptFunctions.h"
#include "board.h"
#include "cli.h"
#include "main.h"

#endif
