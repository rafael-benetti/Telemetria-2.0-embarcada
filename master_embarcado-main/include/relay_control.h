#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include "Arduino.h"

#ifndef RELAY_PIN
#define RELAY_PIN 22
#endif

#ifndef RELAY_ACTIVE_HIGH
#define RELAY_ACTIVE_HIGH false
#endif

void RELAY_init();
void RELAY_taskUpdate();
void RELAY_requestPulse(uint32_t durationMs, const char *commandName = "relay");
void RELAY_requestPulseOnPin(uint8_t pin, bool activeHigh, uint32_t durationMs, const char *commandName);
void RELAY_requestStateOnPin(uint8_t pin, bool activeHigh, bool on, const char *commandName);
void RELAY_scan(uint32_t durationMs = 500);
void RELAY_setPin(uint8_t pin);
void RELAY_setActiveHigh(bool activeHigh);
bool RELAY_isBusy();
uint32_t RELAY_lastDuration();
const char *RELAY_currentPinName();

#endif
