#ifndef BYTEBEAM_ARDUINO_DEFINES_H
#define BYTEBEAM_ARDUINO_DEFINES_H

//
// Enable the define as per your connectivity interface
// Make sure you enable only one interface at a time
//

// defines for using the wifi interface
#define BYTEBEAM_ARDUINO_USE_WIFI

// defines for using the modem interface
// #define TINY_GSM_MODEM_SIM7600
// #define BYTEBEAM_ARDUINO_USE_MODEM

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    // ESP32 architecture dependent defines
    #define BYTEBEAM_ARDUINO_ARCH_ESP32
    #define BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FS
    #define BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FATFS
    #define BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SPIFFS
    #define BYTEBEAM_ARDUINO_ARCH_SUPPORTS_LITTLEFS
    #define BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SD

    // ESP32 architecture dependent includes
    #include <FS.h>
    #include <FFat.h>
    #include <SPIFFS.h>
    #include <LittleFS.h>
    #include <Preferences.h>

    #ifdef BYTEBEAM_ARDUINO_USE_WIFI
        #include <WiFi.h>
        #include <WiFiUdp.h>
        #include <NTPClient.h>
        #include <WiFiClientSecure.h>
        #include <HTTPUpdate.h>
    #endif
    
    #ifdef BYTEBEAM_ARDUINO_USE_MODEM
        #include <time.h>
        #include <TinyGsmClient.h>
        #include <SSLClient.h>
        #include "BytebeamHTTPUpdate.h"
    #endif
#elif defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
    // ESP8266 architecture dependent defines
    #define BYTEBEAM_ARDUINO_ARCH_ESP8266
    #define BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FS
    #define BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SPIFFS
    #define BYTEBEAM_ARDUINO_ARCH_SUPPORTS_LITTLEFS
    #define BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SD

    // ESP8266 architecture dependent includes
    #include <FS.h>
    #include <LittleFS.h>
    #include <WiFiClientSecureBearSSL.h>
    #include <ESP8266httpUpdate.h>
    #include <EEPROM.h>
    #include <ESP8266WiFi.h>
    #include <WiFiUdp.h>
    #include <NTPClient.h>
#endif

#endif /* BYTEBEAM_ARDUINO_DEFINES_H */