#ifndef GSM_H
#define GSM_H


#define TINY_GSM_MODEM_SIM800   // Modem is SIM800

#include "TinyGsmClient.h"
        

extern TinyGsmClient gsmClient;
extern String GSM_apn;
extern String GSM_login;
extern String GSM_pass;
extern String GSM_pin;

void GSM_init();
bool GSM_connect();
bool GSM_isConected();
bool GSM_ota(String binName);
int8_t GSM_getSignal();



#endif
