#include "time.h"
#include <WiFi.h>
#include <BytebeamArduino.h>
#include <FS.h>
#include <SPIFFS.h>

// on board led
#define BOARD_LED 2

// Wifi Credentials
const char* WIFI_SSID     = "Mayank";
const char* WIFI_PASSWORD = "mayank_777";

// SNTP Credentials
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 3600;

// tetsing variables
int ledState = 1;
int pubSubCnt = 0;
const char* mqttTopic = "/tenants/espbytebeamsdktest/devices/1/events/device_shadow/jsonarray";

// function to get the time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

// function for toggleBoardLed action handler
int toggleBoardLed(char* args, char* actionId) {
  Serial.printf("*** args : %s , actionId : %s ***\n", args, actionId);

  ledState = !ledState;
  digitalWrite(BOARD_LED, ledState);

  Bytebeam.publishActionCompleted(actionId);
}

// function to publish payload to device shadow
void publishToDeviceShadow() {
  static int sequence = 0;
  const char* payload = "";
  String actionStatusStr = "";
  StaticJsonDocument<1024> doc;

  sequence++;
  long long milliseconds = getTime() * 1000LL;

  JsonArray actionStatusJsonArray = doc.to<JsonArray>();
  JsonObject actionStatusJsonObj_1 = actionStatusJsonArray.createNestedObject();

  actionStatusJsonObj_1["timestamp"] = milliseconds;
  actionStatusJsonObj_1["sequence"]  = sequence;
  actionStatusJsonObj_1["Status"]    = "dummy device shaodow status";
  actionStatusJsonObj_1["name"]      = "mayank";
  
  serializeJson(actionStatusJsonArray, actionStatusStr);
  payload = actionStatusStr.c_str();

  Serial.printf("publishing %s to device shadow\n", payload);

  Bytebeam.publishToStream("device_shadow", payload);
}

// function to connect to wifi with predefined credentials
void setupWifi() {
  WiFi.mode(WIFI_STA);                                                        // set the wifi to station mode to connect to a access point
  WiFi.begin(WIFI_SSID , WIFI_PASSWORD);                                      // connect to acesss point with ssid and password

  Serial.println();
  Serial.print("Connecting to " + String(WIFI_SSID));                         // wait till chip is being connected to wifi  (Blocking Mode)
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }
  Serial.println();
  
  Serial.print("Connected to " + String(WIFI_SSID) + ", Got IP address : ");   // now it is connected to access point
  Serial.println(WiFi.localIP());                                              // print the ip assigned to chip
  Serial.println();
}

// function to sync time from ntp server with predefined credentials
void syncTimeFromNtp() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);                    // set the ntp server and offset

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {                                               // sync the current time from ntp server
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(BOARD_LED, OUTPUT);
  digitalWrite(BOARD_LED, ledState);
  delay(10);

  setupWifi();
  syncTimeFromNtp();
  
  Bytebeam.begin();
  // Bytebeam.subscribe(mqttTopic);
  Bytebeam.addActionHandler(toggleBoardLed, "toggleBoardLed");
  publishToDeviceShadow();
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  Bytebeam.loop();
  // Bytebeam.publish(mqttTopic, "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}");
  delay(5000);

  // pubSubCnt++;
  // if(pubSubCnt == 5) {
  //   Bytebeam.unsubscribe(mqttTopic);
  //   Bytebeam.end();
  // }

  // Bytebeam.connected();
}