#ifndef BYTEBEAM_TIME_H
#define BYTEBEAM_TIME_H

#include "BytebeamArduinoDefines.h"

class BytebeamTime {
public:
    // contructor
    BytebeamTime();

    // destructor
    ~BytebeamTime();

    // public functions
    #ifdef BYTEBEAM_ARDUINO_USE_MODEM
        void setModemInstance(TinyGsm* modem);
    #endif

    bool begin();
    bool getEpochMillis();
    bool end();

    // public variables
    unsigned long long beginMillis;
    unsigned long long prevMillis;
    unsigned long long nowMillis;
    unsigned long long endMillis;
    unsigned long long durationMillis;

private:
    // private variables
    #ifdef BYTEBEAM_ARDUINO_USE_WIFI
        WiFiUDP udpClient;
        NTPClient timeClient;
    #endif

    #ifdef BYTEBEAM_ARDUINO_USE_MODEM
        TinyGsm* modem;
    #endif
};

#endif /* BYTEBEAM_TIME_H */