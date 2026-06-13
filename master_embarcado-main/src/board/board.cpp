#include "includes.h"

uint16_t BOARD_deviceId;
String BOARD_topicSub;
String BOARD_deviceName;
void BOARD_init(){
    BOARD_deviceId = MEM_readUShort(ADR_DEVICE_ID);
    BOARD_topicSub = MQTT_TOPIC_COMMAND_PREFIX + (String) BOARD_deviceId + MQTT_TOPIC_COMMAND_SUFFIX;
    BOARD_deviceName = "BLK-" + (String) BOARD_deviceId;
    
    DBG_tag("Board :", BOARD_deviceName);

}
