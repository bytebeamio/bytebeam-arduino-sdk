#include <time.h>
#include <WiFi.h>
#include <BytebeamArduino.h>

// on board led pin number
#define BOARD_LED 2

// led state variable
int ledState = 0;

// wifi credentials
const char* WIFI_SSID     = "Mayank";
const char* WIFI_PASSWORD = "mayank_777";

// sntp credentials
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 3600;
const char* ntpServer = "pool.ntp.org";

// function for ToggleLED action
int toggleLed(char* args, char* actionId) {
  Serial.printf("*** args : %s , actionId : %s ***\n", args, actionId);

  ledState = !ledState;
  digitalWrite(BOARD_LED, ledState);

  Bytebeam.publishActionCompleted(actionId);
  return 0;
}

// function to setup the predefined led 
void setupLED() {
  pinMode(BOARD_LED, OUTPUT);
  digitalWrite(BOARD_LED, ledState);
}

// function to setup the wifi with predefined credentials
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
  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();

  setupLED();
  setupWifi();
  syncTimeFromNtp();
  
  Bytebeam.begin();
  Bytebeam.addActionHandler(toggleLed, "ToggleLED");
}

void loop() {
  // put your main code here, to run repeatedly:
  Bytebeam.loop();
  delay(5000);
}