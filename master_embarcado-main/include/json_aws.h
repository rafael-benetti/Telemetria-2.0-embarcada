
#ifndef JSON_AWS_H
#define JSON_AWS_H
void JSON_getJson(PinInput_t data, char * buffer);
void JSON_getJsonResponse(String action, bool response, char *buffer);
void JSON_getJsonUpdate(char * buffer);
void JSON_getRelayConfirmation(const char *command, const char *status, uint32_t durationMs, char *buffer);

#endif
