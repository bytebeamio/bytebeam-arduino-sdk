#ifndef BYTEBEAM_ARDUINO_H
#define BYTEBEAM_ARDUINO_H

#include <time.h>
#include <SPIFFS.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "BytebeamOTA.h"

/* This macro is used to debug the library, we will keep all the unnecessary print under this macro */
#define DEBUG_BYTEBEAM_ARDUINO false

/* This macro is used to specify the maximum size of device config json in bytes that need to be handled for particular device */
#define DEVICE_CONFIG_STR_LENGTH 8192

/* This macro is used to specify the maximum number of actions that need to be handled for particular device */
#define BYTEBEAM_CONNECT_MAX_RETRIES 5

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
    boolean handleActions(char* actionReceivedStr);
    boolean addActionHandler(int (*funcPtr)(char* args, char* actionId), char* actionName);
    boolean removeActionHandler(char* actionName);
    boolean updateActionHandler(int (*newFuncPtr)(char* args, char* actionId), char* actionName);
    void printActionHandlerArray();
    boolean publishActionCompleted(char* actionId);
    boolean publishActionFailed(char* actionId);
    boolean publishActionProgress(char* actionId, int progressPercentage);
    boolean publishToStream(char* streamName, const char* payload);
    boolean handleOTA();
    void end();
    
private:
    // private functions 
    void initActionHandlerArray();
    boolean readDeviceConfigFile();
    boolean parseDeviceConfigFile();
    boolean setupBytebeamClient();
    boolean subscribe(const char* topic);
    boolean unsubscribe(const char* topic);
    boolean publish(const char* topic, const char* payload);
    boolean subscribeToActions();
    boolean unsubscribeToActions();
    boolean publishActionStatus(char* actionId, int progressPercentage, char* status, char* error);

    // private variables
    int mqttPort;
    const char* mqttBrokerUrl; 
    const char* deviceId;
    const char* projectId;
    const char* caCertPem;
    const char* clientCertPem;
    const char* clientKeyPem;
    WiFiClientSecure secureClient;
    int actionFuncsHandlerIdx;
    actionFunctionsHandler actionFuncs[BYTEBEAM_NUMBER_OF_ACTIONS];
    char deviceConfigStr[DEVICE_CONFIG_STR_LENGTH];
    StaticJsonDocument<DEVICE_CONFIG_STR_LENGTH> deviceConfigJson;
};

extern BytebeamArduino Bytebeam;

#endif /* BYTEBEAM_ARDUINO_H */ 
