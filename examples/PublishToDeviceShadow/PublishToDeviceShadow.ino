#include <time.h>
#include <WiFi.h>
#include <BytebeamArduino.h>

// wifi credentials
const char* WIFI_SSID     = "Mayank";
const char* WIFI_PASSWORD = "mayank_777";

// sntp credentials
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 3600;
const char* ntpServer = "pool.ntp.org";

// function to get the time 
unsigned long long getEpochTime() {
  const long  gmtOffset_sec = 19800;
  const int   daylightOffset_sec = 3600;
  const char* ntpServer = "pool.ntp.org";

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
  
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  
  unsigned long long time = ((unsigned long long)now * 1000) + (millis() % 1000);
  return time;
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

// function to publish payload to device shadow
void publishToDeviceShadow() {
  static int sequence = 0;
  const char* payload = "";
  String deviceShadowStr = "";
  StaticJsonDocument<1024> doc;

  char deviceShadowStream[] = "device_shadow";

  sequence++;
  unsigned long long milliseconds = getEpochTime();

  JsonArray deviceShadowJsonArray = doc.to<JsonArray>();
  JsonObject deviceShadowJsonObj_1 = deviceShadowJsonArray.createNestedObject();

  deviceShadowJsonObj_1["timestamp"] = milliseconds;
  deviceShadowJsonObj_1["sequence"]  = sequence;
  deviceShadowJsonObj_1["Status"]    = "test device shaodow status";
  
  serializeJson(deviceShadowJsonArray, deviceShadowStr);
  payload = deviceShadowStr.c_str();
  Serial.printf("publishing %s to %s\n", payload, deviceShadowStream);

  Bytebeam.publishToStream(deviceShadowStream, payload);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();

  setupWifi();
  syncTimeFromNtp();
  
  Bytebeam.begin();
  publishToDeviceShadow();
}

void loop() {
  // put your main code here, to run repeatedly:
  Bytebeam.loop();
  delay(5000);
}