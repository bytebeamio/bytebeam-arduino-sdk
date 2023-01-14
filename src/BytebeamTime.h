#ifndef BYTEBEAM_TIME_H
#define BYTEBEAM_TIME_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

class BytebeamTime {
public:
    // contructor
    BytebeamTime();

    // destructor
    ~BytebeamTime();

    // public functions
    boolean begin();
    boolean getEpochMillis();
    boolean end();

    // public variables
    unsigned long long beginMillis;
    unsigned long long prevMillis;
    unsigned long long nowMillis;
    unsigned long long endMillis;
    unsigned long long durationMillis;
};

#endif /* BYTEBEAM_TIME_H */