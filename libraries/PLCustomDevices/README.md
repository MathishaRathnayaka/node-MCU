# PERFECT LIGHTS LIBRARY

**The official library of the [Perfect Lights Custom Devices](https://perfectlights.oguzkagan.xyz/)**

## Requirements

- [PubSubClient](https://github.com/knolleary/pubsubclient)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

# API Documentation

## Library version: 0.1.0

### Constructor

- [PLCustomDevices(client, host, port)](#plcustomdevicesclient-host-port)

### Function

- [void setAuthInfos(deviceID, mqttUsername, mqttPassword)](#void-setauthinfosdeviceid-mqttusername-mqttpassword)
- [void init()](#void-init)
- [void connect(waitUntilConnect)](#void-connectwaituntilconnect)
- [void loop()](#void-loop)
- [bool sendResponse()](#bool-sendresponse)
- [PLCustomDevices *setMqttCallback(callback)](#plcustomdevices-setmqttcallbackcallback)

### Variable

- [bool commandAvailable](#bool-commandavailable)
- [StaticJsonDocument<JSON_BUFFER> doc](#staticjsondocumentjson_buffer-doc)

## PLCustomDevices(client, host, port)

Creates a partially initialised client instance.
Before it can be used, the server details and authentication information must be configured:

```C
const char *mqttHOST = "mqtt.oguzkagan.xyz";
uint16_t mqttPORT = 1883;
const char *clientID = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"; // The ID (device ID) of your custom device
const char *username = "username";                             // The MQTT username of your custom device
const char *pass = "supersecretpass";                          // The MQTT password of your custom device

WiFiClient myWifiClient;
PLCustomDevices myDevice(&myWifiClient, mqttHOST, &mqttPORT);

void setup() {
    // Connect to the network
    myDevice.setAuthInfos(clientID, username, pass); // Set mqtt authentication information
    myDevice.init();                                 // Initialize myDevice
    // myDevice is now ready for use
}
```

### Parameters

- **client** the network client to use, for example `WiFiClient`
- **host** `const char[]` the address of the server
- **port** `uint16_t` the port to connect to

## void setAuthInfos(deviceID, mqttUsername, mqttPassword)

Sets the MQTT server's authentication informations.

### Parameters

- **deviceID** `const char[]` the deviceID to use when connecting to the MQTT server.
- **mqttUsername** `const char[]` the username to use when connecting to the MQTT server.
- **mqttPassword** `const char[]` the password to use when connecting to the MQTT server.

## void init()

Initializes the client

## void connect(waitUntilConnect)

Connects the client to the MQTT server. 

### Parameters

- **waitUntilConnect** `bool` if it sets `true` it will wait until connection established

## void loop()

This should be called regularly to allow the client to process incoming messages and maintain its connection to the server.

## bool sendResponse()

Sends `StaticJsonDocument<JSON_BUFFER> doc` to the MQTT server and sets `bool commandAvailable` to the `false`

### Returns

- `false` publish failed, either connection lost or message too large
- `true` publish succeeded

## PLCustomDevices *setMqttCallback(callback)

Sets the [command callback function](#command-callback-function).

### Parameters

- **callback** `function*` a pointer to a message callback function called when a new command arrived

### Returns

- `PLCustomDevices*` the client instance, allowing the function to be chained

## bool commandAvailable

- If you dont want to use [command callback function](#command-callback-function) you can check this variable in the main loop() function then you can do the things to do in the callback function.

```C
void loop()
{
  myDevice.loop();
  if (myDevice.commandAvailable)
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
      // When you use this method of instead the callback
      // You must call the myDevice.sendResponse() function or set myDevice.commandAvailable to the false
    }
    
  }
}
```

## StaticJsonDocument<JSON_BUFFER> doc

- When new command arrived you can read it from this variable
- Also when you send response you will clear the variable `myDevice.doc.clear()` then set the variable to the response

```C
if (strcmp(myDevice.doc["command"], "synk") == 0) // Read the command
{
  myDevice.doc.clear();                                   // Clear the variable
  myDevice.doc["online"] = true;                          // Set the response
  myDevice.doc["on"] = true;                              // Set the response
  myDevice.doc["brightness"] = 100;                       // Set the response
  myDevice.doc["color"]["spectrumRGB"] = 16711935;        // Set the response
  Serial.printf("RESPONSE: %d", myDevice.sendResponse()); // Send the response
}
```

## Command Callback Function
This function is called when new command arrive.
The callback function has the following signature:
`void callback(char *topic, byte *payload, unsigned int length)`

```C
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
```

### Parameters

- **topic** `const char[]` the topic the command arrived on
- **payload** `byte[]` the command payload
- **length** `unsigned int` the length of the command payload