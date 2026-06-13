#ifndef BLE_H_
#define BLE_H_

/* Custom Characteristc UUIDs */
#define UUID_WIFI_SSID "d1ebfe5d-381b-4100-92b9-b29562a67924"
#define UUID_WIFI_PSW "b9ddfa5f-4e1c-4139-afcf-02b71be0ade5"
#define UUID_NETWORK "cba653ea-8581-4bd2-b419-45f52cb6fc51"
#define UUID_GSM_APN "58a98885-4264-4803-b459-609804ce4d95"
#define UUID_GSM_LOGIN "8e42ef88-d385-4ff3-95af-0e939b856b95"
#define UUID_GSM_PASS "8eae2a0b-5dfc-4889-9ff1-287a07a6fa5d"
#define UUID_GSM_PIN "1a944cf0-29c7-49f9-89f8-cb6fd50b2ade"

/* Functions */
void BLE_task(void *pvt);

#endif