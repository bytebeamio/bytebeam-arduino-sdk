#include "BytebeamArduino.h"

void subscribeCallback(char* topic, byte* message, unsigned int length) {
  Serial.println("I am subscribeCallback()");

  Serial.print("{Topic : ");
  Serial.print(topic);
  Serial.print(", Message : ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println("}");

  Bytebeam.handleActions((char*)message);
}

BytebeamArduino::BytebeamArduino() {
  Serial.println("I am BytebeamArduino::BytebeamArduino()");
}

BytebeamArduino::~BytebeamArduino() {
  Serial.println("I am BytebeamArduino::~BytebeamArduino()");
} 

bool BytebeamArduino::readDeviceConfigFile() {
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

  secureClient.setCACert(this->caCertPem);
  secureClient.setCertificate(this->clientCertPem); 
  secureClient.setPrivateKey(this->clientKeyPem);

  return true;
}

void BytebeamArduino::initActionHandlerArray() {
  int loopVar = 0;
  for (loopVar = 0; loopVar < BYTEBEAM_NUMBER_OF_ACTIONS; loopVar++) {
    this->actionFuncs[loopVar].func = NULL;
    this->actionFuncs[loopVar].name = NULL;
  }
}

boolean BytebeamArduino::subscribeToActions() {
  int maxLen = 200;
  char topic[maxLen] = { 0 };
  int tempVar = snprintf(topic, maxLen, "/tenants/%s/devices/%s/actions", this->projectId, this->deviceId);

  if(tempVar > maxLen) {
    Serial.println("subscribe topic size exceeded buffer size");
    return false;
  }

  return subscribe(topic);
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

  PubSubClient::setClient(secureClient);
  PubSubClient::setCallback(subscribeCallback);
  PubSubClient::setServer(mqttBrokerUrl, mqttPort);

  initActionHandlerArray();

  if(!PubSubClient::connect("BytebeamArduino")) {
    Serial.println("error while connecting to server...");
    return false;
  }
  Serial.println("bytebeam client connected successfully !");

  if(!subscribeToActions()) {
    Serial.println("error while subscribe to actions...");
    return false;
  }
  Serial.println("bytebeam client subscribed to actions successfully !");

  return true;
}

boolean BytebeamArduino::loop() {
  Serial.println("I am BytebeamArduino::loop()");

  if(!PubSubClient::loop()) {
    Serial.println("error while maintaining the connection to the server...");
    return false;
  }

  return true;
}

boolean BytebeamArduino::connected() {
  if(!PubSubClient::connected()) {
    Serial.println("bytebeam client is not connected to the server !");
    return false;
  } else {
    Serial.println("bytebeam client is connected to the server !");
    return true;
  }
}

boolean BytebeamArduino::publish(const char* topic, const char* payload) {
  if(!PubSubClient::connected()) {
    Serial.printf("publish abort, topic : %s. bytebeam client is not connected to the server...\n", topic);
    return false;
  }

  if(!PubSubClient::publish(topic, payload)) {
    Serial.printf("publish failed, topic : %s\n", topic);
    return false;
  } else {
    Serial.printf("publish success, topic : %s\n", topic);
    return true;
  }
}

boolean BytebeamArduino::subscribe(const char* topic) {
  if(!PubSubClient::connected()) {
    Serial.printf("subscribe abort, topic : %s. bytebeam client is not connected to the server...\n", topic);
    return false;
  }

  if(!PubSubClient::subscribe(topic)) {
    Serial.printf("subscribe failed, topic : %s\n", topic);
    return false;
  } else {
    Serial.printf("subscribe success, topic : %s\n", topic);
    return true;
  }
}

boolean BytebeamArduino::unsubscribe(const char* topic) {
  if(!PubSubClient::connected()) {
    Serial.printf("unsubscribe abort, topic : %s. bytebeam client is not connected to the server...\n", topic);
    return false;
  }

  if(!PubSubClient::unsubscribe(topic)) {
    Serial.printf("unsubscribe failed, topic : %s\n", topic);
    return false;
  } else {
    Serial.printf("unsubscribe success, topic : %s\n", topic);
    return true;
  }
}

boolean BytebeamArduino::handleActions(char* actionReceivedStr) {
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
  while(actionFuncs[actionIterator].name) {
    if (!strcmp(actionFuncs[actionIterator].name, name)) {
        actionFuncs[actionIterator].func((char*)payload, (char*)id);
        break;
    }
    actionIterator++;
  }

  if(actionFuncs[actionIterator].name == NULL) {
    Serial.printf("invalid action : %s\n", name);
  }

  return true;
}

boolean BytebeamArduino::addActionHandler(int (*func_ptr)(char* args, char* actionId), char* func_name) {
  static int functionHandlerIndex = 0;
  if (functionHandlerIndex >= BYTEBEAM_NUMBER_OF_ACTIONS) {
    Serial.println("creation of new action handler failed...");
    return false;
  }

  this->actionFuncs[functionHandlerIndex].func = func_ptr;
  this->actionFuncs[functionHandlerIndex].name = func_name;

  functionHandlerIndex = functionHandlerIndex + 1;

  return true;
}

void BytebeamArduino::end() {
  PubSubClient::disconnect();
  Serial.println("bytebeam client disconnected successfully !");
}

BytebeamArduino Bytebeam;