#ifndef BYTEBEAM_ARDUINO_H
#define BYTEBEAM_ARDUINO_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>
#include <PubSubClient.h>
#include "BytebeamTime.h"
#include "BytebeamOTA.h"
#include "BytebeamArchDefines.h"

/**
 * @enum deviceConfigFileSystem
 * This sturct contains name and function pointer for particular action 
 * @var deviceConfigFileSystem::FATFS_FILE_SYSTEM
 * Use FATFS file system for provisioning the device
 * @var deviceConfigFileSystem::SPIFFS_FILE_SYSTEM
 * Use SPIFFS file system for provisioning the device
 */
typedef enum {
    FATFS_FILE_SYSTEM,
    SPIFFS_FILE_SYSTEM,
    LITTLEFS_FILE_SYSTEM,
    SD_FILE_SYSTEM
} deviceConfigFileSystem;

/* This macro is used to debug the library, we will keep all the unnecessary print under this macro */
#define DEBUG_BYTEBEAM_ARDUINO false

/* This macro is used to specify the file system used for provisioning the device */
#define DEVICE_CONFIG_FILE_SYSTEM SPIFFS_FILE_SYSTEM

/* This macro is used to specify the name of the device config file */
#define DEVICE_CONFIG_FILE_NAME "/device_config.json"

/* This macro is used to specify the maximum number of attempts we will perform to reconnect to the server in case the client got disconnect */
#define BYTEBEAM_CONNECT_MAX_RETRIES 5

/* This macro is used to specify the maximum number of actions that need to be handled for particular device */
#define BYTEBEAM_NUMBER_OF_ACTIONS 10 

/* This macro is used to enable the OTA i.e disable this flag to completely remove OTA from compilation phase thereby saving flash size too */
#define BYTEBEAM_OTA_ENABLE true

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
    void resetActionHandlerArray();
    boolean publishActionCompleted(char* actionId);
    boolean publishActionFailed(char* actionId);
    boolean publishActionProgress(char* actionId, int progressPercentage);
    boolean publishToStream(char* streamName, const char* payload);
    
    #if BYTEBEAM_OTA_ENABLE
        boolean enableOTA();
        boolean disableOTA();
    #endif

    void end();
    
private:
    // private functions 
    boolean subscribe(const char* topic);
    boolean unsubscribe(const char* topic);
    boolean publish(const char* topic, const char* payload);
    boolean subscribeToActions();
    boolean unsubscribeToActions();
    boolean publishActionStatus(char* actionId, int progressPercentage, char* status, char* error);
    boolean readDeviceConfigFile();
    boolean parseDeviceConfigFile();
    boolean setupBytebeamClient();

    // private variables
    int mqttPort;
    const char* mqttBrokerUrl; 
    const char* deviceId;
    const char* projectId;
    const char* caCertPem;
    const char* clientCertPem;
    const char* clientKeyPem;
    int actionFuncsHandlerIdx;
    actionFunctionsHandler actionFuncs[BYTEBEAM_NUMBER_OF_ACTIONS];
    char* deviceConfigStr;
    bool isClientActive;
    bool isOTAEnable;

    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
        WiFiClientSecure secureClient;
    #endif

    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
        BearSSL::X509List* rootCA = NULL;
        BearSSL::X509List* clientCert = NULL;
        BearSSL::PrivateKey* clientKey = NULL;
        BearSSL::WiFiClientSecure secureClient;
    #endif
};

extern BytebeamArduino Bytebeam;

#endif /* BYTEBEAM_ARDUINO_H */