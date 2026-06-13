#include "includes.h"
#include "NimBLEDevice.h"
#include "ble.h"
#include "bleCallbacks.h"

void BLE_task(void *pvt)
{
    NimBLEDevice::init(BOARD_deviceName.c_str());
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    String buffer;

    /* Create the server */
    BLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ble_server_callback());

    /* Create the services*/
    BLEService *pDi = pServer->createService(NimBLEUUID((uint16_t)0x180A));  /* Device Information  */
    BLEService *pHid = pServer->createService(NimBLEUUID((uint16_t)0x1812)); /* Human Interface Device  */

    /* Create the DI characteristics */

    /* Serial Number UUID */
    NimBLECharacteristic *pSerialNumber = pDi->createCharacteristic(NimBLEUUID((uint16_t)0x2A25),
                                                                    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC);
    pSerialNumber->setValue(BOARD_deviceId);

    /* Manufactor Name UUID */
    NimBLECharacteristic *pManufactorName = pDi->createCharacteristic(NimBLEUUID((uint16_t)0x2A29),
                                                                      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC);
    pManufactorName->setValue(MANUFACTOR_NAME);

    /* Hardware Revision UUID */
    NimBLECharacteristic *pHardwareRevision = pDi->createCharacteristic(NimBLEUUID((uint16_t)0x2A27),
                                                                        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC);
    pHardwareRevision->setValue(HW_REVISION);

    /* Model Number UUID */
    NimBLECharacteristic *pModel = pDi->createCharacteristic(NimBLEUUID((uint16_t)0x2A24),
                                                             NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC);
    pModel->setValue(BOARD_MODEL);

    /* Firmware Revision UUID */
    NimBLECharacteristic *pFirmware = pDi->createCharacteristic(NimBLEUUID((uint16_t)0x2A26),
                                                                NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC);
    pFirmware->setValue(FW_VERSION);

    /* Create the HID characteristics */

    /* WIFI SSID UUID */
    NimBLECharacteristic *pWifiSSID = pHid->createCharacteristic(NimBLEUUID(UUID_WIFI_SSID),
                                                                 NIMBLE_PROPERTY::READ |
                                                                     NIMBLE_PROPERTY::WRITE |
                                                                     NIMBLE_PROPERTY::READ_ENC |
                                                                     NIMBLE_PROPERTY::WRITE_ENC);
    buffer = MEM_readString(ADR_WIFI_SSID);
    pWifiSSID->setValue((const uint8_t *)buffer.c_str(), buffer.length());
    pWifiSSID->setCallbacks(new ble_charac_wifi_ssid_callback);

    /* WIFI PSW UUID */
    NimBLECharacteristic *pWifiPSW = pHid->createCharacteristic(NimBLEUUID(UUID_WIFI_PSW),
                                                                NIMBLE_PROPERTY::READ |
                                                                    NIMBLE_PROPERTY::WRITE |
                                                                    NIMBLE_PROPERTY::READ_ENC |
                                                                    NIMBLE_PROPERTY::WRITE_ENC);
    buffer = MEM_readString(ADR_WIFI_PSW);
    pWifiPSW->setValue((const uint8_t *)buffer.c_str(), buffer.length());
    pWifiPSW->setCallbacks(new ble_charac_wifi_psw_callback);

    /* NETWORK UUID */
    NimBLECharacteristic *pNetwork = pHid->createCharacteristic(NimBLEUUID(UUID_NETWORK),
                                                                NIMBLE_PROPERTY::READ |
                                                                    NIMBLE_PROPERTY::WRITE |
                                                                    NIMBLE_PROPERTY::READ_ENC |
                                                                    NIMBLE_PROPERTY::WRITE_ENC);
    pNetwork->setValue(MEM_readChar(ADR_REDE));
    pNetwork->setCallbacks(new ble_charac_network_callback);

    /* GSM APN UUID */
    NimBLECharacteristic *pGsmApn = pHid->createCharacteristic(NimBLEUUID(UUID_GSM_APN),
                                                               NIMBLE_PROPERTY::READ |
                                                                   NIMBLE_PROPERTY::WRITE |
                                                                   NIMBLE_PROPERTY::READ_ENC |
                                                                   NIMBLE_PROPERTY::WRITE_ENC);
    buffer = MEM_readString(ADR_GSM_APN);
    pGsmApn->setValue((const uint8_t *)buffer.c_str(), buffer.length());
    pGsmApn->setCallbacks(new ble_charac_gsm_apn_callback);

    /* GSM LOGIN UUID */
    NimBLECharacteristic *pGsmLogin = pHid->createCharacteristic(NimBLEUUID(UUID_GSM_LOGIN),
                                                                 NIMBLE_PROPERTY::READ |
                                                                     NIMBLE_PROPERTY::WRITE |
                                                                     NIMBLE_PROPERTY::READ_ENC |
                                                                     NIMBLE_PROPERTY::WRITE_ENC);
    
    buffer = MEM_readString(ADR_GSM_LOGIN);
    pGsmLogin->setValue((const uint8_t *)buffer.c_str(), buffer.length());
    pGsmLogin->setCallbacks(new ble_charac_gsm_login_callback);

    /* GSM PASS UUID */
    NimBLECharacteristic *pGsmPass = pHid->createCharacteristic(NimBLEUUID(UUID_GSM_PASS),
                                                                NIMBLE_PROPERTY::READ |
                                                                    NIMBLE_PROPERTY::WRITE |
                                                                    NIMBLE_PROPERTY::READ_ENC |
                                                                    NIMBLE_PROPERTY::WRITE_ENC);
    buffer = MEM_readString(ADR_GSM_PASS);
    pGsmPass->setValue((const uint8_t *)buffer.c_str(), buffer.length());
    pGsmPass->setCallbacks(new ble_charac_gsm_pass_callback);

    /* GSM PIN UUID */
    NimBLECharacteristic *pGsmPin = pHid->createCharacteristic(NimBLEUUID(UUID_GSM_PIN),
                                                               NIMBLE_PROPERTY::READ |
                                                                   NIMBLE_PROPERTY::WRITE |
                                                                   NIMBLE_PROPERTY::READ_ENC |
                                                                   NIMBLE_PROPERTY::WRITE_ENC);
    buffer = MEM_readString(ADR_GSM_PIN);
    pGsmPin->setValue((const uint8_t *)buffer.c_str(), buffer.length());
    pGsmPin->setCallbacks(new ble_charac_gsm_pin_callback);

    /* Start the services */
    pDi->start();
    pHid->start();

    /* Create de Advertising */
    NimBLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->setAppearance(0x0180); // Generic Remote Interface
    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setStaticPIN(999999);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
    while (1)
        vTaskDelay(100);
}
