#ifndef BYTEBEAM_OTA_H
#define BYTEBEAM_OTA_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BytebeamArduino.h"
#include "BytebeamArchDefines.h"

/* This macro is used to debug the library, we will keep all the unnecessary print under this macro */
#define DEBUG_BYTEBEAM_OTA false

/* This macro is used to specify the maximum size of ota action id string length in bytes */
#define OTA_ACTION_ID_STR_LEN 20

/* This macro is used to define the on-board led, we will be using this while HTTP OTA to show the process */
#define BYTEBEAM_OTA_LED 2

class BytebeamOTA {
public:
    // constructor
    BytebeamOTA();

    // destructor
    ~BytebeamOTA();

    // public functions
    void saveOTAInfo();
    void retrieveOTAInfo();
    void clearOTAInfo();
    boolean parseOTAJson(char* otaPayloadStr, char* urlStringReturn); 
    boolean performOTA(char* actionId, char* otaUrl);

    // public variables
    bool otaUpdateFlag;
    char otaActionId[OTA_ACTION_ID_STR_LEN];

    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
        WiFiClientSecure secureOTAClient;
    #endif
};

#endif /* BYTEBEAM_OTA_H */