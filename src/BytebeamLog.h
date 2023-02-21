#ifndef BYTEBEAM_LOG_H
#define BYTEBEAM_LOG_H

#include <Arduino.h>
#include <ArduinoJson.h>

typedef enum {
    BYTEBEAM_LOG_LEVEL_NONE,
    BYTEBEAM_LOG_LEVEL_ERROR,
    BYTEBEAM_LOG_LEVEL_WARN,
    BYTEBEAM_LOG_LEVEL_INFO,
    BYTEBEAM_LOG_LEVEL_DEBUG,
    BYTEBEAM_LOG_LEVEL_VERBOSE,
}bytebeamLogLevel_t;

static const char* bytebeamLogLevelStr[6] = {
    [BYTEBEAM_LOG_LEVEL_NONE]    = "None",
    [BYTEBEAM_LOG_LEVEL_ERROR]   = "Error",
    [BYTEBEAM_LOG_LEVEL_WARN]    = "Warn",
    [BYTEBEAM_LOG_LEVEL_INFO]    = "Info",
    [BYTEBEAM_LOG_LEVEL_DEBUG]   = "Debug",
    [BYTEBEAM_LOG_LEVEL_VERBOSE] = "Verbose"
};

class BytebeamLog {
public:
    // contructor
    BytebeamLog();

    // destructor
    ~BytebeamLog();

    // public functions
    void enableCloudLogging();
    boolean isCloudLoggingEnabled();
    void disableCloudLogging();

    void setLogLevel(bytebeamLogLevel_t level);
    bytebeamLogLevel_t getLogLevel();

    void logError(const char* tag, const char* message);
    void logErrorln(const char* tag, const char* message);
    void logErrorf(const char* tag, const char* fmt, ...);

    void logWarn(const char* tag, const char* message);
    void logWarnln(const char* tag, const char* message);
    void logWarnf(const char* tag, const char* fmt, ...);

    void logInfo(const char* tag, const char* message);
    void logInfoln(const char* tag, const char* message);
    void logInfof(const char* tag, const char* fmt, ...);

    void logDebug(const char* tag, const char* message);
    void logDebugln(const char* tag, const char* message);
    void logDebugf(const char* tag, const char* fmt, ...);

    void logVerbose(const char* tag, const char* message);
    void logVerboseln(const char* tag, const char* message);
    void logVerbosef(const char* tag, const char* fmt, ...);

private:
    // private functions
    void log(const char* level, const char* tag, const char* message);
    void logPrint(bytebeamLogLevel_t level, const char* tag, const char* message);
    void logPrintln(bytebeamLogLevel_t level, const char* tag, const char* message);
    void logPrintf(bytebeamLogLevel_t level, const char* tag, const char* fmt, va_list args);
    boolean logPublish(const char* level, const char* tag, const char* message);

    // private variables
    char* logStream;
    bytebeamLogLevel_t logLevel;
    bool isCloudLoggingEnable;
};

#endif /* BYTEBEAM_LOG_H */