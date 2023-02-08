#include "BytebeamLog.h"
#include "BytebeamArduino.h"

BytebeamLog::BytebeamLog() {
    //
    // Initailizing all the variables with default values here
    //

    this->logStream = (char*)"Logs";
    this->logLevel = BYTEBEAM_LOG_LEVEL_INFO;
}

BytebeamLog::~BytebeamLog() {
    //
    // Nothing much to do here, just print the log :)
    //

    Serial.println("I am BytebeamLog::~BytebeamLog()");
}

void BytebeamLog::logPrint(bytebeamLogLevel_t level, const char* tag, const char* message) {
    //
    // Publish the log to cloud and if publish to cloud succedded make sure to print the print the log to serial
    // format : { level (millis) tag : message } i.e this format is adopted from the esp idf logging library
    //

    const char* levelStr = bytebeamLogLevelStr[level];

    if(level <= this->logLevel) {
        if(!logPublish(levelStr, tag, message)) {
            Serial.println("failed to publish bytebeam log.");
        } else {
            Serial.print(levelStr[0]);
            Serial.print(" (");
            Serial.print(millis());
            Serial.print(") ");
            Serial.print(tag);
            Serial.print(" : ");
            Serial.print(message);
        }
    }
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
        Serial.println("failed to get log message size");
        return;
    }

    char* msgStr = (char*) malloc(strSize);
    
    // if memory allocation fails just log the failure to serial and return :)
    if(msgStr == NULL) {
        Serial.println("failed to allocate the memory for log message");
        return;
    }

    tempVar = vsnprintf(msgStr, strSize, fmt, args);

    if(tempVar >= strSize) {
        Serial.println("failed to get log message");
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
    BytebeamTime LogTime;

    // begin the time client
    if(!LogTime.begin()) {
        Serial.println("time client begin failed");
        return false;
    }

    sequence++;
    unsigned long long milliseconds = LogTime.nowMillis;

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

void BytebeamLog::setLogLevel(bytebeamLogLevel_t level) {
  this->logLevel = level;
}

bytebeamLogLevel_t BytebeamLog::getLogLevel(void) {
  return this->logLevel;
}

void BytebeamLog::logError(const char* tag, const char* message) {
  this->logPrint(BYTEBEAM_LOG_LEVEL_ERROR, tag, message);
}

void BytebeamLog::logErrorln(const char* tag, const char* message) {
  this->logPrintln(BYTEBEAM_LOG_LEVEL_ERROR, tag, message);
}

void BytebeamLog::logErrorf(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->logPrintf(BYTEBEAM_LOG_LEVEL_ERROR, tag, fmt, args);
  va_end(args);
}

void BytebeamLog::logWarn(const char* tag, const char* message) {
  this->logPrint(BYTEBEAM_LOG_LEVEL_WARN, tag, message);
}

void BytebeamLog::logWarnln(const char* tag, const char* message) {
  this->logPrintln(BYTEBEAM_LOG_LEVEL_WARN, tag, message);
}

void BytebeamLog::logWarnf(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->logPrintf(BYTEBEAM_LOG_LEVEL_WARN, tag, fmt, args);
  va_end(args);
}

void BytebeamLog::logInfo(const char* tag, const char* message) {
  this->logPrint(BYTEBEAM_LOG_LEVEL_INFO, tag, message);
}

void BytebeamLog::logInfoln(const char* tag, const char* message) {
  this->logPrintln(BYTEBEAM_LOG_LEVEL_INFO, tag, message);
}

void BytebeamLog::logInfof(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->logPrintf(BYTEBEAM_LOG_LEVEL_INFO, tag, fmt, args);
  va_end(args);
}

void BytebeamLog::logDebug(const char* tag, const char* message) {
  this->logPrint(BYTEBEAM_LOG_LEVEL_DEBUG, tag, message);
}

void BytebeamLog::logDebugln(const char* tag, const char* message) {
  this->logPrintln(BYTEBEAM_LOG_LEVEL_DEBUG, tag, message);
}

void BytebeamLog::logDebugf(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->logPrintf(BYTEBEAM_LOG_LEVEL_DEBUG, tag, fmt, args);
  va_end(args);
}

void BytebeamLog::logVerbose(const char* tag, const char* message) {
  this->logPrint(BYTEBEAM_LOG_LEVEL_VERBOSE, tag, message);
}

void BytebeamLog::logVerboseln(const char* tag, const char* message) {
  this->logPrintln(BYTEBEAM_LOG_LEVEL_VERBOSE, tag, message);
}

void BytebeamLog::logVerbosef(const char* tag, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  this->logPrintf(BYTEBEAM_LOG_LEVEL_VERBOSE, tag, fmt, args);
  va_end(args);
}