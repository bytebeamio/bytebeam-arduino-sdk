#ifndef BYTEBEAM_HTTP_UPDATE_H
#define BYTEBEAM_HTTP_UPDATE_H

#include "BytebeamArduinoDefines.h"

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32

#include <Update.h>
#include <ArduinoHttpClient.h>

enum BytebeamHTTPUpdateResult {
    HTTP_UPDATE_FAILED,
    HTTP_UPDATE_NO_UPDATES,
    HTTP_UPDATE_OK
};

typedef BytebeamHTTPUpdateResult t_httpUpdate_return; // backward compatibility

using BytebeamHTTPUpdateStartCB = std::function<void()>;
using BytebeamHTTPUpdateEndCB = std::function<void()>;
using BytebeamHTTPUpdateErrorCB = std::function<void(int)>;
using BytebeamHTTPUpdateProgressCB = std::function<void(int, int)>;

class BytebeamHTTPUpdate {
public:   
    // constructor
    BytebeamHTTPUpdate();

    // destructor
    ~BytebeamHTTPUpdate();

    // set reboot on update flag
    void rebootOnUpdate(bool reboot) { _rebootOnUpdate = reboot; }

    // set status led pin
    void setLedPin(int ledPin = -1, uint8_t ledOn = HIGH) { _ledPin = ledPin; _ledOn = ledOn; }

    // Notification callbacks
    void onStart(BytebeamHTTPUpdateStartCB cbOnStart)          { _cbStart = cbOnStart; }
    void onEnd(BytebeamHTTPUpdateEndCB cbOnEnd)                { _cbEnd = cbOnEnd; }
    void onError(BytebeamHTTPUpdateErrorCB cbOnError)          { _cbError = cbOnError; }
    void onProgress(BytebeamHTTPUpdateProgressCB cbOnProgress) { _cbProgress = cbOnProgress; }

    // update the new image
    t_httpUpdate_return update(Client& client, const String& url, const String& currentVersion = "");

private:
    // reboot on update flag
    bool _rebootOnUpdate;

    // status led pin
    int _ledPin;
    uint8_t _ledOn;

    // Callbacks
    BytebeamHTTPUpdateStartCB    _cbStart;
    BytebeamHTTPUpdateEndCB      _cbEnd;
    BytebeamHTTPUpdateErrorCB    _cbError;
    BytebeamHTTPUpdateProgressCB _cbProgress;
};

extern BytebeamHTTPUpdate BytebeamhttpUpdate;

#endif /* BYTEBEAM_ARDUINO_ARCH_ESP32 */

#endif /* BYTEBEAM_HTTP_UPDATE_H */