#include <Arduino.h>
#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.
#include "MqttPlantHandler.h"

// -- Pins
uint8_t DATA_PIN = A0;
uint8_t RESET_PIN = D1;
uint8_t LED_PIN = 5;

// -- IotWebConf things
String chipID(ESP.getChipId(), HEX);
String thingName = ("SmartPlant" + chipID);
const char wifiInitialApPassword[] = "smart8266";

DNSServer dnsServer;
WebServer server(80);
PlantMqtt plantmqtt;

iotwebconf::WifiAuthInfo wifiAuthInfo;
char mqttServer[64];
char mqttUserName[32];
char mqttUserPassword[32];
char mqttTopicPrefix[32];

IotWebConf iotWebConf(thingName.c_str(), &dnsServer, &server, wifiInitialApPassword);
iotwebconf::ParameterGroup mqttgroup = iotwebconf::ParameterGroup("mqttgroup", "MQTT parameters");
iotwebconf::TextParameter mqttServerParam = iotwebconf::TextParameter("MQTT server", "mqttServer", mqttServer, sizeof(mqttServer));
iotwebconf::TextParameter mqttUserNameParam = iotwebconf::TextParameter("MQTT username", "mqttUser", mqttUserName, sizeof(mqttUserName));
iotwebconf::PasswordParameter mqttUserPasswordParam = iotwebconf::PasswordParameter("MQTT password", "mqttPass", mqttUserPassword, sizeof(mqttUserPassword), "password");
iotwebconf::TextParameter mqttTopicParam = iotwebconf::TextParameter("MQTT Topic", "mqttTopic (without "
                                                                                   "/"
                                                                                   " at the end)",
                                                                     mqttTopicPrefix, sizeof(mqttTopicPrefix));

void onWifiConnected()
{
  if (!plantmqtt.setup(mqttServer, 1883, chipID, mqttTopicPrefix, mqttUserName, mqttUserPassword, mqttTopicPrefix, true, "BATTERY DIED"))
  {
    Serial.println("[MQTT] Server connection failed!");
  }
  digitalWrite(LED_BUILTIN, LOW);
}

iotwebconf::WifiAuthInfo *onWifiFailed()
{
  WiFi.forceSleepWake();
  WiFi.persistent(false);
  wifiAuthInfo = iotWebConf.getWifiAuthInfo();
  return &wifiAuthInfo;
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 01 Minimal</title></head><body>";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  }
  pinMode(LED_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);
  pinMode(RESET_PIN, INPUT);
  iotWebConf.blink(500, 100);
  Serial.println();
  Serial.println("Starting up...");
  Serial.print("[iot] Thing Name:");
  Serial.println(iotWebConf.getThingName());
  WiFi.forceSleepWake();
  WiFi.persistent(false);

  // -- Initializing the configuration.
  // iotWebConf.setConfigPin(RESET_PIN);
  iotWebConf.skipApStartup();
  iotWebConf.setWifiConnectionTimeoutMs(10e3);

  mqttgroup.addItem(&mqttServerParam);
  mqttgroup.addItem(&mqttUserNameParam);
  mqttgroup.addItem(&mqttUserPasswordParam);
  mqttgroup.addItem(&mqttTopicParam);
  iotWebConf.addParameterGroup(&mqttgroup);
  iotWebConf.setWifiConnectionCallback(&onWifiConnected);
  iotWebConf.setWifiConnectionFailedHandler(&onWifiFailed);

  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []
            { iotWebConf.handleConfig(); });
  server.onNotFound([]()
                    { iotWebConf.handleNotFound(); });

  Serial.println("Ready.");
  Serial.println("[iot] ... Waiting for Connection ...");
}
void loop()
{
  iotWebConf.doLoop();
  if (plantmqtt.mqtt.connected())
  {
    Serial.println("[MQTT] Sending Message" + String(millis()));
    digitalWrite(LED_PIN, HIGH);
    plantmqtt.mqtt.loop();
    int humidity = analogRead(DATA_PIN);
    plantmqtt.publishPlantState(humidity);
    Serial.println("[MQTT] Done going to sleep");
    digitalWrite(LED_PIN, LOW);
    delay(0);
    ESP.deepSleep(60e6, WAKE_RF_DEFAULT);
  }
}
