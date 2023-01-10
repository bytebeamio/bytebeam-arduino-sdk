#include "BytebeamOTA.h"

static char tempOtaActionId[OTA_ACTION_ID_STR_LEN] = "";

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
  HTTPUpdate& BytebeamUpdate =  httpUpdate;
#endif

void HTTPUpdateStarted() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void HTTPUpdateFinished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}

void HTTPUpdateProgress(int cur, int total) {
  static int loopVar = 0;
  static int percentOffset = 10;
  static int progressPercent = 0;

  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
  progressPercent = (((float)cur / (float)total) * 100.00);

  if(progressPercent == loopVar ) {
    #if DEBUG_BYTEBEAM_OTA
      Serial.println(progressPercent);
      Serial.println(BytebeamOta.otaActionId);
    #endif

    if(loopVar == 100) {
      if(!Bytebeam.publishActionCompleted(tempOtaActionId)) {
        Serial.println("failed to publish ota progress status...");
      }
      loopVar = 0;
      progressPercent = 0;
    } else {
      if(!Bytebeam.publishActionProgress(tempOtaActionId, progressPercent)) {
        Serial.println("failed to publish ota progress status...");
      }
      loopVar = loopVar + percentOffset;
    }
  }
}

void HTTPUpdateError(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void rebootEspWithReason() {
  Serial.println("RESTART: booting new firmware !");
  delay(3000);
  ESP.restart();
}

BytebeamOTA::BytebeamOTA() {
  this->otaUpdateFlag = false;
  strcpy(this->otaActionId, "");
}

BytebeamOTA::~BytebeamOTA() {
  Serial.println("I am BytebeamOTA::~BytebeamOTA()");
}

void BytebeamOTA::saveOTAInfo() {
  Preferences preferences;

  preferences.begin("OTAInfo");
  preferences.putBool("OTAUpdateFlag", this->otaUpdateFlag);
  preferences.putString("OTAActionId", this->otaActionId);
  preferences.end();

  Serial.println("NVS: Saving OTA Information");
}

void BytebeamOTA::retrieveOTAInfo() {
  Preferences preferences;

  preferences.begin("OTAInfo");
  this->otaUpdateFlag = preferences.getBool("OTAUpdateFlag");
  preferences.getString("OTAActionId", this->otaActionId, OTA_ACTION_ID_STR_LEN);
  preferences.end();

  Serial.println("NVS: Retrieving OTA Information");
}

void BytebeamOTA::clearOTAInfo() {
  Preferences preferences;

  preferences.begin("OTAInfo");
  preferences.clear();
  preferences.end();

  Serial.println("NVS: Clearing OTA Information");
}

boolean BytebeamOTA::parseOTAJson(char* otaPayloadStr, char* urlStringReturn) {
  StaticJsonDocument<1024> otaPayloadJson;
  DeserializationError err = deserializeJson(otaPayloadJson, otaPayloadStr);

  if(err) {
    Serial.printf("deserializeJson() failed : %s\n", err.c_str());
    return false;
  } else {
    Serial.println("deserializeJson() success");
  }
  
  Serial.println("Obtaining ota variables");
  
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
      Serial.printf("- failed to obtain %s\n", argsName[argIterator]);
      return false;
    }
  }
  Serial.println("- obtain ota variables");

#if DEBUG_BYTEBEAM_OTA
  Serial.println(length);
  Serial.println(status);
  Serial.println(url);
  Serial.println(version);
#endif

  int maxLen = 200;
  int tempVar = snprintf(urlStringReturn, maxLen,  "%s", url);

  if(tempVar > maxLen) {
    Serial.println("firmware upgrade url size exceeded buffer size");
    return false;
  }

  Serial.printf("constructed url is %s\n", url);
  return true;
}

boolean BytebeamOTA::performOTA(char* actionId, char* otaUrl) {
  Serial.println("Performing OTA...");

  this->otaUpdateFlag = true;
  strcpy(this->otaActionId, actionId);
  strcpy(tempOtaActionId, actionId);

  /* set the status led pin and disable the auto reboot, we will manually reboot after saving some information */
  BytebeamUpdate.rebootOnUpdate(false);
  BytebeamUpdate.setLedPin(BYTEBEAM_OTA_LED, LOW);

  /* set the update callbacks */
  BytebeamUpdate.onStart(HTTPUpdateStarted);
  BytebeamUpdate.onEnd(HTTPUpdateFinished);
  BytebeamUpdate.onProgress(HTTPUpdateProgress);
  BytebeamUpdate.onError(HTTPUpdateError);

  t_httpUpdate_return ret = BytebeamUpdate.update(this->secureOTAClient, otaUrl);

  switch (ret) {
    case HTTP_UPDATE_FAILED:

      /* If update failed then we will reach here, just log the error and send failure message to the server */
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", BytebeamUpdate.getLastError(), BytebeamUpdate.getLastErrorString().c_str());
      if(!Bytebeam.publishActionFailed(this->otaActionId)) {
        Serial.println("failed to publish negative response for firmware upgarde failure...");
      }
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");

      /* If update is successfull then we will reach here, just save the OTA information and reboot the chip */
      saveOTAInfo();
      rebootEspWithReason();
      break;
  }

  if(ret != HTTP_UPDATE_OK) {
    this->otaUpdateFlag = false;
    strcpy(this->otaActionId, "");
    strcpy(tempOtaActionId, "");
    return false;
  }

  return true;
}