#include <time.h>
#include <WiFi.h>
#include <BytebeamArduino.h>
#include "arduino_secrets.h"

// temperature stream name
char tempStream[] = "chip_temperature";

// wifi credentials
const char* WIFI_SSID     = SECRET_SSID;
const char* WIFI_PASSWORD = SECRET_PASS;

// sntp credentials
const long  gmtOffset_sec = 19800;      // GMT + 5:30h
const int   daylightOffset_sec = 0;
const char* ntpServer = "pool.ntp.org";

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

// function to setup the wifi with predefined credentials
void setupWifi() {
  // set the wifi to station mode to connect to a access point
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID , WIFI_PASSWORD);

  Serial.println();
  Serial.print("Connecting to " + String(WIFI_SSID));

  // wait till chip is being connected to wifi  (Blocking Mode)
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }

  // now it is connected to the access point just print the ip assigned to chip
  Serial.println();
  Serial.print("Connected to " + String(WIFI_SSID) + ", Got IP address : ");
  Serial.println(WiFi.localIP());
}

// function to sync time from ntp server with predefined credentials
void syncTimeFromNtp() {
  // sync the time from ntp server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;

  // get the current time
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // log the time info to serial :)
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.println();
}

// function to get the time 
unsigned long long getEpochMillis() {
  time_t now;
  struct tm timeinfo;

  // get the current time i.e make sure the device is in sync with the ntp server
  if (!getLocalTime(&timeinfo)) {
    Serial.println("failed to obtain time");
    return 0;
  }

  // get the epoch time
  time(&now);

  // generate the epoch millis
  unsigned long long timeMillis = ((unsigned long long)now * 1000) + (millis() % 1000);

  return timeMillis;
}

// function to publish chip temperature to strem
bool publishChipTemperature(char* stream) {
  static int sequence = 0;
  unsigned long long milliseconds = 0;
  uint8_t chipTemp = 0;

  const char* payload = "";
  String chipTemperatureStr = "";
  StaticJsonDocument<1024> doc;

  // get the current epoch millis
  milliseconds = getEpochMillis();

  // make sure you got the millis
  if(milliseconds == 0) {
    Serial.println("failed to get epoch millis");
    return false;
  }

  // increment the sequence counter
  sequence++;

  // get the chip temperature 
  chipTemp = (temprature_sens_read() - 32) / 1.8;

  JsonArray chipTemperatureJsonArray = doc.to<JsonArray>();
  JsonObject chipTemperatureJsonObj_1 = chipTemperatureJsonArray.createNestedObject();

  chipTemperatureJsonObj_1["timestamp"]   = milliseconds;
  chipTemperatureJsonObj_1["sequence"]    = sequence;
  chipTemperatureJsonObj_1["temperature"] = chipTemp;
  
  serializeJson(chipTemperatureJsonArray, chipTemperatureStr);
  payload = chipTemperatureStr.c_str();

  Serial.printf("publishing %s to %s\n", payload, stream);

  return Bytebeam.publishToStream(stream, payload);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  setupWifi();
  syncTimeFromNtp();

  // setting up the device info i.e to be seen in the device shadow
  Bytebeam.status          = "Device is Up!";
  Bytebeam.softwareType    = "publish-chip-temp-ino";
  Bytebeam.softwareVersion = "1.0.0";
  Bytebeam.hardwareType    = "ESP32 Dev Module";
  Bytebeam.hardwareVersion = "rev1";

  // begin the bytebeam client
  if(!Bytebeam.begin()) {
    Serial.println("Bytebeam Client Initialization Failed.");
  } else {
    Serial.println("Bytebeam Client is Initialized Successfully.");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  // bytebeam client loop
  Bytebeam.loop();

  // publish chip temperature to temperature stream
  if(!publishChipTemperature(tempStream)) {
    Serial.printf("Failed to publish chip temperature to %s", tempStream);
  }

  // hold on execution for some time
  delay(10000);
}