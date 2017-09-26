#include "PID.h"
#include <PID_v1.h>

static double Input, Output, SetPoint;

PID myPID(&Input, &Output, &SetPoint, 0, 0, 0, DIRECT);

struct TEMPS
{
    float Kp, Ki, Kd;
} Temps = { 50, 0, 0};

bool updatedTuningConstants=true; //so teh default constants get set

size_t AddFloat(const char *var, double val, char *out)
{
    char tmp[20];
    dtostrf((double)val, 0, 6, tmp);

    strcat(out, "\"");
    strcat(out, var);
    strcat(out, "\":");
    strcat( out, String(tmp).c_str());
    return strlen(out);
}

size_t pidGetVars(char *out)
{
    int p = 0;
    p += AddFloat("fSetPoint", SetPoint, &out[p]);
    p += sprintf ( &out[p], ",");
    p += AddFloat("fKp", Temps.Kp, &out[p]);
    p += sprintf ( &out[p], ",");
    p += AddFloat("fKi", Temps.Ki, &out[p]);
    p += sprintf ( &out[p], ",");
    p += AddFloat("fKd", Temps.Kd, &out[p]);
    return p;
}

bool pidSetVars(String name, String val)
{
    float valf = atof(val.c_str());

    if (name == "fSetPoint")
    {
        SetPoint = valf;
        return true;
    }
    if (name == "fKp")
    {
        Temps.Kp = valf;
        updatedTuningConstants = true;
        return true;
    }
    else if (name == "fKi")
    {
        Temps.Ki = valf;
        updatedTuningConstants = true;
        return true;
    }
    else if (name == "fKd")
    {
        Temps.Kd = valf;
        updatedTuningConstants = true;
        return true;
    }

    return false;
}

void pidDoManual(double output)
{
    if (myPID.GetMode() == AUTOMATIC)
    {
        myPID.SetMode(MANUAL);
    }

    Output = output;
}

void pidDoAutomatic()
{
    if (myPID.GetMode() == MANUAL)
    {
        Output = 0; //this resets the integral term
        myPID.SetMode(AUTOMATIC);
        updatedTuningConstants=true;
    }

    if (updatedTuningConstants==true)
    {
        myPID.SetTunings(Temps.Kp, Temps.Ki, Temps.Kd);
        updatedTuningConstants=false;
    }

    myPID.Compute();
}

double pidGetSetPoint() {
    return SetPoint;
}
void pidSetInput(float val) {
    Input = val;
};
int pidGetOutput() {
    return (int)Output;
}
void pidSetOutput(float val) {
    Input = val;
}
