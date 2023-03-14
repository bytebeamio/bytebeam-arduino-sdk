#ifndef BYTEBEAM_LOGGER_H
#define BYTEBEAM_LOGGER_H

#include <Arduino.h>

class BytebeamLogger {
public:    
    enum DebugLevel{
        LOG_ERROR,
        LOG_WARN,
        LOG_INFO,
        LOG_DEBUG,
        LOG_TRACE
    };
    
    // constructor
    BytebeamLogger() { /* nothing to do here*/ }

    // destructor
    ~BytebeamLogger() { /* nothing to do here */ }

    // public functions

    static void setLogLevel(DebugLevel level) {
        logLevel = level;
    }

    template <typename... Args>
    static void Error(const char* module, const char* func, const char* message, Args... args) {
        print(LOG_ERROR, module, func, message, args...);
    }

    template <typename... Args>
    static void Warn(const char* module, const char* func, const char* message, Args... args) {
        print(LOG_WARN, module, func, message, args...);
    }

    template <typename... Args>
    static void Info(const char* module, const char* func, const char* message, Args... args) {
        print(LOG_INFO, module, func, message, args...);
    }

    template <typename... Args>
    static void Debug(const char* module, const char* func, const char* message, Args... args) {
        print(LOG_DEBUG, module, func, message, args...);
    }

    template <typename... Args>
    static void Trace(const char* module, const char* func, const char* message, Args... args) {
        print(LOG_TRACE, module, func, message, args...);
    }
    
private:
    // private variables
    static DebugLevel logLevel;

    // private functions
    static void printPrefix(DebugLevel level, const char* module, const char* func) {
        // print the bytebeam logger prefix
        Serial.print("BytebeamLogger::");

        // print the debug level
        switch (level) {
            case LOG_ERROR  : Serial.print("LOG_ERROR"); break;
            case LOG_WARN   : Serial.print("LOG_WARN");  break;
            case LOG_INFO   : Serial.print("LOG_INFO");  break;
            case LOG_DEBUG  : Serial.print("LOG_DEBUG"); break;
            case LOG_TRACE  : Serial.print("LOG_TRACE"); break;

            default: Serial.print("UnKnown Log Level");
        }

        // get the module string
        String moduleObj = String(module);

        int startIdx = moduleObj.lastIndexOf('\\');

        //fix for linux            
        if (startIdx == -1){
            startIdx = moduleObj.lastIndexOf('/');
        }

        startIdx += 1;

        int endIdx = moduleObj.lastIndexOf('.');
        
        moduleObj = moduleObj.substring(startIdx, endIdx);

        const char* moduleStr = moduleObj.c_str();

        // print the module name
        Serial.print(" (");
        Serial.print(moduleStr);
        Serial.print(") ");

        // print the function name
        Serial.print(func);
        Serial.print(" : ");
    }

    template <typename... Args>
    static void print(DebugLevel level, const char* module, const char* func, const char* message, Args... args) {
        // check the current log level and serial status
        if(level > logLevel || !Serial) {
            return;
        }

        // print prefix
        printPrefix(level, module, func);
        
        // print the actual message
        Serial.printf(message, args...);
        Serial.println();
    }
};

#endif /* BYTEBEAM_LOGGER_H */