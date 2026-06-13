#ifndef REDE_H
#define REDE_H

enum eREDE
{
    GSM,
    WIFI
};


enum eREDE_STATUS
{
    CONNECTING_WIFI,
    CONNECTED,
    CONNECTING_AWS,
    SETUP_REDE
};

extern eREDE REDE_conexao;
void REDE_getClient(PubSubClient *client);
void REDE_init();
bool REDE_connect();
bool REDE_isConected();
int8_t REDE_getSignal();
bool REDE_ota(String fileName);
#endif