#ifndef BYTEBEAM_OTA_H
#define BYTEBEAM_OTA_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "BytebeamArduino.h"
#include "BytebeamArduinoDefines.h"

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
    #ifdef BYTEBEAM_ARDUINO_USE_MODEM
        void setupGsmClient(TinyGsm* modem);
    #endif

    void saveOTAInfo();
    void retrieveOTAInfo();
    void clearOTAInfoFromFlash();
    void clearOTAInfoFromRAM();
    void setupSecureOTAClient(const void* caCert, const void* clientCert, const void* clientKey);
    void clearSecureOTAClient();
    bool updateFirmware(char* otaPayloadStr, char* actionId);

    // public variables
    bool otaUpdateFlag;
    char otaActionId[BYTEBEAM_OTA_ACTION_ID_STR_LEN];

private:
    // private functions
    bool parseOTAJson(char* otaPayloadStr, char* urlStringReturn);
    bool performOTA(char* actionId, char* otaUrl);

    // private variables
    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
        #ifdef BYTEBEAM_ARDUINO_USE_WIFI
            WiFiClientSecure secureOTAClient;
            HTTPUpdate& BytebeamUpdate =  httpUpdate;
        #endif

        #ifdef BYTEBEAM_ARDUINO_USE_MODEM
            TinyGsmClient gsmOTAClient;
            SSLClient secureOTAClient;
            BytebeamHTTPUpdate& BytebeamUpdate =  BytebeamhttpUpdate;
        #endif
    #endif

    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
        BearSSL::WiFiClientSecure secureOTAClient;
        ESP8266HTTPUpdate& BytebeamUpdate =  ESPhttpUpdate;
    #endif
};

#endif /* BYTEBEAM_OTA_H */