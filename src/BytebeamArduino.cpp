#include "BytebeamArduino.h"

#if BYTEBEAM_OTA_ENABLE
  #include "BytebeamOTA.h"
#endif

/* This flag will prevent the use of handle actions api directly as this api is meant for internal usage
 * If you really want to call the handle actions api directly, probably you have to step into debug mode 
 */
bool handleActionFlag = false;

void SubscribeCallback(char* topic, byte* message, unsigned int length) {
  Serial.println("I am SubscribeCallback()");

  Serial.print("{topic : ");
  Serial.print(topic);
  Serial.print(", message : ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println("}");

  handleActionFlag = true;
  Bytebeam.handleActions((char*)message);
}

unsigned long long getMilliseconds() {
  const long  gmtOffset_sec = 19800;
  const int   daylightOffset_sec = 3600;
  const char* ntpServer = "pool.ntp.org";

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 
  
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  
  unsigned long long time = ((unsigned long long)now * 1000) + (millis() % 1000);
  return time;
}

boolean BytebeamArduino::subscribe(const char* topic) {
  /* skipping the connection checking here as we are not calling this function directly */
  // if(!PubSubClient::connected()) {
  //   Serial.printf("subscribe abort, topic : %s. bytebeam client is not connected to the server...\n", topic);
  //   return false;
  // }

  if(!PubSubClient::subscribe(topic)) {
    Serial.printf("subscribe failed, topic : %s\n", topic);
    return false;
  } else {
    Serial.printf("subscribe success, topic : %s\n", topic);
    return true;
  }
}

boolean BytebeamArduino::unsubscribe(const char* topic) {
  /* skipping the connection checking here as we are not calling this function directly */
  // if(!PubSubClient::connected()) {
  //   Serial.printf("unsubscribe abort, topic : %s. bytebeam client is not connected to the server...\n", topic);
  //   return false;
  // }

  if(!PubSubClient::unsubscribe(topic)) {
    Serial.printf("unsubscribe failed, topic : %s\n", topic);
    return false;
  } else {
    Serial.printf("unsubscribe success, topic : %s\n", topic);
    return true;
  }
}

boolean BytebeamArduino::publish(const char* topic, const char* payload) {
  /* skipping the connection checking here as we are not calling this function directly */
  // if(!PubSubClient::connected()) {
  //   Serial.printf("publish abort, topic : %s. bytebeam client is not connected to the server...\n", topic);
  //   return false;
  // }

  if(!PubSubClient::publish(topic, payload)) {
    Serial.printf("publish failed, {topic : %s, message : %s}\n", topic, payload);
    return false;
  } else {
    Serial.printf("publish success, {topic : %s, message : %s}\n", topic, payload);
    return true;
  }
}

boolean BytebeamArduino::subscribeToActions() {
  if(!PubSubClient::connected()) {
    Serial.printf("subscribe to actions abort, bytebeam client is not connected to the server...\n");
    return false;
  }

  int maxLen = 200;
  char topic[maxLen] = { 0 };
  int tempVar = snprintf(topic, maxLen, "/tenants/%s/devices/%s/actions", this->projectId, this->deviceId);

  if(tempVar > maxLen) {
    Serial.println("subscribe action topic size exceeded topic buffer size");
    return false;
  }

  return subscribe(topic);
}

boolean BytebeamArduino::unsubscribeToActions() {
  if(!PubSubClient::connected()) {
    Serial.printf("unsubscribe to actions abort, bytebeam client is not connected to the server...\n");
    return false;
  }

  int maxLen = 200;
  char topic[maxLen] = { 0 };
  int tempVar = snprintf(topic, maxLen, "/tenants/%s/devices/%s/actions", this->projectId, this->deviceId);

  if(tempVar > maxLen) {
    Serial.println("unsubscribe action topic size exceeded topic buffer size");
    return false;
  }

  return unsubscribe(topic);
}

boolean BytebeamArduino::publishActionStatus(char* actionId, int progressPercentage, char* status, char* error) {
  /* skipping the connection checking here as we are not calling this function directly */
  // if(!PubSubClient::connected()) {
  //   Serial.printf("publish action status abort, bytebeam client is not connected to the server...\n");
  //   return false;
  // }

  static int sequence = 0;
  const char* payload = "";
  String actionStatusStr = "";
  StaticJsonDocument<1024> doc;

  sequence++;
  unsigned long long milliseconds = getMilliseconds();

  JsonArray actionStatusJsonArray = doc.to<JsonArray>();
  JsonObject actionStatusJsonObj_1 = actionStatusJsonArray.createNestedObject();

  actionStatusJsonObj_1["timestamp"] = milliseconds;
  actionStatusJsonObj_1["sequence"]  = sequence;
  actionStatusJsonObj_1["state"]     = status;
  actionStatusJsonObj_1["errors"][0] = error;
  actionStatusJsonObj_1["id"]        = actionId;
  actionStatusJsonObj_1["progress"]  = progressPercentage;
  
  serializeJson(actionStatusJsonArray, actionStatusStr);
  payload = actionStatusStr.c_str();

  #if DEBUG_BYTEBEAM_ARDUINO
    Serial.println(payload);
  #endif
  
  int maxLen = 300;
  char topic[maxLen] = { 0 };
  int tempVar = snprintf(topic, maxLen,  "/tenants/%s/devices/%s/action/status", this->projectId, this->deviceId);

  if(tempVar > maxLen) {
    Serial.println("action status topic size exceeded topic buffer size");
    return false;
  }
  
  return publish(topic, payload);
}

boolean BytebeamArduino::readDeviceConfigFile() {
  if (!SPIFFS.begin(true, "/spiffs")) {
    Serial.println("spiffs mount failed");
    return false;
  } else {
    Serial.println("spiffs mount success");
  }

  const char * path = "/device_config.json";
  Serial.printf("Reading file : %s\n", path);

  fs::FS &fs = SPIFFS;
  File file = fs.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return false;
  }

  char chr = ' ';
  int strIndex = 0;

  Serial.println("- read from file");
  while (file.available()) {
    chr = file.read();
    this->deviceConfigStr[strIndex++] = chr;
  }

  file.close();
  SPIFFS.end();

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println("deviceConfigStr :");
  Serial.println(this->deviceConfigStr);
#endif

  return true;
}

boolean BytebeamArduino::parseDeviceConfigFile() {
  DeserializationError err = deserializeJson(this->deviceConfigJson, this->deviceConfigStr);
  if(err) {
    Serial.printf("deserializeJson() failed : %s\n", err.c_str());
    return false;
  } else {
    Serial.println("deserializeJson() success");
  }
  
  Serial.println("Obtaining device variables");

  this->mqttPort      = this->deviceConfigJson["port"];
  this->mqttBrokerUrl = this->deviceConfigJson["broker"];
  this->deviceId      = this->deviceConfigJson["device_id"];
  this->projectId     = this->deviceConfigJson["project_id"];
  this->caCertPem     = this->deviceConfigJson["authentication"]["ca_certificate"];
  this->clientCertPem = this->deviceConfigJson["authentication"]["device_certificate"];
  this->clientKeyPem  = this->deviceConfigJson["authentication"]["device_private_key"];
  
  const char* name[] = {"broker", "device_id", "project_id", "ca_certificate", "device_certificate", "device_private_key"};
  const char* args[] = {this->mqttBrokerUrl, this->deviceId, this->projectId, this->caCertPem, this->clientCertPem, this->clientKeyPem};
  int numArg = sizeof(args)/sizeof(args[0]);
  
  int argIterator = 0;
  for(argIterator = 0; argIterator < numArg; argIterator++) {
    if(args[argIterator] == NULL) {
      Serial.printf("- failed to obtain %s\n", name[argIterator]);
      return false;
    }
  }
  Serial.println("- obtain device variables");

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println(this->mqttPort);
  Serial.println(this->mqttBrokerUrl);
  Serial.println(this->deviceId);
  Serial.println(this->projectId);
  Serial.println(this->caCertPem);
  Serial.println(this->clientCertPem);
  Serial.println(this->clientKeyPem);
#endif

  return true;
}

boolean BytebeamArduino::setupBytebeamClient() {
  secureClient.setCACert(this->caCertPem);
  secureClient.setCertificate(this->clientCertPem); 
  secureClient.setPrivateKey(this->clientKeyPem);

  PubSubClient::setClient(secureClient);
  PubSubClient::setCallback(SubscribeCallback);
  PubSubClient::setServer(this->mqttBrokerUrl, this->mqttPort);

  return PubSubClient::connect("BytebeamClient");
}

BytebeamArduino::BytebeamArduino() {
  this->mqttPort = -1;
  this->mqttBrokerUrl = NULL;
  this->deviceId = NULL;
  this->projectId = NULL;
  this->caCertPem = NULL;
  this->clientCertPem = NULL;
  this->clientKeyPem = NULL;

  this->actionFuncsHandlerIdx = -1;
  resetActionHandlerArray();

  isClientActive = false;
}

BytebeamArduino::~BytebeamArduino() {
  Serial.println("I am BytebeamArduino::~BytebeamArduino()");
} 

boolean BytebeamArduino::begin() {
  if(!readDeviceConfigFile()) {
    Serial.println("begin abort, error while reading the device config file...");
    return false;
  }
            
  if(!parseDeviceConfigFile()) {
    Serial.println("begin abort, error while parsing the device config file...");
    return false;
  }

#if BYTEBEAM_OTA_ENABLE  
  BytebeamOta.retrieveOTAInfo();

  if(!BytebeamOta.otaUpdateFlag) {
    Serial.println("RESTART: normal reboot !");
  } else {
    Serial.println("RESTART: reboot after successfull OTA update !");
  }
#endif

  Serial.printf("connecting to bytebeam server, project_id is %s and device_id is %s : ", this->projectId, this->deviceId);
  if(!setupBytebeamClient()) {
    Serial.println("begin abort, error while connecting to server...");
    return false;
  }
  Serial.println("connected"); 

#if BYTEBEAM_OTA_ENABLE  
  if(BytebeamOta.otaUpdateFlag) {
    if(!Bytebeam.publishActionStatus(BytebeamOta.otaActionId, 100, "Completed", "OTA Success")) {
      Serial.println("failed to publish ota complete status...");
    }
    BytebeamOta.otaUpdateFlag = false;
    strcpy(BytebeamOta.otaActionId, "");

    BytebeamOta.clearOTAInfo();
  }
#endif

  if(!subscribeToActions()) {
    Serial.println("begin abort, error while subscribe to actions...");
    return false;
  }

  isClientActive = true;
  Serial.println("bytebeam client activated successfully !"); 

#if BYTEBEAM_OTA_ENABLE
  if(!handleOTA()) {
    Serial.println("begin abort, error while handling OTA...");
    return false;
  }
#else 
  Serial.println("Skipped Bytebeam OTA from compilation phase i.e saving flash size");
#endif

  return true;
}

boolean BytebeamArduino::loop() {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::loop() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

#if DEBUG_BYTEBEAM_ARDUINO
  static int counter = 0;
  counter++;
  if(counter == 10) { 
    PubSubClient::disconnect(); 
  }
#endif

  Serial.println("I am BytebeamArduino::loop()");
  if(!PubSubClient::connected()) {
    Serial.println("bytebean client connection broken");

    int tryCount = 0;
    while(!PubSubClient::connected()) {
      if(tryCount == BYTEBEAM_CONNECT_MAX_RETRIES) {
        Serial.println("Maximum attempt limit reached, can't reconnect to the server...");
        return false;
      }
     
      tryCount++;
      Serial.printf("Attempt : %d, trying to reconnect to the server : ", tryCount);

      if(!PubSubClient::connect("BytebeamClient")) {
        Serial.println("error while reconnecting to the server...");
        delay(1000);
      } else {
        Serial.println("connected"); 
        if(!subscribeToActions()) {
          Serial.println("error while subscribe to actions...");
          return false;
        }
      }
    }
  }

  return PubSubClient::loop();
}

boolean BytebeamArduino::connected() {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::connected() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  if(!PubSubClient::connected()) {
    Serial.println("bytebeam client is not connected to the server !");
    return false;
  } else {
    Serial.println("bytebeam client is connected to the server !");
    return true;
  }
}

boolean BytebeamArduino::handleActions(char* actionReceivedStr) {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::handleActions() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println(actionReceivedStr);
#else  
  if(!handleActionFlag) {
    Serial.println("I am BytebeamArduino::handleActions()");
    Serial.println("This api is meant to handle the actions that comes from the cloud as a subsciption packet");
    Serial.println("You can not call this api directly, If you are devloper step into debug mode");
    return false;
  }
#endif
  
  StaticJsonDocument<1024> actionReceivedJson;
  DeserializationError err = deserializeJson(actionReceivedJson, actionReceivedStr);
  
  if(err) {
    Serial.printf("deserializeJson() failed : %s\n", err.c_str());
    return false;
  } else {
    Serial.println("deserializeJson() success");
  }
  
  Serial.println("Obtaining action variables");
  
  const char* name     = actionReceivedJson["name"];
  const char* id       = actionReceivedJson["id"];
  const char* payload  = actionReceivedJson["payload"];
  const char* kind     = actionReceivedJson["kind"];
  
  const char* argsName[] = {"name", "id", "payload", "kind"};
  const char* argsStr[] = {name, id, payload, kind};
  int numArg = sizeof(argsStr)/sizeof(argsStr[0]);
  
  int argIterator = 0;
  for(argIterator = 0; argIterator < numArg; argIterator++) {
    if(argsStr[argIterator] == NULL) {
      Serial.printf("- failed to obtain %s\n", argsName[argIterator]);
      return false;
    }
  }
  Serial.println("- obtain action variables");

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println(name);
  Serial.println(id);
  Serial.println(payload);
  Serial.println(kind);
#endif

  int actionIterator = 0;
  while(this->actionFuncs[actionIterator].name) {
    if (!strcmp(this->actionFuncs[actionIterator].name, name)) {
        this->actionFuncs[actionIterator].func((char*)payload, (char*)id);
        break;
    }
    actionIterator++;
  }

  if(this->actionFuncs[actionIterator].name == NULL) {
    Serial.printf("invalid action : %s\n", name);
  }

  handleActionFlag = false;
  return true;
}

boolean BytebeamArduino::addActionHandler(int (*funcPtr)(char* args, char* actionId), char* actionName) {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::addActionHandler() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  if(this->actionFuncsHandlerIdx + 1 >= BYTEBEAM_NUMBER_OF_ACTIONS) {
    Serial.println("maximum actions limit reached, can't create new action at the moment");
    return false;
  }

  int actionIterator = 0;
  for(actionIterator = 0; actionIterator <= this->actionFuncsHandlerIdx; actionIterator++) {
    if(!strcmp(this->actionFuncs[actionIterator].name, actionName)) {
      Serial.printf("action : %s is already there at index %d, update the action instead\n", actionName, actionIterator);
      return false;
    }
  }

  this->actionFuncsHandlerIdx += 1;
  this->actionFuncs[this->actionFuncsHandlerIdx].func = funcPtr;
  this->actionFuncs[this->actionFuncsHandlerIdx].name = actionName;

  return true;
}

boolean BytebeamArduino::removeActionHandler(char* actionName) {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::removeActionHandler() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  int actionIterator = 0;
  int targetActionIdx = -1;

  for(actionIterator = 0; actionIterator <= this->actionFuncsHandlerIdx; actionIterator++) {
    if(!strcmp(this->actionFuncs[actionIterator].name, actionName)) {
      targetActionIdx = actionIterator;
    }
  }

  if(targetActionIdx == -1) {
    Serial.printf("action : %s not found \n", actionName);
    return false;
  } else {
    for(actionIterator = targetActionIdx; actionIterator != this->actionFuncsHandlerIdx; actionIterator++) {
      this->actionFuncs[actionIterator].func = this->actionFuncs[actionIterator+1].func;
      this->actionFuncs[actionIterator].name = this->actionFuncs[actionIterator+1].name;
    }

    this->actionFuncs[this->actionFuncsHandlerIdx].func = NULL;
    this->actionFuncs[this->actionFuncsHandlerIdx].name = NULL;
    this->actionFuncsHandlerIdx -= 1;
    return true;
  }
}

boolean BytebeamArduino::updateActionHandler(int (*newFuncPtr)(char* args, char* actionId), char* actionName) {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::updateActionHandler ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  int actionIterator = 0;
  int targetActionIdx = -1;

  for(actionIterator = 0; actionIterator <= this->actionFuncsHandlerIdx; actionIterator++) {
    if(!strcmp(this->actionFuncs[actionIterator].name, actionName)) {
      targetActionIdx = actionIterator;
    }
  }

  if(targetActionIdx == -1) {
    Serial.printf("action : %s not found\n", actionName);
    return false;
  } else {
    this->actionFuncs[targetActionIdx].func = newFuncPtr;
    return true;
  }
}

void BytebeamArduino::printActionHandlerArray() {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::printActionHandlerArray() ---> bytebeam client is not active yet, begin the bytebeam client");
    return;
  }

  int actionIterator = 0;

  Serial.println("["); 
  for(actionIterator = 0; actionIterator < BYTEBEAM_NUMBER_OF_ACTIONS; actionIterator++) {
    if(this->actionFuncs[actionIterator].name != NULL) {
      Serial.printf("       { %s : %s }        \n", this->actionFuncs[actionIterator].name, "*******");
    } else {
      Serial.printf("       { %s : %s }        \n", "NULL", "NULL");
    }
  }
  Serial.println("]"); 
}

void BytebeamArduino::resetActionHandlerArray() {
  int loopVar = 0;
  for (loopVar = 0; loopVar < BYTEBEAM_NUMBER_OF_ACTIONS; loopVar++) {
    this->actionFuncs[loopVar].func = NULL;
    this->actionFuncs[loopVar].name = NULL;
  }
}

boolean BytebeamArduino::publishActionCompleted(char* actionId) {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::publishActionCompleted() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  if(!PubSubClient::connected()) {
    Serial.printf("publish action completed abort, bytebeam client is not connected to the server...\n");
    return false;
  }

  char tempActionId[20];
  strcpy(tempActionId, actionId);

  if(!publishActionStatus(tempActionId, 100, "Completed", "No Error")) {
    Serial.printf("publish action completed response failed, action_id : %s\n", tempActionId);
    return false;
  } else {
    Serial.printf("publish action completed response success, action_id : %s\n", tempActionId);
    return true;
  }
}

boolean BytebeamArduino::publishActionFailed(char* actionId) {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::publishActionFailed() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  if(!PubSubClient::connected()) {
    Serial.printf("publish action failed abort, bytebeam client is not connected to the server...\n");
    return false;
  }

  char tempActionId[20];
  strcpy(tempActionId, actionId);

  if(!publishActionStatus(tempActionId, 0, "Failed", "Action Failed")) {
    Serial.printf("publish action failed response failed, action_id : %s\n", tempActionId);
    return false;
  } else {
    Serial.printf("publish action failed response success, action_id : %s\n", tempActionId);
    return true;
  }
}

boolean BytebeamArduino::publishActionProgress(char* actionId, int progressPercentage) {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::publishActionProgress() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  if(!PubSubClient::connected()) {
    Serial.printf("publish action progress abort, bytebeam client is not connected to the server...\n");
    return false;
  }

  char tempActionId[20];
  strcpy(tempActionId, actionId);

  if(!publishActionStatus(tempActionId, progressPercentage, "Progress", "No Error")) {
    Serial.printf("publish action progress response failed, action_id : %s\n", tempActionId);
    return false;
  } else {
    Serial.printf("publish action progress response success, action_id : %s\n", tempActionId);
    return true;
  }
}

boolean BytebeamArduino::publishToStream(char* streamName, const char* payload) {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::publishToStream() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  if(!PubSubClient::connected()) {
    Serial.printf("publish to stream abort, bytebeam client is not connected to the server...\n");
    return false;
  }

  int maxLen = 200;
  char topic[maxLen] = { 0 };
  int tempVar = snprintf(topic, maxLen,  "/tenants/%s/devices/%s/events/%s/jsonarray", this->projectId, this->deviceId, streamName);

  if(tempVar > maxLen) {
    Serial.println("publish topic size exceeded buffer size");
    return false;
  }

  return publish(topic, payload);
}

void BytebeamArduino::end() {
  if(!isClientActive) {
    Serial.println("BytebeamArduino::end() ---> bytebeam client is not active yet, begin the bytebeam client");
    return;
  }

  this->mqttPort = -1;
  this->mqttBrokerUrl = NULL;
  this->deviceId = NULL;
  this->projectId = NULL;
  this->caCertPem = NULL;
  this->clientCertPem = NULL;
  this->clientKeyPem = NULL;

  this->actionFuncsHandlerIdx = -1;
  resetActionHandlerArray();

  isClientActive = false;
  PubSubClient::disconnect();
  Serial.println("bytebeam client deactivated successfully !");
}

#if BYTEBEAM_OTA_ENABLE
  int handleFirmwareUpdate(char* otaPayloadStr, char* actionId) {
    char constructedUrl[200] = { 0 };

    if(!BytebeamOta.parseOTAJson(otaPayloadStr, constructedUrl)) {
      Serial.println("ota abort, error while parsing the ota json...");
      return false;
    }

    if(!BytebeamOta.performOTA(actionId, constructedUrl)) {
      Serial.println("ota abort, error while performing https ota...");
      return false;
    }

    return 0;
  }

  boolean BytebeamArduino::handleOTA() {
    BytebeamOta.secureOtaClient.setCACert(this->caCertPem);
    BytebeamOta.secureOtaClient.setCertificate(this->clientCertPem); 
    BytebeamOta.secureOtaClient.setPrivateKey(this->clientKeyPem);

    return Bytebeam.addActionHandler(handleFirmwareUpdate, "update_firmware");
  }

  boolean BytebeamArduino::disableOTA() {
    if(!removeActionHandler("update_firmware")) {
      Serial.println("OTA disable fail !");
      return false;
    } else {
      Serial.println("OTA disable success !");
      return true;
    }
  }
#endif

BytebeamArduino Bytebeam;