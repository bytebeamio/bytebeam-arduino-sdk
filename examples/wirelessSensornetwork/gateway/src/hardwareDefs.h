#ifndef HARDWAREDEFS_H
#define HARDWAREDEFS_H


enum DeviceType {
    DT_Gateway        = 1,
    DT_Node        
};


#define INVALID_TEMP_READING      99
#define INVALID_HUMIDITY_READING  -1


#define AP_MODE_SSID "Gateway-"
#define AP_MODE_PASS "admin"

String formApSsidName(String deviceId) {
  return String(AP_MODE_SSID + deviceId);
}


#define DEVICE_ID_DEFAULT "00:00"
//ESP NOW configuration
#define ESP_NOW_INIT                            1
#define ESPNOW_CHANNEL                          1

#define DATA_SEND_PERIODICITY_SECS              600 // 10 mins
#define PROCESS_DATA_INTERVAL_SECS              120  // 5 mins


#define MAX_MODEM_PWR_RETRIES                   5
#define MAX_WIFI_CONN_RETRIES                   3



#define MILLI_SECS_MULTIPLIER                   1000
#define MICRO_SECS_MULITPLIER                   1000000
#define HTTP_CONNEC_TIMEOUT_IN_MS               100

#define EEPROM_STORE_SIZE                       512
#define EEPROM_STORAGE_FORMAT_VERSION           "c1"
#define EEPROM_STARTING_ADDRESS                 0

#define WAN_WIFI_SSID_DEFAULT                    "Sarthak"
#define WAN_WIFI_PASS_DEFAULT                    "wireless18"

#define HW_REV                                  1
#define FW_REV                                  2



#ifdef DEBUG_SERIAL
#define DEBUG_PRINTF(...)           Serial.printf(__VA_ARGS__)
#define DEBUG_PRINTLN(...)          Serial.println(__VA_ARGS__)
#define DEBUG_PRINT(...)            Serial.print(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINT(...)
#endif


#endif // HARDWAREDEFS_H
