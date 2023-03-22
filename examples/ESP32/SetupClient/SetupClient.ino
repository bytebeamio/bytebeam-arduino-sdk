#include <time.h>
#include <WiFi.h>
#include <BytebeamArduino.h>      

//
// By including the above header you got the access to the gloabl object named Bytebeam
// Use Bytebeam global object to perform the required operations
//

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  setupWifi();
  syncTimeFromNtp();

  //
  //  Your other application setup stuff goes here
  //

  // This method will initialize and start the bytebeam client 
  // You can over-ride the default file system, file name and library logging options, if needed
  // eg. Bytebeam.begin(BytebeamArduino::LITTLEFS_FILE_SYSTEM, "/my_device_config.json", BytebeamLogger::LOG_INFO)
  if(!Bytebeam.begin()) {
    Serial.println("Bytebeam Client Initialization Failed.");
  } else {
    Serial.println("Bytebeam Client is Initialized Successfully.");
  }

  //
  // If above call is successfull then the bytebeam client is now configured for the use
  // You can always check for the logs in serial monitor for the status of the above call
  //

  // check if bytebeam client is connected or disconnected
  bool connectionStatus = Bytebeam.isConnected();

  if(!connectionStatus) {
    Serial.println("Bytebeam Client is Disconnected.");
  } else {
    Serial.println("Bytebeam Client is Connected.");
  }

  // Call the end method to stop and de-initialize the bytebeam client at any point of time in the code
  // Bytebeam.end();
}

void loop() {
  // put your main code here, to run repeatedly:

  //
  //  Your application regular stuff goes here
  //

  // This method will let you maintain the connection with the bytebeam cloud, In case
  // the connection is lost, it will attempt to reconnect to the bytebeam cloud
  Bytebeam.loop();

  // software delay, you can customize it as per your application needs
  delay(5000);
}