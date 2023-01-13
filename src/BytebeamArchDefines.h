#ifndef BYTEBEAM_ARCH_DEFINES_H
#define BYTEBEAM_ARCH_DEFINES_H

#ifndef FILE_READ
#define FILE_READ "r"
#endif

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    /* ESP32 architecture dependent includes */
    #include <FS.h>
    #include <FFat.h>
    #include <SPIFFS.h>
    #include <WiFiClientSecure.h>
    #include <HTTPUpdate.h>
    #include <Preferences.h>

    /* ESP32 architecture dependent defines */
    #define BYTEBEAM_ARDUINO_ARCH_ESP32
    #define BYTEBEAM_ARDUINO_ARCH_FS
    #define BYTEBEAM_ARDUINO_ARCH_FATFS
    #define BYTEBEAM_ARDUINO_ARCH_SPIFFS
    #define BYTEBEAM_ARDUINO_ARCH_LITTLEFS
    #define BYTEBEAM_ARDUINO_ARCH_SD
#elif defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
    /* ESP8266 architecture dependent includes */
    #include <FS.h>
    #include <WiFiClientSecureBearSSL.h>
    #include <ESP8266httpUpdate.h>

    /* ESP8266 architecture dependent defines */
    #define BYTEBEAM_ARDUINO_ARCH_ESP8266
    #define BYTEBEAM_ARDUINO_ARCH_FS
    #define BYTEBEAM_ARDUINO_ARCH_SPIFFS
    #define BYTEBEAM_ARDUINO_ARCH_LITTLEFS
    #define BYTEBEAM_ARDUINO_ARCH_SD
#endif

#endif /* BYTEBEAM_ARCH_DEFINES_H */