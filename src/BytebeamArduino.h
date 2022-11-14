#ifndef BYTEBEAM_ARDUINO_H
#define BYTEBEAM_ARDUINO_H

#include <FS.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>


#define DEVICE_CONFIG_STR_LENGTH 8192  // maximum size of json object in bytes

class Bytebeam : private PubSubClient {
public:
    // contructor
    Bytebeam();

    // destructor
    ~Bytebeam();

    // public functions
    boolean begin();
    void setSubscribeCallback(MQTT_CALLBACK_SIGNATURE);
    boolean publish(const char* topic, const char* payload);
    boolean subscribe(const char* topic);
    boolean unsubscribe(const char* topic);
    boolean loop();
    boolean connected();
    void end();

private:
    // private functions 
    void readDeviceConfigFile();
    void parseDeviceConfigFile();

    // private variables
    char deviceConfigStr[DEVICE_CONFIG_STR_LENGTH];
    StaticJsonDocument<DEVICE_CONFIG_STR_LENGTH> deviceConfigJson;

    uint16_t mqttPort;
    const char* mqttBrokerUrl; 
    const char* deviceId;
    const char* projectId;

    const char* caCertPem;
    const char* clientCertPem;
    const char* clientKeyPem;
  
    WiFiClientSecure secureClient;
};

#endif /* BYTEBEAM_ARDUINO_H */ 
