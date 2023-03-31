#include "BytebeamArduino.h"

/* This object will represent the Time library, We will be exposing the necessary functionality and info
 * for the usage, If you want to do any Timing related stuff this guy is for you.
 */
static BytebeamTime BytebeamTime;

/* This object will represent the OTA library, We will be exposing the necessary functionality and info
 * for the usage, If you want to do any OTA stuff this guy is for you.
 */
#if BYTEBEAM_OTA_ENABLE
  static BytebeamOTA BytebeamOTA;
#endif

/* This flag will prevent the use of handle actions api directly as this api is meant for internal usage
 * If you really want to call the handle actions api directly, probably you have to step into debug mode 
 */
static bool handleActionFlag = false;

static void BytebeamActionsCallback(char* topic, byte* message, unsigned int length) {
  BytebeamLogger::Info(__FILE__, __func__, "I am BytebeamActionsCallback()");

  int strSize = 0;
  int tempVar = 0;

  strSize = length + 1;

  if(strSize <= 1) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to get action json size");
    return;
  }

  char* actionJsonStr = (char*) malloc(strSize);

  // if memory allocation fails just log the failure to serial and return :)
  if(actionJsonStr == NULL) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to allocate the memory for action json");
    return;
  }

  tempVar = snprintf(actionJsonStr, strSize, "%s", (char*)message);

  // make sure you not loose any packet upto strSize
  if(tempVar < strSize - 1) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to get action json");
    return;
  }

  // log the recieved action to serial :)
  BytebeamLogger::Info(__FILE__, __func__, "just subscribed, {topic : %s, message : %s}", topic, actionJsonStr);

  // set the handle action flag
  handleActionFlag = true;

  // handle the received action
  Bytebeam.handleActions(actionJsonStr);

  // release the allocated memory :)
  free(actionJsonStr);
}

static void rebootESPWithReason(char* reason) {
  BytebeamLogger::Warn(__FILE__, __func__, reason);
  delay(3000);
  ESP.restart();
}

void BytebeamArduino::printArchitectureInfo() {
  //
  // log the usefull architecture information to serial :)
  //

  BytebeamLogger::Info(__FILE__, __func__, "* ****************************************************************** *");

#if defined(BYTEBEAM_ARDUINO_ARCH_ESP32)
  // log esp32 arch info to serial :)
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t Architecture : ESP32 \t\t\t\t*");
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t Chip Model   : %s \t\t\t*", ESP.getChipModel());
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t CPU Freq     : %d MHz \t\t\t*", ESP.getCpuFreqMHz());
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t Flash Size   : %d MB \t\t\t\t*", ((ESP.getFlashChipSize())/1024)/1024);
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t Free Heap    : %d KB \t\t\t\t*", (ESP.getFreeHeap())/1024);
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t SDK Version  : %s \t\t\t\t*", ESP.getSdkVersion());
#elif defined(BYTEBEAM_ARDUINO_ARCH_ESP8266)
  // log esp8266 arch info to serial :)
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t Architecture : ESP8266 \t\t\t*");
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t Chip Id      : %d \t\t\t*", ESP.getChipId());
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t CPU Freq     : %d MHz \t\t\t\t*", ESP.getCpuFreqMHz());
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t Flash Size   : %d MB \t\t\t\t*", ((ESP.getFlashChipSize())/1024)/1024);
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t Free Heap    : %d KB \t\t\t\t*", (ESP.getFreeHeap())/1024);
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t SDK Version  : %s \t\t*", ESP.getSdkVersion());
  BytebeamLogger::Info(__FILE__, __func__, "*\t\t\t Core Version : %s \t\t\t\t*", ESP.getCoreVersion());
#else
  // log unknown arch info to serial :)
  BytebeamLogger::Info(__FILE__, __func__, "Unknown Architecture");
#endif

  BytebeamLogger::Info(__FILE__, __func__, "* ****************************************************************** *");
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

bool BytebeamArduino::subscribe(const char* topic, uint8_t qos) {
  //
  // skipping the connection checking here as we are not calling this function directly
  //

  if(!PubSubClient::subscribe(topic, qos)) {
    BytebeamLogger::Error(__FILE__, __func__, "subscribe failed, {topic : %s, qos : %d}", topic, qos);
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "subscribe success, {topic : %s, qos : %d}", topic, qos);
    return true;
  }
}

bool BytebeamArduino::unsubscribe(const char* topic) {
  //
  // skipping the connection checking here as we are not calling this function directly
  //

  if(!PubSubClient::unsubscribe(topic)) {
    BytebeamLogger::Error(__FILE__, __func__, "unsubscribe failed, {topic : %s}", topic);
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "unsubscribe success, {topic : %s}", topic);
    return true;
  }
}

bool BytebeamArduino::publish(const char* topic, const char* payload) {
  //
  // skipping the connection checking here as we are not calling this function directly
  //

  if(!PubSubClient::publish(topic, payload)) {
    BytebeamLogger::Error(__FILE__, __func__, "publish failed, {topic : %s, message : %s}", topic, payload);
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "publish success, {topic : %s, message : %s}", topic, payload);
    return true;
  }
}

bool BytebeamArduino::subscribeToActions() {
  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    BytebeamLogger::Error(__FILE__, __func__, "subscribe to actions failed, bytebeam client is not connected to the cloud");
    return false;
  }

  uint8_t qos = 1;
  char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = { 0 };

  int maxLen = BYTEBEAM_MQTT_TOPIC_STR_LEN;
  int tempVar = snprintf(topic, maxLen, "/tenants/%s/devices/%s/actions", this->projectId, this->deviceId);

  if(tempVar >= maxLen) {
    BytebeamLogger::Error(__FILE__, __func__, "subscribe action topic size exceeded topic buffer size");
    return false;
  }

  BytebeamLogger::Debug(__FILE__, __func__, "qos : %d", qos);
  BytebeamLogger::Debug(__FILE__, __func__, "topic : %s", topic);

  return subscribe(topic, qos);
}

bool BytebeamArduino::unsubscribeToActions() {
  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    BytebeamLogger::Error(__FILE__, __func__, "unsubscribe to actions abort, bytebeam client is not connected to the cloud");
    return false;
  }

  char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = { 0 };

  int maxLen = BYTEBEAM_MQTT_TOPIC_STR_LEN;
  int tempVar = snprintf(topic, maxLen, "/tenants/%s/devices/%s/actions", this->projectId, this->deviceId);

  if(tempVar >= maxLen) {
    BytebeamLogger::Error(__FILE__, __func__, "unsubscribe action topic size exceeded topic buffer size");
    return false;
  }

  BytebeamLogger::Debug(__FILE__, __func__, "topic : %s", topic); 

  return unsubscribe(topic);
}

bool BytebeamArduino::publishActionStatus(char* actionId, int progressPercentage, char* status, char* error) {
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
    BytebeamLogger::Error(__FILE__, __func__, "failed to get epoch millis");
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
    BytebeamLogger::Error(__FILE__, __func__, "action status topic size exceeded topic buffer size");
    return false;
  }

  BytebeamLogger::Debug(__FILE__, __func__, "topic : %s", topic); 
  BytebeamLogger::Debug(__FILE__, __func__, "payload : %s", payload); 

  return publish(topic, payload);
}

#ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FS
  bool BytebeamArduino::readDeviceConfigFile() {

    /* This file system pointer will store the address of the selected file system, So after begin and end operations
     * we can utilize this file system pointer to do the file operations.
     */
    fs::FS* ptrToFS = NULL;

    switch(this->fileSystem) {

    /* We need to do conditional compilation here beacuse different architecture supports different file systems
     * So based on the architecture we should define the flags for the supported file system.
     */
  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FATFS
      case FATFS_FILE_SYSTEM:
        BytebeamLogger::Info(__FILE__, __func__, "FATFS file system detected !");

        // initalize the FATFS file system
        if(!FFat.begin()) {
          BytebeamLogger::Error(__FILE__, __func__, "FATFS mount failed");
          return false;
        }

        // set the file system pointer to FATFS Object
        ptrToFS = &FFat;

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SPIFFS
      case SPIFFS_FILE_SYSTEM:
        BytebeamLogger::Info(__FILE__, __func__, "SPIFFS file system detected !");

        // initalize the SPIFFS file system
        if(!SPIFFS.begin()) {
          BytebeamLogger::Error(__FILE__, __func__, "SPIFFS mount failed");
          return false;
        }

        // set the file system pointer to SPIFFS Object
        ptrToFS = &SPIFFS;

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_LITTLEFS
      case LITTLEFS_FILE_SYSTEM:
        BytebeamLogger::Info(__FILE__, __func__, "LITTLEFS file system detected !");

        // initalize the LITTLEFS file system
        if(!LittleFS.begin()) {
          BytebeamLogger::Error(__FILE__, __func__, "LITTLEFS mount failed");
          return false;
        }

        // set the file system pointer to LITTLEFS Object
        ptrToFS = &LittleFS;

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SD
      case SD_FILE_SYSTEM:
        BytebeamLogger::Info(__FILE__, __func__, "SD file system detected !");

        // Just print the log and return :)
        BytebeamLogger::Info(__FILE__, __func__, "SD file system is not supported by the SDK yet");
        return false;

        break;
  #endif
  
      default:
        BytebeamLogger::Info(__FILE__, __func__, "Unknown file system detected !");

        // Just print the log and return :)
        BytebeamLogger::Info(__FILE__, __func__, "Make sure the architecture supports the selected file system");
        return false;

        break;
    }

    const char* path = this->fileName;
    BytebeamLogger::Info(__FILE__, __func__, "Reading file : %s", path);

    File file = ptrToFS->open(path, FILE_READ);
    if (!file || file.isDirectory()) {
      BytebeamLogger::Error(__FILE__, __func__, "- failed to open device config file for reading");
      return false;
    }

    char chr = ' ';
    int strIndex = 0;
    int strSize = 0;

    strSize = file.size() + 1;

    if(strSize <= 1) {
      BytebeamLogger::Error(__FILE__, __func__, "failed to get device config file size");
      return false;
    }

    this->deviceConfigStr = (char*) malloc(strSize);

    // if memory allocation fails just log the failure to serial and return :)
    if(this->deviceConfigStr == NULL) {
      BytebeamLogger::Error(__FILE__, __func__, "failed to allocate the memory for device config file");
      return false;
    }

    BytebeamLogger::Info(__FILE__, __func__, "- read from file");
    while (file.available()) {
      chr = file.read();
      this->deviceConfigStr[strIndex++] = chr;
    }

    file.close();

    switch(this->fileSystem) {

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
        LittleFS.end();

        break;
  #endif

  #ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_SD
      case SD_FILE_SYSTEM:
        // de-initalize the SD file system
        // nothing to do here yet

        break;
  #endif

      default:
        // nothing to do here yet

        break;
    }

    BytebeamLogger::Debug(__FILE__, __func__, "deviceConfigStr : %s", this->deviceConfigStr);

    return true;
  }
#endif

bool BytebeamArduino::parseDeviceConfigFile() {
  // before going ahead make sure you are parsing something
  if(this->deviceConfigStr == NULL) {
    BytebeamLogger::Error(__FILE__, __func__, "device config file is empty");
    return false;
  }

  StaticJsonDocument<1024> deviceConfigJson;
  DeserializationError err = deserializeJson(deviceConfigJson, this->deviceConfigStr);

  if(err) {
    BytebeamLogger::Error(__FILE__, __func__, "deserializeJson() failed : %s", err.c_str());
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "deserializeJson() success");
  }
  
  BytebeamLogger::Info(__FILE__, __func__, "Obtaining device config file variables");

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
      BytebeamLogger::Error(__FILE__, __func__, "- failed to obtain %s", name[argIterator]);
      return false;
    }
  }
  BytebeamLogger::Info(__FILE__, __func__, "- obtain device config file variables");

  BytebeamLogger::Debug(__FILE__, __func__, "mqttPort : %d", this->mqttPort);
  BytebeamLogger::Debug(__FILE__, __func__, "mqttBrokerUrl : %s", this->mqttBrokerUrl);
  BytebeamLogger::Debug(__FILE__, __func__, "deviceId : %s", this->deviceId);
  BytebeamLogger::Debug(__FILE__, __func__, "projectId : %s", this->projectId);
  BytebeamLogger::Debug(__FILE__, __func__, "caCertPem : \n%s", this->caCertPem);
  BytebeamLogger::Debug(__FILE__, __func__, "clientCertPem : \n%s", this->clientCertPem);
  BytebeamLogger::Debug(__FILE__, __func__, "clientKeyPem : \n%s", this->clientKeyPem);
  BytebeamLogger::Debug(__FILE__, __func__, "clientId : %s", this->clientId);

  BytebeamLogger::Info(__FILE__, __func__, "Project Id : %s and Device Id : %s", this->projectId, this->deviceId);

  return true;
}

bool BytebeamArduino::setupBytebeamClient() {

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
    BytebeamLogger::Error(__FILE__, __func__, "failed to allocate the memory for root ca");
    return false;
  }

  this->clientCert = new BearSSL::X509List(this->clientCertPem);

  // if memory allocation fails just log the failure to serial and return :)
  if(this->clientCert == NULL) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to allocate the memory for client cert");
    return false;
  }

  this->clientKey = new BearSSL::PrivateKey(this->clientKeyPem);

  // if memory allocation fails just log the failure to serial and return :)
  if(this->clientKey == NULL) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to allocate the memory for client key");
    return false;
  }

  this->secureClient.setBufferSizes(2048, 2048);
  this->secureClient.setTrustAnchors(this->rootCA);
  this->secureClient.setClientRSACert(this->clientCert, this->clientKey);
#endif

  PubSubClient::setClient(this->secureClient);
  PubSubClient::setCallback(BytebeamActionsCallback);
  PubSubClient::setServer(this->mqttBrokerUrl, this->mqttPort);

  BytebeamLogger::Info(__FILE__, __func__, "Connecting To Bytebeam Cloud ... ");

  if(!PubSubClient::connect(this->clientId)) {
    BytebeamLogger::Error(__FILE__, __func__, "ERROR");
    return false;
  }

 BytebeamLogger::Info(__FILE__, __func__, "CONNECTED");

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

  BytebeamLogger::Info(__FILE__, __func__, "Disconnecting To Bytebeam Cloud ... ");

  PubSubClient::disconnect();

  BytebeamLogger::Info(__FILE__, __func__, "DISCONNECTED");
}

bool BytebeamArduino::initSDK() {
  // It's much better to pump up architecture inforamtion in the very beginning
  printArchitectureInfo();

#ifdef BYTEBEAM_ARDUINO_ARCH_SUPPORTS_FS
  if(!readDeviceConfigFile()) {
    BytebeamLogger::Error(__FILE__, __func__, "Initialization failed, error while reading the device config file.\n");
    return false;
  }
#else
  BytebeamLogger::Info(__FILE__, __func__, "Architecture doesn't support file system");
#endif
            
  if(!parseDeviceConfigFile()) {
    BytebeamLogger::Error(__FILE__, __func__, "Initialization failed, error while parsing the device config file.\n");
    return false;
  }

#if BYTEBEAM_OTA_ENABLE  
  BytebeamOTA.retrieveOTAInfo();

  if(!BytebeamOTA.otaUpdateFlag) {
    BytebeamLogger::Info(__FILE__, __func__, "RESTART: Normal Reboot !");
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "RESTART: Reboot After Successfull OTA Update !");
  }
#endif

  if(!setupBytebeamClient()) {
    BytebeamLogger::Error(__FILE__, __func__, "Initialization failed, error while connecting to cloud.\n");
    return false;
  }

  if(!subscribeToActions()) {
    BytebeamLogger::Error(__FILE__, __func__, "Initialization failed, error while subscribe to actions.\n");
    return false;
  }

  // this flag marks the client initialization
  this->isClientActive = true;

#if BYTEBEAM_OTA_ENABLE  
  if(BytebeamOTA.otaUpdateFlag) {
    if(!publishActionCompleted(BytebeamOTA.otaActionId)) {
      BytebeamLogger::Error(__FILE__, __func__, "Failed to publish OTA complete status.");
    }

    BytebeamOTA.clearOTAInfoFromRAM();
    BytebeamOTA.clearOTAInfoFromFlash();
  }
#else
  BytebeamLogger::Info(__FILE__, __func__, "Skipped Bytebeam OTA from compilation phase i.e saving flash size");
#endif

  BytebeamLogger::Info(__FILE__, __func__, "Bytebeam Client Initialized Successfully !\n");

  return true;
}

bool BytebeamArduino::isInitialized() {
  // return the client status i.e initialized or de-initialized
  return this->isClientActive;
}

BytebeamArduino::BytebeamArduino()
  #ifdef BYTEBEAM_ARDUINO_USE_MODEM
    : secureClient(&gsmClient)
  #endif
{
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

  this->isClientActive = false;
  this->isOTAEnable = false;
}

BytebeamArduino::~BytebeamArduino() {
  //
  // Nothing much to do here, just print the log to serial :)
  //

  BytebeamLogger::Info(__FILE__, __func__, "I am BytebeamArduino::~BytebeamArduino()");
}

#ifdef BYTEBEAM_ARDUINO_USE_WIFI
  bool BytebeamArduino::begin( const deviceConfigFileSystem fileSystem, 
                                  const char* fileName, 
                                  BytebeamLogger::DebugLevel level) {
    // set the device config file system
    this->fileSystem = fileSystem;

    // set the device config file name
    this->fileName = fileName;

    // set the bytbeam logger log level 
    BytebeamLogger::setLogLevel(level);

    // fix : ensure wifi status before using ntp methods o/w they will give hard fault
    if(WiFi.status() != WL_CONNECTED) {
      BytebeamLogger::Error(__FILE__, __func__, "Begin abort, could not find WiFi connectivity.\n");
      return false;
    }

    // so we got the wifi conectivity at this point
    // before initializing the core sdk make sure time client is working fine with wifi
    if(!BytebeamTime.begin()) {
      BytebeamLogger::Error(__FILE__, __func__, "Begin abort, time client begin failed.\n");
      return false;
    }

    // share the time instance with the log module
    BytebeamLog::setTimeInstance(&BytebeamTime);

    // initialize the core sdk and give back the status to the user
    bool result = initSDK();

    return result;
  }
#endif

#ifdef BYTEBEAM_ARDUINO_USE_MODEM
  bool BytebeamArduino::begin( TinyGsm* modem, 
                                  const deviceConfigFileSystem fileSystem, 
                                  const char* fileName,
                                  BytebeamLogger::DebugLevel level) {
    // set the device config file system
    this->fileSystem = fileSystem;

    // set the device config file name
    this->fileName = fileName;

    // set the bytbeam logger log level 
    BytebeamLogger::setLogLevel(level);

    // fix : ensure modem instance before using modem class methods o/w they will give hard fault
    if(!modem) {
      BytebeamLogger::Error(__FILE__, __func__, "Begin abort, failed to get Modem instance.\n");
      return false;
    }

    // initiaize the gsm client with the modem instance
    this->gsmClient.init(modem, 0);

    // share the modem instance with the time module
    BytebeamTime.setModemInstance(modem);

    // setup the gsm OTA client
    BytebeamOTA.setupGsmClient(modem);

    // so we got the modem conectivity at this point
    // before initializing the core sdk make sure time client is working fine with modem
    if(!BytebeamTime.begin()) {
      BytebeamLogger::Error(__FILE__, __func__, "Begin abort, time client begin failed.\n");
      return false;
    }

    // share the time instance with the log module
    BytebeamLog::setTimeInstance(&BytebeamTime);

    // initialize the core sdk and give back the status to the user
    bool result = initSDK();

    return result;
  }
#endif

bool BytebeamArduino::loop() {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

#if 0 // Enable this code fragment to force disconnect from cloud, can be used in debugging loop section
  static int counter = 0;
  counter++;
  if(counter == 10) { 
    PubSubClient::disconnect(); 
  }
#endif

  if(!PubSubClient::connected()) {
    BytebeamLogger::Info(__FILE__, __func__, "Bytebean Client Connection Broken");

    int tryCount = 0;
    while(!PubSubClient::connected()) {
      if(tryCount == BYTEBEAM_CONNECT_MAX_RETRIES) {
        BytebeamLogger::Error(__FILE__, __func__, "Maximum attempt limit reached, can't reconnect to the cloud");
        return false;
      }
     
      tryCount++;
      BytebeamLogger::Info(__FILE__, __func__, "Attempt : %d, Trying To Reconnect To The Cloud ... ", tryCount);

      if(!PubSubClient::connect("BytebeamClient")) {
        BytebeamLogger::Info(__FILE__, __func__, "ERROR");
        delay(1000);
      } else {
        BytebeamLogger::Info(__FILE__, __func__, "CONNECTED");
        if(!subscribeToActions()) {
          BytebeamLogger::Error(__FILE__, __func__, "Subscribe to actions failed");
          return false;
        }
      }
    }
  }

  if(!PubSubClient::loop()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam loop failed");
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "I am BytebeamArduino::loop()");
    return true;
  }
}

bool BytebeamArduino::isConnected() {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  // return the connection status i.e connected or disconnected
  return PubSubClient::connected();
}

bool BytebeamArduino::handleActions(char* actionReceivedStr) {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  BytebeamLogger::Debug(__FILE__, __func__, "actionReceivedStr : %s", actionReceivedStr);

// Enable this code fragment to debug this api externally
// #if 0
  if(!handleActionFlag) {
    BytebeamLogger::Info(__FILE__, __func__, "This api is meant to handle the actions that comes from the cloud as a subsciption packet");
    BytebeamLogger::Info(__FILE__, __func__, "You can not call this api directly, If you are devloper step into debug mode");
    return false;
  }
// #endif
  
  StaticJsonDocument<1024> actionReceivedJson;
  DeserializationError err = deserializeJson(actionReceivedJson, actionReceivedStr);
  
  if(err) {
    BytebeamLogger::Error(__FILE__, __func__, "deserializeJson() failed : %s", err.c_str());
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "deserializeJson() success");
  }
  
  BytebeamLogger::Info(__FILE__, __func__, "Obtaining action variables");

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
      BytebeamLogger::Error(__FILE__, __func__, "- failed to obtain %s", argsName[argIterator]);
      return false;
    }
  }
  BytebeamLogger::Info(__FILE__, __func__, "- obtain action variables");

  BytebeamLogger::Debug(__FILE__, __func__, "name : %s", name);
  BytebeamLogger::Debug(__FILE__, __func__, "id : %s", id);
  BytebeamLogger::Debug(__FILE__, __func__, "payload : %s", payload);
  BytebeamLogger::Debug(__FILE__, __func__, "kind : %s", kind);

  /* Above way of extracting json will give the pointers to the json itself, So If you want to use the parameter
   * beyond the scope of this function then you must create a copy of it and then pass it. i.e (id and payload)
   */

  int strSize = 0;
  int tempVar = 0;

  strSize = snprintf(NULL, 0, "%s", id) + 1;

  if(strSize <= 1) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to get action id size");
    return false;
  }

  char* idStr = (char*) malloc(strSize);

  // if memory allocation fails just log the failure to serial and return :)
  if(idStr == NULL) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to allocate the memory for action id");
    return false;
  }

  tempVar = snprintf(idStr, strSize, "%s", id);

  if(tempVar >= strSize) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to get action id");
    return false;
  }

  strSize = snprintf(NULL, 0, "%s", payload) + 1;

  if(strSize <= 1) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to get payload size");
    return false;
  }

  char* payloadStr = (char*) malloc(strSize);

  // if memory allocation fails just log the failure to serial and return :)
  if(payloadStr == NULL) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to allocate the memory for payload");
    return false;
  }

  tempVar = snprintf(payloadStr, strSize, "%s", payload);

  if(tempVar >= strSize) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to get payload");
    return false;
  }

#if 0 // Enable this code fragment to check malloc generated buffers
  BytebeamLogger::Debug(__FILE__, __func__, "idStr : %s", idStr);
  BytebeamLogger::Debug(__FILE__, __func__, "payloadStr : %s", payloadStr);
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
    BytebeamLogger::Error(__FILE__, __func__, "invalid action : %s", name);
  }

  // release the allocated memory :)
  free(idStr);
  free(payloadStr);

  // reset the handle action flag
  handleActionFlag = false;

  return true;
}

bool BytebeamArduino::addActionHandler(int (*funcPtr)(char* args, char* actionId), char* actionName) {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  if(this->actionFuncsHandlerIdx + 1 >= BYTEBEAM_NUMBER_OF_ACTIONS) {
    BytebeamLogger::Error(__FILE__, __func__, "maximum actions limit reached, can't create new action at the moment");
    return false;
  }

  int actionIterator = 0;
  for(actionIterator = 0; actionIterator <= this->actionFuncsHandlerIdx; actionIterator++) {
    if(!strcmp(this->actionFuncs[actionIterator].name, actionName)) {
      BytebeamLogger::Error(__FILE__, __func__, "action : %s is already there at index %d, update the action instead", actionName, actionIterator);
      return false;
    }
  }

  this->actionFuncsHandlerIdx += 1;
  this->actionFuncs[this->actionFuncsHandlerIdx].func = funcPtr;
  this->actionFuncs[this->actionFuncsHandlerIdx].name = actionName;

  return true;
}

bool BytebeamArduino::removeActionHandler(char* actionName) {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
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
    BytebeamLogger::Error(__FILE__, __func__, "action : %s not found, can't remove", actionName);
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

bool BytebeamArduino::updateActionHandler(int (*newFuncPtr)(char* args, char* actionId), char* actionName) {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
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
    BytebeamLogger::Error(__FILE__, __func__, "action : %s not found, can't update", actionName);
    return false;
  } else {
    this->actionFuncs[targetActionIdx].func = newFuncPtr;
    return true;
  }
}

bool BytebeamArduino::isActionHandlerThere(char* actionName) {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
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
    BytebeamLogger::Info(__FILE__, __func__, "action : %s not found", actionName);
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "action : %s found at index %d", actionName, targetActionIdx);
    return true;
  }
}

bool BytebeamArduino::printActionHandlerArray() {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  int actionIterator = 0;

  BytebeamLogger::Info(__FILE__, __func__, "["); 
  for(actionIterator = 0; actionIterator < BYTEBEAM_NUMBER_OF_ACTIONS; actionIterator++) {
    if(this->actionFuncs[actionIterator].name != NULL) {
      BytebeamLogger::Info(__FILE__, __func__, "       { %s : %s }        ", this->actionFuncs[actionIterator].name, "*******");
    } else {
      BytebeamLogger::Info(__FILE__, __func__, "       { %s : %s }        ", "NULL", "NULL");
    }
  }
  BytebeamLogger::Info(__FILE__, __func__, "]");

  return true;
}

bool BytebeamArduino::resetActionHandlerArray() {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  initActionHandlerArray();

  return true;
}

bool BytebeamArduino::publishActionCompleted(char* actionId) {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    BytebeamLogger::Error(__FILE__, __func__, "publish action completed abort, bytebeam client is not connected to the cloud");
    return false;
  }

  if(!publishActionStatus(actionId, 100, "Completed", "")) {
    BytebeamLogger::Error(__FILE__, __func__, "publish action completed response failed, action_id : %s", actionId);
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "publish action completed response success, action_id : %s", actionId);
    return true;
  }
}

bool BytebeamArduino::publishActionFailed(char* actionId, char* error) {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    BytebeamLogger::Error(__FILE__, __func__, "publish action failed abort, bytebeam client is not connected to the cloud");
    return false;
  }

  if(!publishActionStatus(actionId, 0, "Failed", error)) {
    BytebeamLogger::Error(__FILE__, __func__, "publish action failed response failed, action_id : %s", actionId);
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "publish action failed response success, action_id : %s", actionId);
    return true;
  }
}

bool BytebeamArduino::publishActionProgress(char* actionId, int progressPercentage, char* status) {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    BytebeamLogger::Error(__FILE__, __func__, "publish action progress abort, bytebeam client is not connected to the cloud");
    return false;
  }

  if(!publishActionStatus(actionId, progressPercentage, status, "")) {
    BytebeamLogger::Error(__FILE__, __func__, "publish action progress response failed, action_id : %s", actionId);
    return false;
  } else {
    BytebeamLogger::Info(__FILE__, __func__, "publish action progress response success, action_id : %s", actionId);
    return true;
  }
}

bool BytebeamArduino::publishToStream(char* streamName, const char* payload) {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  // before going ahead make sure you are connected
  if(!PubSubClient::connected()) {
    BytebeamLogger::Error(__FILE__, __func__, "publish to stream abort, bytebeam client is not connected to the cloud");
    return false;
  }

  char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = { 0 };

  int maxLen = BYTEBEAM_MQTT_TOPIC_STR_LEN;
  int tempVar = snprintf(topic, maxLen,  "/tenants/%s/devices/%s/events/%s/jsonarray", this->projectId, this->deviceId, streamName);

  if(tempVar >= maxLen) {
    BytebeamLogger::Error(__FILE__, __func__, "publish topic size exceeded buffer size");
    return false;
  }

  BytebeamLogger::Debug(__FILE__, __func__, "topic : %s", topic);
  BytebeamLogger::Debug(__FILE__, __func__, "payload : %s", payload);

  return publish(topic, payload);
}

bool BytebeamArduino::end() {
  // client should be initialized and if not just log the info to serial and abort :)
  if(!isInitialized()) {
    BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
    return false;
  }

  if(!BytebeamTime.end()) {
    BytebeamLogger::Error(__FILE__, __func__, "End abort, time client end failed.\n");
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
#if BYTEBEAM_OTA_ENABLE
  if(isOTAEnabled()) {
    disableOTA();
  }
#endif

  initActionHandlerArray();

  clearBytebeamClient();

  // log bytebeam client duration to serial :)
  BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client Duration : %d ms", BytebeamTime.durationMillis);

  // this flag marks the client de-initialization
  this->isClientActive = false;

  BytebeamLogger::Info(__FILE__, __func__, "Bytebeam Client De-Initialized Successfully !\n");

  return true;
}

#if BYTEBEAM_OTA_ENABLE
  static int handleFirmwareUpdate(char* otaPayloadStr, char* actionId) {
    //
    //  Handle The Firmware Update Here
    //

    if(!BytebeamOTA.updateFirmware(otaPayloadStr, actionId)) {
      BytebeamLogger::Error(__FILE__, __func__, "Firmware Upgrade Failed.");

      // publish firmware update failure message to cloud
      if(!Bytebeam.publishActionFailed(actionId, BytebeamOTA.otaError)) {
        BytebeamLogger::Error(__FILE__, __func__, "Failed to publish negative response for firmware upgarde failure.");
      }

      // clear OTA information from RAM
      BytebeamOTA.clearOTAInfoFromRAM();

      return -1;
    } else {
      BytebeamLogger::Info(__FILE__, __func__, "Firmware Upgrade Success.");

      // save the OTA information in flash
      BytebeamOTA.saveOTAInfo();

      // reboot the chip to boot the new firmware
      rebootESPWithReason("RESTART: Booting New Firmware !");

      return 0;
    }
  }

  bool BytebeamArduino::enableOTA() {
    // client should be initialized and if not just log the info to serial and abort :)
    if(!isInitialized()) {
      BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
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
      BytebeamLogger::Error(__FILE__, __func__, "OTA Enable Failed.");
      return false;
    }

    // this flag marks the OTA disabled
    this->isOTAEnable = true;

    BytebeamLogger::Info(__FILE__, __func__, "OTA Enable Success.");

    return true;
  }

  bool BytebeamArduino::isOTAEnabled() {
    // client should be initialized and if not just log the info to serial and abort :)
    if(!isInitialized()) {
      BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
      return false;
    }

    // return the OTA status i.e enabled or disabled
    return this->isOTAEnable;
  }

  bool BytebeamArduino::disableOTA() {
    // client should be initialized and if not just log the info to serial and abort :)
    if(!isInitialized()) {
      BytebeamLogger::Error(__FILE__, __func__, "Bytebeam Client is not Initialized.");
      return false;
    }

    // before going ahead make sure OTA is enabled
    if(!isOTAEnabled()) {
      BytebeamLogger::Error(__FILE__, __func__, "Bytebeam OTA is not enabled.");
      return false;
    }

    // clear the secure OTA client
    BytebeamOTA.clearSecureOTAClient();

    // remove the OTA action handler
    if(!removeActionHandler("update_firmware")) {
      BytebeamLogger::Error(__FILE__, __func__, "OTA Disable Failed.");
      return false;
    }

    // this flag marks the OTA disabled
    this->isOTAEnable = false;

    BytebeamLogger::Info(__FILE__, __func__, "OTA Disable Success.");

    return true;
  }
#endif

BytebeamArduino Bytebeam;
BytebeamLogger::DebugLevel BytebeamLogger::logLevel = BytebeamLogger::LOG_WARN;