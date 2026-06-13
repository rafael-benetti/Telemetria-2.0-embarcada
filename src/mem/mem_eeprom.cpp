#include "includes.h"

SemaphoreHandle_t memSemaphore;

void MEM_writeInt(uint32_t ADR, uint32_t data);
void MEM_writeString(uint32_t ADR, String data);
uint32_t MEM_readInt(uint32_t ADR);
String MEM_readString(uint32_t ADR);

void MEM_init()
{
    memSemaphore = xSemaphoreCreateMutex();
    if (!memSemaphore)
    {
        DBG_PRINTLN("Fail to create the Memory Semaphore.");
        ESP.restart();
    }
}

void MEM_writeString(uint32_t ADR, String data)
{
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.writeString(ADR, data);
    EEPROM.commit();
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
}

void MEM_writeInt(uint32_t ADR, uint32_t data)
{
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.writeInt(ADR, data);
    EEPROM.commit();
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
}

uint32_t MEM_readInt(uint32_t ADR)
{
    uint32_t buffer;
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    buffer = EEPROM.readInt(ADR);
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
    return buffer;
}

String MEM_readString(uint32_t ADR)
{
    String buffer;
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    buffer = EEPROM.readString(ADR);
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
    return buffer;
}

void MEM_writeChar(uint32_t ADR, uint8_t data)
{
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.writeChar(ADR, data);
    EEPROM.commit();
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
}

uint8_t MEM_readChar(uint32_t ADR)
{
    uint8_t buffer;
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    buffer = EEPROM.readChar(ADR);
    EEPROM.commit();
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
    return buffer;
}

uint16_t MEM_readUShort(uint32_t ADR)
{
    uint16_t buffer;
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    buffer = EEPROM.readUShort(ADR);
    EEPROM.commit();
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
    return buffer;
}

void MEM_writeUShort(uint32_t ADR, uint16_t data)
{
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.writeUShort(ADR, data);
    EEPROM.commit();
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
}

void MEM_writePinInput(uint32_t ADR, PinInput_t data)
{
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(ADR, data);
    EEPROM.commit();
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
}

PinInput_t MEM_readPinInput(uint32_t ADR)
{
    PinInput_t buffer;
    xSemaphoreTake(memSemaphore, portMAX_DELAY);
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(ADR, buffer);
    EEPROM.commit();
    EEPROM.end();
    xSemaphoreGive(memSemaphore);
    return buffer;
}