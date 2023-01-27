
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <BytebeamArduino.h>
#include <esp_wifi.h>
#include <SPIFFS.h>

AsyncWebServer server(80);

// wifi credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char HTML_FORM_PROVISION[] PROGMEM = R"rawliteral(
    <!DOCTYPE HTML><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>Bytebeam</title>
	
    <style>body { background-color: #000000 ; font-family: Arial, Helvetica, Sans-Serif; Color: #FFFFFF; } input[type="submit"]{background-color: #616A6B; border: none;color: white;padding:15px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}</style></head>
    <body><center>
        <h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">Bytebeam</h1>
        <br><label style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;font-size: 24px;padding-top: 5px;padding-bottom: 10px;">Provision your device </label><br><br>
        <FORM action="/provision" method= "POST" enctype="multipart/form-data">
            <P><label style="font-family:Times New Roman">Upload device provisioning JSON file</label><br><br><input type="file" name="data"/><br><br><input type="submit" name="upload" value="Upload" title="Upload File"></P>
        </FORM>
    </center></body></html>
    )rawliteral";

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

    if (!SPIFFS.begin(true))
    {
        Serial.println("spiffs mount failed");
        return;
    }
    else
    {
        Serial.println("spiffs mount success");
    }
    setupWifi();
    servePortal();
    server.begin();    
}

void loop(){

}

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

void servePortal()
{

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", HTML_FORM_PROVISION); });

    // run handleUpload function when any file is uploaded
    server.on(
        "/provision", HTTP_POST, [](AsyncWebServerRequest *request)
        { request->send(200); },
        handleUpload);

    server.onNotFound(_handleNotFound);
    yield();
}

/**
       @brief:
       Helper funtion To handle upload
       @param:
       AsyncWebServerRequest
    */

static void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index)
    {
        logmessage = "Upload Start: " + String(filename);
        // open the file on first call and store the file handle in the request object
        request->_tempFile = SPIFFS.open("/" + filename, "w");
        Serial.println(logmessage);
    }

    if (len)
    {
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
        logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        Serial.println(logmessage);
    }

    if (final)
    {
        logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
        // close the file handle as the upload is now done
        request->_tempFile.close();
        Serial.println(logmessage);
        request->redirect("/");
    }
}

/**
      @brief:
      Helper funtion for unexpected error
      @param:
      AsyncWebServerRequest
   */
static void _handleNotFound(AsyncWebServerRequest *request)
{
    String message = "File Not Found\n\n";
    request->send(404, "text/plain", message);
}
