#include "includes.h"

volatile unsigned long filtroISR[MAX_PINS_INPUT] = {0};
extern hw_timer_t *timer;

void IRAM_ATTR interruptEntrada01()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[0]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[0] = xTaskGetTickCountFromISR();
        GPIO_inputs[0].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}

void IRAM_ATTR interruptEntrada02()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[1]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[1] = xTaskGetTickCountFromISR();
        GPIO_inputs[1].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada03()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[2]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[2] = xTaskGetTickCountFromISR();
        GPIO_inputs[2].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada04()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[3]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[3] = xTaskGetTickCountFromISR();
        GPIO_inputs[3].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada05()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[4]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[4] = xTaskGetTickCountFromISR();
        GPIO_inputs[4].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada06()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[5]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[5] = xTaskGetTickCountFromISR();
        GPIO_inputs[5].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada07()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[6]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[6] = xTaskGetTickCountFromISR();
        GPIO_inputs[6].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada08()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[7]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[7] = xTaskGetTickCountFromISR();
        GPIO_inputs[7].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada09()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[8]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[8] = xTaskGetTickCountFromISR();
        GPIO_inputs[8].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada10()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[9]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[9] = xTaskGetTickCountFromISR();
        GPIO_inputs[9].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada11()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[10]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[10] = xTaskGetTickCountFromISR();
        GPIO_inputs[10].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada12()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[11]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[11] = xTaskGetTickCountFromISR();
        GPIO_inputs[11].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}
void IRAM_ATTR interruptEntrada13()
{
    if ((xTaskGetTickCountFromISR() - filtroISR[12]) >= DELAY_FILTRO_ISR)
    {
        filtroISR[12] = xTaskGetTickCountFromISR();
        GPIO_inputs[12].qtd++;
        FAST_timeoutPulso = 0;
        timerAlarmEnable(timer);
        digitalWrite(PIN_LED_PULSO, HIGH);
    }
}