#ifndef FAST_H
#define FAST_H

void FAST_init(void);

void FAST_setFlagCredito(bool flag);

bool FAST_getFlagCredito(void);

extern volatile uint32_t FAST_timeoutPulso;

#endif