/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
  Example based on the Universal Arduino Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/blob/master/examples/ESP8266/FlashLED/FlashLED.ino
*/
#include "main.h"

// Available commands
String cmdStart = "/start"; // Start command, does same as /help
String cmdHelp = "/help";   // Send help message
String cmdMenu = "/menu";   // Send control menu

X509List cert(TELEGRAM_CERTIFICATE_ROOT);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int longPoll = 60;              // Wait N seconds for response
unsigned long botRequestDelay = 1000; // Checks for new messages every N ms
unsigned long lastCheck;

// In case the connection is lost we will try to reconnect
unsigned long wiFiReconnectInterval = 1000; // ms
unsigned long previousMillis = 0;           // Used to hold data for reconnection routine

// Test user name is in trusted user names
bool isTrusted(String username)
{
  if (username.isEmpty())
    return false;

  for (unsigned long i = 0; i < sizeof(trustedUsers); i++)
  {
    if (trustedUsers[i] == username)
    {
      return true;
    }
  }

  return false;
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    // Chat id of the requester
    String fromUserId = String(bot.messages[i].from_id);
    String fromUserName = String(bot.messages[i].from_name);
    String chatId = String(bot.messages[i].chat_id);

    Serial.println("Message from " + fromUserName);

    if (!isTrusted(fromUserId))
    {
      bot.sendMessage(chatId, "Access denied 🔒", "");
      continue;
    }

    String text = bot.messages[i].text;

    // Handle buttons
    if (bot.messages[i].type == "callback_query")
    {
      Serial.println("Data on the button: " + text);

      Serial.println("Set pin " + text + " high");
      digitalWrite(text.toInt(), HIGH);
      digitalWrite(LED_BUILTIN, LOW);
      delay(holdHigh);
      Serial.println("Set pin " + text + " low");
      digitalWrite(text.toInt(), LOW);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
      digitalWrite(LED_BUILTIN, HIGH);

      bot.sendMessage(bot.messages[i].from_id, "Success! 🚀", "");
    }
    // Handle regular commands
    else
    {
      Serial.println("Raw text: " + text);

      if (text.startsWith(cmdHelp, 0) || text.startsWith(cmdStart, 0))
      {
        String reply = "Welcome, " + fromUserName + ".\n";
        reply += "The following commands are available for you!\n\n";
        reply += cmdHelp + " - send this help message.\n";
        reply += cmdMenu + " - send main control block.";
        bot.sendMessage(chatId, reply, "");
        continue;
      }

      if (text.startsWith(cmdMenu, 0))
      {
        if (sizeof(controlPins))
        {
          String control = "";
          for (unsigned long i = 0; i < sizeof(controlPins); i++)
          {
            control += "[{ \"text\" : \"" + controlLabels[i] + "\", \"callback_data\" : " + controlPins[i] + "}]";
            if (i != sizeof(controlPins) - 1) {
              control += ",";
            }
          }
          control = "[" + control + "]";
          bot.sendMessageWithInlineKeyboard(chatId, "Available controls 🔐", "", control);
        }
        else
        {
          bot.sendMessage(chatId, "Controls are not defined ⚠️", "");
        }
        continue;
      }
    }
  }
}

void setup()
{
  // Setup serial connection (log output to console)
  Serial.begin(115200);
  Serial.println("Configuration started");

  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  client.setTrustAnchors(&cert);    // Add root certificate for api.telegram.org

  pinMode(LED_BUILTIN, OUTPUT);
  delay(10);
  digitalWrite(LED_BUILTIN, HIGH); // initialize pin as off (active LOW)

  // Configure ports as output
  for (unsigned long i = 0; i < sizeof(controlPins); i++)
  {
    pinMode(controlPins[i], OUTPUT);
  }

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.hostname(PROJECT_NAME);
  WiFi.setAutoReconnect(true);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi...");
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }

  bot.longPoll = 60;

  Serial.println("Configuration finished");
}

void loop()
{
  if (!WiFi.isConnected())
  {
    Serial.println("Reconnecting...");
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  else if (millis() - lastCheck > botRequestDelay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("Handle message");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastCheck = millis();
  }
}
