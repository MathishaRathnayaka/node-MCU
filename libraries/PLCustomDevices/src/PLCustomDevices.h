//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#ifndef JSON_BUFFER
#define JSON_BUFFER 400
#endif

#ifndef PLCustomDevices_h
#define PLCustomDevices_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#if defined(ESP8266) || defined(ESP32)

#include <functional>

#define CUSTOM_CALLBACK_SIGNATURE std::function<void(char *, uint8_t *, unsigned int)> customCallback
#define CUSTOM_CALLBACK_SIGNATURE_JSON std::function<void( JsonDocument& )> customCallback_json
#else
#define CUSTOM_CALLBACK_SIGNATURE void (*customCallback)(char *, uint8_t *, unsigned int)
#define CUSTOM_CALLBACK_SIGNATURE_JSON void (*customCallback_json)( JsonDocument&)
#endif


class PLCustomDevices {
private:
    CUSTOM_CALLBACK_SIGNATURE;
    CUSTOM_CALLBACK_SIGNATURE_JSON;
    char responseBuffer[JSON_BUFFER];
    const char *host;
    const uint16_t *port;
    const char *userID;
    const char *clientID;
    const char *username;
    const char *pass;
    WiFiClient *wifiClient = NULL;
    PubSubClient *mqttClient = NULL;

    void callback(char *topic, uint8_t *message, uint16_t length);

    bool noAutoResponse;

public:
    PLCustomDevices(WiFiClient *wifiClient, const char *mqttHost, const uint16_t *port);

    void setUserID(const char *userID);

    void setAuthInfos(const char *deviceID, const char *mqttUsername, const char *mqttPass);

    void init();

    void connect(bool waitUntilConnect);

    void loop();

    bool sendSyncReq();

    bool commandAvailable = false;
    StaticJsonDocument<JSON_BUFFER> doc;

    PLCustomDevices &setMqttCallback(CUSTOM_CALLBACK_SIGNATURE, bool noResponse = false);

    PLCustomDevices &setJSONCallback(CUSTOM_CALLBACK_SIGNATURE_JSON, bool noResponse = false);

    bool sendResponse();

    bool mqttConnected();

    bool mqttDisconnect();

    ~PLCustomDevices();
};

#endif