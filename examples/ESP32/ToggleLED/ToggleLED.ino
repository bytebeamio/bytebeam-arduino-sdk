#include <time.h>
#include <WiFi.h>
#include <BytebeamArduino.h>

// on board led pin number
#define BOARD_LED 2

// led state variable
int ledState = 0;

// wifi credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// sntp credentials
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 3600;
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

// function to setup the predefined led
void setupLED() {
  pinMode(BOARD_LED, OUTPUT);
  digitalWrite(BOARD_LED, ledState);
}

// function to toggle the predefined led
void toggleLED() {
  ledState = !ledState;
  digitalWrite(BOARD_LED, ledState);
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

// function to publish payload to device shadow
boolean publishToDeviceShadow() {
  static int sequence = 0;
  unsigned long long milliseconds = 0;
  char ledStatus[200] = "";
  char deviceShadowStream[] = "device_shadow";

  const char* payload = "";
  String deviceShadowStr = "";
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

  // generate the led status message string
  sprintf(ledStatus, "LED is %s !", ledState == true? "ON" : "OFF");

  JsonArray deviceShadowJsonArray = doc.to<JsonArray>();
  JsonObject deviceShadowJsonObj_1 = deviceShadowJsonArray.createNestedObject();

  deviceShadowJsonObj_1["timestamp"] = milliseconds;
  deviceShadowJsonObj_1["sequence"]  = sequence;
  deviceShadowJsonObj_1["Status"]    = ledStatus;
  
  serializeJson(deviceShadowJsonArray, deviceShadowStr);
  payload = deviceShadowStr.c_str();

  Serial.printf("publishing %s to %s\n", payload, deviceShadowStream);

  return Bytebeam.publishToStream(deviceShadowStream, payload);
}

// handler for ToggleLED action
int ToggleLED_Hanlder(char* args, char* actionId) {
  Serial.println("ToggleLED Action Received !");
  Serial.printf("<--- args : %s, actionId : %s --->\n", args, actionId);

  // toggle the led
  toggleLED();

  // publish led state to device shadow
  if(!publishToDeviceShadow()) {
    // publish action failed status
    if(!Bytebeam.publishActionFailed(actionId)) {
      Serial.println("Failed to publish action failed response for Toggle LED action");
    }

    Serial.println("Failed to publish led state to device shadow");
    return -1;
  }

  // publish action completed status
  if(!Bytebeam.publishActionCompleted(actionId)) {
    Serial.println("Failed to publish action completed response for Toggle LED action");
    return -1;
  }

  return 0;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  setupWifi();
  syncTimeFromNtp();

  // setup the gpio led
  setupLED();
  
  // begin the bytebeam client
  Bytebeam.begin();

  // add the handler for toggle led action
  Bytebeam.addActionHandler(ToggleLED_Hanlder, "ToggleLED");
}

void loop() {
  // put your main code here, to run repeatedly:

  // bytebeam client loop
  Bytebeam.loop();

  // hold on the execution for some time
  delay(5000);
}