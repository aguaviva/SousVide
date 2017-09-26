#include <ESPAsyncTCP.h>
#include <SyncClient.h>
#include "Logger.h"
#include "config.h"

SyncClient client;

String LoggerUrl = LOGGER_URL;
String LoggerResult = "Uninitialized";
unsigned long LoggerDelay = 0;

size_t loggerGetVars(char *out)
{
    int p = 0;
    p += sprintf ( &out[p], ",\"sLoggerUrl\":\"%s\"", LoggerUrl.c_str());
    p += sprintf ( &out[p], ",\"iLoggerDelay\":\"%i\"", LoggerDelay/1000);
    p += sprintf ( &out[p], ",\"sLoggerResult\":\"%s\"", LoggerResult.c_str());
    return p;
}

bool loggerSetVars(String name, String val)
{
    if (name == "sLoggerUrl")
    {
        LoggerUrl = val;
        return true;
    }
    else if (name == "iLoggerDelay")
    {
        LoggerDelay = val.toInt()*1000;
        return true;
    }

    return false;
}

enum LOGGERSTATE
{
    LOGGERSTATE_LoggerIdle = 0,
    LOGGERSTATE_LoggerWaitingForSeparator,
    LOGGERSTATE_LoggerGetBody
};

static LOGGERSTATE LoggerState = LOGGERSTATE_LoggerIdle;
static unsigned long loggerOldTime = 0;


void loggerUpdate(float Input, int Output)
{
    if (LoggerDelay == 0)
        return;

    if (loggerOldTime == 0)
        loggerOldTime = millis();

    if (LoggerState == LOGGERSTATE_LoggerIdle)
    {
        unsigned long curr = millis();
        if (curr > LoggerDelay + loggerOldTime)
        {
            loggerOldTime = curr;

            String Host;
            String Url;

            {
                int slash = LoggerUrl.indexOf('/');
                Host = LoggerUrl.substring(0, slash);
                Url = LoggerUrl.substring(slash);
            }

            const int httpPort = 80;
            if (client.connect(Host.c_str(), httpPort))
            {
                int c1 = Url.indexOf('%');
                int c2 = Url.indexOf('%', c1 + 1);

                char out[512];
                strcpy(out, "GET ");
                if (c1 >= 0)
                {
                    strcat(out, Url.substring(0, c1).c_str());
                    dtostrf((double)Input, 0, 6, &out[strlen(out)]);
                    if (c2 >= 0)
                    {
                        strcat(out, Url.substring(c1 + 1, c2).c_str());
                        dtostrf((double)Output, 0, 6, &out[strlen(out)]);
                        strcat(out, Url.substring(c2 + 1).c_str());
                    }
                    else
                    {
                        strcat(out, Url.substring(c1 + 1).c_str());
                    }
                }

                strcat(out, " HTTP/1.1\r\n");
                strcat(out, "Host: ");
                strcat(out, Host.c_str());
                strcat(out, "\r\n");
                strcat(out, "Connection: close\r\n\r\n");

                client.printf(out);

                LoggerState = LOGGERSTATE_LoggerWaitingForSeparator;
                LoggerResult = "Expecting RES";
            }
            else
            {
                LoggerResult = "cannot connect";
            }
        }
    }

    if (LoggerState == LOGGERSTATE_LoggerWaitingForSeparator)
    {
        static int cnt = 0;
        char str[] = "\r\n\r\n";

        int len = client.available();
        for (int i = 0; i < len; i++)
        {
            char c = client.read();
            Serial.print(c);
            if (c == str[cnt])
            {
                cnt++;
                if (cnt == 4)
                {
                    LoggerState = LOGGERSTATE_LoggerGetBody;
                    cnt = 0;
                    break;
                }
            }
            else
                cnt = 0;
        }
    }

    if (LoggerState == LOGGERSTATE_LoggerGetBody)
    {
        if (client.available() > 0)
        {
            LoggerResult = client.readString();
            Serial.print(">");
            Serial.print(LoggerResult.c_str());
            Serial.print("<");
            LoggerState = LOGGERSTATE_LoggerIdle;
        }
    }
}
