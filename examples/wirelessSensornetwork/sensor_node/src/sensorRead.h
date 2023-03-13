#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "Adafruit_SHT31.h"
#include "hardwaredefs.h"
#include "sensor_payload.h"
#include "runTime.h"


/**
   @brief:
   Class for runtime Device status
*/

Adafruit_SHT31 sht31(&Wire);





bool shtInit()
{
  if (!sht31.begin(0x44))
  { // Set to 0x45 for alternate i2c addr
    DEBUG_PRINTLN("Couldn't find SHT31");
  }
  return true;
}



bool readSHT()
{
  auto tempHumid = sht31.readTempHumidity();

  float temp = tempHumid.first;

  float humid = tempHumid.second;

  if (isnan(temp))
  { // check if 'is not a number'
    DEBUG_PRINTLN("Failed to read temperature");
    return false;
  }
  if (isnan(humid))
  { // check if 'is not a number'
    DEBUG_PRINTLN("Failed to read humidity");
    return false;
  }
  RSTATE.temperature = temp;
  RSTATE.humidity = humid;

  return true;
}


#endif
