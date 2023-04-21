#include <time.h>
#include <WiFi.h>
#include <BytebeamArduino.h>
#include "arduino_secrets.h"

const char* fwVersion = "1.0.0";

// wifi credentials
const char* WIFI_SSID     = SECRET_SSID;
const char* WIFI_PASSWORD = SECRET_PASS;

// sntp credentials
const long  gmtOffset_sec = 19800;      // GMT + 5:30h
const int   daylightOffset_sec = 0;
const char* ntpServer = "pool.ntp.org";

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  setupWifi();
  syncTimeFromNtp();

  // setting up the device info i.e to be seen in the device shadow
  Bytebeam.status          = "Device is Up!";
  Bytebeam.softwareType    = "basic-wifi-ota-ino";
  Bytebeam.softwareVersion = "1.0.0";
  Bytebeam.hardwareType    = "ESP32 Dev Module";
  Bytebeam.hardwareVersion = "rev1";

  // begin the bytebeam client
  if(!Bytebeam.begin()) {
    Serial.println("Bytebeam Client Initialization Failed.");
  } else {
    Serial.println("Bytebeam Client is Initialized Successfully.");
  }

  // check if OTA is enabled or disabled for your device
  bool OTAStatus = Bytebeam.isOTAEnabled();

  if(!OTAStatus) {
    Serial.println("OTA is Disabled.");
  } else {
    Serial.println("OTA is Enabled.");
  }

  // enable OTA updates for your device
  Bytebeam.enableOTA();

  // disable OTA updates for your device (default)
  // Bytebeam.disableOTA();

  Serial.printf("Application Firmware Version : %s\n", fwVersion);
}

void loop() {
  // put your main code here, to run repeatedly:

  // bytebeam client loop
  Bytebeam.loop();

  // hold on execution for some time
  delay(5000);
}