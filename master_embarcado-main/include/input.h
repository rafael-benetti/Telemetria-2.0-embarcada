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
    volatile uint16_t qtd; // quantity of pulse or event on GPIO
    Actions_t action;      // Event type stored in the queue
    uint8_t pinNumber;     // Index of pin in the board
    bool isOff;            // Data was saved while offline
} PinInput_t;

#endif
