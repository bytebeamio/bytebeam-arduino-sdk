
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <BytebeamArduino.h>
#include <esp_wifi.h>
#include "FS.h"
#include "LittleFS.h"


AsyncWebServer server(80);

/* This macro is used to specify the base path of the spiffs partiiton */
#define LITTLEFS_BASE_PATH "/littlefs"

/* This macro is used to format the spiffs if in case the spiffs initialization fails i.e reset if not required */
#define FORMAT_SPIFFS_IF_FAILED true

/* This macro is used to format the spiffs in the beginnig i.e reset if not required */
#define FORMAT_SPIFSS_IN_BEGINNING true

/* This macro is used to specify the name of the device config file */
#define DEVICE_CONFIG_FILE_NAME "/device_config.json"

/* This macro is used to specify the maximum size of device config json in bytes that need to be handled for particular device */
#define DEVICE_CONFIG_STR_LENGTH 8192

/* This macro is used to print the device config read/write buffers to serial monitor i.e set to debug any issue */
#define PRINT_BUFFERS_TO_SERIAL false

char deviceConfigReadStr[DEVICE_CONFIG_STR_LENGTH] = "";

// wifi credentials
const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

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

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory : %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print(" - DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print(" - FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void readFile(fs::FS &fs, const char *path, char *message)
{
    Serial.printf("Reading file : %s\r\n", path);

    File file = fs.open(path, FILE_READ);
    if (!file || file.isDirectory())
    {
        Serial.println("- failed to open file for reading");
        return;
    }

    char chr = ' ';
    int strIndex = 0;

    Serial.println("- read from file");
    while (file.available())
    {
        chr = file.read();
        message[strIndex++] = chr;
    }
    file.close();
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);

#if FORMAT_SPIFSS_IN_BEGINNING
    if (!LittleFS.format())
    {
        Serial.println("spiffs format failed");
        return;
    }
    else
    {
        Serial.println("spiffs format success");
    }
#endif

    if (!LittleFS.begin(FORMAT_SPIFFS_IF_FAILED, SPIFFS_BASE_PATH))
    {
        Serial.println("spiffs mount failed");
        return;
    }
    else
    {
        Serial.println("spiffs mount success");
    }

    listDir(LittleFS, "/", 0);                                           //  list the directories in the spiffs
  
    setupWifi();
    servePortal();
    server.begin();
}

void loop()
{
}

void setupWifi()
{
    WiFi.mode(WIFI_STA);                  // set the wifi to station mode to connect to a access point
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // connect to acesss point with ssid and password

    Serial.println();
    Serial.print("Connecting to " + String(WIFI_SSID)); // wait till chip is being connected to wifi  (Blocking Mode)
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(250);
    }
    Serial.println();

    Serial.print("Connected to " + String(WIFI_SSID) + ", Got IP address : "); // now it is connected to access point
    Serial.println(WiFi.localIP());                                            // print the ip assigned to chip
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
        request->_tempFile = fs.open("/" + filename, "w");
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
        readFile(LittleFS, DEVICE_CONFIG_FILE_NAME, deviceConfigReadStr);    //  read the device configuration string from spiffs
        // de-initalize the spiffs file system 
        LittleFS.end();
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
