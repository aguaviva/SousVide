#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>

#include "helpers/History.h"
#include "Relay.h"
#include "Logger.h"
#include "PID.h"
#include "helpers/pwm.h"
#include "TemperatureSensor.h"
#include "MachineState.h"
#include "config.h"

//////////////////////////////////////////////////////////////////

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

PWM DataStreamer(1000); //update temperature graph every second

struct STATE
{
  float Input;
  float Output;
};

History<STATE> histData;

///////////////////////////////////////////////////////////////////////////

void websocketsOnConnect(AsyncWebSocketClient * client)
{
    unsigned long OnTimeSecs = millis() / 1000;

    //AsyncWebSocketMessageBuffer *buffer = ws.makeBuffer(2*1024);
    //char *str = (char *)buffer->get();
    char str[2*1024];
    char *ptr = str;

    ptr += sprintf ( ptr, "{ \"vars\": {");
    ptr += pidGetVars(ptr);
    ptr += loggerGetVars(ptr);
    ptr += machineStateGetVars(ptr);
    ptr += sprintf ( ptr, "}, \"const\":{");
    ptr += machineStateGetConsts(ptr);
    ptr += sprintf ( ptr, ",\"tOnTimeSecs\":%u", OnTimeSecs);
    ptr += sprintf ( ptr, ",\"tRelayTimeOnSecs\":%u", relayGetTimeOn());
    ptr += sprintf ( ptr, ",\"histMaxSize\":%i", HISTORY_SIZE);
    ptr += sprintf ( ptr, ",\"histSize\":%i", histData.GetSize());
    ptr += sprintf ( ptr, ",\"GetSize1\":%i", histData.GetSize1());
    ptr += sprintf ( ptr, ",\"GetSize2\":%i", histData.GetSize2());
    ptr += sprintf ( ptr, "}");
    ptr += sprintf ( ptr, "}");
    client->text(str);

    int len1 = histData.GetSize1();
    client->binary( (uint8_t*)histData.GetData(0), len1 * sizeof(STATE));
    int len2 = histData.GetSize2();
    client->binary( (uint8_t*)histData.GetData(len1), len2 * sizeof(STATE));
}

///////////////////////////////////////////////////////////////////////////

void setup ( void )
{
  Serial.begin ( 115200 );
  Serial.setDebugOutput(true);

  // Wait for connection
  Serial.print( "\nConnecting to wifi." );
  WiFi.hostname(HOST_NAME);
  WiFi.begin ( SSID_NAME, SSID_PASSWORD );
  while ( WiFi.status() != WL_CONNECTED )
  {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.printf ( "\nConnected to %s\n", SSID_NAME );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );

  #ifdef OTA
  ArduinoOTA.setHostname(HOST_NAME);
  ArduinoOTA.begin();
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", HOST_NAME);
  #endif

  // Start MSDN
  if ( MDNS.begin ( HOST_NAME ) )
  {
    MDNS.addService("http", "tcp", 80);
    Serial.println ( "MDNS responder started" );
  }

  // Start Spiffs
  {
    SPIFFS.begin();
    server.addHandler(new SPIFFSEditor(HTTP_USERNAME, HTTP_PASSWORD));
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");
    Serial.println ( "SPIFFS started" );
  }

  //start and init winsockets
  ws.onEvent([](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
  {
    if(type == WS_EVT_CONNECT)
    {
      Serial.printf("ws[%s][%u] connected\n", server->url(), client->id());
      client->ping();
      websocketsOnConnect(client);
    }
  });
  server.addHandler(&ws);

  server.on("/update.html", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String name = request->getParam("name")->value();
    String val = request->getParam("val")->value();

    if (pidSetVars(name, val))
    {}
    else if (machineStateSetVars(name, val))
    {}
    else if (loggerSetVars(name, val))
    {}
    else
    {
      request->send( 200, "application/json", "{ \"res\":\"var not found\" }");
      return;
    }

    request->send ( 200, "application/json", "{ \"res\":\"ok\" }" );
  });

  server.onNotFound([](AsyncWebServerRequest *request)
  {
    request->send(404);
  });

  server.begin();
  Serial.println ( "Webserver started " );
  temperatureSensorInit();
  Serial.println ( "sensor started " );
  relayInit();
  Serial.println ( "relay started " );
}

void loop ( void )
{
  //Get sensor data and set it as inut for the PID
  //
  static double Input;
  temperatureGetReading(&Input);
  pidSetInput(Input);

  //log temperature
  //
  loggerUpdate(Input, pidGetOutput());

  // Do the functionality a typical oven does
  //
  double Output = machineStateUpdate(Input);

  //If the status changes notify everyone
  //
  char msg[256];
  if (machineStateGetStatusJSON(msg))
  {
    ws.textAll(msg);
  }

  // do relay low freq pwm
  //
  relaySetFreq(Output);

  // update clients with latest Input(temperatura) and Output
  //
  if (DataStreamer.tick())
  {
    STATE st = { (float)Input, (float)Output };
    histData.Queue(st);

    ws.binaryAll((char*)&st, sizeof(STATE));
  }

  // let other modules update
  MDNS.update();
  ArduinoOTA.handle();
}
