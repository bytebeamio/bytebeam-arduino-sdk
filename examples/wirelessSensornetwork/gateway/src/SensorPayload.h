#ifndef SENSOR_H
#define SENSOR_H
#include "hardwareDefs.h"
#include "assert.h"
#include "deviceState.h"


enum SensorProfile {
  SensorNone  = 1,
  SensorTemp,
  SensorTH,
  SensorGas,
};


struct SensorPayload {
public:
    SensorPayload() :  
      sensorProfile(SensorProfile::SensorNone), 
      hwRev(HW_REV), 
      fwRev(FW_REV), 
      deviceType(DeviceType::DT_Node),
      batteryPercentage(0)
      { 
        
        }
    uint8_t sensorProfile;           // identifies sensor profile
    uint8_t hwRev;                  // identifies hw revision
    uint8_t fwRev;                 // identifies fw revision
    uint8_t deviceType;           // identifies device type
    uint8_t batteryPercentage;  //batteryPercentage of nodes
}__attribute__ ((packed));

// temprature only definition
struct SensorPayloadTemp : public SensorPayload {
    SensorPayloadTemp() : temperature(INVALID_TEMP_READING) {
        sensorProfile = SensorProfile::SensorTemp;
    }
    float temperature;
}__attribute__ ((packed));


struct SensorPayloadTH : public SensorPayloadTemp {
    SensorPayloadTH() : humidity(INVALID_HUMIDITY_READING) {
        sensorProfile = SensorProfile::SensorTH;
    }
    float humidity;
} __attribute__ ((packed));



SensorPayload* copyPayloadObject(const SensorPayload *payload) {
    
    if (!payload) {
        return nullptr;
    }

    switch (payload->sensorProfile) {
    case SensorProfile::SensorNone:
    {
        SensorPayload *newPayload = new SensorPayload;
        *newPayload = *payload;
        return newPayload;
    }
        break;
    case SensorProfile::SensorTemp:
    {
        SensorPayloadTemp *newPayload = new SensorPayloadTemp;
        SensorPayloadTemp *oldPayload = (SensorPayloadTemp*)payload;
        *newPayload = *oldPayload;
        DEBUG_PRINTF("value of fw from old payload : %d, and new payload is : %d\n", oldPayload->fwRev, newPayload->fwRev);
        return newPayload;
    }
        break;
    case SensorProfile::SensorTH:
    {
        SensorPayloadTH *newPayload = new SensorPayloadTH;
        SensorPayloadTH *oldPayload = (SensorPayloadTH*)payload;
        *newPayload = *oldPayload;
        return newPayload;
    }
        break;
    default: // should never reach here
        //assert(0);
        DEBUG_PRINTF("Sensor Payload copy called with invalid sensor payload type: %d, we have a null ptr somewhere down the line\n", payload->sensorProfile);
        break;
    }
    return nullptr;
}

void freePayloadObject(SensorPayload *payload) {
    if (!payload) {
        return;
    }

    switch (payload->sensorProfile) {
    case SensorProfile::SensorNone:
    case SensorProfile::SensorTemp:
    {
        SensorPayloadTemp *oldPayload = (SensorPayloadTemp*)payload;
        delete oldPayload;
    }
        break;
    case SensorProfile::SensorTH:
    {
        SensorPayloadTH *oldPayload = (SensorPayloadTH*)payload;
        delete oldPayload;
    }
        break;
    default: // should never reach here
        assert(0);
        break;
    }
}

size_t sizeofPaylodObject(const SensorPayload *payload)
{
    switch (payload->sensorProfile) {
    case SensorProfile::SensorNone:
        return sizeof (SensorPayload);
    case SensorProfile::SensorTemp:
        return sizeof (SensorPayloadTemp);
    case SensorProfile::SensorTH:
        return sizeof (SensorPayloadTH);
    default: // should never reach here
        assert(0);
        break;
    }
}


#endif
