#include <WiFi.h>
#include <driver/adc.h>
#include "sensor_payload.h"
#include "esputils.h"
#include <esp_now.h>
#include <rom/rtc.h>
#include "sensorRead.h"
#include "runTime.h"

/**
   @brief:
   Structure RTC State
*/
typedef struct {
  uint32_t wakeupsCount            = 0;
  uint32_t gatewayNotFoundCount    = 0;
  uint8_t sendFailures             = 0;
  uint8_t gatewayValidCount        = 0;
  uint8_t gatewaymacs[3][6]        = {{ 0 }};
  int8_t rssi[3]                   = {0};
} RTCState;

#define SLEEP_BASE_FOR_RETRY_SECS              20
#define NETWORK_RETRY_BACKOFF_MULTIPLIER       2

RTC_DATA_ATTR RTCState rtcState;
esp_now_peer_info_t slave;
const esp_now_peer_info_t *peer = &slave;


SensorPayloadTemp SensorTemperature;
SensorPayloadTH   SensorTempHumid;


int sendStatusCB = 0;
uint8_t invalidMac[MAC_LEN] = { 0 };

uint8_t successfulMAC[MAC_LEN];


// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  status == ESP_NOW_SEND_SUCCESS ? sendStatusCB = 1 : sendStatusCB = 0;
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


//Step 1- initialise ESPNOW
void initESP() {
  //DEBUG_PRINTLN(WiFi.disconnect() ? "Disconnected" : "Not Disconnected");

  WiFi.mode(WIFI_STA); // Station mode for esp-now sensor node
  DEBUG_PRINTF("This mac: %s, ", WiFi.macAddress().c_str());
  if (esp_now_init() == 0 ) {
    DEBUG_PRINTLN("Initialization Successful");
  } else {
    DEBUG_PRINTLN("Initialization failed");
    ESP.restart();
  }
  //    esp_now_set_self_role(2);

  esp_now_register_send_cb(OnDataSent);
}

//Step 2 Scan for the available peers

bool scanPeers() {
  int16_t scanResults = WiFi.scanNetworks(false, false, false, 300, ESPNOW_CHANNEL); // Scan only on one channel
  // reset on each scan
  bool slaveFound = 0;
  memset(&slave, 0, sizeof(slave));

  Serial.println("");
  if (scanResults == 0) {
    Serial.println("No WiFi devices in AP Mode found");
  } else {
    Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");
    for (int i = 0; i < scanResults; ++i) {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      delay(10);
      // Check if the current device starts with `Slave`
      if (SSID.indexOf("Gateway-") == 0) {
        // SSID of interest
        Serial.println("Found a Slave.");
        Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
        // Get BSSID => Mac Address of the Slave
        int mac[6];
        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            slave.peer_addr[ii] = (uint8_t) mac[ii];
          }
        }

        slave.channel = ESPNOW_CHANNEL; // pick a channel
        slave.encrypt = 0; // no encryption

        slaveFound = 1;
        // we are planning to have only one slave in this example;
        // Hence, break after we find one, to be a bit efficient
        break;
      }
    }
  }

  if (slaveFound) {
    Serial.println("Slave Found, processing..");
  } else {
    Serial.println("Slave Not Found, trying again.");
  }

  // clean up ram
  WiFi.scanDelete();
}


//Step 3 Scan for the available peers
bool addPeers() {
   bool exists = esp_now_is_peer_exist(slave.peer_addr);
    if ( exists) {
      // Slave already paired.
      Serial.println("Already Paired");
      return true;
    } else {
      // Slave not paired, attempt pair
      esp_err_t addStatus = esp_now_add_peer(&slave);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("Pair success");
        return true;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW Not Init");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println("Peer list full");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Out of memory");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer Exists");
        return true;
      } else {
        Serial.println("Not sure what happened");
        return false;
      }
    }
}



void deletePeer() {
  esp_err_t delStatus = esp_now_del_peer(slave.peer_addr);
  Serial.print("Slave Delete Status: ");
  if (delStatus == ESP_OK) {
    // Delete success
    Serial.println("Success");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW Not Init");
  } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
}





void storeAndSleep(int sleepSecs)
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  digitalWrite(VOLTAGE_DIV_PIN, HIGH);
  DEBUG_PRINTLN("going to sleep");
  esp_sleep_enable_timer_wakeup(SECS_MULTIPLIER_DEEPSLEEP * MICRO_SECS_MULITPLIER);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_deep_sleep_start();
}

void yieldingDelay(uint32_t delayms, uint32_t stepSizems)
{
  uint32_t loopCount = delayms / stepSizems;
  for ( int i = 0; i < loopCount; i++) {
    yield();
    delay(1);
  }
}

void readSensorTH() {
  //shtInit();
  //readSHT();
  RSTATE.temperature = 12.0;
  RSTATE.humidity = 78;
  SensorTempHumid.temperature = RSTATE.temperature;
  SensorTempHumid.humidity = RSTATE.humidity;
  //SensorTempHumid.batteryPercentage = getBatteryPercentage(readBatValue());
  SensorTempHumid.batteryPercentage = 100;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.mode(WIFI_STA);
  DEBUG_PRINTLN("  ");
  DEBUG_PRINTLN("STA MAC: "); DEBUG_PRINTLN(WiFi.macAddress());
  initESP();
}


void loop() {

  DEBUG_PRINTLN("processing stored gateways");

  scanPeers();
  
  // now we have valid gateways
  addPeers();
  //vTaskSuspend( xHandle );

  int sendStatus = 1; // default assumes failure
  int aggSendStatus = 1;
  readSensorTH();
  DEBUG_PRINTF("Temp= %.1f, battery= %d, humidity=%.1f\n", SensorTempHumid.temperature, SensorTempHumid.batteryPercentage, SensorTempHumid.humidity);
  const uint8_t *peer_addr = slave.peer_addr;
  sendStatus = esp_now_send(peer_addr, (uint8_t*)&SensorTempHumid, sizeof(SensorTempHumid)); // NULL means send to all peers
  DEBUG_PRINTF("Send status= %d\n", sendStatus );

  // wait for sendstatus callback for about 10ms
  yieldingDelay(10, 1);


  DEBUG_PRINTF("Send status= %d\n", sendStatus );

  if (!aggSendStatus) {
    DEBUG_PRINTLN("Send status successful" );
    sendStatusCB = 1;
  }


  storeAndSleep(30);
}
