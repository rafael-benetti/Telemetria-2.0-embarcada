#include "includes.h"

static uint8_t relayPin = RELAY_PIN;
static bool relayActiveHigh = RELAY_ACTIVE_HIGH;
static bool relayBusy = false;
static uint32_t relayOffAt = 0;
static uint32_t relayDuration = 0;

static void relayWrite(bool on)
{
  const uint8_t level = (on == relayActiveHigh) ? HIGH : LOW;
  digitalWrite(relayPin, level);
}

static void relaySetupPin(uint8_t pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, relayActiveHigh ? LOW : HIGH);
}

static void relayCancelPulse()
{
  relayBusy = false;
  relayOffAt = 0;
  relayDuration = 0;
}

void RELAY_init()
{
  relaySetupPin(relayPin);
  relayWrite(false);
  DBG_PRINT("Relay pin used: ");
  DBG_PRINTLN(relayPin);
  DBG_PRINT("Relay active high: ");
  DBG_PRINTLN(relayActiveHigh ? "true" : "false");
}

void RELAY_setPin(uint8_t pin)
{
  relayCancelPulse();
  relayWrite(false);
  relayPin = pin;
  relaySetupPin(relayPin);
  relayWrite(false);
  DBG_PRINT("Relay pin used: ");
  DBG_PRINTLN(relayPin);
}

void RELAY_setActiveHigh(bool activeHigh)
{
  relayCancelPulse();
  relayWrite(false);
  relayActiveHigh = activeHigh;
  relaySetupPin(relayPin);
  relayWrite(false);
  DBG_PRINT("Relay active high: ");
  DBG_PRINTLN(relayActiveHigh ? "true" : "false");
}

void RELAY_taskUpdate()
{
  if (!relayBusy)
    return;

  if ((int32_t)(millis() - relayOffAt) >= 0)
  {
    relayWrite(false);
    relayBusy = false;
    DBG_PRINTLN("Relay OFF");
  }
}

void RELAY_requestPulse(uint32_t durationMs, const char *commandName)
{
  RELAY_requestPulseOnPin(relayPin, relayActiveHigh, durationMs, commandName);
}

void RELAY_requestPulseOnPin(uint8_t pin, bool activeHigh, uint32_t durationMs, const char *commandName)
{
  relayPin = pin;
  relayActiveHigh = activeHigh;
  relaySetupPin(relayPin);

  if (durationMs < 100)
  {
    DBG_PRINTLN("Invalid duration, forcing minimum 100 ms");
    durationMs = 100;
  }
  if (durationMs > 5000)
  {
    DBG_PRINTLN("Invalid duration, forcing maximum 5000 ms");
    durationMs = 5000;
  }

  relayDuration = durationMs;
  relayBusy = true;
  relayOffAt = millis() + durationMs;
  DBG_PRINT("MQTT command received: ");
  DBG_PRINTLN(commandName);
  DBG_PRINTLN("Relay ON");
  relayWrite(true);
}

void RELAY_requestStateOnPin(uint8_t pin, bool activeHigh, bool on, const char *commandName)
{
  relayPin = pin;
  relayActiveHigh = activeHigh;
  relaySetupPin(relayPin);
  relayCancelPulse();
  DBG_PRINT("MQTT command received: ");
  DBG_PRINTLN(commandName);
  DBG_PRINT("Relay state command: ");
  DBG_PRINTLN(on ? "ON" : "OFF");
  relayWrite(on);
  DBG_PRINTLN(on ? "Relay ON" : "Relay OFF");
}

void RELAY_scan(uint32_t durationMs)
{
  static const uint8_t scanPins[] = {25, 26, 27, 32, 33, 23, 22, 21, 19, 18, 17, 16};
  DBG_PRINTLN("Relay scan start");
  for (uint8_t pin : scanPins)
  {
    RELAY_setPin(pin);
    DBG_PRINT("Scan pin ");
    DBG_PRINTLN(pin);

    RELAY_setActiveHigh(true);
    relayWrite(true);
    DBG_PRINTLN("Relay ON");
    vTaskDelay(durationMs / portTICK_PERIOD_MS);
    relayWrite(false);
    DBG_PRINTLN("Relay OFF");
    vTaskDelay(250 / portTICK_PERIOD_MS);

    RELAY_setActiveHigh(false);
    relayWrite(true);
    DBG_PRINTLN("Relay ON");
    vTaskDelay(durationMs / portTICK_PERIOD_MS);
    relayWrite(false);
    DBG_PRINTLN("Relay OFF");
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
  RELAY_setActiveHigh(RELAY_ACTIVE_HIGH);
  RELAY_setPin(RELAY_PIN);
  DBG_PRINTLN("Relay scan end");
}

bool RELAY_isBusy()
{
  return relayBusy;
}

uint32_t RELAY_lastDuration()
{
  return relayDuration;
}

const char *RELAY_currentPinName()
{
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%u", relayPin);
  return buffer;
}
