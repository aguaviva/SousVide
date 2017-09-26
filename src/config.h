//---------------------------------------------
// MNDS name
#define HOST_NAME "sousvide"

//pin where the relay is connected to
#define RELAY_PIN 5

//pin where the thermometer is connected to
#define ONE_WIRE_BUS 14

//passwd for the SPIFFS editor/file manager
#define HTTP_USERNAME "admin"
#define HTTP_PASSWORD "admin"

//this is to enable OTA updates
#define OTA 1

// url of where the temperature and power will get logged.
#define LOGGER_URL "192.168.1.5/log.php?1=%&2=%"

//--------------------------------------------

// no need to touch this
#define HISTORY_SIZE 600
#define RELAY_PWM_PERIOD 32
#define RELAY_PWM_CLOCK 250
