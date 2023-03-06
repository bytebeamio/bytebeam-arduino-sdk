#include "BytebeamHTTPUpdate.h"

#if defined(BYTEBEAM_ARDUINO_ARCH_ESP32) && defined(BYTEBEAM_ARDUINO_USE_MODEM)

bool BytebeamHTTPUpdate::runUpdate(Stream& in, uint32_t size) {
  StreamString error;

  if(_cbProgress) {
    Update.onProgress(_cbProgress);
  }

  if(!Update.begin(size, U_FLASH, _ledPin, _ledOn)) {
    _lastError = Update.getError();
    Update.printError(error);
    error.trim(); // remove line ending
    Serial.printf("Update.begin failed! (%s)\n", error.c_str());
    return false;
  }

  if(_cbProgress) {
    _cbProgress(0, size);
  }

  // To do: the MD5 could be checked if the server sends it
  // To do: the SHA256 could be checked if the server sends it

  if(Update.writeStream(in) != size) {
    _lastError = Update.getError();
    Update.printError(error);
    error.trim(); // remove line ending
    Serial.printf("Update.writeStream failed! (%s)\n", error.c_str());
    return false;
  }

  if(_cbProgress) {
    _cbProgress(size, size);
  }

  if(!Update.end()) {
    _lastError = Update.getError();
    Update.printError(error);
    error.trim(); // remove line ending
    Serial.printf("Update.end failed! (%s)\n", error.c_str());
    return false;
  }

  return true;
}

BytebeamHTTPUpdate::BytebeamHTTPUpdate() {
  //
  // Initailizing all the variables with default values here
  //

  this->_rebootOnUpdate = true;
  this->_ledPin = -1;
  this->_ledOn = HIGH;
}

BytebeamHTTPUpdate::~BytebeamHTTPUpdate() {
  //
  // Nothing much to do here, just print the log to serial :)
  //

  Serial.println("I am BytebeamHTTPUpdate::~BytebeamHTTPUpdate()");
}

int BytebeamHTTPUpdate::getLastError(void) {
  return _lastError;
}

String BytebeamHTTPUpdate::getLastErrorString(void) {
  if(_lastError == 0) {
    return String(); // no error
  }

  // error from Update class
  if(_lastError > 0) {
    StreamString error;
    Update.printError(error);
    error.trim(); // remove line ending
    return String("Update error: ") + error;
  }

  switch(_lastError) {
    case HTTP_UE_TOO_LESS_SPACE:
        return "Not Enough space";
    case HTTP_UE_SERVER_NOT_REPORT_SIZE:
        return "Server Did Not Report Size";
    case HTTP_UE_BIN_VERIFY_HEADER_FAILED:
        return "Verify Bin Header Failed";
    case HTTP_UE_NO_PARTITION:
        return "Partition Could Not be Found";
  }

  return String("HTTP Error");
}

t_httpUpdate_return BytebeamHTTPUpdate::update(Client& client, const String& url, const String& currentVersion) {
  // url will come in this format "https://firmware.cloud.bytebeam.io/api/v1/firmwares/{version}/artifact\"
  const uint16_t port = 443;
  const String server = url.substring(8, 34);
  const String resource = url.substring(34, url.length());
  BytebeamHTTPUpdateResult ret = HTTP_UPDATE_FAILED;

  HttpClient http(client, server, port);

  Serial.printf("ESP32 info:\n");
  Serial.printf(" - free Space: %d\n", ESP.getFreeSketchSpace());
  Serial.printf(" - current Sketch Size: %d\n", ESP.getSketchSize());

  int statusCode = 0;
  
  if(http.get(resource) == 0) {
    // read status code
    statusCode = http.responseStatusCode();
    Serial.print("Status code : ");
    Serial.println(statusCode);

    // read headers
    while(http.headerAvailable()) {
      Serial.print(http.readHeaderName());
      Serial.print(" : ");
      Serial.println(http.readHeaderValue());
    }
  } else {
    Serial.println("Unable to connect");
    return HTTP_UPDATE_FAILED;
  }

  // image size
  size_t len = 0;

  // read content length
  len = http.contentLength();
  Serial.print("Content Length : ");
  Serial.println(len);

  ///< OK (Start Update)
  if(statusCode == 200) {
    if(len > 0) {
      bool startUpdate = true;

      // check if space is available
      int sketchFreeSpace = ESP.getFreeSketchSpace();
      if(!sketchFreeSpace){
        _lastError = HTTP_UE_NO_PARTITION;
        return HTTP_UPDATE_FAILED;
      }

      // make sure we have enough space available
      if(len > sketchFreeSpace) {
        Serial.printf("FreeSketchSpace to low (%d) needed: %d\n", sketchFreeSpace, len);
        startUpdate = false;
      }

      if(!startUpdate) {
        _lastError = HTTP_UE_TOO_LESS_SPACE;
        ret = HTTP_UPDATE_FAILED;
      } else {
        // Warn main app we're starting up...
        if (_cbStart) {
          _cbStart();
        }

        if(http.peek() != 0xE9) {
          Serial.println("Magic header does not start with 0xE9\n");
          _lastError = HTTP_UE_BIN_VERIFY_HEADER_FAILED;
          http.stop();
          return HTTP_UPDATE_FAILED;
        }

        if(runUpdate(http, len)) {
            ret = HTTP_UPDATE_OK;
            log_d("Update ok\n");
            http.stop();

            // Warn main app we're all done
            if (_cbEnd) {
                _cbEnd();
            }

            if(_rebootOnUpdate) {
              Serial.println("Rebooting...");
              ESP.restart();
            }
        } else {
            ret = HTTP_UPDATE_FAILED;
            Serial.println("Update failed\n");
        }
      }
    } else {
      _lastError = HTTP_UE_SERVER_NOT_REPORT_SIZE;
      ret = HTTP_UPDATE_FAILED;
      Serial.println("Content-Length was 0 or wasn't set by Server?!\n");
    }
  } else {
    _lastError = statusCode;
    ret = HTTP_UPDATE_FAILED;
    Serial.printf("HTTP Code is (%d)\n", statusCode);
  }

  http.stop();
  return ret;
}

BytebeamHTTPUpdate BytebeamhttpUpdate;

#endif /* #ifdef BYTEBEAM_ARDUINO_ARCH_ESP32  BYTEBEAM_ARDUINO_USE_MODEM */