#ifndef BLE_CALLBACKS_H_
#define BLE_CALLBACKS_H_

class ble_charac_wifi_ssid_callback : public NimBLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        String buffer =  (String) pCharacteristic->getValue().c_str();
        MEM_writeString(ADR_WIFI_SSID,buffer);
        WIFI_ssid = buffer;
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
    }
};

class ble_charac_wifi_psw_callback : public NimBLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        String buffer =  (String) pCharacteristic->getValue().c_str();
        MEM_writeString(ADR_WIFI_PSW,buffer);
        WIFI_psw = buffer;

    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
    }
};

class ble_charac_network_callback : public NimBLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        uint8_t buffer = pCharacteristic->getValue<uint8_t>();
        DBG_PRINTLN(buffer);
        MEM_writeChar(ADR_REDE, buffer);
        REDE_conexao = (eREDE) buffer;
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
    }
};

class ble_charac_gsm_apn_callback : public NimBLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        String buffer =  (String) pCharacteristic->getValue().c_str();
        MEM_writeString(ADR_GSM_APN,buffer);
        GSM_apn = buffer;
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
    }
};

class ble_charac_gsm_login_callback : public NimBLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        String buffer =  (String) pCharacteristic->getValue().c_str();
        MEM_writeString(ADR_GSM_LOGIN,buffer);
        GSM_login = buffer;
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
    }
};

class ble_charac_gsm_pass_callback : public NimBLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        String buffer =  (String) pCharacteristic->getValue().c_str();
        MEM_writeString(ADR_GSM_PASS,buffer);
        GSM_pass = buffer;
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
    }
};

class ble_charac_gsm_pin_callback : public NimBLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        String buffer =  (String) pCharacteristic->getValue().c_str();
        MEM_writeString(ADR_GSM_PIN,buffer);
        GSM_pin = buffer;
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
    }
};

class ble_server_callback : public NimBLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        BLEDevice::startAdvertising();
    }

    void onDisconnect(BLEServer *pServer)
    {
        BLEDevice::startAdvertising();
    }

    /***** Security *******/

    uint32_t onPassKeyRequest()
    {
        return 123456;
    }

    bool onConfirmPIN(uint32_t pass_key)
    {
        return false;
    }

    void onPassKeyNotify(uint32_t pass_key)
    {
    }

    bool onSecurityRequest()
    {
        return false;
    }

    void onAuthenticationComplete(ble_gap_conn_desc *desc)
    {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!desc->sec_state.encrypted)
        {
            /** NOTE: createServer returns the current server reference unless one is not already created */
            NimBLEDevice::createServer()->disconnect(desc->conn_handle);
            return;
        }
    };
};

#endif