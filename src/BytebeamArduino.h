#ifndef BYTEBEAM_ARDUINO_H
#define BYTEBEAM_ARDUINO_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include "BytebeamLog.h"
#include "BytebeamTime.h"
#include "BytebeamOTA.h"
#include "BytebeamArduinoDefines.h"

/**
 * @enum deviceConfigFileSystem
 * This sturct contains the available file systems that can be used for provisioning the device
 * @var deviceConfigFileSystem::FATFS_FILE_SYSTEM
 * Use FATFS file system for provisioning the device
 * @var deviceConfigFileSystem::SPIFFS_FILE_SYSTEM
 * Use SPIFFS file system for provisioning the device
 * @var deviceConfigFileSystem::LITTLEFS_FILE_SYSTEM
 * Use LITTLEFS file system for provisioning the device
 * @var deviceConfigFileSystem::SD_FILE_SYSTEM
 * Use SD file system for provisioning the device
 */
typedef enum {
    FATFS_FILE_SYSTEM,
    SPIFFS_FILE_SYSTEM,
    LITTLEFS_FILE_SYSTEM,
    SD_FILE_SYSTEM
} deviceConfigFileSystem;

/* This macro is used to debug the library, we will keep all the unnecessary print under this macro */
#define DEBUG_BYTEBEAM_ARDUINO false

/* This macro is used to specify the maximum length of bytebeam mqtt topic string */
#define BYTEBEAM_MQTT_TOPIC_STR_LEN 200

/* This macro is used to specify the maximum number of actions that need to be handled for particular device */
#define BYTEBEAM_NUMBER_OF_ACTIONS 10

/* This macro is used to specify the file system used for provisioning the device */
#if defined(BYTEBEAM_ARDUINO_ARCH_ESP32)
    // defaults to spiffs for esp32 arch (usual filesystem)
    #define DEVICE_CONFIG_FILE_SYSTEM SPIFFS_FILE_SYSTEM
#elif defined(BYTEBEAM_ARDUINO_ARCH_ESP8266)
    // defaults to littlefs for esp8266 arch (community recommended)
    #define DEVICE_CONFIG_FILE_SYSTEM LITTLEFS_FILE_SYSTEM
#endif

/* This macro is used to specify the name of the device config file */
#define DEVICE_CONFIG_FILE_NAME "/device_config.json"

/* This macro is used to specify the maximum number of attempts we will perform to reconnect to the server in case the client got disconnect */
#define BYTEBEAM_CONNECT_MAX_RETRIES 5

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

class BytebeamArduino : private PubSubClient, public BytebeamLog {
public:
    // contructor
    BytebeamArduino();

    // destructor
    ~BytebeamArduino();

    // public functions
    #ifdef BYTEBEAM_ARDUINO_USE_WIFI
        boolean begin();
    #endif

    #ifdef BYTEBEAM_ARDUINO_USE_MODEM
        boolean begin(TinyGsm* modem);
    #endif

    boolean isBegined();
    boolean loop();
    boolean isConnected();
    boolean handleActions(char* actionReceivedStr);
    boolean addActionHandler(int (*funcPtr)(char* args, char* actionId), char* actionName);
    boolean removeActionHandler(char* actionName);
    boolean updateActionHandler(int (*newFuncPtr)(char* args, char* actionId), char* actionName);
    boolean isActionHandlerThere(char* actionName);
    boolean printActionHandlerArray();
    boolean resetActionHandlerArray();
    boolean publishActionCompleted(char* actionId);
    boolean publishActionFailed(char* actionId);
    boolean publishActionProgress(char* actionId, int progressPercentage);
    boolean publishToStream(char* streamName, const char* payload);
    
    #if BYTEBEAM_OTA_ENABLE
        boolean enableOTA();
        boolean isOTAEnabled();
        boolean disableOTA();
    #endif

    boolean end();
    
private:
    // private functions
    boolean init();
    void printArchitectureInfo();
    void initActionHandlerArray();
    boolean subscribe(const char* topic, uint8_t qos);
    boolean unsubscribe(const char* topic);
    boolean publish(const char* topic, const char* payload);
    boolean subscribeToActions();
    boolean unsubscribeToActions();
    boolean publishActionStatus(char* actionId, int progressPercentage, char* status, char* error);
    boolean readDeviceConfigFile();
    boolean parseDeviceConfigFile();
    boolean setupBytebeamClient();
    void clearBytebeamClient();

    // private variables
    int mqttPort;
    const char* mqttBrokerUrl; 
    const char* deviceId;
    const char* projectId;
    const char* caCertPem;
    const char* clientCertPem;
    const char* clientKeyPem;
    const char* clientId;
    int actionFuncsHandlerIdx;
    actionFunctionsHandler actionFuncs[BYTEBEAM_NUMBER_OF_ACTIONS];
    char* deviceConfigStr;
    bool isClientActive;
    bool isOTAEnable;

    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
        #ifdef BYTEBEAM_ARDUINO_USE_WIFI
            WiFiClientSecure secureClient;
        #endif

        #ifdef BYTEBEAM_ARDUINO_USE_MODEM
            TinyGsmClient gsmClient;
            SSLClient secureClient;
        #endif
    #endif

    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
        const BearSSL::X509List* rootCA = NULL;
        const BearSSL::X509List* clientCert = NULL;
        const BearSSL::PrivateKey* clientKey = NULL;
        BearSSL::WiFiClientSecure secureClient;
    #endif
};

extern BytebeamArduino Bytebeam;

#endif /* BYTEBEAM_ARDUINO_H */