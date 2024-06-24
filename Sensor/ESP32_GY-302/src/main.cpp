#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <Ticker.h>

#include "PubSubClient.h"
#include "Config.h"
#include "BH1750.h"
#include "ArduinoJson.h"

#define uS_TO_S_FACTOR 1000000

const char *ssid = "foo_bar";
const char *password = "FOOBAR";

const long gmtOffset = 8 * 60 * 60;
const char *ntpServer = "ntp.aliyun.com";

const char *mqttBroker = "192.168.50.10";
const int mqttPort = 1883;
const char *publishTopic = "home/balcony/light";
const char *subscribeTopic = "sensor/{mac}/config";
const char *mqttUsername = "foo_bar";
const char *mqttPassword = "FOOBAR";

WiFiClient espClient;
PubSubClient client(espClient);
Config config("light_sensor");
BH1750 lightMeter;
Ticker publishTicker;

void connectToWiFi()
{
  Serial.println("Connecting to WiFi: " + String(ssid));
  WiFi.mode(WIFI_STA);
  String mac = WiFi.macAddress();
  Serial.println("Mac Address: " + mac);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Attempting WiFi connection...");
    delay(500);
  }
  Serial.println("WiFi connected");
}

void connectToMQTT()
{
  String clientId = "sensor-light-";
  String mac = WiFi.macAddress();
  clientId += mac;
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(clientId.c_str(), mqttUsername, mqttPassword))
    {
      Serial.println("MQTT Broker connected");
      String topic = String(subscribeTopic);
      topic.replace("{mac}", WiFi.macAddress());
      client.subscribe(topic.c_str());
    }
    else
    {
      Serial.print("Failed to connect to MQTT. Error code: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

float readLightLevel()
{
  if (!lightMeter.measurementReady(true))
  {
    yield();
  }
  return lightMeter.readLightLevel();
}

void deepSleep(uint64_t seconds)
{
  Serial.println("Entering deep sleep...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void publishLux()
{
  digitalWrite(2, HIGH);
  unsigned long startMillis = millis();
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    deepSleep(30);
    return;
  }
  time_t timestamp = mktime(&timeinfo);
  float lux = readLightLevel();
  JsonDocument doc;
  doc["mac"] = WiFi.macAddress();
  doc["lux"] = round(lux);
  doc["timestamp"] = timestamp;
  String json;
  serializeJson(doc, json);
  Serial.println("Data: " + json);
  if (!client.publish(publishTopic, json.c_str()))
  {
    Serial.println("Failed to publish message");
  }
  unsigned long endMillis = millis();
  unsigned long millsSpend = endMillis - startMillis;
  if (millsSpend < 200)
  {
    delay(200 - millsSpend);
  }
  digitalWrite(2, LOW);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  digitalWrite(2, HIGH);
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.println("Message: " + message);
  digitalWrite(2, LOW);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error)
  {
    Serial.print("Failed to parse configuration: ");
    Serial.println(error.c_str());
    return;
  }

  if (doc.containsKey("sleep"))
  {
    uint64_t sleepTime = doc["sleep"].as<uint64_t>();
    ;
    Serial.print("Setting sleep time to: ");
    Serial.println(sleepTime);
    deepSleep(sleepTime);
  }

  if (doc.containsKey("publish_interval"))
  {
    int publishInterval = doc["publish_interval"].as<int>();
    Serial.print("Reset publish interval to: ");
    Serial.println(publishInterval);
    publishTicker.detach();
    publishTicker.attach(publishInterval, publishLux);
    config.setPublishInterval(publishInterval);
  }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  pinMode(2, OUTPUT);

  connectToWiFi();
  configTime(gmtOffset, 0, ntpServer);
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);
  connectToMQTT();
  if (lightMeter.begin())
  {
    Serial.println("BH1750 initialized");
  }
  else
  {
    Serial.println("Failed to initialize BH1750");
  }
  publishTicker.attach(config.getPublishInterval(), publishLux);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectToWiFi();
  }

  if (!client.connected())
  {
    connectToMQTT();
  }

  client.loop();
}
