#include <time.h>
#include <WiFi.h>
#include <BytebeamArduino.h>
#include "arduino_secrets.h"

// wifi credentials
const char* WIFI_SSID     = SECRET_SSID;
const char* WIFI_PASSWORD = SECRET_PASS;

// sntp credentials
const long  gmtOffset_sec = 19800;      // GMT + 5:30h
const int   daylightOffset_sec = 0;
const char* ntpServer = "pool.ntp.org";

// device status 
char deviceStatus[200] = "";

// cloud logging tag
const char* TAG = "BYTEBEAM_CLOUD_LOGGING_SKETCH";

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

// function to test the cloud logging feature
void bytebeamCloudLoggingTest() {
  //
  // default bytebeam log level is BYTEBEAM_LOG_LEVEL_INFO, so logs beyond info level will not work :) 
  //

  Bytebeam.logErrorf(TAG, "I am %s Log", "Error");
  Bytebeam.logWarnf(TAG, "I am %s Log", "Warn");
  Bytebeam.logInfof(TAG, "I am %s Log", "Info");
  Bytebeam.logDebugf(TAG, "I am %s Log", "Debug");                   // debug log will not appear to cloud
  Bytebeam.logVerbosef(TAG, "I am %s Log", "Verbose");               // verbose log will not appear to cloud

  // changing log level to Verbose for showing use case   
  Bytebeam.setLogLevel(BYTEBEAM_LOG_LEVEL_VERBOSE); 

  //
  // now bytebeam log level is BYTEBEAM_LOG_LEVEL_VERBOSE, so every logs should work now :)
  //

  Bytebeam.logErrorf(TAG, "This is %s Log", "Error");
  Bytebeam.logWarnf(TAG, "This is %s Log", "Warn");
  Bytebeam.logInfof(TAG, "This is %s Log", "Info");
  Bytebeam.logDebugf(TAG, "This is %s Log", "Debug");                // debug log should appear to cloud
  Bytebeam.logVerbosef(TAG, "This is %s Log", "Verbose");            // verbose log should appear to cloud

  // changing log level back to Info for meeting initial conditions
  Bytebeam.setLogLevel(BYTEBEAM_LOG_LEVEL_INFO);

  Serial.println("Bytebeam Cloud Logging Test Executed Successfully !");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  setupWifi();
  syncTimeFromNtp();
  
  // begin the bytebeam client
  if(!Bytebeam.begin()) {
    Serial.println("Bytebeam Client Initialization Failed.");
  } else {
    Serial.println("Bytebeam Client is Initialized Successfully.");
  }

  // check if cloud logging is enabled or disabled for your device
  bool cloudLoggingStatus = Bytebeam.isCloudLoggingEnabled();

  if(!cloudLoggingStatus) {
    Serial.println("Cloud Logging is Disabled.");
  } else {
    Serial.println("Cloud Logging is Enabled.");
  }

  // enable cloud logging for your device (default)
  // Bytebeam.enableCloudLogging();

  // disable cloud logging for your device
  // Bytebeam.disableCloudLogging();

  // get the log stream name
  // char* log_stream_name = Bytebeam.getLogStream();

  // configure the log stream if needed, defaults to "logs"
  // Bytebeam.setLogStream("device_logs");

  // get the bytebeam log level
  // int current_log_level = Bytebeam.getLogLevel();

  // set the bytebeam log level
  // Bytebeam.setLogLevel(log_level_to_set);

  // test the cloud logging feature
  // bytebeamCloudLoggingTest();
}

void loop() {
  // put your main code here, to run repeatedly:

  // bytebeam client loop
  Bytebeam.loop();

  // get the connection status
  bool connectionStatus = Bytebeam.isConnected();

  // get the device status
  sprintf(deviceStatus, "Device Status : %s !", connectionStatus? "Connected" : "Disconnected");

  // log the device status to cloud
  Bytebeam.logInfoln(TAG, deviceStatus);

  // hold on the execution for some time
  delay(10000);
}