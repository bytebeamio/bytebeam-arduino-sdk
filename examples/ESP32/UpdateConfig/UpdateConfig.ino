#include <time.h>
#include <WiFi.h>
#include <BytebeamArduino.h>

// on board led pin number
#define BOARD_LED 2

// duty cycle for the led
int ledDutyCycle = 0;

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
  float brightness = (ledDutyCycle/255.0) * 100;
  sprintf(ledStatus, "LED Brighntness is %.2f%% !", brightness);

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

// handler for update_config action
int UpdateConfig_Hanlder(char* args, char* actionId) {
  Serial.println("UpdateConfig Action Received !");
  Serial.printf("<--- args : %s, actionId : %s --->\n", args, actionId);

  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, args);

  if(err) {
    // publish action failed status
    if(!Bytebeam.publishActionFailed(actionId)) {
      Serial.println("Failed to publish action failed response for Update Config action");
    }

    Serial.printf("deserializeJson() failed : %s\n", err.c_str());
    return -1;
  }

  const char* name     = doc["name"];
  const char* version  = doc["version"];
  ledDutyCycle         = doc["step_value"];

  // generate the pwm signal
  analogWrite(BOARD_LED, ledDutyCycle);

  // publish led brightness to device shadow
  if(!publishToDeviceShadow()) {
    // publish action failed status
    if(!Bytebeam.publishActionFailed(actionId)) {
      Serial.println("Failed to publish action failed response for Update Config action");
    }

    Serial.println("Failed to publish led brightness to device shadow");
    return -1;
  }

  // publish action completed status
  if(!Bytebeam.publishActionCompleted(actionId)) {
    Serial.println("Failed to publish action completed response for Update Config action");
    return -1;
  }

  return 0;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();

  setupWifi();
  syncTimeFromNtp();
  
  // begin the bytebeam client
  Bytebeam.begin();

  // add the handler for update config action
  Bytebeam.addActionHandler(UpdateConfig_Hanlder, "update_config");
}

void loop() {
  // put your main code here, to run repeatedly:

  // bytebeam client loop
  Bytebeam.loop();

  // hold on the execution for some time
  delay(5000);
}