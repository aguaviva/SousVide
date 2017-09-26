#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "TemperatureSensor.h"
#include "config.h"

static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature sensors(&oneWire);

static unsigned long delayInMillis = 0;
static unsigned long lastTempRequest = 0;

void temperatureSensorInit()
{
    //set resolution to 12 bits;
    int resolution = 12;

    Serial.println ( "Temp sensor..." );

    sensors.begin();
    sensors.setResolution(resolution);
    int sensorCount = sensors.getDeviceCount();

    Serial.printf("found: %i\n", sensorCount);

    if (sensorCount > 0)
    {
        sensors.requestTemperatures();
        sensors.setWaitForConversion(false);

        delayInMillis = 750 / (1 << (12 - resolution));

        //wait until temp sensor is ready
        double Input;
        while (temperatureGetReading(&Input) == false)
        {
            yield();
        }
    }
}

bool temperatureGetReading(double *t0)
{
    unsigned long currTime = millis();

    if (lastTempRequest==0)
        lastTempRequest = currTime;

    if ( (currTime - lastTempRequest) >= delayInMillis) // waited long enough??
    {
        float t = sensors.getTempCByIndex(0);
        if (t > 0)
        {
            *t0 = t;
        }
        else
        {
            //on error report a huge temp so heater gets disconnected.
            *t0 = 1000;
        }
        sensors.requestTemperatures(); // prime the pump for the next one - but don't wait
        lastTempRequest = currTime;
        return true;
    }

    return false;
}
