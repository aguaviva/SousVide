#include <Arduino.h>
size_t pidGetVars(char *out);
bool pidSetVars(String name, String val);
void pidDoAutomatic();
void pidDoManual(double output);

int pidGetOutput();
void pidSetInput(float val);
void pidSetOutput(float val);
double pidGetSetPoint();
