#include "includes.h"

uint16_t DATAOFF_flagData;

void DATAOFF_save(PinInput_t data)
{
    PinInput_t pin;
    pin = MEM_readPinInput(ADR_PINS + (data.pinNumber - 1) * MAX_SIZE_PIN_INPUT);
    data.qtd += pin.qtd;
    MEM_writePinInput(ADR_PINS + (data.pinNumber - 1) * MAX_SIZE_PIN_INPUT, data);
    DATAOFF_flagData |= (1 << (data.pinNumber - 1));
    MEM_writeUShort(ADR_DATA_OFFLINE, DATAOFF_flagData);
}

void DATAOFF_init()
{
    DATAOFF_flagData = MEM_readUShort(ADR_DATA_OFFLINE);
    if (DATAOFF_flagData == 0xFFFF)
    {
        DATAOFF_flagData = 0;
        MEM_writeUShort(ADR_DATA_OFFLINE, 0);
    }

    for (size_t i = 0; i < MAX_PINS_INPUT; i++)
    {
        PinInput_t pin = MEM_readPinInput(ADR_PINS + MAX_SIZE_PIN_INPUT * i);
        if (pin.pinNumber == 0xFF || pin.pinNumber == 0 || pin.pinNumber > MAX_PINS_INPUT || pin.qtd == 0xFFFF)
        {
            pin.pinNumber = i + 1;
            pin.qtd = 0;
            pin.action = eCREDIT;
            pin.isOff = false;
            MEM_writePinInput(ADR_PINS + MAX_SIZE_PIN_INPUT * i, pin);
        }
    }

}

void DATAOFF_sendData()
{
    PinInput_t pin;
    for (size_t i = 0; i < MAX_PINS_INPUT; i++)
    {
        if ((DATAOFF_flagData & (1 << i)))
        {
            pin = MEM_readPinInput(ADR_PINS + MAX_SIZE_PIN_INPUT * i);
            if (pin.qtd)
            {
                pin.isOff = true;
                xQueueSend(xMessageQueue, &pin, portMAX_DELAY);
                pin.qtd = 0;
                MEM_writePinInput(ADR_PINS + MAX_SIZE_PIN_INPUT * i, pin);
            }
        }
    }
    DATAOFF_flagData = 0;
    MEM_writeUShort(ADR_DATA_OFFLINE, 0);
}

uint16_t DATAOFF_get()
{
    return DATAOFF_flagData;
}
