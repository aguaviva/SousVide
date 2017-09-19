#include <Arduino.h>

class PWM
{
  unsigned long freq = 0;
  unsigned long graphStartTime = 0;
public:
  PWM(unsigned long f)
  {
    freq = f;
  }

  bool tick()
  {
    if (graphStartTime==0)
      graphStartTime = millis();

    if ((millis() - graphStartTime) > freq)
    { //time to shift the update Window
      graphStartTime += freq;
      return true;
    }

    return false;
  }
};
