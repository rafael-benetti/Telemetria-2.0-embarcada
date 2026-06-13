#include "includes.h"

#ifdef MACHINE_MODEL_RL

#define TIMEOUT_PULSO 10000 // Depois de x tempo vai entrar no laço principal (loop()) para enviar os pulsos MQTT

#else

#define TIMEOUT_PULSO 1000

#endif

hw_timer_t *timer = NULL;

volatile uint32_t FAST_timeoutPulso = 0;
volatile bool FAST_flagCredito = false; // habilita para entrar no laço principal (loop())

void IRAM_ATTR FAST_callback()
{
    FAST_timeoutPulso++;
    if (FAST_timeoutPulso > TIMEOUT_PULSO)
    {
        FAST_flagCredito = true;
        timerAlarmDisable(timer);
    }
}

void FAST_init()
{
    //inicialização do timer. Parametros:
    /* 0 - seleção do timer a ser usado, de 0 a 3.
      240 - prescaler. O clock principal do ESP32 é 240MHz. Dividimos por 240 para ter 1us por tick.
    true - true para contador progressivo, false para regressivo
    */
    timer = timerBegin(0, 240, true);

    /*conecta à interrupção do timer
     - timer é a instância do hw_timer
     - endereço da função a ser chamada pelo timer
     - edge=true gera uma interrupção
    */
    timerAttachInterrupt(timer, &FAST_callback, true);

    /* - o timer instanciado no inicio
       - o valor em us para 1ms
       - auto-reload. true para repetir o alarme
    */
    timerAlarmWrite(timer, 1000, true);

    //ativa o alarme
    timerAlarmEnable(timer);
}
void FAST_setFlagCredito(bool credito)
{
    FAST_flagCredito = credito;
}
bool FAST_getFlagCredito(void)
{
    return FAST_flagCredito;
}