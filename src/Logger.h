#include <Arduino.h>

size_t loggerGetVars(char *out);
bool loggerSetVars(String name, String val);
void loggerUpdate(float Input, int Output);
