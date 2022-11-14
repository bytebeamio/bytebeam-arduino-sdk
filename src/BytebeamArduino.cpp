#include "BytebeamArduino.h"

Bytebeam::Bytebeam() {
  Serial.println("I am Bytebeam::Bytebeam()");
}

Bytebeam::~Bytebeam() {
  Serial.println("I am Bytebeam::~Bytebeam()");
} 

void Bytebeam::readDeviceConfigFile() {
  Serial.println("I am Bytebeam::readDeviceConfigFile()");

  if (!SPIFFS.begin(true, "/spiffs")) {
    Serial.println("spiffs mount failed");
    return;
  } else {
    Serial.println("spiffs mount success");
  }

  const char * path = "/device_config.json";
  Serial.printf("Reading file : %s\r\n", path);

  fs::FS &fs = SPIFFS;
  File file = fs.open(path, FILE_READ);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  char chr = ' ';
  int strIndex = 0;

  Serial.println("- read from file");
  while (file.available()) {
    chr = file.read();
    this->deviceConfigStr[strIndex++] = chr;
  }
  file.close();

  // Serial.println("deviceConfigStr :");
  // Serial.println(this->deviceConfigStr);

  SPIFFS.end();
}

void Bytebeam::parseDeviceConfigFile() {
  Serial.println("I am Bytebeam::parseDeviceConfigFile()");

  deserializeJson(this->deviceConfigJson, this->deviceConfigStr);
  
  this->mqttPort      = this->deviceConfigJson["port"];
  this->mqttBrokerUrl = this->deviceConfigJson["broker"];
  this->deviceId      = this->deviceConfigJson["device_id"];
  this->projectId     = this->deviceConfigJson["project_id"];
  this->caCertPem     = this->deviceConfigJson["authentication"]["ca_certificate"];
  this->clientCertPem = this->deviceConfigJson["authentication"]["device_certificate"];
  this->clientKeyPem  = this->deviceConfigJson["authentication"]["device_private_key"];
  
  // Serial.println(this->mqttPort);
  // Serial.println(this->mqttBrokerUrl);
  // Serial.println(this->deviceId);
  // Serial.println(this->projectId);
  // Serial.println(this->caCertPem);
  // Serial.println(this->clientCertPem);
  // Serial.println(this->clientKeyPem);

  secureClient.setCACert(this->caCertPem);
  secureClient.setCertificate(this->clientCertPem); 
  secureClient.setPrivateKey(this->clientKeyPem);
}

boolean Bytebeam::begin() {
  Serial.println("I am Bytebeam::begin()");

  readDeviceConfigFile();
  parseDeviceConfigFile();

  PubSubClient::setClient(secureClient);
  PubSubClient::setServer(mqttBrokerUrl, mqttPort);
  return PubSubClient::connect("BytebeamArduino");
}

void Bytebeam::setSubscribeCallback(MQTT_CALLBACK_SIGNATURE) {
  Serial.println("I am Bytebeam::setSubscribeCallback()");

  PubSubClient::setCallback(callback);
}

boolean Bytebeam::publish(const char* topic, const char* payload) {
  Serial.println("I am Bytebeam::publish()");

  return PubSubClient::publish(topic, payload);
}

boolean Bytebeam::subscribe(const char* topic) {
  Serial.println("I am Bytebeam::subscribe()");

  return  PubSubClient::subscribe(topic);
}

boolean Bytebeam::unsubscribe(const char* topic) {
  Serial.println("I am Bytebeam::unsubscribe()");

  return  PubSubClient::unsubscribe(topic);
}

boolean Bytebeam::loop() {
  Serial.println("I am Bytebeam::loop()");

  return  PubSubClient::loop();
}

boolean Bytebeam::connected() {
  Serial.println("I am Bytebeam::connected()");

  return  PubSubClient::connected();
}

void Bytebeam::end() {
  Serial.println("I am Bytebeam::end()");

  PubSubClient::disconnect();
}