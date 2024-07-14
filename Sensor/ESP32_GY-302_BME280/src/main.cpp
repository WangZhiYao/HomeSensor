#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <Ticker.h>

#include "Config.h"
#include "PubSubClient.h"
#include "BH1750.h"
#include "Adafruit_BME280.h"
#include "ArduinoJson.h"

#define LED_PIN 2

const char *ssid = "ssid";
const char *password = "password";

const long gmtOffset = 28800;
const char *ntpServer = "ntp.aliyun.com";

const char *mqttBroker = "mqttBroker";
const int mqttPort = 1883;
const char *mqttUsername = "mqttUsername";
const char *mqttPassword = "mqttPassword";
const char *balconyTopic = "home/balcony";
const char *lightTopic = "home/balcony/light";

Config config;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

BH1750 bh1750;
Adafruit_BME280 bme280;

Ticker ticker;

char sensorId[13];
unsigned long ledOnTime = 0;
bool ledOn = false;

void toggleLed(bool on);
void generateSensorId();
void connectToWiFi();
void connectToMQTTBroker();
void initBH1750();
void initBME280();
void callback(char *topic, byte *payload, unsigned int length);
void handleConfig(String configJson);
float readLightLevel();
void publishData();

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  pinMode(LED_PIN, OUTPUT);
  toggleLed(true);

  generateSensorId();
  connectToWiFi();
  connectToMQTTBroker();
  configTime(gmtOffset, 0, ntpServer);
  initBH1750();
  initBME280();

  ticker.attach(config.getPublishInterval(), publishData);

  toggleLed(false);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    toggleLed(true);
    connectToWiFi();
  }

  if (!client.connected())
  {
    toggleLed(true);
    connectToMQTTBroker();
  }

  client.loop();
  if (ledOn && (millis() - ledOnTime >= 200))
  {
    toggleLed(false);
  }
}

void toggleLed(bool on)
{
  if (on)
  {
    digitalWrite(LED_PIN, HIGH);
    ledOnTime = millis();
    ledOn = true;
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
    ledOn = false;
  }
}

void generateSensorId()
{
  uint64_t mac = ESP.getEfuseMac();
  sprintf(sensorId, "%012llx", mac);
}

void connectToWiFi()
{
  Serial.println("Connecting to WiFi: " + String(ssid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected");
}

void connectToMQTTBroker()
{
  Serial.println("Attempting MQTT connection...");
  client.setServer(mqttBroker, mqttPort)
      .setCallback(callback);
  String id = "sensor_" + String(sensorId);
  while (!client.connect(id.c_str(), mqttUsername, mqttPassword))
  {
    Serial.print("Failed to connect to MQTT. Error code: ");
    Serial.println(client.state());
    delay(500);
  }
  Serial.println("MQTT Broker connected");
  String configTopic = String("sensor/{sensorId}/config");
  configTopic.replace("{sensorId}", sensorId);
  client.subscribe(configTopic.c_str());
  Serial.println("Subscribe topic: " + configTopic);
}

void initBH1750()
{
  Serial.println("Iinitilizing BH1750");
  while (!bh1750.begin())
  {
    Serial.println("Failed to initilize BH1750");
    delay(500);
  }
  Serial.println("Iinitilize BH1750 success");
}

void initBME280()
{
  Serial.println("Iinitilizing BME280");
  while (!bme280.begin(BME280_ADDRESS_ALTERNATE))
  {
    Serial.println("Failed to initilize BME280");
    delay(500);
  }
  Serial.println("Iinitilize BME280 success");
}

void callback(char *topic, byte *payload, unsigned int length)
{
  toggleLed(true);
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.println("Message: \n" + message);
  handleConfig(message);
}

void handleConfig(String configJson)
{
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configJson);
  if (error)
  {
    Serial.print("Failed to parse configuration: ");
    Serial.println(error.c_str());
    return;
  }

  config.updateConfig(doc);

  if (doc.containsKey(KEY_PUBLISH_INTERVAL))
  {
    int pubInterval = doc[KEY_PUBLISH_INTERVAL].as<int>();
    ticker.detach();
    ticker.attach(pubInterval, publishData);
  }
}

float readLightLevel()
{
  if (!bh1750.measurementReady(true))
  {
    yield();
  }
  return bh1750.readLightLevel();
}

void publishData()
{
  if (WiFi.status() != WL_CONNECTED || !client.connected())
  {
    return;
  }

  toggleLed(true);
  unsigned long startMillis = millis();
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  time_t timestamp = mktime(&timeinfo);
  float temperature = bme280.readTemperature();
  float pressure = bme280.readPressure() / 100.0F;
  float humidity = bme280.readHumidity();

  JsonDocument doc;
  doc["sensor_id"] = sensorId;
  doc["type"] = "thp";
  JsonObject data = doc["data"].to<JsonObject>();
  data["temperature"] = temperature;
  data["humidity"] = humidity;
  data["pressure"] = pressure;
  doc["timestamp"] = timestamp;

  String json;
  serializeJson(doc, json);
  if (client.publish(balconyTopic, json.c_str()))
  {
    Serial.println("Publish success: [" + String(balconyTopic) + "] data: " + json);
  }
  else
  {
    Serial.println("Publish failed: [" + String(balconyTopic) + "] data: " + json);
  }

  if (config.isLightEnable())
  {
    doc["type"] = "light";
    data.clear();
    float light = readLightLevel();
    data["light"] = round(light);
    serializeJson(doc, json);
    if (client.publish(lightTopic, json.c_str()))
    {
      Serial.println("Publish success: [" + String(lightTopic) + "] data: " + json);
    }
    else
    {
      Serial.println("Publish failed: [" + String(lightTopic) + "] data: " + json);
    }
  }
}
