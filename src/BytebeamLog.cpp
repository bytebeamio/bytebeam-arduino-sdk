#include "BytebeamLog.h"
#include "BytebeamArduino.h"
#include "BytebeamLogger.h"

void BytebeamLog::log(const char* level, const char* tag, const char* message) {
  //
  // this function will print the log in the following format
  // format : { level (millis) tag : message } i.e this format is adopted from the esp idf logging library
  //

  Serial.print(level[0]);
  Serial.print(" (");
  Serial.print(millis());
  Serial.print(") ");
  Serial.print(tag);
  Serial.print(" : ");
  Serial.print(message);
}

void BytebeamLog::logPrint(bytebeamLogLevel_t level, const char* tag, const char* message) {
  //
  // this function represents high level wrapper for the cloud logging
  //

  // before going ahead make sure log level is valid
  if(level > this->logLevel || !Serial) {
    return;
  }

  // get the level string
  const char* levelStr = bytebeamLogLevelStr[level];

  // if cloud logging is disabled, print the log to serial and return :)
  if(!this->isCloudLoggingEnable) {
    log(levelStr, tag, message);
    return;
  }

  // if publish to cloud succedded, print the log to serial else print error log :)
  if(!logPublish(levelStr, tag, message)) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to publish bytebeam log.");
    return;
  }

  log(levelStr, tag, message);
}

void BytebeamLog::logPrintln(bytebeamLogLevel_t level, const char* tag, const char* message) {
  //
  // Just call the logPrint function and give the new line to serial at the end
  //

  logPrint(level, tag, message);
  Serial.println();
}

void BytebeamLog::logPrintf(bytebeamLogLevel_t level, const char* tag, const char* fmt, va_list args) {
  //
  // Generate the log message from the varibale arguments and call the logPrint function
  //

  int strSize = 0;
  int tempVar = 0;

  strSize = vsnprintf(NULL, 0, fmt, args) + 1;

  if(strSize <= 1) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to get log message size");
    return;
  }

  char* msgStr = (char*) malloc(strSize);
  
  // if memory allocation fails just log the failure to serial and return :)
  if(msgStr == NULL) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to allocate the memory for log message");
    return;
  }

  tempVar = vsnprintf(msgStr, strSize, fmt, args);

  if(tempVar >= strSize) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to get log message");
    return;
  }

  logPrint(level, tag, msgStr);
}

boolean BytebeamLog::logPublish(const char* level, const char* tag, const char* message) {
  //
  // Publish the log info to cloud :)
  //

  static int sequence = 0;
  const char* payload = "";
  String logMessageStr = "";
  StaticJsonDocument<1024> doc;

  // get the current epoch millis
  if(!Time->getEpochMillis()) {
    BytebeamLogger::Error(__FILE__, __func__, "failed to get epoch millis");
    return false;
  }

  sequence++;
  unsigned long long milliseconds = Time->nowMillis;

  JsonArray logMessageJsonArray = doc.to<JsonArray>();
  JsonObject logMessageJsonObj_1 = logMessageJsonArray.createNestedObject();

  logMessageJsonObj_1["timestamp"] = milliseconds;
  logMessageJsonObj_1["sequence"]  = sequence;
  logMessageJsonObj_1["level"]     = level;
  logMessageJsonObj_1["tag"]       = tag;
  logMessageJsonObj_1["message"]   = message;

  serializeJson(logMessageJsonArray, logMessageStr);
  payload = logMessageStr.c_str();

  return Bytebeam.publishToStream(this->logStream, payload);
}

BytebeamLog::BytebeamLog() {
  //
  // Initailizing all the variables with default values here
  //

  strcpy(this->logStream, "logs");
  this->logLevel = BYTEBEAM_LOG_LEVEL_INFO;
  this->isCloudLoggingEnable = true;
}

BytebeamLog::~BytebeamLog() {
  //
  // Nothing much to do here, just print the log :)
  //

  BytebeamLogger::Info(__FILE__, __func__, "I am BytebeamLog::~BytebeamLog()");
}

void BytebeamLog::setTimeInstance(BytebeamTime* Time) {
  this->Time = Time;
}

void BytebeamLog::enableCloudLogging() {
  this->isCloudLoggingEnable = true;
}

boolean BytebeamLog::isCloudLoggingEnabled() {
  return this->isCloudLoggingEnable;
}

void BytebeamLog::disableCloudLogging() {
  this->isCloudLoggingEnable = false;
}

void BytebeamLog::setLogStream(char* stream) {
  int tempVar = snprintf(this->logStream, BYTEBEAM_LOG_STREAM_STR_LEN, "%s", stream);

  if(tempVar >= BYTEBEAM_LOG_STREAM_STR_LEN) {
    BytebeamLogger::Error(__FILE__, __func__, "log stream size exceeded buffer size");
    return;
  }
}

char* BytebeamLog::getLogStream() {
  return this->logStream;
}

void BytebeamLog::setLogLevel(bytebeamLogLevel_t level) {
  this->logLevel = level;
}

bytebeamLogLevel_t BytebeamLog::getLogLevel() {
  return this->logLevel;
}

void BytebeamLog::logError(const char* tag, const char* message) {
  logPrint(BYTEBEAM_LOG_LEVEL_ERROR, tag, message);
}

void BytebeamLog::logErrorln(const char* tag, const char* message) {
  logPrintln(BYTEBEAM_LOG_LEVEL_ERROR, tag, message);
}

void BytebeamLog::logErrorf(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logPrintf(BYTEBEAM_LOG_LEVEL_ERROR, tag, fmt, args);
  va_end(args);
}

void BytebeamLog::logWarn(const char* tag, const char* message) {
  logPrint(BYTEBEAM_LOG_LEVEL_WARN, tag, message);
}

void BytebeamLog::logWarnln(const char* tag, const char* message) {
  logPrintln(BYTEBEAM_LOG_LEVEL_WARN, tag, message);
}

void BytebeamLog::logWarnf(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logPrintf(BYTEBEAM_LOG_LEVEL_WARN, tag, fmt, args);
  va_end(args);
}

void BytebeamLog::logInfo(const char* tag, const char* message) {
  logPrint(BYTEBEAM_LOG_LEVEL_INFO, tag, message);
}

void BytebeamLog::logInfoln(const char* tag, const char* message) {
  logPrintln(BYTEBEAM_LOG_LEVEL_INFO, tag, message);
}

void BytebeamLog::logInfof(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logPrintf(BYTEBEAM_LOG_LEVEL_INFO, tag, fmt, args);
  va_end(args);
}

void BytebeamLog::logDebug(const char* tag, const char* message) {
  logPrint(BYTEBEAM_LOG_LEVEL_DEBUG, tag, message);
}

void BytebeamLog::logDebugln(const char* tag, const char* message) {
  logPrintln(BYTEBEAM_LOG_LEVEL_DEBUG, tag, message);
}

void BytebeamLog::logDebugf(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logPrintf(BYTEBEAM_LOG_LEVEL_DEBUG, tag, fmt, args);
  va_end(args);
}

void BytebeamLog::logVerbose(const char* tag, const char* message) {
  logPrint(BYTEBEAM_LOG_LEVEL_VERBOSE, tag, message);
}

void BytebeamLog::logVerboseln(const char* tag, const char* message) {
  logPrintln(BYTEBEAM_LOG_LEVEL_VERBOSE, tag, message);
}

void BytebeamLog::logVerbosef(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  logPrintf(BYTEBEAM_LOG_LEVEL_VERBOSE, tag, fmt, args);
  va_end(args);
}