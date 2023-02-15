#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "secrets.h"

#define PROJECT_NAME "Master of keys"

// Define array of trusted telegram user ids. User id could be gained from this bot https://t.me/RawDataBot
// Send any message to bot and copy chat id.
const String trustedUsers[] = {"621060119"};

// Define control mapping, pin and it's label
const uint8_t controlPins[] = {D0};
const String controlLabels[] = {"Front door"};
const uint16_t holdHigh = 1000; // Hold pin high for N ms
