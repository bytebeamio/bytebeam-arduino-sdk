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

/* This macro is used to specify the maximum number of actions that need to be handled for particular device */
#define BYTEBEAM_NUMBER_OF_ACTIONS 10 

/**
 * @struct actionFunctionsHandler
 * This sturct contains name and function pointer for particular action 
 * @var actionFunctionsHandler::name
 * Name of particular action 
 * @var actionFunctionsHandler::func
 * Pointer to action handler function for particular action
 */
typedef struct {
    const char* name;
    int (*func)(char* args, char* actionId);
} actionFunctionsHandler;

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

    /* bytebeam action handling api's*/
    boolean handleActions(char* actionReceivedStr);
    boolean addActionHandler(int (*func_ptr)(char* args, char* actionId), char* func_name);

    void end();

private:
    // private functions 
    boolean readDeviceConfigFile();
    boolean parseDeviceConfigFile();
    void initActionHandlerArray();
    boolean subscribeToActions();

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
    
    actionFunctionsHandler actionFuncs[BYTEBEAM_NUMBER_OF_ACTIONS];

    WiFiClientSecure secureClient;
};

extern BytebeamArduino Bytebeam;

#endif /* BYTEBEAM_ARDUINO_H */ 
