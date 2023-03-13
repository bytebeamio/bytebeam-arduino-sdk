#ifndef RUN_TIME_H
#define RUN_TIME_H
#include "hardwaredefs.h"
#include "sensor_payload.h"

class RunTimeState {
  public:
    RunTimeState():
      batteryPercentage(BATT_VOL_100),
      temperature(INVALID_TEMP_READING),
      humidity(INVALID_HUMIDITY_READING)
    {

    }

    int batteryPercentage;
    float temperature;
    float humidity; 
};

RunTimeState RSTATE;

#endif
