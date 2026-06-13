#include "includes.h"

eREDE REDE_conexao;
eREDE_STATUS connectionStatus;

void REDE_init()
{
    String buffer;
    REDE_conexao = (eREDE)MEM_readChar(ADR_REDE);
    if (REDE_conexao != WIFI && REDE_conexao != GSM)
    {
        REDE_conexao = WIFI;
        MEM_writeChar(ADR_REDE, (uint8_t)REDE_conexao);
    }
    buffer = (REDE_conexao == WIFI) ? "WIFI" : "GSM";
    DBG_tag("Network: ", buffer);
    WIFI_init();
    GSM_init();
}

bool REDE_connect()
{
    bool conectionState = false;

    if (REDE_conexao == WIFI)
    {
        conectionState = WIFI_connect();
    }
    else if (REDE_conexao == GSM)
    {
        conectionState = GSM_connect();
    }
    return conectionState;
}

void REDE_getClient(PubSubClient *client)
{
    if (REDE_conexao == WIFI)
    {
        client->setClient(wifiClient);
    }
    if (REDE_conexao == GSM)
    {
        client->setClient(gsmClient);
    }
}

bool REDE_isConected()
{
    if (REDE_conexao == WIFI)
        return WIFI_isConected();

    if (REDE_conexao == GSM)
        return GSM_isConected();

    return false;
}

int8_t REDE_getSignal()
{
    int8_t rssi = -1;

    if (REDE_conexao == WIFI)
        rssi = WiFi.RSSI();
    else if (REDE_conexao == GSM)
        rssi = GSM_getSignal();

    if (rssi != -1)
    {
        /* Threshold */
        if (rssi > -50)
        {
            rssi = -50;
        }
        else if (rssi < -100)
        {
            rssi = -100;
        }

        /* Transform to percent */
        rssi = 2 * rssi + 200;
    }
    return rssi;
}

bool REDE_ota(String fileName)
{
    if (REDE_conexao == WIFI)
        return WIFI_ota(fileName);

    if (REDE_conexao == GSM)
        return GSM_ota(fileName);
    return false;
}
