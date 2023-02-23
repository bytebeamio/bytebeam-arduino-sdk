#ifndef BYTEBEAM_OTA_H
#define BYTEBEAM_OTA_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BytebeamArduino.h"
#include "BytebeamArchDefines.h"

/* This macro is used to debug the library, we will keep all the unnecessary print under this macro */
#define DEBUG_BYTEBEAM_OTA false

/* This macro is used to specify the maximum length of bytebeam OTA url string */
#define BYTEBEAM_OTA_URL_STR_LEN 200

/* This macro is used to specify the maximum length of bytebeam OTA action id string */
#define BYTEBEAM_OTA_ACTION_ID_STR_LEN 20

/* This macro is used to define the on-board led, we will be using this while HTTP OTA to show the process */
#define BYTEBEAM_OTA_BUILT_IN_LED 2

class BytebeamOTA {
public:
    // constructor
    BytebeamOTA();

    // destructor
    ~BytebeamOTA();

    // public functions
    void saveOTAInfo();
    void retrieveOTAInfo();
    void clearOTAInfoFromFlash();
    void clearOTAInfoFromRAM();
    void setupSecureOTAClient(const void* caCert, const void* clientCert, const void* clientKey);
    void clearSecureOTAClient();
    boolean updateFirmware(char* otaPayloadStr, char* actionId);

    // public variables
    bool otaUpdateFlag;
    char otaActionId[BYTEBEAM_OTA_ACTION_ID_STR_LEN];

private:
    // private functions
    boolean parseOTAJson(char* otaPayloadStr, char* urlStringReturn);
    boolean performOTA(char* actionId, char* otaUrl);

    // private variables
    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
        WiFiClientSecure secureOTAClient;
        HTTPUpdate& BytebeamUpdate =  httpUpdate;
    #endif

    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
        BearSSL::WiFiClientSecure secureOTAClient;
        ESP8266HTTPUpdate& BytebeamUpdate =  ESPhttpUpdate;
    #endif
};

#endif /* BYTEBEAM_OTA_H */