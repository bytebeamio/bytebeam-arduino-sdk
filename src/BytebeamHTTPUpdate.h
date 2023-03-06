#ifndef BYTEBEAM_HTTP_UPDATE_H
#define BYTEBEAM_HTTP_UPDATE_H

#include "BytebeamArduinoDefines.h"

#if defined(BYTEBEAM_ARDUINO_ARCH_ESP32) && defined(BYTEBEAM_ARDUINO_USE_MODEM)

#include <Update.h>
#include <StreamString.h>
#include <ArduinoHttpClient.h>

/// note we use HTTP client errors too so we start at 100
#define HTTP_UE_TOO_LESS_SPACE              (-100)
#define HTTP_UE_SERVER_NOT_REPORT_SIZE      (-101)
#define HTTP_UE_SERVER_FILE_NOT_FOUND       (-102)
#define HTTP_UE_SERVER_FORBIDDEN            (-103)
#define HTTP_UE_SERVER_WRONG_HTTP_CODE      (-104)
#define HTTP_UE_SERVER_FAULTY_MD5           (-105)
#define HTTP_UE_BIN_VERIFY_HEADER_FAILED    (-106)
#define HTTP_UE_BIN_FOR_WRONG_FLASH         (-107)
#define HTTP_UE_NO_PARTITION                (-108)

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

    // Error infos
    int getLastError(void);
    String getLastErrorString(void);

    // update the new image
    t_httpUpdate_return update(Client& client, const String& url, const String& currentVersion = "");

private:
    // mark the update
    bool runUpdate(Stream& in, uint32_t size);

    // last error
    int _lastError;

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

#endif /* BYTEBEAM_ARDUINO_ARCH_ESP32 && BYTEBEAM_ARDUINO_USE_MODEM */

#endif /* BYTEBEAM_HTTP_UPDATE_H */