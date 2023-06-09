/****************************************************************************************************************************
  WiFiMQTTSecureAWS.ino

  MQTT and MQTT over WebSoket Client for Arduino

  For nRF52, SAMD21, SAMD51, STM32F/L/H/G/WB/MP1, Teensy, SAM DUE, RP2040-based boards, besides ESP8266,
  ESP32 (ESP32, ESP32_S2, ESP32_S3 and ESP32_C3) and WT32_ETH01

  Ethernet shields W5100, W5200, W5500, ENC28J60, Teensy 4.1 NativeEthernet/QNEthernet.

  Based on and modified from MQTTPubSubClient Library (https://github.com/hideakitai/MQTTPubSubClient)

  Built by Khoi Hoang https://github.com/khoih-prog/MQTTPubSubClient_Generic
  Licensed under MIT license
 *****************************************************************************************************************************/

// this example is based on
// https://savjee.be/2019/07/connect-esp32-to-aws-iot-with-arduino-code/

#if !( defined(ESP32) )
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

#if defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
#else
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecure.h>
#endif

#include <MQTTPubSubClient_Generic.h>

#include <ArduinoJson.h>

// The MQTTT endpoint for the device (unique for each AWS account but shared amongst devices within the account)
const char* AWS_IOT_ENDPOINT = "your-endpoint.iot.ap-somewhre.amazonaws.com";

#define AWS_IOT_PORT     8883

// The name of the device MUST match up with the name defined in the AWS console
const char* DEVICE_NAME = "your-device-name";

// Amazon's root CA. This should be the same for everyone.
const char AWS_CERT_CA[] =
  "-----BEGIN CERTIFICATE-----\n"
  "-----END CERTIFICATE-----\n";

// The private key for your device
const char AWS_CERT_PRIVATE[] =
  "-----BEGIN RSA PRIVATE KEY-----\n"
  "-----END RSA PRIVATE KEY-----\n";

// The certificate for your device
const char AWS_CERT_CRT[] =
  "-----BEGIN CERTIFICATE-----\n"
  "-----END CERTIFICATE-----\n";

char ssid[] = "YOUR_SSID";        // your network SSID (name)
char pass[] = "12345678";         // your network password

int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiClientSecure client;
MQTTPubSubClient mqttClient;

// Topic to publish
const char *PubTopic    = "/mqttPubSub";

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  // you're connected now, so print out the data
  Serial.print(F("You're connected to the network, IP = "));
  Serial.println(WiFi.localIP());

  Serial.print(F("SSID: "));
  Serial.print(WiFi.SSID());

  // print the received signal strength:
  int32_t rssi = WiFi.RSSI();
  Serial.print(F(", Signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

void setup()
{
  Serial.begin(115200);

  while (!Serial && millis() < 5000);

  Serial.print(F("\nStart WiFiMQTTSecureAWS on "));
  Serial.println(ARDUINO_BOARD);
  Serial.println(MQTT_PUBSUB_CLIENT_GENERIC_VERSION);

  Serial.print(F("Connecting to SSID: "));
  Serial.println(ssid);

  status = WiFi.begin(ssid, pass);

  delay(1000);

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED)
  {
    delay(500);

    // Connect to WPA/WPA2 network
    status = WiFi.status();
  }

  // you're connected now, so print out the data
  printWifiStatus();

  Serial.print("connecting to host...");
  Serial.print("Connecting to secured-host:port = ");
  Serial.print(AWS_IOT_ENDPOINT);
  Serial.print(":");
  Serial.println(AWS_IOT_PORT);

  // connect to aws endpoint with certificates and keys
  client.setCACert(AWS_CERT_CA);
  client.setCertificate(AWS_CERT_CRT);
  client.setPrivateKey(AWS_CERT_PRIVATE);

  while (!client.connect(AWS_IOT_ENDPOINT, AWS_IOT_PORT))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected!");

  // initialize mqtt client
  mqttClient.begin(client);

  Serial.print("connecting to AWS MQTT broker...");

  while (!mqttClient.connect(DEVICE_NAME))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println(" connected!");

  // subscribe callback which is called when every packet has come
  mqttClient.subscribe([](const String & topic, const String & payload, const size_t size)
  {
    (void) size;

    Serial.println("MQTT received: " + topic + " - " + payload);
  });

  // subscribe topic and callback which is called when /hello has come
  mqttClient.subscribe(PubTopic, [](const String & payload, const size_t size)
  {
    (void) size;

    Serial.print("Subcribed to ");
    Serial.print(PubTopic);
    Serial.print(" => ");
    Serial.println(payload);
  });
}

void loop()
{
  mqttClient.update();

  static uint32_t prev_ms = millis();

  if (millis() > prev_ms + 30000)
  {
    prev_ms = millis();

    StaticJsonDocument<128> doc;
    doc["from"] = "thing";
    char buffer[128];
    serializeJson(doc, buffer);

    mqttClient.publish(PubTopic, buffer);
  }
}
