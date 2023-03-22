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

// handler for hello world action
int HelloWorld_Hanlder(char* args, char* actionId) {
  //
  // nothing much to do here
  //

  Serial.println("Hello World");

  return 0;
}

// yet another handler for hello world action
int YetAnotherHelloWorld_Hanlder(char* args, char* actionId) {
  //
  // nothing much to do here
  //

  Serial.println("Yet Another Hello World");

  return 0;
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

  // add the handler for hello world led action
  Bytebeam.addActionHandler(HelloWorld_Hanlder, "HelloWorld");

  // Call the isActionHandlerThere method to check if particular action exists or not at any point of time in the code
  // Bytebeam.isActionHandlerThere("HelloWorld");

  // Call the updateActionHandler method to update the particular action at any point of time in the code
  // Bytebeam.updateActionHandler(YetAnotherHelloWorld_Hanlder, "HelloWorld");

  // Call the printActionHandlerArray method to print the action handler array at any point of time in the code
  // Bytebeam.printActionHandlerArray();

  // Call the removeActionHandler method to remove the particular action at any point of time in the code
  // Bytebeam.removeActionHandler("HelloWorld");

  // Call the resetActionHandlerArray method to reset the action handler array at any point of time in the code
  // Bytebeam.resetActionHandlerArray();
}

void loop() {
  // put your main code here, to run repeatedly:

  // bytebeam client loop
  Bytebeam.loop();

  // hold on the execution for some time
  delay(5000);
}