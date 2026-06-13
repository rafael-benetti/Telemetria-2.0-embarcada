#ifndef MEM_EEPROM_H
#define MEM_EEPROM_H

void MEM_writeInt(uint32_t ADR, uint32_t data);
uint32_t MEM_readInt(uint32_t ADR);
void MEM_writeString(uint32_t ADR, String data);
String MEM_readString(uint32_t ADR);
void MEM_init();
uint8_t MEM_readChar(uint32_t ADR);
void MEM_writeChar(uint32_t ADR, uint8_t data);
void MEM_writeUShort(uint32_t ADR, uint16_t data);
uint16_t MEM_readUShort(uint32_t ADR);
PinInput_t MEM_readPinInput(uint32_t ADR);
void MEM_writePinInput(uint32_t ADR, PinInput_t data);






#endif