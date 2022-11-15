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

/* This macro is used to debug the library, we will keep all the unnecessary print under this macro */
#define DEBUG_BYTEBEAM_ARDUINO false

/* This macro is used to specify the maximum size of device config json in bytes that need to be handled for particular device */
#define DEVICE_CONFIG_STR_LENGTH 8192

class BytebeamArduino : private PubSubClient {
public:
    // contructor
    BytebeamArduino();

    // destructor
    ~BytebeamArduino();

    // public functions
    boolean begin();
    boolean loop();
    boolean connected();

    /* pub sub api's */
    boolean subscribe(const char* topic);
    boolean unsubscribe(const char* topic);
    boolean publish(const char* topic, const char* payload);

    void end();

private:
    // private functions 
    boolean readDeviceConfigFile();
    boolean parseDeviceConfigFile();

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

extern BytebeamArduino Bytebeam;

#endif /* BYTEBEAM_ARDUINO_H */ 
