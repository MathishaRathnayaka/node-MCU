#include <Arduino.h>
#include <PLCustomDevices.h>
#include <ESP8266WiFi.h> // Include the Wi-Fi library

const char *ssid = "YOUR WIFI NETWORK NAME"; // The SSID (name) of the Wi-Fi network you want to connect to
const char *password = "YOUR WIFI PASSWARD"; // The password of the Wi-Fi network

const char *mqttHOST = "mqtt.oguzkagan.xyz";
uint16_t mqttPORT = 1883;
const char *clientID = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"; // The ID (device ID) of your custom device
const char *username = "username";                             // The MQTT username of your custom device
const char *pass = "supersecretpass";                          // The MQTT password of your custom device

WiFiClient myWifiClient;
PLCustomDevices myDevice(&myWifiClient, mqttHOST, &mqttPORT);

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("New Command Available");
  if (strcmp(myDevice.doc["command"], "synk") == 0)
  {
    myDevice.doc.clear();
    myDevice.doc["online"] = true;
    myDevice.doc["on"] = true;
    myDevice.doc["brightness"] = 100;
    myDevice.doc["color"]["spectrumRGB"] = 16711935; //Magenta
    Serial.printf("RESPONSE: %d", myDevice.sendResponse());
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("\nIts ON !!!");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  myDevice.setAuthInfos(clientID, username, pass);
  myDevice.init();
  myDevice.setMqttCallback(callback);
}

void loop()
{
  myDevice.loop();
}