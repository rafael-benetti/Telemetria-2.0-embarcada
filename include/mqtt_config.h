#ifndef AWS_H
#define AWS_H

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define AWS_IOT_ENDPOINT "broker.blacktelemetry.com"

#define AWS_IOT_TOPIC_PUBLISH "pub"

#define MQTT_QOS 1

#define MQTT_KEEP_ALIVE 30

#define AWS_PORT 1883 //8883

#define OTA_HOST "blacktelemetry-ota.s3-sa-east-1.amazonaws.com"

#define OTA_PORT 80
#endif