#include "includes.h"

uint16_t BOARD_deviceId;
String BOARD_topicSub;
String BOARD_deviceName;
void BOARD_init(){
    BOARD_deviceId = MEM_readUShort(ADR_DEVICE_ID);
    if (BOARD_deviceId == 0xFFFF || BOARD_deviceId == 0)
    {
        BOARD_deviceId = 1;
        MEM_writeUShort(ADR_DEVICE_ID, BOARD_deviceId);
    }
    BOARD_topicSub = MQTT_TOPIC_COMMAND_PREFIX + (String) BOARD_deviceId + MQTT_TOPIC_COMMAND_SUFFIX;
    BOARD_deviceName = "TEL-" + (String) BOARD_deviceId;
    
    DBG_tag("Board :", BOARD_deviceName);

}
