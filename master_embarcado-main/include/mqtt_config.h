#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifndef MQTT_BROKER_HOST
#define MQTT_BROKER_HOST "192.168.0.20"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

#ifndef MQTT_TOPIC_PUBLISH
#define MQTT_TOPIC_PUBLISH "dc/telemetry"
#endif

#ifndef MQTT_TOPIC_COMMAND_PREFIX
#define MQTT_TOPIC_COMMAND_PREFIX "dc/"
#endif

#ifndef MQTT_TOPIC_COMMAND_SUFFIX
#define MQTT_TOPIC_COMMAND_SUFFIX "/cmd"
#endif

#define MQTT_QOS 1

#define MQTT_KEEP_ALIVE 30

#ifndef OTA_HOST
#define OTA_HOST MQTT_BROKER_HOST
#endif

#ifndef OTA_PORT
#define OTA_PORT 8080
#endif
#endif
