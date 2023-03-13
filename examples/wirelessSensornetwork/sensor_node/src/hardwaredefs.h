#ifndef HARDWAREDEFS_H
#define HARDWAREDEFS_H

// should come from build system, as they are used between QA and prod builds
// comment these out to disable setting them to 0 won't disable them.
#define DEBUG_SERIAL        1 
//#define QUICK_TEST_DATA     1
#define PROD_URLS           1



#define AP_MODE_SSID "ThingHz-"

String formApSsidName(String deviceId) {
  return String(AP_MODE_SSID + deviceId);
}

// hardware identifier and rev
#define DEVICE_TYPE                     DT_Node

// hardware rev is tied to device type, they both form a combo that decies the firmware behaviour
#define HW_REV                          1
// firmware rev
#define FW_REV                          2


// sensor configuration
#define SENSOR_PROFILE                  SensorTH


// wiring configuration
#define TEMP_SENSOR_PIN     32
#define BATTERY_VOL_PIN     36
#define SIG_PIN             26
#define CONFIG_PIN          25
#define VOLTAGE_DIV_PIN     5


#define BATTERY_INITIAL_READING     0
#define INVALID_TEMP_READING        99
#define INVALID_HUMIDITY_READING    -1
#define INVALID_GAS_READING         0


// configuration
#define CAPTIVE_MAX_CLIENT_CONFIG_DURATION      60 //max time for captive client configuration
#define SECS_MULTIPLIER_DEEPSLEEP               900 //900
#define CCS_WARM_DURATION                       10000
#define CCS_READ_DURATION                       5000
#define MILLI_SECS_MULTIPLIER                   1000
#define MICRO_SECS_MULITPLIER                   1000000
#define EEPROM_STORE_SIZE                       512
#define SERIAL_BAUD_RATE                        115200
#define LTE_SWITCHON_WAIT_SECS                  40
#define ESPNOW_MAX_PACKET_SIZE                  249
#define BUZZER_OFF_COUNT                        10
#define ESPNOW_CHANNEL                          1
#define MAC_LEN                                 6
#define MAX_DATA_RETRY_COUNT                    2

#define NODE_CAPTIVE_PORTAL_MAX_CONFIG_TIME_SECS  120

#ifdef QUICK_TEST_DATA
#define DATA_SEND_PERIODICITY_SECS              10    // interval for reading sensor data in gateway
#define PROCESS_DATA_INTERVAL_SECS              20    // interval for processing accumulated data on gateway
#else
#define DATA_SEND_PERIODICITY_SECS              1200 // 20 mins
#define PROCESS_DATA_INTERVAL_SECS              300  // 5 mins
#endif


//battery
#define BATT_VOL_0                 3.0
#define BATT_VOL_100               4.2

#ifdef DEBUG_SERIAL
#define DEBUG_PRINTF(...)           Serial.printf(__VA_ARGS__)
#define DEBUG_PRINTLN(...)          Serial.println(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#define DEBUG_PRINTLN(...)
#endif


#endif // HARDWAREDEFS_H
