#include <Arduino.h>
#include "config.h"
#include "Relay.h"

extern "C"
{
#include "user_interface.h"
}

class pwm
{
int m_oc;
int m_pin;
int m_pinState;
unsigned long m_timeOn;

void SetState(int state)
{
    if (state==HIGH)
        m_timeOn++;

    if (state!=m_pinState)
    {
        digitalWrite(m_pin, state);
        m_pinState = state;
    }
}

public:
pwm(int pin)
{
    m_oc = 0;
    m_pin = pin;
    m_pinState = LOW;
    digitalWrite(m_pin, m_pinState);
    pinMode(m_pin, OUTPUT);
}

// updates the phase correct long term PWM
//
void update(int t)
{
    int c = m_oc & 1;

    int pwm = t % (RELAY_PWM_PERIOD*2);

    if (pwm>=RELAY_PWM_PERIOD)
    {
        pwm = ((RELAY_PWM_PERIOD*2) - 1) - pwm - c;
    }

    SetState((m_oc/2)>pwm);
}

void SetOC(int oc)
{
    m_oc = oc;
}

int GetTimeOnSecs()
{
    return (m_timeOn * RELAY_PWM_CLOCK)/1000;
}
};

static pwm relay(RELAY_PIN);

LOCAL void ICACHE_FLASH_ATTR heartbeat_cb (void * arg)
{
    static unsigned int counter = 0;
    relay.update(counter);
    counter++;
}

static os_timer_t heartbeat_timer = {0};

//////////////////////////////////////////////////////////////////
// Inits the PWM relay
//////////////////////////////////////////////////////////////////
void relayInit()
{
    os_timer_disarm (&heartbeat_timer);
    os_timer_setfn (&heartbeat_timer, (os_timer_func_t *)heartbeat_cb, (void *) 0);
    os_timer_arm (&heartbeat_timer, RELAY_PWM_CLOCK, true);
}

//////////////////////////////////////////////////////////////////
// Sets the PWM rate
//////////////////////////////////////////////////////////////////
void relaySetFreq(double rate)
{
    relay.SetOC((((double)(RELAY_PWM_PERIOD*2+1))*rate)/255);
}

//////////////////////////////////////////////////////////////////
// Gets the number of seconds the relay has been on (this helps compute the consumption)
//////////////////////////////////////////////////////////////////
unsigned long relayGetTimeOn()
{
    return relay.GetTimeOnSecs();
}
