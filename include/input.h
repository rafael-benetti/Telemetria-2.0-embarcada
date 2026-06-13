#ifndef _INPUT_H_
#define _INPUT_H_

#define MAX_SIZE_PIN_INPUT          12 // return sizeof(sPIN_INPUT)

#define GIFT "gift"
#define CREDIT "credit"
#define PING "ping"

typedef enum
{
  eCREDIT = 0,
  eGIFT,
  ePING
} Actions_t;

typedef struct
{
    volatile uint16_t qtd; // quantity of pulse on GPIO
    Actions_t action;        // The type of entry: credit (0), gift (1) and ping(2) 
    uint8_t pinNumber;            // Index of pin in the board
    bool isOff;                 // Data was save in memory
} PinInput_t;

#endif