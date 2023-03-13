#ifndef ESP_UTILS_H
#define ESP_UTILS_H
#include "hardwaredefs.h"

// try not to use global state in any fucntions here,
// if anything needs access to global state, take it as argument and 
// collect it in a reference variable.


const IPAddress apIP(192, 168, 4, 1);
const IPAddress netMsk(255, 255, 255, 0);

/**
   @brief:
   Helper function to get the Battery percentage.
   Battery voltage is mapped to 0 to 100 factor
   @param:
   Battery Voltage in float
   @return:
   battery percentage in int
*/
int getBatteryPercentage(const float battVoltage) {
  int maxVal = BATT_VOL_100 * 100; // scale by 100
  int minVal = BATT_VOL_0 * 100; // scale by 100
  int percentage = 0;
  if (battVoltage > maxVal) {
    maxVal = battVoltage;
  }
  percentage = map(int(battVoltage * 100), minVal, maxVal, 0, 100);
  if (percentage < 0) {
    percentage = 0;
  }  if (percentage > 100) {
    percentage = 100;
  }
  return percentage;
}

/**
   @brief:
   Helper function to get the RSSI percentage from the RSSI in db of available Networks
   @param:
   RSSI in db
   @return:
   rssi in percentage
*/
int getRSSIasQuality() 
{
    int n = WiFi.scanNetworks();
    int rssi = -80;
    for(int i=0;i<=n;i++){
        String ssid = WiFi.SSID(i);
        if(ssid.indexOf("Gateway-")>=0){
            rssi = WiFi.RSSI(i);
        }
    }
    int quality = 0;
    if (rssi <= -100) {
        quality = 0;
    } else if (rssi >= -50) {
        quality = 100;
    } else {
        quality = 2 * (rssi + 100);
    }
    return quality;
}

/**
   @brief:
   gets last 3 bytes of mac
   @param:
   uint8_t mac bool fullSizeMac
   @return:
   returns mac String
*/
String getLast3ByteMac(uint8_t* mac, bool fullSizeMac){
  char macStr[9] = { 0 };
  WiFi.macAddress(mac);
  if(!fullSizeMac){
    sprintf(macStr, "%02X%02X%02X", mac[3], mac[4], mac[5]);
  }else{
    sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2],mac[3], mac[4], mac[5]);
  }
  return String(macStr);
}


/**
   @brief:
   connfigure WiFi radio as AP
   @return:
   bool configure success or not
*/
bool reconnectAP(const String& APssid) {
 WiFi.disconnect();
  uint8_t mac[6];
  String macStr = getLast3ByteMac(mac,false);
  if (WiFi.softAPgetStationNum() > 0) {
    DEBUG_PRINTLN("Station Connected to SoftAP, keeping soft AP alive");
    return true;
  }
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP((APssid+macStr).c_str())) {
    return false;
  }
  delay(100);
  if (!WiFi.softAPConfig(apIP, apIP, netMsk)) {
    return false;
  }
  delay(100);
  DEBUG_PRINTLN("Connected to: ");
  Serial.println(WiFi.softAPIP());
  return true;
}

/**
   @brief:
   read battery value
   @return:
   battery voltage
*/
float readBatValue()
{
  //formula for VD1 = 1M/(3.9M+1M)
  int adcVal = analogRead(BATTERY_VOL_PIN);
  float batVol = adcVal * 0.00127; //finalVolt = (1/1024)(1/VD)    external VD [VD1 = 3.3Mohm/(1Mohm+3.3Mohm)]
  DEBUG_PRINTF("adcVal %d\n", adcVal);
  DEBUG_PRINTF("batteryVoltage %.1f\n", batVol);
  return batVol;
}

#endif //ESP_UTILS_H
