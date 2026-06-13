#ifndef GPIO_H_
#define GPIO_H_

#define MAX_PINS_INPUT 13

#define PIN_RESTART_MACHINE 22
#define PIN_RESTART_PAY 23
#define PIN_REMOTE_CREDIT 13
#define PIN_BOOT 0
#define PIN_CONFIG 03
#define PIN_LED_CONEXAO 05
#define PIN_LED_PULSO 12
#define PIN_RX_MODEM 04
#define PIN_TX_MODEM 02
#define PIN_RST_MODEM 15
const uint8_t PINS_ENTRY[] = {19, 21, 18, 14, 27, 26, 25, 33, 32, 35, 34, 39, 36};

#define DELAY_MQTT 1000

extern PinInput_t GPIO_inputs[MAX_PINS_INPUT];

extern QueueHandle_t xMessageQueue;

void GPIO_init();
void GPIO_restartPIN(const uint32_t pin);
void GPIO_remoteCredit(uint8_t pulses);
void GPIO_interruptStart();

typedef void typeFunction();
#endif