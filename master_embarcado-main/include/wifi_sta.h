#ifndef WIFI_STA_H_
#define WIFI_STA_H_

extern String WIFI_ssid;
extern String WIFI_psw;
extern WiFiClient wifiClient;
void WIFI_init(void);
bool WIFI_connect(void);
bool WIFI_isConected();
bool WIFI_ota(String binName);
#endif