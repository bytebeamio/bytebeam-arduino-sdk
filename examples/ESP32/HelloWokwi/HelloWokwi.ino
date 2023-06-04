#include <time.h>
#include <WiFi.h>
#include <BytebeamArduino.h>
#include "arduino_secrets.h"

// on board led pin number
#define BOARD_LED 2

// led state variable
int ledState = 0;

// wifi credentials
const char* WIFI_SSID     = SECRET_SSID;
const char* WIFI_PASSWORD = SECRET_PASS;

// sntp credentials
const long  gmtOffset_sec = 19800;      // GMT + 5:30h
const int   daylightOffset_sec = 0;
const char* ntpServer = "pool.ntp.org";

// Replace this with your device config json in the same format
char deviceConfigData[] = R"(
{
    "project_id": "demo",
    "broker": "cloud.bytebeam.io",
    "port": 8883,
    "device_id": "1",
    "authentication": {
        "ca_certificate": "-----BEGIN CERTIFICATE-----\nMIIFrDCCA5SgAwIBAgICBMwDQYJAwdzEOMAwGA1UBhMFSW5k\naWExETAPBgNVBAgTCEthcm5hdGFrMRIwEAYcmUxFzAVBgNV\nBAkTDlN1YmJpYWggR2FyZGVuMQ8wDQYDVQQREwY1NjAwMTExFDASBgNVBAoTC0J5\ndGViZWFtLmlvMB4XDTIxMDkwMjExMDYyM1oXDTMxMDkwMjExMDYyM1owdzEOMAwG\nA1UEBhMFSW5kaWExETAPBgNVBAgTCEthcm5hdGFrMRIwEAYDVQQHEwlCYW5nYWxv\ncmUxFzAVBgNVBAkTDlN1YmJpYWggR2FyZGVuMQ8wDQYDVQQREwY1NjAwMTExFDAS\nBgNVBAoTC0J5dGViZWFtLmlvMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKC\nAgEAr/bnOa/8AUGZmd/s+7rejuROgeLqqU9X15KKfKOBqcoMyXsSO65UEwpzadpw\nMl7GDCdHqFTymqdnAnbhgaT1PoIFhOG64y7UiNgiWmbh0XJj8G6oLrW9rQ1gug1Q\n/D7x2fUnza71aixiwEL+KsIFYIdDuzmoRD3rSer/bKOcGGs0WfB54KqIVVZ1DwsU\nk1wx5ExsKo7gAdXMAbdHRI2Szmn5MsZwGL6V0LfsKLE8ms2qlZe50oo2woLNN6XP\nRfRL4bwwkdsCqXWkkt4eUSNDq9hJsuINHdhO3GUieLsKLJGWJ0lq6si74t75rIKb\nvvsFEQ9mnAVS+iuUUsSjHPJIMnn/J64Nmgl/R/8FP5TUgUrHvHXKQkJ9h/a7+3tS\nlV2KMsFksXaFrGEByGIJ7yR4qu9hx5MXf8pf8EGEwOW/H3CdWcC2MvJ11PVpceUJ\neDVwE7B4gPM9Kx02RNwvUMH2FmYqkXX2DrrHQGQuq+6VRoN3rEdmGPqnONJEPeOw\nZzcGDVXKWZtd7UCbcZKdn0RYmVtI/OB5OW8IRoXFYgGB3IWP796dsXIwbJSqRb9m\nylICGOceQy3VR+8+BHkQLj5/ZKTe+AA3Ktk9UADvxRiWKGcejSA/LvyT8qzz0dqn\nGtcHYJuhJ/XpkHtB0PykB5WtxFjx3G/osbZfrNflcQZ9h1MCAwEAAaNCMEAwDgYD\nVR0PAQH/BAQDAgKEMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFKl/MTbLrZ0g\nurneOmAfBHO+LHz+MA0GCSqGSIb3DQEBCwUAA4ICAQAlus/uKic5sgo1d2hBJ0Ak\ns1XJsA2jz+OEdshQHmCCmzFir3IRSuVRmDBaBGlJDHCELqYxKn6dl/sKGwoqoAQ5\nOeR2sey3Nmdyw2k2JTDx58HnApZKAVir7BDxbIbbHmfhJk4ljeUBbertNXWbRHVr\ncs4XBNwXvX+noZjQzmXXK89YBsV2DCrGRAUeZ4hQEqV7XC0VKmlzEmfkr1nibDr5\nqwbI+7QWIAnkHggYi27lL2UTH7TvkwYC4TyDaoQ2ZRpG\nHY+mxXLdftoMv/ZvmyjOPYeTRQbfPqoRqcM6XOPXwSw9B6YddwmnkI7ohNOvAVfD\nwGptUc5OodgFQc3waRljX1q2lawZCTh58IUf32CRtOEL2RIz4VpUrNF/0E2vts1f\npO7V1vY2Qin998Nwqkxdsll0GLtEEE9hUyvk1F8U+fgjJ3Rjn4BxnCN4oCrdJOMa\nJCaysaHV7EEIMqrYP4jH6RzQzOXLd0m9NaL8A/Y9z2a96fwpZZU/fEEOH71t3Eo3\nV/CKlysiALMtsHfZDwHNpa6g0NQNGN5IRl/w1TS1izzjzgWhR6r8wX8OPLRzhNRz\n2HDbTXGYsem0ihC0B8uzujOhTHcBwsfxZUMpGjg8iycJlfpPDWBdw8qrGu8LeNux\na0cIevjvYAtVysoXInV0kg==\n-----END CERTIFICATE-----\n",
        "device_certificate": "-----BEGIN CERTIFICATE-----\nMIIEcjCCAlqgAwIBAgICB+MwhMFSW5k\naWExETAPBgNVBAgTCEthcm5hdGFrMRIwEAYDVQQHEwlCN1YmJpYWggR2FyZGVuMQ8wDQYDVQQREwY1NjAwMTExFDASBgNVBAoTC0J5\ndGViZWFtLmlvMB4XDTIyMTEwMzA3NDcwMloXDTMyMTEKTEbMBkG\nA1UEChMSZXNwYnl0ZWJlYW1zZGt0ZXN0MQowCAYDVQQDEwExMIIBIjANBgkqhkiG\n9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzsm8CF8fKSzWFKZeO52RJRGER4z7sYrgTb9x\noIeljE8kmhR+EW217sxUyUn+rmJSCxRz6jtspKv37POPaQilmwtriVq8L46YCrXa\nbI9manlqw0WqAfaWAlFXzM13CuXeGZKhuAK3INs15jqP0s+oWFvww8+hww540I+s\nn6j5yEUnWKSIgoYEc82cwvj0qOvaxYxjUTXgV2IDLSXdFJbZq4k9LDbzPZdDe0aY\n4X9UpdrjS5cRWT3Ok4VwWpMRwOT654CTwE8WUja/pHTRFbMYkqd1jrD2Joqtm+u9\nplTq7I9/fDnkRqPjueDQ04LKTbUC2UsRKw1VmHPndw22+mDPUwIDAQABo1YwVDAO\nBgNVHQ8BAf8EBAMCBaAwEwYDVR0lBAwwCgYIKwYBBQUHAwIwHwYDVR0jBBgwFoAU\nqX8xNsutnSC6ud46YB8Ec74sfP4wDAYDVR0RBAUwA4IBMTANBgkqhkiG9w0BAQsF\nAAOCAgEAcj5Ytt47AdmsXBsv/K7YkUvkNwV64luBMQtqmTKRDWnIQLaFzPjdpieu\n81ktFKklKw6pReWdUPSBy6hKq3zzGvSsShBSghbOAJvlPkK6jpLy5TzZdIj8ug7g\nMCzZvqdmznekSg4elGHJIYUfHD6ZUJaEkkGD1yQpxOIVCnodhN2246SXJK23itmz\nB5y1AHU5zydN7Ys0oY1l8RCc5Iz24IcQAi//2pIPe9ORks3QBBi+qY9m+emHCqgb\n48hNe5v7X+yF4VITlWs3Mfk0gSgbDQ4OwVf/8/D7o0hQ8EU5bzXNvf49Kd2mWTii\nQdQDdNLxZsdgWNwYdqrQhA0qN02yJxkhNm5GXmP75zmg0bPe4I+pNwuy+B32wFZn\nNcZrh4HZzS1cAyyyr3E59zsPjqcohaUcBpe6oUoQ8z1V7K9FR31TnFfvjg9mbDnX\nv2+PXsp0m7f4e1QfepZxUwyjemxOe/b+IEc7PckIrkPy46JaIvi9ONrTR8Y5wDir\nbGuzpmiyMFlyzK4ULh7kt7MU/jU/ulnqDU8JvAnhhxK3kgV4vv8LSYJSEYYP9i1S\nj/EPPQBANdtr4B2pcV1ig13dinq7rir6Q/pnkKrAFOfZcvOWW2BvPOC2dcnCYJpB\n1R5QJGXqna8Uh+ZwThs0K/Nd9dml1w/Rug8hx0VHoIe6AjXEepQ=\n-----END CERTIFICATE-----\n",
        "device_private_key": "-----BEGIN RSA PRIVATE KEY-----\nMIIEowIBAAKCAQEAzsm8CF8fKSzWFKZeO52RJRmhR+\nEW217sxUyUn+rmJSCxRz6jtspKv37POPaQilmwtriVq8L46YCrXabI9manlqw0Wq\nAfaWAlFXzM13CuXeGZKhuAK3INs15jqP0s+oWFvww8+hww540I+sn6j5yEUnWKSI\ngoYEc82cwvj0qOvaxYxjUTXgV2IDLSXdFJbZq4k9LDbzPZdDe0aY4X9UpdrjS5cR\nWT3Ok4VwWpMRwOT654CTwE8WUja/pHTRFbMYkqd1jrD2Joqtm+u9plTq7I9/fDnk\nRqPjueDQ04LKTbUC2UsRKw1VmHPndw22+mDPUwIDAQABAoIBAHv3A/ogzBVrA4ut\nkKA8fV6zeZFLOzfcAUuakQujReMvLsoPruPT2VUmuU1SRpNT7csmn7azmRW+4gny\nmO5meKDR382fz2DTIuKI0kByVvtNfmtBwAEdSiBpkzD7m3m1A8hg1wHw3sebolw6\njy3Zvxn5RASe3GKKsnKVLu8n5VXgyytn2I6sYOmKoLv3N9V3tziHL3Gl2AfWAd5P\nJHJdxZ50hhjzUnI0MmvUU6M9G26xagxNNibzOWv3BEuBRw5fuVpt/QoBouFhcKF7\nEdCFpiZ8LJFpux6FyLDOOAa7mj+Y6yK1qKsJbFGG8AnhPTO/bAc1m7yQcJygH9cl\nretVDcECgYEA4z+NkjqIPRiAvOsmJD6KfMeABkZc4JmART+KtrpoJmPWc2WDxu8M\ngP1eqmh/aGGORY8hcFyl+SIf5JvP13Dd/47tSK4vhyEGnnqRt+wjjuzci0rDvFdH\ntaUbjlb4kKStssQPTTmG726Zd1f32eKzBnCY8YqWdMMM1HXak20rPo8CgYEA6PN8\nYIcIrRkFzyvam8m5pCeoHylBgSNjN2j8dGC8b6SGMG6y4+rUiuxfhC/Oq7UtOvf+\njYQQgeMjB22dhcvJa6rjuuKXXpbcgNnzcemnySohUr7Nm1bXHwRfoqMAmeh3pkcL\nL+K4Mf6Gd1i2hiYd4Dd59m5/p3W+QUyKa4iaRP0CgYAf2/UZHyOijSDfW4hJZIs9\n2ypTtuGmi160Vqg33gJj/3M9UmobJcB3BQ6UjXnvRF4R2nMxsYuDVglqn32QEr7M\n6VjS67i2FSc8aKqtQmnpy8NPs/elHAdtq+wlFIRcovnHKj2K8hm8z6CsXqTc4y9+\nI6MNmgRl8kKGNs+iA5ggeQKBgBKLN25zsWQeJtE8G3XlVArWQVLhtN4z0/UYPWiC\nPt3gSfJXDZSJIAxDDsN2DsyqaoRUM4ZOagX878/qkOySsWEJxIEfAo+8EKeNMgzy\nXbHs0aRFnhZsjklgzsAim6ykzcmFxEU2lhUcvtWHUVhSdnRf1iyg1Taeb9vA3Q/8\nWtN9AoGBAN+rik+pvcdPAexngqmsUgoW8Abvv7dyBmt99KfnyRDfbt6EM68CZI90\naOlNE9P7XZyOyfQdzOlJWv8B78dwrbwMiwkhGtMjEzdW\nKFI0T+ZOXN99GcKLHeg5GfGqHcHlEhXP9BtRyTJVz914alvOIaiY\n-----END RSA PRIVATE KEY-----\n"
    }
}
)";

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
bool publishToDeviceShadow() {
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
    if(!Bytebeam.publishActionFailed(actionId, "Publish led state to device shadow Failed")) {
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

  // setting up the device info i.e to be seen in the device shadow
  Bytebeam.status          = "Device is Up!";
  Bytebeam.softwareType    = "hello-wokwi-ino";
  Bytebeam.softwareVersion = "1.0.0";
  Bytebeam.hardwareType    = "ESP32 Dev Module";
  Bytebeam.hardwareVersion = "rev1";

  // begin the bytebeam client
  if(!Bytebeam.begin(deviceConfigData, BytebeamLogger::LOG_INFO)) {
    Serial.println("Bytebeam Client Initialization Failed.");
  } else {
    Serial.println("Bytebeam Client is Initialized Successfully.");
  }

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