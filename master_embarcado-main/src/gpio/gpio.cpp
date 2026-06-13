#include "includes.h"
QueueHandle_t xMessageQueue;

PinInput_t GPIO_inputs[MAX_PINS_INPUT];
typeFunction *interruptFunctions[MAX_PINS_INPUT];

void GPIO_init()
{
  gpio_pad_select_gpio(PIN_CONFIG);
  gpio_set_direction(GPIO_NUM_3, GPIO_MODE_INPUT);
  for (uint8_t i = 0; i < 13; i++)
  {
    pinMode(PINS_ENTRY[i], INPUT);
  }
  gpio_pad_select_gpio(PIN_BOOT);
  gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
  pinMode(PIN_LED_CONEXAO, OUTPUT);
  pinMode(PIN_LED_PULSO, OUTPUT);
  pinMode(PIN_RST_MODEM, OUTPUT);
  pinMode(PIN_REMOTE_CREDIT, OUTPUT);
  pinMode(PIN_RESTART_MACHINE, OUTPUT);
  pinMode(PIN_RESTART_PAY, OUTPUT);

  digitalWrite(PIN_RST_MODEM, HIGH);
  digitalWrite(PIN_RESTART_MACHINE, LOW);
  digitalWrite(PIN_RESTART_PAY, LOW);

  interruptFunctions[0] = interruptEntrada01;
  interruptFunctions[1] = interruptEntrada02;
  interruptFunctions[2] = interruptEntrada03;
  interruptFunctions[3] = interruptEntrada04;
  interruptFunctions[4] = interruptEntrada05;
  interruptFunctions[5] = interruptEntrada06;
  interruptFunctions[6] = interruptEntrada07;
  interruptFunctions[7] = interruptEntrada08;
  interruptFunctions[8] = interruptEntrada09;
  interruptFunctions[9] = interruptEntrada10;
  interruptFunctions[10] = interruptEntrada11;
  interruptFunctions[11] = interruptEntrada12;
  interruptFunctions[12] = interruptEntrada13;

  vTaskDelay(100 / portTICK_PERIOD_MS);

  xMessageQueue = xQueueCreate(20, sizeof(PinInput_t));
  if (xMessageQueue == NULL)
  {
    DBG_PRINTLN("Fail to create the queue.");
    ESP.restart();
  }
}

void GPIO_interruptStart()
{

  //TODO: ler da memória

  for (uint8_t i = 1; i < 13; i++)
  {

    attachInterrupt(digitalPinToInterrupt(PINS_ENTRY[i]), interruptFunctions[i], FALLING);
  }
}
void GPIO_restartPIN(const uint32_t pin)
{
  digitalWrite(pin, !digitalRead(pin));
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  digitalWrite(pin, !digitalRead(pin));
}

void GPIO_remoteCredit(uint8_t pulses)
{
  detachInterrupt(PINS_ENTRY[0]);
  for (uint16_t i = 0; i < pulses; i++)
  {
    digitalWrite(PIN_REMOTE_CREDIT, HIGH);
    digitalWrite(PIN_LED_PULSO, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(PIN_REMOTE_CREDIT, LOW);
    digitalWrite(PIN_LED_PULSO, LOW);
    vTaskDelay(pdMS_TO_TICKS(300));
  }
}