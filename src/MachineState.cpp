#include "PID.h"
#include "helpers/Timer.h"

enum CookingStatus
{
  ST_Stopped = 0,
  ST_Start = 1,
  ST_Preheating = 2,
  ST_Cooking = 3,
  ST_Finished = 4
} statusCurrent = ST_Stopped;

const char static *StatusMessages[] = { "Stopped", "Start", "Preheating", "Cooking", "Finished" };

Timer timer;

//////////////////////////////////////////////////////////
static bool statusDirty = true;

static void SetStatus(CookingStatus statusNew)
{
  if (statusNew == statusCurrent)
    return;

    statusDirty = true;

  if (statusNew == ST_Stopped)
    pidDoManual(0);

  statusCurrent = statusNew;
}

size_t machineStateGetVars(char *out)
{
  char *ptr = out;
  ptr += sprintf ( ptr, ",\"tTimeDelay\":%i", timer.GetTimeOut() / 1000);
  return ptr-out;
}

size_t machineStateGetConsts(char *out)
{
  return sprintf ( out, "\"iStatus\":%i,\"sStatusMsg\":\"%s\",\"tTimeLeft\":%i", statusCurrent, StatusMessages[statusCurrent], timer.GetTimeLeft() / 1000);
}

bool machineStateGetStatusJSON(char *out)
{
  if (statusDirty == false)
    return false;

  out += sprintf ( out, "{\"const\":{");
  out += machineStateGetConsts(out);
  out += sprintf ( out, "}}");

  statusDirty = false;
  return true;
}

bool machineStateSetVars(String name, String val)
{
  if (name == "tTimeDelay")
  {
    timer.SetTimeOut(((unsigned long)val.toInt()) * 1000);
    SetStatus(val.toInt() > 0 ? ST_Start : ST_Stopped);
    return true;
  }

  return false;
}

double machineStateUpdate(double Input)
{
  if (statusCurrent == ST_Start)
  {
    timer.Stop();
    SetStatus(ST_Preheating);
    pidDoManual(255);
  }

  if (statusCurrent == ST_Preheating)
  {
    if (Input >=  pidGetSetPoint())
    {
      timer.Start();
      SetStatus(ST_Cooking);
    }
    else if (Input >=  pidGetSetPoint()-2) //ig getting close by 2 degrees to target temp switch to PDI
    {
      pidDoAutomatic();
    }
  }

  if (statusCurrent == ST_Cooking)
  {
    if (timer.GetStatus() == Timer::TIMER_TimedOut)
    {
      SetStatus(ST_Finished);
      pidDoManual(0);
      timer.Stop();
    }
    else
    {
      pidDoAutomatic();
    }
  }

  return pidGetOutput();
}
