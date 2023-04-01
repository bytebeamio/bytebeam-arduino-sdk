#include "BytebeamOTA.h"
#include "BytebeamLogger.h"

#if BYTEBEAM_OTA_ENABLE

static char tempOtaActionId[BYTEBEAM_OTA_ACTION_ID_STR_LEN] = "";

void BytebeamUpdateStarted() {
  BytebeamLogger::Warn(__FILE__, __func__, "CALLBACK:  HTTP update process started");
}

void BytebeamUpdateFinished() {
  BytebeamLogger::Warn(__FILE__, __func__, "CALLBACK:  HTTP update process finished");
}

void BytebeamUpdateProgress(int cur, int total) {
  static int loopVar = 0;
  static int percentOffset = 10;
  static int progressPercent = 0;
  static char* progressStatus = "Downloading";

  BytebeamLogger::Warn(__FILE__, __func__, "CALLBACK:  HTTP update process at %d of %d bytes", cur, total);
  progressPercent = (((float)cur / (float)total) * 100.00);

  if(progressPercent == loopVar) {
    BytebeamLogger::Debug(__FILE__, __func__, "progressPercent : %d", progressPercent);
    BytebeamLogger::Debug(__FILE__, __func__, "tempOtaActionId : %s", tempOtaActionId);

    // If we are done, change the status to downloaded
    if(progressPercent == 100) {
      progressStatus = "Downloaded";
    }

    // publish the OTA progress status
    if(!Bytebeam.publishActionProgress(tempOtaActionId, progressPercent, progressStatus)) {
      BytebeamLogger::Error(__FILE__, __func__, "Failed to publish OTA progress status.");
    }

    if(loopVar == 100) {
      // reset the variables
      loopVar = 0;
      progressPercent = 0;
    } else {
      // mark the next progress stamp
      loopVar = loopVar + percentOffset;
    }
  }
}

void BytebeamUpdateError(int err) {
  BytebeamLogger::Warn(__FILE__, __func__, "CALLBACK:  HTTP update fatal error code %d", err);
}

bool BytebeamOTA::parseOTAJson(char* otaPayloadStr, char* urlStringReturn) {
  StaticJsonDocument<1024> otaPayloadJson;
  DeserializationError err = deserializeJson(otaPayloadJson, otaPayloadStr);

  if(err) {
    BytebeamLogger::Error(__FILE__, __func__, "deserializeJson() failed : %s", err.c_str());
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "deserializeJson() success");
  }

  BytebeamLogger::Info(__FILE__, __func__, "Obtaining ota variables");

  uint32_t length     = otaPayloadJson["content-length"];
  bool status         = otaPayloadJson["status"];
  const char* url     = otaPayloadJson["url"];
  const char* version = otaPayloadJson["version"];

  const char* argsName[] = {"url", "version"};
  const char* argsStr[] = {url, version};
  int numArg = sizeof(argsStr)/sizeof(argsStr[0]);

  int argIterator = 0;
  for(argIterator = 0; argIterator < numArg; argIterator++) {
    if(argsStr[argIterator] == NULL) {
      BytebeamLogger::Error(__FILE__, __func__, "- failed to obtain %s", argsName[argIterator]);
      return false;
    }
  }
  BytebeamLogger::Info(__FILE__, __func__, "- obtain ota variables");

  BytebeamLogger::Debug(__FILE__, __func__, "length : %d", length);
  BytebeamLogger::Debug(__FILE__, __func__, "status : %d", status);
  BytebeamLogger::Debug(__FILE__, __func__, "url : %s", url);
  BytebeamLogger::Debug(__FILE__, __func__, "version : %s", version);

  int maxLen = BYTEBEAM_OTA_URL_STR_LEN;
  int tempVar = snprintf(urlStringReturn, maxLen,  "%s", url);

  if(tempVar >= maxLen) {
    BytebeamLogger::Error(__FILE__, __func__, "Firmware upgrade url size exceeded buffer size");
    return false;
  }

  BytebeamLogger::Info(__FILE__, __func__, "Constructed url is %s", urlStringReturn);

  return true;
}

bool BytebeamOTA::performOTA(char* actionId, char* otaUrl) {
  int maxLen, tempVar;
  BytebeamLogger::Warn(__FILE__, __func__, "Performing OTA...");

  // save the OTA information in RAM
  this->otaUpdateFlag = true;
  strcpy(this->otaActionId, actionId);
  strcpy(tempOtaActionId, this->otaActionId);

  // disable the auto reboot, we will manually reboot after saving some information
  this->BytebeamUpdate.rebootOnUpdate(false);

  // set the status led pin
  this->BytebeamUpdate.setLedPin(BYTEBEAM_OTA_BUILT_IN_LED, LOW);

  // set the update callbacks
  this->BytebeamUpdate.onStart(BytebeamUpdateStarted);
  this->BytebeamUpdate.onEnd(BytebeamUpdateFinished);
  this->BytebeamUpdate.onProgress(BytebeamUpdateProgress);
  this->BytebeamUpdate.onError(BytebeamUpdateError);

  // start the update process
  t_httpUpdate_return ret = this->BytebeamUpdate.update(this->secureOTAClient, otaUrl);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      maxLen = BYTEBEAM_OTA_ERROR_STR_LEN;
      tempVar = snprintf(this->otaError, maxLen, "Error (%d): %s", this->BytebeamUpdate.getLastError(), this->BytebeamUpdate.getLastErrorString().c_str());

      if(tempVar >= maxLen) {
        BytebeamLogger::Error(__FILE__, __func__, "OTA error size exceeded buffer size");
      }

      BytebeamLogger::Info(__FILE__, __func__, "HTTP_UPDATE_FAILED %s", this->otaError);
      break;

    case HTTP_UPDATE_NO_UPDATES:
      BytebeamLogger::Info(__FILE__, __func__, "HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      BytebeamLogger::Info(__FILE__, __func__, "HTTP_UPDATE_OK");
      break;
  }

  if(ret != HTTP_UPDATE_OK) {
    return false;
  } else {
    return true;
  }
}

BytebeamOTA::BytebeamOTA()
  #ifdef BYTEBEAM_ARDUINO_USE_MODEM
    : secureOTAClient(&gsmOTAClient)
  #endif
{
  //
  // Initailizing all the variables with default values here
  //

  this->otaUpdateFlag = false;
  memset(this->otaActionId, 0x00, BYTEBEAM_OTA_ACTION_ID_STR_LEN);
  memset(this->otaError, 0x00, BYTEBEAM_OTA_ERROR_STR_LEN);
}

BytebeamOTA::~BytebeamOTA() {
  //
  // Nothing much to do here, just print the log to serial :)
  //

  BytebeamLogger::Info(__FILE__, __func__, "I am BytebeamOTA::~BytebeamOTA()");
}

#ifdef BYTEBEAM_ARDUINO_USE_MODEM
  void BytebeamOTA::setupGsmClient(TinyGsm* modem) {
    // initiaize the gsm client with the modem instance
    this->gsmOTAClient.init(modem, 1);
  }
#endif

void BytebeamOTA::saveOTAInfo() {

  /* Non Volatile Storage are architecture specific, so we need to provide the implementation for storing
   * the OTA information to storage here based on the arcitecture.
   */
#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
  Preferences preferences;

  preferences.begin("OTAInfo");
  preferences.putBool("OTAUpdateFlag", this->otaUpdateFlag);
  preferences.putString("OTAActionId", this->otaActionId);
  preferences.end();
#endif

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
  int add = 0;
  int size = 512;
  uint8_t value = 0;

  EEPROM.begin(size);

  value = this->otaUpdateFlag;
  EEPROM.write(add, value);

  add++;

  for(int i=0; i<BYTEBEAM_OTA_ACTION_ID_STR_LEN; i++) {
    value =  this->otaActionId[i];
    EEPROM.write(add, value);
    add++;
  }

  // save the changes made to the EEPROM
  EEPROM.commit();

  EEPROM.end();
#endif

  BytebeamLogger::Info(__FILE__, __func__, "NVS: Saving OTA Information");
}

void BytebeamOTA::retrieveOTAInfo() {

  /* Non Volatile Storage are architecture specific, so we need to provide the implementation for retrieving
   * the OTA information from storage here based on the arcitecture.
   */
#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
  Preferences preferences;

  preferences.begin("OTAInfo");
  this->otaUpdateFlag = preferences.getBool("OTAUpdateFlag");
  preferences.getString("OTAActionId", this->otaActionId, BYTEBEAM_OTA_ACTION_ID_STR_LEN);
  preferences.end();
#endif

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
  int add = 0;
  int size = 512;
  uint8_t value = 0;

  EEPROM.begin(size);

  value = EEPROM.read(add);
  this->otaUpdateFlag = value;

  add++;

  for(int i=0; i<BYTEBEAM_OTA_ACTION_ID_STR_LEN; i++) {
    value = EEPROM.read(add);
    this->otaActionId[i] = value;
    add++;
  }

  EEPROM.end();
#endif

  BytebeamLogger::Info(__FILE__, __func__, "NVS: Retrieving OTA Information");
}

void BytebeamOTA::clearOTAInfoFromFlash() {

  /* Non Volatile Storage are architecture specific, so we need to provide the implementation for clearing
   * the OTA information from storage here based on the arcitecture.
   */
#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
  Preferences preferences;

  preferences.begin("OTAInfo");
  preferences.clear();
  preferences.end();
#endif

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
  int add = 0;
  int size = 512;
  uint8_t value = 0;

  EEPROM.begin(size);

  for(int i=0; i<size; i++) {
    EEPROM.write(add, value);
    add++;
  }

  // save the changes made to the EEPROM
  EEPROM.commit();

  EEPROM.end();
#endif

  BytebeamLogger::Info(__FILE__, __func__, "NVS: Clearing OTA Information From Flash");
}

void BytebeamOTA::clearOTAInfoFromRAM() {
  //
  // Clear the OTA Informatiom from the RAM
  //

  this->otaUpdateFlag = false;
  memset(this->otaActionId, 0x00, BYTEBEAM_OTA_ACTION_ID_STR_LEN);
  memset(this->otaError, 0x00, BYTEBEAM_OTA_ERROR_STR_LEN);

  BytebeamLogger::Info(__FILE__, __func__, "NVS: Clearing OTA Information From RAM");
}

void BytebeamOTA::setupSecureOTAClient(const void* caCert, const void* clientCert, const void* clientKey) {
  //
  // Setting up the secure OTA client
  //

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
  this->secureOTAClient.setCACert((const char*)caCert);
  this->secureOTAClient.setCertificate((const char*)clientCert);
  this->secureOTAClient.setPrivateKey((const char*)clientKey);
#endif

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
  this->secureOTAClient.setBufferSizes(2048, 2048);
  this->secureOTAClient.setTrustAnchors((const BearSSL::X509List*)caCert);
  this->secureOTAClient.setClientRSACert((const BearSSL::X509List*)clientCert, (const BearSSL::PrivateKey*)clientKey);
#endif
}

void BytebeamOTA::clearSecureOTAClient() {
  //
  // Clearing up the secure OTA client
  //

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
  this->secureOTAClient.setCACert(NULL);
  this->secureOTAClient.setCertificate(NULL);
  this->secureOTAClient.setPrivateKey(NULL);
#endif

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
  this->secureOTAClient.setBufferSizes(0, 0);
  this->secureOTAClient.setTrustAnchors(NULL);
  this->secureOTAClient.setClientRSACert(NULL, NULL);
#endif
}

bool BytebeamOTA::updateFirmware(char* otaPayloadStr, char* actionId) {
  char constructedUrl[BYTEBEAM_OTA_URL_STR_LEN] = { 0 };

  if(!parseOTAJson(otaPayloadStr, constructedUrl)) {
    BytebeamLogger::Error(__FILE__, __func__, "OTA failed, error while parsing the OTA json");
    return false;
  }

  if(!performOTA(actionId, constructedUrl)) {
    BytebeamLogger::Error(__FILE__, __func__, "OTA failed, error while performing HTTPS OTA");
    return false;
  }

  return true;
}

#endif /* BYTEBEAM_OTA_ENABLE */