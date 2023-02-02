#include "BytebeamArduino.h"

/* This object will represent the Time library, We will be exposing the necessary functionality and info
 * for the usage, If you want to do any Timing related stuff this guy is for you.
 */
static BytebeamTime BytebeamTime;

/* This object will represent the OTA library, We will be exposing the necessary functionality and info
 * for the usage, If you want to do any OTA stuff this guy is for you.
 */
static BytebeamOTA BytebeamOTA;

/* This flag will prevent the use of handle actions api directly as this api is meant for internal usage
 * If you really want to call the handle actions api directly, probably you have to step into debug mode 
 */
static bool handleActionFlag = false;

static void BytebeamActionsCallback(char* topic, byte* message, unsigned int length) {
  Serial.println("I am SubscribeCallback()");

  // log the recieved action to serial :)
  Serial.print("{topic : ");
  Serial.print(topic);
  Serial.print(", message : ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println("}");

  // set the handle action flag
  handleActionFlag = true;

  // handle the received action
  Bytebeam.handleActions((char*)message);
}

static void rebootESPWithReason(char* reason) {
  Serial.println(reason);
  delay(3000);
  ESP.restart();
}

void BytebeamArduino::printArchitectureInfo() {
  //
  // log the usefull architecture information to serial :)
  //

  Serial.println("* ********************************************************************* *");

#if defined(BYTEBEAM_ARDUINO_ARCH_ESP32)
  // log esp32 arch info to serial :)
  Serial.printf("*\t\t\t Architecture : ESP32 \t\t\t\t*\n");
  Serial.printf("*\t\t\t Chip Model   : %s \t\t\t*\n", ESP.getChipModel());
  Serial.printf("*\t\t\t CPU Freq     : %d MHz \t\t\t*\n", ESP.getCpuFreqMHz());
  Serial.printf("*\t\t\t Flash Size   : %d MB \t\t\t\t*\n", ((ESP.getFlashChipSize())/1024)/1024);
  Serial.printf("*\t\t\t Free Heap    : %d KB \t\t\t\t*\n", (ESP.getFreeHeap())/1024);
  Serial.printf("*\t\t\t SDK Version  : %s \t\t\t\t*\n", ESP.getSdkVersion());
#elif defined(BYTEBEAM_ARDUINO_ARCH_ESP8266)
  // log esp8266 arch info to serial :)
  Serial.printf("*\t\t\t Architecture : ESP8266 \t\t\t*\n");
  Serial.printf("*\t\t\t Chip Id      : %d \t\t\t*\n", ESP.getChipId());
  Serial.printf("*\t\t\t CPU Freq     : %d MHz \t\t\t*\n", ESP.getCpuFreqMHz());
  Serial.printf("*\t\t\t Flash Size   : %d MB \t\t\t\t*\n", ((ESP.getFlashChipSize())/1024)/1024);
  Serial.printf("*\t\t\t Free Heap    : %d KB \t\t\t\t*\n", (ESP.getFreeHeap())/1024);
  Serial.printf("*\t\t\t SDK Version  : %s \t\t*\n", ESP.getSdkVersion());
  Serial.printf("*\t\t\t Core Version : %s \t\t\t\t*\n", ESP.getCoreVersion());
#else
  // log unknown arch info to serial :)
  Serial.println("Unknown Architecture");
#endif

  Serial.println("* ********************************************************************* *");
}

void BytebeamArduino::initActionHandlerArray() {
  //
  // Initailize the action handler array with default values i.e NULL for pointers
  //

  int loopVar = 0;
  for (loopVar = 0; loopVar < BYTEBEAM_NUMBER_OF_ACTIONS; loopVar++) {
    this->actionFuncs[loopVar].func = NULL;
    this->actionFuncs[loopVar].name = NULL;
  }

  this->actionFuncsHandlerIdx = -1;
}

boolean BytebeamArduino::subscribe(const char* topic, uint8_t qos) {
  //
  // skipping the connection checking here as we are not calling this function directly
  //

  if(!PubSubClient::subscribe(topic, qos)) {
    Serial.printf("subscribe failed, topic : %s and qos : %d\n", topic, qos);
    return false;
  } else {
    Serial.printf("subscribe success, topic : %s and qos : %d\n", topic, qos);
    return true;
  }
}

boolean BytebeamArduino::unsubscribe(const char* topic) {
  //
  // skipping the connection checking here as we are not calling this function directly
  //

  if(!PubSubClient::unsubscribe(topic)) {
    Serial.printf("unsubscribe failed, topic : %s\n", topic);
    return false;
  } else {
    Serial.printf("unsubscribe success, topic : %s\n", topic);
    return true;
  }
}

boolean BytebeamArduino::publish(const char* topic, const char* payload) {
  //
  // skipping the connection checking here as we are not calling this function directly
  //

  if(!PubSubClient::publish(topic, payload)) {
    Serial.printf("publish failed, {topic : %s, message : %s}\n", topic, payload);
    return false;
  } else {
    Serial.printf("publish success, {topic : %s, message : %s}\n", topic, payload);
    return true;
  }
}

boolean BytebeamArduino::subscribeToActions() {
  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    Serial.printf("subscribe to actions failed, bytebeam client is not connected to the cloud\n");
    return false;
  }

  uint8_t qos = 1;
  char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = { 0 };

  int maxLen = BYTEBEAM_MQTT_TOPIC_STR_LEN;
  int tempVar = snprintf(topic, maxLen, "/tenants/%s/devices/%s/actions", this->projectId, this->deviceId);

  if(tempVar >= maxLen) {
    Serial.println("subscribe action topic size exceeded topic buffer size");
    return false;
  }

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println(qos);
  Serial.println(topic);
#endif

  return subscribe(topic, qos);
}

boolean BytebeamArduino::unsubscribeToActions() {
  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    Serial.printf("unsubscribe to actions abort, bytebeam client is not connected to the cloud\n");
    return false;
  }

  char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = { 0 };

  int maxLen = BYTEBEAM_MQTT_TOPIC_STR_LEN;
  int tempVar = snprintf(topic, maxLen, "/tenants/%s/devices/%s/actions", this->projectId, this->deviceId);

  if(tempVar >= maxLen) {
    Serial.println("unsubscribe action topic size exceeded topic buffer size");
    return false;
  }

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println(topic);
#endif

  return unsubscribe(topic);
}

boolean BytebeamArduino::publishActionStatus(char* actionId, int progressPercentage, char* status, char* error) {
  //
  // skipping the connection checking here as we are not calling this function directly
  //

  static int sequence = 0;
  const char* payload = "";
  String actionStatusStr = "";
  StaticJsonDocument<1024> doc;
  char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = { 0 };

  // get the current epoch millis
  if(!BytebeamTime.getEpochMillis()) {
      Serial.println("failed to get epoch millis");
      return false;
  }

  sequence++;
  unsigned long long milliseconds = BytebeamTime.nowMillis;

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

  int maxLen = BYTEBEAM_MQTT_TOPIC_STR_LEN;
  int tempVar = snprintf(topic, maxLen,  "/tenants/%s/devices/%s/action/status", this->projectId, this->deviceId);

  if(tempVar >= maxLen) {
    Serial.println("action status topic size exceeded topic buffer size");
    return false;
  }

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println(topic);
  Serial.println(payload);
#endif

  return publish(topic, payload);
}

#ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FS
  boolean BytebeamArduino::readDeviceConfigFile() {

    /* This file system pointer will store the address of the selected file system, So after begin and end operations
     * we can utilize this file system pointer to do the file operations.
     */
    fs::FS* ptrToFS = NULL;

    switch(DEVICE_CONFIG_FILE_SYSTEM) {

    /* We need to do conditional compilation here beacuse different architecture supports different file systems
     * So based on the architecture we should define the flags for the supported file system.
     */
  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FATFS
      case FATFS_FILE_SYSTEM:
        Serial.println("FATFS file system detected !");

        // initalize the FATFS file system
        if(!FFat.begin()) {
          Serial.println("FATFS mount failed");
          return false;
        }

        // set the file system pointer to FATFS Object
        ptrToFS = &FFat;

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SPIFFS
      case SPIFFS_FILE_SYSTEM:
        Serial.println("SPIFFS file system detected !");

        // initalize the SPIFFS file system
        if(!SPIFFS.begin()) {
          Serial.println("SPIFFS mount failed");
          return false;
        }

        // set the file system pointer to SPIFFS Object
        ptrToFS = &SPIFFS;

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_LITTLEFS
      case LITTLEFS_FILE_SYSTEM:
        Serial.println("LITTLEFS file system detected !");

        // Just print the log and return :)
        Serial.println("LITTLEFS is not supported by the library yet");
        return false;

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SD
      case SD_FILE_SYSTEM:
        Serial.println("SD file system detected !");

        // Just print the log and return :)
        Serial.println("SD is not supported by the library yet");
        return false;

        break;
  #endif
  

      default:
        Serial.println("Unknown file system detected !");

        // Just print the log and return :)
        Serial.println("Make sure the architecture supports the selcted file system");
        return false;

        break;
    }

    const char* path = DEVICE_CONFIG_FILE_NAME;
    Serial.printf("Reading file : %s\n", path);

    File file = ptrToFS->open(path, FILE_READ);
    if (!file || file.isDirectory()) {
      Serial.println("- failed to open device config file for reading");
      return false;
    }

    char chr = ' ';
    int strIndex = 0;
    int strSize = 0;

    strSize = file.size() + 1;

    if(strSize <= 1) {
      Serial.println("failed to get device config file size");
      return false;
    }

    this->deviceConfigStr = (char*) malloc(strSize);

    // if memory allocation fails just log the failure to serial and return :)
    if(this->deviceConfigStr == NULL) {
      Serial.println("failed to allocate the memory for device config file");
      return false;
    }

    Serial.println("- read from file");
    while (file.available()) {
      chr = file.read();
      this->deviceConfigStr[strIndex++] = chr;
    }

    file.close();

    switch(DEVICE_CONFIG_FILE_SYSTEM) {

    /* We need to do conditional compilation here beacuse different architecture supports different file systems
     * So based on the architecture we should define the flags for the supported file system.
     */
  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FATFS
      case FATFS_FILE_SYSTEM:
        // de-initalize the FATFS file system
        FFat.end();

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SPIFFS
      case SPIFFS_FILE_SYSTEM:
        // de-initalize the SPIFFS file system
        SPIFFS.end();

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_LITTLEFS
      case LITTLEFS_FILE_SYSTEM:
        // de-initalize the LITTLEFS file system
        // nothing to do here yet

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SD
      case SD_FILE_SYSTEM:
        // de-initalize the SD file system
        // nothing to do here yet

        break;
  #endif

      default:
        Serial.println("Unknown file system detected !");

        // Just print the log and return :)
        Serial.println("Make sure the architecture supports the selcted file system");
        return false;

        break;
    }

  #if DEBUG_BYTEBEAM_ARDUINO
    Serial.println("deviceConfigStr :");
    Serial.println(this->deviceConfigStr);
  #endif

    return true;
  }
#endif

boolean BytebeamArduino::parseDeviceConfigFile() {
  // before going ahead make sure you are parsing something
  if(this->deviceConfigStr == NULL) {
    Serial.println("device config file is empty");
    return false;
  }

  StaticJsonDocument<1024> deviceConfigJson;
  DeserializationError err = deserializeJson(deviceConfigJson, this->deviceConfigStr);

  if(err) {
    Serial.printf("deserializeJson() failed : %s\n", err.c_str());
    return false;
  } else {
    Serial.println("deserializeJson() success");
  }
  
  Serial.println("Obtaining device config file variables");

  this->mqttPort      = deviceConfigJson["port"];
  this->mqttBrokerUrl = deviceConfigJson["broker"];
  this->deviceId      = deviceConfigJson["device_id"];
  this->projectId     = deviceConfigJson["project_id"];
  this->caCertPem     = deviceConfigJson["authentication"]["ca_certificate"];
  this->clientCertPem = deviceConfigJson["authentication"]["device_certificate"];
  this->clientKeyPem  = deviceConfigJson["authentication"]["device_private_key"];
  this->clientId      = "BytebeamClient";
  
  const char* name[] = {"broker", "device_id", "project_id", "ca_certificate", "device_certificate", "device_private_key", "clientId"};
  const char* args[] = {this->mqttBrokerUrl, this->deviceId, this->projectId, this->caCertPem, this->clientCertPem, this->clientKeyPem, this->clientId};
  int numArg = sizeof(args)/sizeof(args[0]);
  
  int argIterator = 0;
  for(argIterator = 0; argIterator < numArg; argIterator++) {
    if(args[argIterator] == NULL) {
      Serial.printf("- failed to obtain %s\n", name[argIterator]);
      return false;
    }
  }
  Serial.println("- obtain device config file variables");

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println(this->mqttPort);
  Serial.println(this->mqttBrokerUrl);
  Serial.println(this->deviceId);
  Serial.println(this->projectId);
  Serial.println(this->caCertPem);
  Serial.println(this->clientCertPem);
  Serial.println(this->clientKeyPem);
  Serial.println(this->clientId);
#endif

  Serial.printf("Project Id : %s and Device Id : %s\n", this->projectId, this->deviceId);

  return true;
}

boolean BytebeamArduino::setupBytebeamClient() {

  /* Setting up the bytebeam secure wifi client based on the architecture and before this make sure you have
   * the same secure wifi client object inside the class defination.
   */
#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
  this->secureClient.setCACert(this->caCertPem);
  this->secureClient.setCertificate(this->clientCertPem);
  this->secureClient.setPrivateKey(this->clientKeyPem);
#endif

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
  this->rootCA = new BearSSL::X509List(this->caCertPem);

  // if memory allocation fails just log the failure to serial and return :)
  if(this->rootCA == NULL) {
    Serial.println("failed to allocate the memory for root ca");
    return false;
  }

  this->clientCert = new BearSSL::X509List(this->clientCertPem);

  // if memory allocation fails just log the failure to serial and return :)
  if(this->clientCert == NULL) {
    Serial.println("failed to allocate the memory for client cert");
    return false;
  }

  this->clientKey = new BearSSL::PrivateKey(this->clientKeyPem);

  // if memory allocation fails just log the failure to serial and return :)
  if(this->clientKey == NULL) {
    Serial.println("failed to allocate the memory for client key");
    return false;
  }

  this->secureClient.setBufferSizes(2048, 2048);
  this->secureClient.setTrustAnchors(this->rootCA);
  this->secureClient.setClientRSACert(this->clientCert, this->clientKey);
#endif

  PubSubClient::setClient(this->secureClient);
  PubSubClient::setCallback(BytebeamActionsCallback);
  PubSubClient::setServer(this->mqttBrokerUrl, this->mqttPort);

  Serial.print("Connecting To Bytebeam Cloud : ");

  if(!PubSubClient::connect(this->clientId)) {
    Serial.println("ERROR");
    return false;
  }

  Serial.println("CONNECTED");

  return true;
}

void BytebeamArduino::clearBytebeamClient() {

  /* Clearing up the bytebeam secure wifi client based on the architecture and before this make sure you have
   * the same secure wifi client object inside the class defination.
   */
#ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
  this->secureClient.setCACert(NULL);
  this->secureClient.setCertificate(NULL);
  this->secureClient.setPrivateKey(NULL);
#endif

#ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
  // release the allocated memory :)
  delete this->rootCA;
  this->rootCA = NULL;

  // release the allocated memory :)
  delete this->clientCert;
  this->clientCert = NULL;

  // release the allocated memory :)
  delete this->clientKey;
  this->clientKey = NULL;

  this->secureClient.setBufferSizes(0, 0);
  this->secureClient.setTrustAnchors(NULL);
  this->secureClient.setClientRSACert(NULL, NULL);
#endif

  PubSubClient::setClient(this->secureClient);
  PubSubClient::setCallback(NULL);
  PubSubClient::setServer(this->mqttBrokerUrl, this->mqttPort);

  Serial.print("Disconnecting To Bytebeam Cloud : ");

  PubSubClient::disconnect();

  Serial.println("DISCONNECTED");
}

BytebeamArduino::BytebeamArduino() {
  //
  // Initailizing all the variables with default values here
  //

  this->mqttPort = -1;
  this->mqttBrokerUrl = NULL;
  this->deviceId = NULL;
  this->projectId = NULL;
  this->caCertPem = NULL;
  this->clientCertPem = NULL;
  this->clientKeyPem = NULL;
  this->clientId = NULL;

  this->deviceConfigStr = NULL;

  initActionHandlerArray();

  this->isOTAEnable = false;
  this->isClientActive = false;
}

BytebeamArduino::~BytebeamArduino() {
  //
  // Nothing much to do here, just print the log to serial :)
  //

  Serial.println("I am BytebeamArduino::~BytebeamArduino()");
} 

boolean BytebeamArduino::begin() {
  // It's much better to pump up architecture inforamtion in the very beginning
  printArchitectureInfo();

  if(!BytebeamTime.begin()) {
    Serial.println("begin abort, time client begin failed...\n");
    return false;
  }

#ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FS
  if(!readDeviceConfigFile()) {
    Serial.println("begin abort, error while reading the device config file...\n");
    return false;
  }
#else
  Serial.println("Architecture doesn't support file system");
#endif
            
  if(!parseDeviceConfigFile()) {
    Serial.println("begin abort, error while parsing the device config file...\n");
    return false;
  }

#if BYTEBEAM_OTA_ENABLE  
  BytebeamOTA.retrieveOTAInfo();

  if(!BytebeamOTA.otaUpdateFlag) {
    Serial.println("RESTART: Normal Reboot !");
  } else {
    Serial.println("RESTART: Reboot After Successfull OTA Update !");
  }
#endif

  if(!setupBytebeamClient()) {
    Serial.println("begin abort, error while connecting to server...\n");
    return false;
  }

#if BYTEBEAM_OTA_ENABLE  
  if(BytebeamOTA.otaUpdateFlag) {
    if(!publishActionStatus(BytebeamOTA.otaActionId, 100, "Completed", "OTA Success")) {
      Serial.println("failed to publish ota complete status...");
    }

    BytebeamOTA.clearOTAInfoFromRAM();
    BytebeamOTA.clearOTAInfoFromFlash();
  }
#else
  Serial.println("Skipped Bytebeam OTA from compilation phase i.e saving flash size");
#endif

  if(!subscribeToActions()) {
    Serial.println("begin abort, error while subscribe to actions...\n");
    return false;
  }

  this->isClientActive = true;

  Serial.println("Bytebeam Client Activated Successfully !\n");

  return true;
}

boolean BytebeamArduino::loop() {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
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

  if(!PubSubClient::connected()) {
    Serial.println("Bytebean Client Connection Broken");

    int tryCount = 0;
    while(!PubSubClient::connected()) {
      if(tryCount == BYTEBEAM_CONNECT_MAX_RETRIES) {
        Serial.println("Maximum attempt limit reached, can't reconnect to the cloud");
        return false;
      }
     
      tryCount++;
      Serial.printf("Attempt : %d, Trying To Reconnect To The Cloud : ", tryCount);

      if(!PubSubClient::connect("BytebeamClient")) {
        Serial.println("ERROR");
        delay(1000);
      } else {
        Serial.println("CONNECTED");
        if(!subscribeToActions()) {
          Serial.println("Subscribe to actions failed");
          return false;
        }
      }
    }
  }

  if(!PubSubClient::loop()) {
    Serial.println("Bytebeam loop failed");
    return false;
  } else {
    Serial.println("I am BytebeamArduino::loop()");
    return true;
  }
}

boolean BytebeamArduino::connected() {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
    Serial.println("BytebeamArduino::connected() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  if(!PubSubClient::connected()) {
    Serial.println("Bytebeam Client is not Connected to the Cloud !");
    return false;
  } else {
    Serial.println("Bytebeam Client is Connected to the Cloud !");
    return true;
  }
}

boolean BytebeamArduino::handleActions(char* actionReceivedStr) {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
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

  /* Above way of extracting json will give the pointers to the json itself, So If you want to use the parameter
   * beyond the scope of this function then you must create a copy of it and then pass it. i.e (id and payload)
   */

  int strSize = 0;
  int tempVar = 0;

  strSize = snprintf(NULL, 0, id) + 1;

  if(strSize <= 1) {
    Serial.println("failed to get action id size");
    return false;
  }

  char* idStr = (char*) malloc(strSize);

  // if memory allocation fails just log the failure to serial and return :)
  if(idStr == NULL) {
    Serial.println("failed to allocate the memory for action id");
    return false;
  }

  tempVar = snprintf(idStr, strSize, id);

  if(tempVar >= strSize) {
    Serial.println("failed to get action id");
    return false;
  }

  strSize = snprintf(NULL, 0, payload) + 1;

  if(strSize <= 1) {
    Serial.println("failed to get payload size");
    return false;
  }

  char* payloadStr = (char*) malloc(strSize);

  // if memory allocation fails just log the failure to serial and return :)
  if(payloadStr == NULL) {
    Serial.println("failed to allocate the memory for payload");
    return false;
  }

  tempVar = snprintf(payloadStr, strSize, payload);

  if(tempVar >= strSize) {
    Serial.println("failed to get payload");
    return false;
  }

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println(idStr);
  Serial.println(payloadStr);
#endif

  int actionIterator = 0;
  while(this->actionFuncs[actionIterator].name) {
    if (!strcmp(this->actionFuncs[actionIterator].name, name)) {
        this->actionFuncs[actionIterator].func(payloadStr, idStr);
        break;
    }
    actionIterator++;
  }

  if(this->actionFuncs[actionIterator].name == NULL) {
    Serial.printf("invalid action : %s\n", name);
  }

  // release the allocated memory :)
  free(idStr);
  free(payloadStr);

  // reset the handle action flag
  handleActionFlag = false;

  return true;
}

boolean BytebeamArduino::addActionHandler(int (*funcPtr)(char* args, char* actionId), char* actionName) {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
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
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
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
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
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

boolean BytebeamArduino::isActionHandlerThere(char* actionName) {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
    Serial.println("BytebeamArduino::isActionHandlerThere() ---> bytebeam client is not active yet, begin the bytebeam client");
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
    Serial.printf("action : %s found at index %d\n", actionName, targetActionIdx);
    return true;
  }
}

boolean BytebeamArduino::printActionHandlerArray() {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
    Serial.println("BytebeamArduino::printActionHandlerArray() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
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

  return true;
}

boolean BytebeamArduino::resetActionHandlerArray() {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
    Serial.println("BytebeamArduino::resetActionHandlerArray() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  initActionHandlerArray();

  return true;
}

boolean BytebeamArduino::publishActionCompleted(char* actionId) {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
    Serial.println("BytebeamArduino::publishActionCompleted() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    Serial.printf("publish action completed abort, bytebeam client is not connected to the cloud\n");
    return false;
  }

  if(!publishActionStatus(actionId, 100, "Completed", "No Error")) {
    Serial.printf("publish action completed response failed, action_id : %s\n", actionId);
    return false;
  } else {
    Serial.printf("publish action completed response success, action_id : %s\n", actionId);
    return true;
  }
}

boolean BytebeamArduino::publishActionFailed(char* actionId) {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
    Serial.println("BytebeamArduino::publishActionFailed() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    Serial.printf("publish action failed abort, bytebeam client is not connected to the cloud\n");
    return false;
  }

  if(!publishActionStatus(actionId, 0, "Failed", "Action Failed")) {
    Serial.printf("publish action failed response failed, action_id : %s\n", actionId);
    return false;
  } else {
    Serial.printf("publish action failed response success, action_id : %s\n", actionId);
    return true;
  }
}

boolean BytebeamArduino::publishActionProgress(char* actionId, int progressPercentage) {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
    Serial.println("BytebeamArduino::publishActionProgress() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    Serial.printf("publish action progress abort, bytebeam client is not connected to the cloud\n");
    return false;
  }

  if(!publishActionStatus(actionId, progressPercentage, "Progress", "No Error")) {
    Serial.printf("publish action progress response failed, action_id : %s\n", actionId);
    return false;
  } else {
    Serial.printf("publish action progress response success, action_id : %s\n", actionId);
    return true;
  }
}

boolean BytebeamArduino::publishToStream(char* streamName, const char* payload) {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
    Serial.println("BytebeamArduino::publishToStream() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    Serial.printf("publish to stream abort, bytebeam client is not connected to the cloud\n");
    return false;
  }

  char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = { 0 };

  int maxLen = BYTEBEAM_MQTT_TOPIC_STR_LEN;
  int tempVar = snprintf(topic, maxLen,  "/tenants/%s/devices/%s/events/%s/jsonarray", this->projectId, this->deviceId, streamName);

  if(tempVar >= maxLen) {
    Serial.println("publish topic size exceeded buffer size");
    return false;
  }

#if DEBUG_BYTEBEAM_ARDUINO
  Serial.println(topic);
  Serial.println(payload);
#endif

  return publish(topic, payload);
}

boolean BytebeamArduino::end() {
  // client should be active and if not just log the info to serial and abort :)
  if(!this->isClientActive) {
    Serial.println("BytebeamArduino::end() ---> bytebeam client is not active yet, begin the bytebeam client");
    return false;
  }

  if(!BytebeamTime.end()) {
    Serial.println("end abort, time client end failed...\n");
    return false;
  }

  this->mqttPort = -1;
  this->mqttBrokerUrl = NULL;
  this->deviceId = NULL;
  this->projectId = NULL;
  this->caCertPem = NULL;
  this->clientCertPem = NULL;
  this->clientKeyPem = NULL;
  this->clientId = NULL;

  // release the allocated memory :)
  free(this->deviceConfigStr);
  this->deviceConfigStr = NULL;

  // disable the OTA if it was enabled
  if(this->isOTAEnable) {
    disableOTA();
  }

  initActionHandlerArray();

  clearBytebeamClient();

  // log bytebeam client duration to serial :)
  Serial.print("Bytebeam Client Duration : ");
  Serial.print(BytebeamTime.durationMillis);
  Serial.println("ms");

  this->isClientActive = false;

  Serial.println("Bytebeam Client Deactivated Successfully !\n");

  return true;
}

#if BYTEBEAM_OTA_ENABLE
  static int handleFirmwareUpdate(char* otaPayloadStr, char* actionId) {
    //
    //  Handle The Firmware Update Here
    //

    if(!BytebeamOTA.updateFirmware(otaPayloadStr, actionId)) {
      Serial.println("Firmware Upgrade Failed.");

      // clear OTA information from RAM
      BytebeamOTA.clearOTAInfoFromRAM();

      // publish firmware update failure message to cloud
      if(!Bytebeam.publishActionFailed(actionId)) {
        Serial.println("failed to publish negative response for firmware upgarde failure");
      }

      return -1;
    } else {
      Serial.println("Firmware Upgrade Success.");

      // save the OTA information in flash
      BytebeamOTA.saveOTAInfo();

      // reboot the chip to boot the new firmware
      rebootESPWithReason("RESTART: Booting New Firmware !");

      return 0;
    }
  }

  boolean BytebeamArduino::enableOTA() {
    // client should be active and if not just log the info to serial and abort :)
    if(!this->isClientActive) {
      Serial.println("BytebeamArduino::enableOTA() ---> bytebeam client is not active yet, begin the bytebeam client");
      return false;
    }

    // setup the secure OTA client
    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP32
      BytebeamOTA.setupSecureOTAClient(this->caCertPem, this->clientCertPem, this->clientKeyPem);
    #endif

    #ifdef BYTEBEAM_ARDUINO_ARCH_ESP8266
      BytebeamOTA.setupSecureOTAClient(this->rootCA, this->clientCert, this->clientKey);
    #endif

    // add the OTA action handler
    if(!addActionHandler(handleFirmwareUpdate, "update_firmware")) {
      Serial.println("OTA Enable Failed.");
      return false;
    }

    this->isOTAEnable = true;
    Serial.println("OTA Enable Success.");

    return true;
  }

  boolean BytebeamArduino::isOTAEnabled() {
    // client should be active and if not just log the info to serial and abort :)
    if(!this->isClientActive) {
      Serial.println("BytebeamArduino::isOTAEnabled() ---> bytebeam client is not active yet, begin the bytebeam client");
      return false;
    }

    if(!this->isOTAEnable) {
      Serial.println("OTA is Disabled.");
      return false;
    } else {
      Serial.println("OTA is Enabled.");
      return true;
    }
  }

  boolean BytebeamArduino::disableOTA() {
    // client should be active and if not just log the info to serial and abort :)
    if(!this->isClientActive) {
      Serial.println("BytebeamArduino::disableOTA() ---> bytebeam client is not active yet, begin the bytebeam client");
      return false;
    }

    // before going ahead make sure OTA is enabled
    if(!this->isOTAEnable) {
      Serial.println("BytebeamArduino::disableOTA() ---> OTA is not enabled yet, enable the OTA");
      return false;
    }

    // clear the secure OTA client
    BytebeamOTA.clearSecureOTAClient();

    // remove the OTA action handler
    if(!removeActionHandler("update_firmware")) {
      Serial.println("OTA Disable Failed.");
      return false;
    }

    this->isOTAEnable = false;
    Serial.println("OTA Disable Success.");

    return true;
  }
#endif

BytebeamArduino Bytebeam;