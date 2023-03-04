#include "BytebeamHTTPUpdate.h"

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32

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

t_httpUpdate_return BytebeamHTTPUpdate::update(Client& client, const String& url, const String& currentVersion) {
  // url will come in this format "https://firmware.cloud.bytebeam.io/api/v1/firmwares/{version}/artifact\"
  const uint16_t port = 443;
  const String server = url.substring(8, 34);
  const String resource = url.substring(34, url.length());

  HttpClient http(client, server, port);

  if(http.get(resource) == 0) {
    // read status code
    int statusCode = http.responseStatusCode();
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
  size_t size = 0;

  // read content length
  size = http.contentLength();
  Serial.print("Content Length : ");
  Serial.println(size);

  // Warn main app we're starting up...
  if (_cbStart) {
    _cbStart();
  }

  // set the progress callback, if any
  if (_cbProgress) {
    Update.onProgress(_cbProgress);
  }

  if(!Update.begin(size, U_FLASH, _ledPin, _ledOn)) {
    Serial.println("Update begin failed !");
    return HTTP_UPDATE_FAILED;
  }

  if(Update.writeStream(http) != size) {
    Serial.println("Update writeStream failed !");
    return HTTP_UPDATE_FAILED;
  }

  if(!Update.end()) {
    Serial.println("Update end failed !");
    return HTTP_UPDATE_FAILED;
  }
  
  // Warn main app we're all done
  if (_cbEnd) {
    _cbEnd();
  }

  if(_rebootOnUpdate) {
    ESP.restart();
  }

  return HTTP_UPDATE_OK;
}

BytebeamHTTPUpdate BytebeamhttpUpdate;

#endif /* #ifdef BYTEBEAM_ARDUINO_ARCH_ESP32 */